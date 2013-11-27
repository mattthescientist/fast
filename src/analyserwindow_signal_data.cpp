// The FTS Atomic Spectrum Tool (FAST)
// Copyright (C) 2011-2013 M. P. Ruffoni
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//==============================================================================
// AnalyserWindow class (analyserwindow_signal_data.cpp)
//==============================================================================
// This file contains the signal handlers for the "Data" menu.

//------------------------------------------------------------------------------
// loadXGremlinData () : Manages the duel loading of an XGremlin spectrum and
// line list. If the user cancels the loading of a spectrum, no prompt is given
// to load an associated line list.
//
void AnalyserWindow::loadXGremlinData () {
  try {
    do_load_expt_spectrum ();
  } catch (Error e) {
    // A problem was encountered. Return without prompting for a line list.
    return;
  }
  on_data_load_line_list ();
}

//------------------------------------------------------------------------------
// on_data_load_kurucz () : Opens a FileChooserDialog prompting the user to
// select a data file containing a list of lines from the Kurucz database. These
// lines will then be loaded into a KzList object, which will automatically sort
// them into branches from given upper levels. These levels are then added to
// the treeLevels list to be displayed on the left of the AnalyserWindow.
//
void AnalyserWindow::on_data_load_kurucz () {
  Gtk::FileChooserDialog dialog("Select a list of target lines",
    Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);
  vector <LinePair> MatchedLines;

  // Add response buttons the the dialog
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  // Add filters, so that only certain file types can be selected
  Gtk::FileFilter filter_text;
  filter_text.set_name("Text files");
  filter_text.add_mime_type("text/plain");
  dialog.add_filter(filter_text);

  dialog.set_current_folder (DefaultFolder);
  int result = dialog.run ();

  // Handle the response 
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    { 
      ostringstream oss;
      try {
        KuruczList.read (dialog.get_filename());
        size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
        DefaultFolder = dialog.get_filename().substr (0, FilePos);
        getLinePairs();
        refreshKuruczList ();

        oss << "Successfully loaded " << dialog.get_filename().substr(FilePos) << ".";
        Status.push (oss.str());
        projectHasChanged (true);
      } catch (Error Err) {
        display_error (Err);
        return;
      }
      break;
    }
    default:
    {
      break;
    }
  }
}


//------------------------------------------------------------------------------
// on_data_save_kurucz () : Saves the currently loaded Kurucz data to a user
// specified ASCII file.
//
void AnalyserWindow::on_data_save_kurucz () {
  Gtk::FileChooserDialog dialog("Save the list of target lines as",
  Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_transient_for(*this);
  vector <LinePair> MatchedLines;

  // Add response buttons the the dialog
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  // Add filters, so that only certain file types can be selected
  Gtk::FileFilter filter_text;
  filter_text.set_name("Text files");
  filter_text.add_mime_type("text/plain");
  dialog.add_filter(filter_text);

  dialog.set_current_folder (DefaultFolder);
  int result = dialog.run ();

  // Handle the response 
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    { 
      try {
        KuruczList.save (dialog.get_filename());
      } catch (Error Err) {
        display_error (Err);
        return;
      }
      size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
      DefaultFolder = dialog.get_filename().substr (0, FilePos);
      ostringstream oss;
      oss << "Successfully saved " << dialog.get_filename().substr(FilePos);
      Status.push (oss.str());
      break;
    }
    default:
    {
      break;
    }
  }
}


//------------------------------------------------------------------------------
// on_data_load_expt_spectrum () : A wrapper for do_load_expt_spectrum (), which
// handles any residual thrown errors.
//
void AnalyserWindow::on_data_load_expt_spectrum () { 
  try {
    on_data_load_expt_spectrum ();
  } catch (Error Err) {
    // Do nothing. The error has been handled elsewhere. This function is just
    // a wrapper to complement AnalyserWindow::loadXGremlinData (), above.
  }
}


