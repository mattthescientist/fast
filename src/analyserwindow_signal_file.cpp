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
// AnalyserWindow class (analyserwindow_signal_file.cpp)
//==============================================================================
// This file contains the signal handlers for the "File" menu. 
//
// A word of CAUTION: If you edit any of the project save routines, make sure
// you update the associated load routine as well (and vice versa). Each load
// function must read out data using the exact format in which it was saved. If
// it doesn't, serious errors (seg faults etc.) WILL occur!

#include "TypeDefs.h"

using namespace::std;

//------------------------------------------------------------------------------
// on_file_new () : Removes all loaded spectra and line lists and so creates a
// new project for the user.
//
void AnalyserWindow::on_file_new () {
  newProject ();
}


//------------------------------------------------------------------------------
// on_file_open () : Handles the selection of an FTS file that is to be opened
//
void AnalyserWindow::on_file_open () {
  Glib::RefPtr<Gtk::TreeSelection> Select;
  Gtk::TreeModel::Row Row;
  Gtk::FileChooserDialog dialog("Open a project",
    Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);

  // Add response buttons the the dialog
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  // Add filters, so that only certain file types can be selected
  Gtk::FileFilter filter_fts, filter_all;
  filter_fts.set_name("FTS Analysis Files");
  filter_fts.add_pattern("*.fts");
  dialog.add_filter(filter_fts);
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
      fileOpen (dialog.get_filename());
      size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
      DefaultFolder = dialog.get_filename().substr (0, FilePos);
      
/*      if (levelTreeModel->children().size() > 0) {
        Select = treeLevels.get_selection ();
        Row = levelTreeModel->children()[0];
        Select->select (Row);
        Select = treeLevelsBF.get_selection ();
        Row = modelLevelsBF->children()[0];
        Select->select (Row);
      
        updatePlottedData();
      }*/
      break;
    }
  }
}