//------------------------------------------------------------------------------
// do_load_expt_spectrum () : Loads a user specified XGremlin spectrum
//
void AnalyserWindow::do_load_expt_spectrum () throw (Error) { 
  Gtk::FileChooserDialog dialog("Select an expeimental data file to load",
    Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);

  // Add response buttons the the dialog
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  // Add filters, so that only certain file types can be selected
  Gtk::FileFilter filter_dat, filter_asc, filter_all;
  filter_dat.set_name("XGremlin DAT files");
  filter_dat.add_pattern("*.dat");
  dialog.add_filter(filter_dat);
  filter_asc.set_name("ASCII files");
  filter_asc.add_pattern("*.asc");
  dialog.add_filter(filter_asc);
  filter_all.set_name("All files");
  filter_all.add_pattern("*");
  dialog.add_filter(filter_all);

  dialog.set_current_folder (DefaultFolder);
  int result = dialog.run ();

  // Handle the response 
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    { 
      ostringstream oss;
      // Attempt to load the spectrum into a new XgSpectrum object.
      try {
        size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
        DefaultFolder = dialog.get_filename().substr (0, FilePos);

        XgSpectrum NewSpectrum;
        try {
          NewSpectrum.loadDat (dialog.get_filename());
        } catch (Error e) {
          if (e.code == FLT_FILE_HEAD_ERROR) {
            NewSpectrum.loadAscii (dialog.get_filename());
          } else {
            throw (e);
          }
        }
        NewSpectrum.name (dialog.get_filename().substr(FilePos));
        string a; a.push_back (char (ExptSpectra.size () + ASCII_A));
        NewSpectrum.index (a);
        ExptSpectra.push_back (NewSpectrum);
        oss << "Successfully added " << NewSpectrum.name() << ".";
        Status.push (oss.str());
        
        // Fill the TreeView's model
        Gtk::TreeModel::Row row = *(m_refTreeModel->append());
        row[m_Columns.name] = dialog.get_filename().substr(FilePos);
        row[m_Columns.emin] = int(NewSpectrum.data()[0].x + 0.5);
        row[m_Columns.emax] = int(NewSpectrum.data()[NewSpectrum.data().size () - 1].x + 0.5);
        row[m_Columns.index] = int(ExptSpectra.size ()) - 1;
        row[m_Columns.line_index] = -1;
        row[m_Columns.bg_colour] = Gdk::Color (AW_SPECTRUM_COLOUR);
        if (ExptSpectra.size () == 1) {
          row[m_Columns.ref] = true;
          ExptSpectra[0].isReference (true);
        } else {
          row[m_Columns.ref] = false;
        }
        row[m_Columns.label] = NewSpectrum.index();

        Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeSpectra.get_selection();
        if (treeSelection) {
          treeSelection->select(row);
        }
        projectHasChanged (true);

        
      // If a file reading error occurs, inform the user. If another error has
      // occurred, continue to throw it.
      } catch (Error Err) {
        display_error (Err);
        throw Error (FLT_DIALOG_CANCEL);
      }          
      break;
    }
    
    // User clicked cancel or closed the dialog without clicking OK. Assume they
    // wish to abort loading of spectrum. Do nothing.
    default:
    {
      throw Error (FLT_DIALOG_CANCEL);
      break;
    }
  }
}