//------------------------------------------------------------------------------
// fileOpen : Opens a previously saved project from an FTS file.
//
void AnalyserWindow::fileOpen (string Filename) {
  Glib::RefPtr<Gtk::TreeSelection> Select;
  Gtk::TreeModel::Row Row;
  ifstream BinIn (Filename.c_str (), ios::in|ios::binary);
  ostringstream oss;
  int FileVersion;
  if (BinIn.is_open ()) {
    LevelLines.clear ();
    KuruczList.clear ();
    ExptSpectra.clear ();
    LinkedSpectra.clear ();
    lineDataTreeModel -> clear ();
    levelTreeModel -> clear ();
    modelLevelsBF -> clear ();
    m_refTreeModel -> clear ();
    modelDataXGr -> clear ();
    modelDataBF -> clear ();
    modelDataComp -> clear ();
    clearDisplayedPlots ();
    CurrentFilename = "";
    projectHasChanged (false);
    
    // First read the FTS file version from the input file.
    FileVersion = readFileVersion (&BinIn);

    // If FileVersion is greater than FTS_FILE_VERSION, the user must be
    // running an old version of FAST and thus attempting to load a file
    // that is newer than, and so not compatible with the running code.
    // Catch this error and abort.
    if (FileVersion > FTS_FILE_VERSION) {
    	BinIn.close ();
    	oss << "Error : Unable to open " << Filename;
    	Gtk::MessageDialog dialog(*this, oss.str (), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    	dialog.set_secondary_text("This file was saved with a newer version of FAST.\nPlease upgrade FAST to the latest version and try again.");
    	dialog.run();
    	return;
    }

    // File version is OK, so continue loading.
    loadKuruczList (&BinIn);
    loadExptSpectra (&BinIn);
    loadInterface (&BinIn, FileVersion);
          
    BinIn.close ();
    
    CurrentFilename = Filename;
    size_t FilePos = Filename.find_last_of ("/\\") + 1;
    oss << "Successfully loaded " << Filename.substr(FilePos) << ".";
    Status.push (oss.str());
    
    if (levelTreeModel->children().size() > 0) {
      Select = treeLevels.get_selection ();
      Row = levelTreeModel->children()[0];
      Select->select (Row);
      Select = treeLevelsBF.get_selection ();
      Row = modelLevelsBF->children()[0];
      Select->select (Row);
    
      updatePlottedData();
      writeConfigFile ();
    }
  } else {
    oss << "Error : Unable to open " << Filename;
    Gtk::MessageDialog dialog(*this, oss.str (), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    dialog.set_secondary_text("Check the file exists and is readable.");
    dialog.run();
    removeFromConfigFile (Filename);
  }
}


//------------------------------------------------------------------------------
// on_file_print_level () : Called when the user requests a printout of the
// currently selected level.
//
void AnalyserWindow::on_file_print_level () {

/*  Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeLevels.get_selection();
  if (treeSelection) {
    Gtk::TreeModel::iterator iter = treeLevels.get_selection()->get_selected();
    if (iter) {
      ostringstream oss;

      Gtk::FileChooserDialog dialog("Save plots of lines in the current level",
        Gtk::FILE_CHOOSER_ACTION_SAVE);
      dialog.set_transient_for(*this);

      // Add response buttons the the dialog
      dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

      // Add filters, so that only certain file types can be selected
      Gtk::FileFilter filter_text;
      filter_text.set_name("PDF files");
      filter_text.add_mime_type("application/pdf");
      dialog.add_filter(filter_text);
      oss.precision (1);
      oss << fixed << (double)(*iter)[levelCols.jupper] << "_" << (string)(*iter)[levelCols.config];
      string InitialName = oss.str ();
      replace (InitialName.begin (), InitialName.end (), ' ', '_');
      replace (InitialName.begin (), InitialName.end (), '.', '_');
      replace (InitialName.begin (), InitialName.end (), '/', '_');
      replace (InitialName.begin (), InitialName.end (), '\\', '_');
      dialog.set_current_name (InitialName);

      int result = dialog.run ();

      // Handle the response 
      switch(result)
      {
        case(Gtk::RESPONSE_OK):
        {
          vector <LatexPlot> PlotNames;
          LatexPlot NextPlot;
          string RootName, Caption;
          
          for (unsigned int i = 0; i < LevelLines.size (); i ++) {
            oss.str ("");
            oss << dialog.get_filename() << "_" << ExptSpectra[i].index();
            RootName = oss.str ();
            NextPlot.Name.clear ();
            for (unsigned int j = 0; j < LevelLines[i].size (); j ++) {
              if (LevelLines[i][j].xgLine.wavenumber () > 0.0) {
                try {
                  oss.str ("");
                  oss << RootName << "_" << LevelLines[i][j].xgLine.line () << ".ps";
                  plotLine (LevelLines[i][j].xgLine, LineBoxes[i][j]->getPlotData(0),
                    LineBoxes[i][j]->getResidualData(0), oss.str ());
                  NextPlot.Name.push_back (oss.str ());
                } catch (int e) {
                  if (e == ERR_GRAPH_INDEX_TOO_LOW) {
                    cout << "Plot index too low. Cannot print it." << endl;
                  } else {
                    cout << "Plot index too high. There are only " << e << 
                      " plots on graph " << i << " (j was " << j << ")" << endl;
                  }
                }
              }
            }
            oss.str ("");
            oss << "Transitions from upper level " 
              << (double)(*iter)[levelCols.jupper] << " " 
              << (string)(*iter)[levelCols.config] << " seen in " 
              << ExptSpectra[i].name();
            Caption = oss.str ();
            replace (Caption.begin (), Caption.end (), '_', '-');
            NextPlot.Caption = Caption;
            PlotNames.push_back (NextPlot);
          }
          combinePlotsWithLatex (PlotNames, dialog.get_filename());
          oss.str ("");
          size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
          oss << "Successfully plotted " << dialog.get_filename().substr(FilePos) << ".";
          Status.push (oss.str());
        }
      } 
    }
  }*/
}


//------------------------------------------------------------------------------
// on_file_save () : Triggered when the user asks to save the current project.
//
void AnalyserWindow::on_file_save () {
  if (CurrentFilename == "") { 
    on_file_save_as ();
  } else {
    try {
      saveProject (CurrentFilename);
    } catch (Error *e) {
      // Error message already displayed. Do nothing.
    }
  }
}
    


//------------------------------------------------------------------------------
// on_file_save_as () : Triggered when the user asks to save the current project
// under a new name, or on the first attempt to save a new project.
//
void AnalyserWindow::on_file_save_as () {
  Gtk::FileChooserDialog dialog("Save the current project",
    Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_transient_for(*this);

  // Add response buttons the the dialog
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  // Add filters, so that only certain file types can be selected
  Gtk::FileFilter filter_fts, filter_all;
  filter_fts.set_name("FTS Analysis Files");
  filter_fts.add_pattern("*.fts");
  dialog.add_filter(filter_fts);
  filter_all.set_name("All files");
  filter_all.add_pattern("*");
  dialog.add_filter(filter_all);

  dialog.set_current_folder (DefaultFolder);
  int result = dialog.run ();
  string Filename;

  // Handle the response 
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      Filename = dialog.get_filename();
      size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
      DefaultFolder = dialog.get_filename().substr (0, FilePos);
      if (Filename.substr (Filename.size () - 4, 4) != ".fts") {
        Filename = Filename + ".fts";
      }

      // Check whether or not the file already exists. If it does, ask the user
      // if they want to overwrite it.
      ifstream CheckFile (Filename.c_str ());
      if (CheckFile.is_open ()) {
        CheckFile.close ();
        ostringstream oss;
        oss << Filename << " already exists";
        Gtk::MessageDialog message(*this, oss.str(),
          false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
        message.set_secondary_text("Do you want to overwrite it?");
        result = message.run();
        if (result == Gtk::RESPONSE_NO) { throw (result); }
      }
      try {
        saveProject (Filename);
        writeConfigFile ();
      } catch (Error *e) {
        // Error message already displayed. Do nothing.
      }
      break;
    }
  }
}


//------------------------------------------------------------------------------
// saveProject (string) : Saves the current project to the file named at arg1.
//
void AnalyserWindow::saveProject (string Filename) throw (Error){
  ofstream BinOut (Filename.c_str(), ios::out|ios::binary);
  ostringstream oss;
  if (BinOut.is_open ()) {
    writeFileVersion (&BinOut);
    saveKuruczList (&BinOut);
    saveExptSpectra (&BinOut);
    saveInterface (&BinOut);
    BinOut.close ();
  
    CurrentFilename = Filename;
    size_t FilePos = CurrentFilename.find_last_of ("/\\") + 1;
    oss << "Successfully saved " << CurrentFilename.substr(FilePos) << ".";
    Status.push (oss.str());
    projectHasChanged (false);
  } else {
    oss << "Error : Unable to save " << Filename;
    Gtk::MessageDialog dialog(*this, oss.str (), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    dialog.set_secondary_text("Check you have write permissions for the specified location.");
    dialog.run();
    Status.push (oss.str());
    throw (Error (FLT_FILE_WRITE_ERROR));
  }
}

//------------------------------------------------------------------------------
// on_file_export_project () : 
//
void AnalyserWindow::on_file_export_project () {
  Gtk::FileChooserDialog dialog("Export the current project",
    Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_transient_for(*this);

  // Add response buttons the the dialog
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  // Add filters, so that only certain file types can be selected
  Gtk::FileFilter filter_fts, filter_all;
  filter_fts.set_name("FTS Analysis Files");
  filter_fts.add_pattern("*.fts");
  dialog.add_filter(filter_fts);
  filter_all.set_name("All files");
  filter_all.add_pattern("*");
  dialog.add_filter(filter_all);

  dialog.set_current_folder (DefaultFolder);
  int result = dialog.run ();
  string Filename;
  FILE * TxtOut;

  // Handle the response 
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
      Filename = dialog.get_filename();

      // Check whether or not an exported project already exists in this 
      // location. If it does, ask the user if they want to overwrite it.
#if defined (__linux__)
      int MkStatus = mkdir (Filename.c_str(), 0775);
#elif defined (_WIN32)
      int MkStatus = _mkdir (Filename.c_str());
#endif
      if (MkStatus == -1 && errno == EEXIST) {
        size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
        DefaultFolder = dialog.get_filename().substr (0, FilePos);
        
        ostringstream oss;
        oss << "There is already an exported project called " << Filename.substr(FilePos).c_str() << " in this location";
        Gtk::MessageDialog message(*this, oss.str(),
          false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
        message.set_secondary_text("Do you want to overwrite it?");
        int result = message.run();
        if (result == Gtk::RESPONSE_NO) { return; }
      }
      
      // First save the Kurucz line list
      ostringstream oss;
      oss << Filename << "/" << AW_DEF_TARGETS_NAME;
      KuruczList.save (oss.str ());
      
      // Then save the experimental spectra
      vector < vector <XgLine> >* Lines;
      vector < vector <char> > LinHeaders;
      vector <Coord> StdLamp, Response;
      vector <ErrRange> RadianceErr;
      for (unsigned int Spec = 0; Spec < ExptSpectra.size (); Spec ++) {
      
        // Save the spectrum DAT and HDR files
        oss.str ("");
        oss << Filename << "/" << ExptSpectra[Spec].name ();
        ExptSpectra[Spec].save (oss.str ());
        
        // Save the line lists
        Lines = ExptSpectra[Spec].linesPtr2 ();
        LinHeaders = ExptSpectra[Spec].linHeaders ();
        for (unsigned int List = 0; List < Lines -> size (); List ++) {
          vector <XgLine> LinesToExport = 
            ExptSpectra[Spec].linesPtr2 () -> at (List);
          oss.str ("");
          oss << Filename << "/" << LinesToExport[0].name();
          if (LinHeaders[List].size () > 0) {
            writeLinFile (oss.str (), LinHeaders[List], LinesToExport);
          } else {
            writeLines (LinesToExport, oss.str ());
          }
        }
        
        // Save the standard lamp spectra
        StdLamp = ExptSpectra[Spec].standard_lamp_spectrum ();
        oss.str ("");
        oss << Filename << "/" << ExptSpectra[Spec].standard_lamp_file ();
        if (StdLamp.size () != 0) {
          TxtOut = fopen (oss.str().c_str(), "w");
          fprintf (TxtOut, "# Standard lamp spectrum created by FAST\n#\n");
          fprintf (TxtOut, "# this was attached to %*s\n#\n", 
            (int)ExptSpectra[Spec].name ().size (), ExptSpectra[Spec].name ().c_str());
          for (unsigned int i = 0; i < StdLamp.size (); i ++) {
            fprintf (TxtOut, "%13.5f %13.6E\n", StdLamp[i].x, StdLamp[i].y);
          }
          fclose (TxtOut);
        }

        // Save the standard lamp radiance data
        StdLamp = ExptSpectra[Spec].radiance ();
        RadianceErr = ExptSpectra[Spec].radiance_error_ranges ();
        oss.str ("");
        oss << Filename << "/" << ExptSpectra[Spec].radiance_file ();
        if (StdLamp.size () != 0) {
          TxtOut = fopen (oss.str().c_str(), "w");
          fprintf (TxtOut, "# Standard lamp radiance file created by FAST\n#\n");
          fprintf (TxtOut, "# this was attached to %*s\n#\n", 
            (int)ExptSpectra[Spec].name ().size (), ExptSpectra[Spec].name ().c_str());
          for (unsigned int i = 0; i < StdLamp.size (); i ++) {
            fprintf (TxtOut, "%13.5f %13.6E\n", StdLamp[i].x, exp(StdLamp[i].y));
          }
          fprintf (TxtOut, "U\n");
          fprintf (TxtOut, "# Min wavelength   Max wavelength   Uncertainty / %%\n");
          fprintf (TxtOut, "# -------------------------------------------------\n");
          for (unsigned int i = 0; i < RadianceErr.size (); i ++) {
            fprintf (TxtOut, "  %14.2f   %14.2f   %14.2f\n", 
              RadianceErr[i].min, RadianceErr[i].max, RadianceErr[i].err);
          }
          fclose (TxtOut);
        }
        
        // Save any calculated spectral response functions
        Response = ExptSpectra[Spec].response ();
        oss.str ("");
        oss << Filename << "/" << ExptSpectra[Spec].name () << ".response";
        if (Response.size () > 0) {
          TxtOut = fopen (oss.str().c_str(), "w");
          fprintf (TxtOut, "# Spectral response function created by FAST\n#\n");
          fprintf (TxtOut, "# Attached to spectrum : %*s\n", 
            (int)ExptSpectra[Spec].name ().size (), 
            ExptSpectra[Spec].name ().c_str());
          fprintf (TxtOut, "# Std. Lamp Spectrum   : %*s \n", 
            (int)ExptSpectra[Spec].standard_lamp_file ().size (), 
            ExptSpectra[Spec].standard_lamp_file ().c_str());
          fprintf (TxtOut, "# Std. Lamp Radiance   : %*s \n#\n", 
            (int)ExptSpectra[Spec].radiance_file ().size (), 
            ExptSpectra[Spec].radiance_file ().c_str());
          for (unsigned int i = 0; i < Response.size (); i ++) {
            fprintf (TxtOut, "%13.5f %13.6E\n", Response[i].x, Response[i].y);
          }
          fclose (TxtOut);
        }
      }
      oss.str ("");
      oss << "Project exported to " << Filename;
      Status.push (oss.str ());
      break;
    }
  }
}


//------------------------------------------------------------------------------
// on_file_save_level () : 
//
void AnalyserWindow::on_file_save_level () {
/*  Gtk::FileChooserDialog dialog("Save lines in the current level",
    Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_transient_for(*this);

  // Add response buttons the the dialog
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  // Add filters, so that only certain file types can be selected
  Gtk::FileFilter filter_text;
  filter_text.set_name("Text files");
  filter_text.add_mime_type("text/plain");
  dialog.add_filter(filter_text);

  int result = dialog.run ();

  // Handle the response 
  switch(result)
  {
    case(Gtk::RESPONSE_OK):
    {
    
      Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeLevels.get_selection();
      if (treeSelection) {
        Gtk::TreeModel::iterator iter = treeLevels.get_selection()->get_selected();
        if (iter) {
          ostringstream oss;
          
          oss << dialog.get_filename() << ".krz";
          int Level = (*iter)[levelCols.index];
          KuruczList.upperLevel(Level).save(oss.str());
          vector <XgLine> LinesToSave;
          for (unsigned int i = 0; i < LevelLines.size (); i ++) {
            oss.str ("");
            oss << dialog.get_filename() << "_" << ExptSpectra[i].index() << ".aln";
            LinesToSave.clear ();
            for (int j = LevelLines[i].size () - 1; j >= 0; j --) {
              if (LevelLines[i][j].xgLine.wavenumber () > 0.0) {
                LinesToSave.push_back (LevelLines[i][j].xgLine);
              }
            }
            writeLines (LinesToSave, oss.str().c_str());
          }
          
          size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
          oss << "Successfully saved " << dialog.get_filename().substr(FilePos) << ".";
          Status.push (oss.str());
        }
      }
  
      
    }
  }*/
}