//------------------------------------------------------------------------------
// on_data_load_line_list () : Loads a user specified XGremlin line list and
// attaches it to the spectrum currently selected in treeSpectra in the bottom
// left of the main FAST window.
//
void AnalyserWindow::on_data_load_line_list () {

  // Before attempting to load any data, check that an experimental spectrum has
  // previously been loaded and is selected.
  Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeSpectra.get_selection();
  Gtk::TreeModel::iterator iter;
  if (treeSelection) {
    iter = treeSpectra.get_selection()->get_selected();
    if (iter) {

      // A selected experimental spectrum exists in treeSpectra. A line list can
      // be attached to it. Prepare the file open dialog and display it.
      Gtk::FileChooserDialog dialog("Select an experimental line list to load",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
      dialog.set_transient_for(*this);
      dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

      // Add filters, so that only certain file types can be selected
      Gtk::FileFilter filter_dat, filter_asc, filter_all;
      filter_dat.set_name("XGremlin LIN files");
      filter_dat.add_pattern("*.lin");
      dialog.add_filter(filter_dat);
      filter_asc.set_name("ASCII files");
      filter_asc.add_pattern("*.asc");
      dialog.add_filter(filter_asc);
      filter_all.set_name("All files");
      filter_all.add_pattern("*");
      dialog.add_filter(filter_all);
      
      dialog.set_current_folder (DefaultFolder);
      int result = dialog.run ();
  
      // Handle the response from the user
      switch(result)
      {
        case(Gtk::RESPONSE_OK):
        {
          try {
            // Read the XGremlin line list and store it in the Lines vector
            Gtk::TreeModel::Row SelectedRow = *(iter);
            vector <XgLine> NewLines;
            vector <char> LinHeader;
            try {
              if (dialog.get_filename().substr(dialog.get_filename().size() - 4, 4) == ".lin") {
                NewLines = readLinFile (dialog.get_filename());
                LinHeader = readLinFileHeader (dialog.get_filename());
              } else {
                NewLines = readLineList (dialog.get_filename());
              }
            } catch (const char* e) {
              cout << e << endl;
              throw (Error (FLT_FILE_READ_ERROR, "File read error, check the terminal for details"));
            }
            
            // Remove lines that are outside the spectrum
            for (int i = NewLines.size () - 1; i >= 0; i --) {
              if (NewLines[i].wavenumber () < ExptSpectra[SelectedRow[m_Columns.index]].data (0).x ||
                NewLines[i].wavenumber () > ExptSpectra[SelectedRow[m_Columns.index]].data 
                  (ExptSpectra[SelectedRow[m_Columns.index]].numDataPoints() - 1).x){
                NewLines.erase (NewLines.begin () + i);
              }
            }
            addNewLines (&ExptSpectra[SelectedRow[m_Columns.index]], NewLines);
            ExptSpectra[SelectedRow[m_Columns.index]].lin_headers_push_back (LinHeader);
            getLinePairs ();

            // Attach the line list to treeSpectra as a child of the selected
            // experimental spectrum.
            Gtk::TreeModel::Row row = *(m_refTreeModel->append(SelectedRow->children()));
            size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
            DefaultFolder = dialog.get_filename().substr (0, FilePos);
            int Index = (*iter)[m_Columns.index];
            int LineIndex = ExptSpectra[Index].lines().size () - 1;
            bool Ref = (*iter)[m_Columns.ref];
            row[m_Columns.index] = Index;
            row[m_Columns.line_index] = LineIndex;
            row[m_Columns.ref] = Ref;
            row[m_Columns.emin] = ExptSpectra[Index].lines()[LineIndex][0].wavenumber ();
            row[m_Columns.emax] = ExptSpectra[Index].lines()[LineIndex][ExptSpectra[Index].lines()[LineIndex].size () - 1].wavenumber ();
            row[m_Columns.name] = ExptSpectra[Index].lines()[LineIndex][0].name ();
            row[m_Columns.bg_colour] = Gdk::Color (AW_LINELIST_COLOUR);
            updateKuruczCompleteness ();

            // Let the user know that the list has been attached successfully.
            ostringstream oss;
            string SpectrumName = SelectedRow[m_Columns.name];
            oss << "Successfully attached " 
              << dialog.get_filename().substr(FilePos) << " to "
              << SpectrumName << ".";
            Status.push (oss.str());
            projectHasChanged (true);

          } catch (Error Err) {
            display_error (Err);
            return;
          }
          break;
        }

        // User clicked cancel or closed the dialog without clicking OK. Assume 
        // they wish to abort loading the list list, so do nothing.
        default:
        {
          break;
        }
      }
      updatePlottedData ();
    }
  }
}


//------------------------------------------------------------------------------
// on_data_attach_standard_lamp_radiance () : Loads a user specified radiance
// file and attaches the data to the spectrum currently selected in treeSpectra
// in the bottom left of the main FAST window.
//
void AnalyserWindow::on_data_attach_standard_lamp_radiance () {

  // Before attempting to load any data, check if a radiance file has
  // previously been loaded and is selected.
  Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeSpectra.get_selection();
  Gtk::TreeModel::iterator iter;
  if (treeSelection) {
    iter = treeSpectra.get_selection()->get_selected();
    if (iter) {
      Gtk::TreeModel::Row SelectedRow = *(iter);
    
      // A selected experimental spectrum exists in treeSpectra. Check to see if
      // a radiance file is already attached to it. If so, pop-up a question
      //  dialog to confirm the user wants replace it.
      if (ExptSpectra[SelectedRow[m_Columns.index]].radiance ().size () > 0) {
        Gtk::MessageDialog replace(*this, "Attaching a new radiance file will replace the existing one. Is this OK?",
          false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
        replace.set_secondary_text("Only one radiance file can be attached to each spectrum.");

        if (replace.run() != Gtk::RESPONSE_YES) return;
      }

      // Ask the user for a new standard lamp radiance file to attach.
      Gtk::FileChooserDialog dialog("Select an ASCII radiance file",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
      dialog.set_transient_for(*this);
      dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

      // Add filters, so that only certain file types can be selected
      Gtk::FileFilter filter_dat, filter_all;
      filter_dat.set_name("Radiance File (*.rad)");
      filter_dat.add_pattern("*.rad");
      dialog.add_filter(filter_dat);
      filter_all.set_name("All files (*.*)");
      filter_all.add_pattern("*");
      dialog.add_filter(filter_all);

      dialog.set_current_folder (DefaultFolder);
      int result = dialog.run ();

      // Handle the response from the user
      switch(result)
      {
        case(Gtk::RESPONSE_OK):
        {
          size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
          DefaultFolder = dialog.get_filename().substr (0, FilePos);

          // Attach the radiance file to the currently selected spectrum.
          try {
            ExptSpectra[SelectedRow[m_Columns.index]].radiance (dialog.get_filename());
          } catch (Error Err) {
            if (Err.code == XGSPEC_NO_RAD_UNCERTAINTIES) {
              // Warn the user that no radiance uncertainties were found
              Gtk::MessageDialog Warning(*this, Err.message,
                false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK);
              Warning.set_secondary_text(Err.subtext);
              Warning.run();
            } else {
              // A more serious error occurred, so abort
              display_error (Err);
              return;
            }
          }
            
          // Add the radiance file to treeSpectra
          Gtk::TreeModel::Row row = *(m_refTreeModel->append(SelectedRow->children()));
          int Index = (*iter)[m_Columns.index];
          bool Ref = (*iter)[m_Columns.ref];
          row[m_Columns.index] = Index;
          row[m_Columns.line_index] = -1;
          row[m_Columns.ref] = Ref;
          row[m_Columns.emin] = ExptSpectra[Index].radiance()[0].x;
          row[m_Columns.emax] = ExptSpectra[Index].radiance()[ExptSpectra[Index].radiance().size() - 1].x;
          row[m_Columns.name] = ExptSpectra[Index].radiance_file ();
          row[m_Columns.bg_colour] = Gdk::Color (AW_RESPONSE_COLOUR);

          // Let the user know that the list has been attached successfully.
          ostringstream oss;
          string SpectrumName = SelectedRow[m_Columns.name];
          oss << "Successfully attached standard lamp radiance " 
            << dialog.get_filename().substr(FilePos) << " to "
            << SpectrumName << ".";
          Status.push (oss.str());
          projectHasChanged (true);
          updateKuruczCompleteness ();
          updatePlottedData ();
          break;
        }

        // User clicked cancel or closed the dialog without clicking OK. Assume 
        // they wish to abort loading the list list, so do nothing.
        default:
        {
          break;
        }
      }
    }
  }
}


//------------------------------------------------------------------------------
// on_data_attach_standard_lamp_spectrum () : Loads a user specified standard
// lamp spectrum and attaches it to the spectrum currently selected in 
// treeSpectra in the bottom left of the main FAST window.
//
void AnalyserWindow::on_data_attach_standard_lamp_spectrum () {

  // Before attempting to load any data, check that an experimental spectrum has
  // previously been loaded and is selected.
  Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeSpectra.get_selection();
  Gtk::TreeModel::iterator iter;
  if (treeSelection) {
    iter = treeSpectra.get_selection()->get_selected();
    if (iter) {
      Gtk::TreeModel::Row SelectedRow = *(iter);
    
      // A selected experimental spectrum exists in treeSpectra. Check to see if
      // a response functin is already attached to it. If so, pop-up a question
      //  dialog to confirm the user wants replace it.
      if (ExptSpectra[SelectedRow[m_Columns.index]].standard_lamp_spectrum ().size () > 0) {
        Gtk::MessageDialog replace(*this, "Attaching a new standard lamp spectrum will replace the existing one. Is this OK?",
          false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
        replace.set_secondary_text("Only one standard lamp spectrum can be attached to each line spectrum.");

        if (replace.run() != Gtk::RESPONSE_YES) return;
      }

      // Ask the user for a new standard lamp spectrum to attach.
      Gtk::FileChooserDialog dialog("Select an ASCII standard lamp spectrum",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
      dialog.set_transient_for(*this);
      dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

      // Add filters, so that only certain file types can be selected
      Gtk::FileFilter filter_dat, filter_all;
      filter_dat.set_name("XGremlin ASC File");
      filter_dat.add_pattern("*.asc");
      dialog.add_filter(filter_dat);
      filter_all.set_name("All files");
      filter_all.add_pattern("*");
      dialog.add_filter(filter_all);

      dialog.set_current_folder (DefaultFolder);
      int result = dialog.run ();

      // Handle the response from the user
      switch(result)
      {
        case(Gtk::RESPONSE_OK):
        {
          try {
            // Attach the response function to the currently selected spectrum.
            size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
            DefaultFolder = dialog.get_filename().substr (0, FilePos);
            ExptSpectra[SelectedRow[m_Columns.index]].standard_lamp_spectrum (dialog.get_filename());
            
            // Add the response function to treeSpectra
            Gtk::TreeModel::Row row = *(m_refTreeModel->append(SelectedRow->children()));
            int Index = (*iter)[m_Columns.index];
            bool Ref = (*iter)[m_Columns.ref];
            row[m_Columns.index] = Index;
            row[m_Columns.line_index] = -2;
            row[m_Columns.ref] = Ref;
            row[m_Columns.emin] = ExptSpectra[Index].standard_lamp_spectrum()[0].x;
            row[m_Columns.emax] = ExptSpectra[Index].standard_lamp_spectrum()
              [ExptSpectra[Index].standard_lamp_spectrum().size() - 1].x;
            row[m_Columns.name] = ExptSpectra[Index].standard_lamp_file ();
            row[m_Columns.bg_colour] = Gdk::Color (AW_RESPONSE_COLOUR);

            // Let the user know that the list has been attached successfully.
            ostringstream oss;
            string SpectrumName = SelectedRow[m_Columns.name];
            oss << "Successfully attached standard lamp spectrum " 
              << dialog.get_filename().substr(FilePos) << " to "
              << SpectrumName << ".";
            Status.push (oss.str());
            projectHasChanged (true);
            updateKuruczCompleteness ();
            updatePlottedData ();
            
          } catch (Error Err) {
            display_error (Err);
            return;
          }
          break;
        }

        // User clicked cancel or closed the dialog without clicking OK. Assume 
        // they wish to abort loading the list list, so do nothing.
        default:
        {
          break;
        }
      }
    }
  }
}


//------------------------------------------------------------------------------
// on_data_remove_spectrum () : Deletes the line spectrum currently selected in 
// treeSpectra in the bottom left of the main FAST window.
//
void AnalyserWindow::on_data_remove_spectrum () {
  int Index;
  string a;
  ostringstream oss;

  // Now obtain an iterator for the active selection and check to see if it
  // is a root node (and thus a spectrum) or a child (and thus a line list).    
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeSpectra.get_selection();
  if(refSelection) {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter) {
      Index = (*iter)[m_Columns.index];
      clearDisplayedPlots ();
      a = ExptSpectra[Index].name ();
      ExptSpectra.erase (ExptSpectra.begin() + Index);
      m_refTreeModel->erase (iter);
      projectHasChanged (true);
      getLinePairs ();
      updateKuruczCompleteness ();
      updatePlottedData ();
      oss << "Removed spectrum " << a << ".";
      Status.push (oss.str ());
    }
  }
  
  // Update the spectrum label and index for all the remaining spectra
  typedef Gtk::TreeModel::Children type_children;
  type_children children = m_refTreeModel->children();
  Index = -1;
  for(type_children::iterator iter = children.begin(); iter != children.end(); ++iter) {
    Index ++;
    a = ""; a.push_back (char (Index + ASCII_A));
    ExptSpectra[Index].index (a);
    (*iter)[m_Columns.index] = Index;
    (*iter)[m_Columns.label] = ExptSpectra[Index].index();
  }
}

//------------------------------------------------------------------------------
// on_data_output_results () : Displays the output results window.
//
void AnalyserWindow::on_data_output_results () {
  vector < vector <LinePair *> > OrderedPairs;
  vector <LinePair *> NextPairSet;
  vector <string> SpectrumLabels;
  vector <unsigned int> SpectrumOrder;
  unsigned int RefIndex = 0;  
  
  vector <vector <DataBF> > Results;
  vector <vector <XgLine> > Fits;
  vector <XgLine> NextLevelFits;
  vector <vector <KzLine> > Targets;
  vector <KzLine> NextLevelTargets;

  // Find the reference spectrum. This will be plotted first.
  for (unsigned int i = 0; i < ExptSpectra.size (); i ++) {
    if (ExptSpectra[i].isReference()) {
      RefIndex = i;
      break;
    }
  }
  
  // Construct the list of Kurucz/XGremlin line pairs, with the reference
  // spectrum being first in the resulting arrays
  for (unsigned int Level = 0; Level < LevelLines.size (); Level ++) {
    SpectrumLabels.clear ();
    SpectrumOrder.clear ();
    OrderedPairs.clear ();
    
    SpectrumLabels.push_back (ExptSpectra [RefIndex].index());
    SpectrumOrder.push_back (RefIndex);
    NextPairSet.clear ();
    for (unsigned int i = 0; i < LevelLines[Level][RefIndex].size (); i ++){
      NextPairSet.push_back (&LevelLines[Level][RefIndex][i]);
    }
    OrderedPairs.push_back (NextPairSet);

    // Now add the lines from all the other spectra to OrderedPairs
    for (unsigned int i = 0; i < LevelLines[Level].size (); i ++) {
      if (i != RefIndex) {
        NextPairSet.clear ();
        for (unsigned int j = 0; j < LevelLines[Level][i].size (); j ++) {
          NextPairSet.push_back (&LevelLines[Level][i][j]);
        }
        SpectrumLabels.push_back (ExptSpectra [i].index());
        SpectrumOrder.push_back (i);
        OrderedPairs.push_back (NextPairSet);
      }
    }

    // Obtain the Branching Fraction results
    ScalingFactors = updateComparisonList (OrderedPairs, SpectrumLabels, SpectrumOrder);
    Results.push_back (calculateBranchingFractions 
      (OrderedPairs, SpectrumLabels, SpectrumOrder));
    
    // Obtain the target lines and XGremlin fit parameters
    NextLevelFits.clear ();
    NextLevelTargets.clear ();
    for (unsigned int i = 0; i < OrderedPairs[0].size (); i ++) {
      for (unsigned int j = 0; j < OrderedPairs.size (); j ++) {
        if (OrderedPairs[j][i] -> xgLine->wavenumber () > 0.0) {
          if (OrderedPairs[j][i] -> plot -> selected ()) {
            NextLevelFits.push_back (*(OrderedPairs[j][i] -> xgLine));
            NextLevelTargets.push_back (*(OrderedPairs[j][i] -> kzLine));
          }
        }
      }
    }
    Fits.push_back (NextLevelFits);
    Targets.push_back (NextLevelTargets);
  }

  Output.set_results (Results, Fits, Targets);  
  Output.set_modal (true);  
  Gtk::Main::run(Output);
}







