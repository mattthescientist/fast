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
// AnalyserWindow class (analyserwindow_signal.cpp)
//==============================================================================
// This file contains general signal handlers that do not belong in any of the
// other analyserwindow_signal_*.cpp files.

#ifndef ASCII_A
  #define ASCII_A 65
#endif

//------------------------------------------------------------------------------
// ref_spectrum_toggled (const Glib::ustring&) : Handles signal toggled events
// from the reference spectrum tick boxes in the treeSpectra list.
//
void AnalyserWindow::ref_spectrum_toggled (const Glib::ustring& PathIn) {
  Gtk::TreePath Path (PathIn);
  Gtk::TreeModel::iterator ClickedIter = m_refTreeModel -> get_iter (Path);
  Gtk::TreeModel::Row ClickedRow = (*ClickedIter);
  Path.up ();
  Gtk::TreeModel::iterator ParentIter = m_refTreeModel -> get_iter (Path);
  Gtk::TreeModel::Row ParentRow = (*ParentIter);
  Gtk::TreeModel::Row ActiveRow;
  bool ToggleValue;
  
  // Determine whether a spectrum or line list has been clicked
  if (Path.up ()) {
    ActiveRow = ParentRow;  // A line list has been clicked
  } else {
    ActiveRow = ClickedRow; // A spectrum has been clicked
  }
  ToggleValue = !ActiveRow[m_Columns.ref];
  
  // If ToggleValue is true, clear all previously ticked tick boxes
  if (ToggleValue) {
    ParentRow = *(m_refTreeModel -> get_iter (Path));
    for (Gtk::TreeModel::iterator iter = ParentRow.children().begin(); 
      iter != ParentRow.children().end(); ++iter) {
      (*iter)[m_Columns.ref] = false;
      for (Gtk::TreeModel::iterator iter2 = (*iter).children().begin(); 
        iter2 != (*iter).children().end(); ++iter2) {
        (*iter2)[m_Columns.ref] = false;
      }
    }
    for (unsigned int i = 0; i < ExptSpectra.size (); i ++) {
      ExptSpectra[i].isReference (false);
    }
  }
  
  // Set the clicked box and the value of all child boxes to ToggleValue
  ActiveRow.set_value (m_Columns.ref, ToggleValue);
  ExptSpectra[ActiveRow[m_Columns.index]].isReference (ToggleValue);
  for (Gtk::TreeModel::iterator iter = ActiveRow.children().begin(); 
    iter != ActiveRow.children().end(); ++iter) {
    (*iter)[m_Columns.ref] = ToggleValue;
  }
  updateKuruczCompleteness ();
  updatePlottedData ();
  projectHasChanged (true);
}


//------------------------------------------------------------------------------
// newProject () : Clears all currently loaded data and so creates a fresh work
// space.
//
void AnalyserWindow::newProject () {

  // Pop-up a question dialog to confirm the user wants a new project
  if (ProjectChangedSinceSave) {
    Gtk::MessageDialog confirm(*this, "Would you like to save the current project?",
      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    confirm.set_secondary_text(" Click cancel to return to the project.");
    confirm.add_button ("Yes", Gtk::RESPONSE_YES);
    confirm.add_button ("No", Gtk::RESPONSE_NO);
    confirm.add_button ("Cancel", Gtk::RESPONSE_CANCEL);
    int Result = confirm.run ();
    if (Result == Gtk::RESPONSE_YES) {
      try {
        if (CurrentFilename == "") { 
          on_file_save_as ();
          if (CurrentFilename == "") { throw Error (FLT_FILE_WRITE_ERROR); }
        } else {
          saveProject (CurrentFilename);
        }
      } catch (Error &e) {
        // Do nothing else here
        return;
      }
    } else if (Result == Gtk::RESPONSE_CANCEL) {
      return;
    }
  }
  
  // If the user confirms the new project, delete all current data
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
  
  Status.push ("New project created");
}


//------------------------------------------------------------------------------
// clearDisplayedPlots () : Clears the plot area in the top-right pane of the
// AnalyserWindow object.
//
void AnalyserWindow::clearDisplayedPlots () {
  for (int i = LineBoxes.size () - 1; i >= 0; i --) {
    delete hboxSpectrumPlots [i];
    delete frameSpectrumPlots [i];
  }
  LineBoxes.clear ();
  hboxSpectrumPlots.clear ();
  frameSpectrumPlots.clear ();
}


//------------------------------------------------------------------------------
// lifetimeValidatedOnCellData (CellRenderer* const TreeModel::iterator&) :
// This function is called to update the contents of the lifetime field in the
// Kurucz branching fraction data list after it has been edited.
//
void AnalyserWindow::lifetimeValidatedOnCellData (Gtk::CellRenderer*, const Gtk::TreeModel::iterator& iter) {
  if (iter) {
    Gtk::TreeModel::Row row = *iter;
    ostringstream oss;
    
    oss << row[levelCols.lifetime];
    Glib::ustring ViewText = oss.str().c_str();
    
    #ifdef GLIBMM_PROPERTIES_ENABLED
      textLifetime.property_text() = ViewText;
    #else
      textLifetime.set_property("text", ViewText);
    #endif
  }
}


//------------------------------------------------------------------------------
// lifetimeErrorValidatedOnCellData (CellRenderer* const TreeModel::iterator&) :
// This function is called to update the contents of the lifetime error field in
// the Kurucz branching fraction data list after it has been edited.
//
void AnalyserWindow::lifetimeErrorValidatedOnCellData (Gtk::CellRenderer*,
  const Gtk::TreeModel::iterator& iter) {
  if (iter) {
    Gtk::TreeModel::Row row = *iter;
    ostringstream oss;
    
    oss << row[levelCols.err_lifetime];
    Glib::ustring ViewText = oss.str().c_str();
    
    #ifdef GLIBMM_PROPERTIES_ENABLED
      textLifetimeError.property_text() = ViewText;
    #else
      textLifetimeError.set_property("text", ViewText);
    #endif
  }
}


//------------------------------------------------------------------------------
// lifetimeEditStarted (Gtk::CellEditable* , const Glib::ustring&) : This is
// called when the user starts to edit the upper level lifetime field in the 
// list of Kurucz branching fraction data.
//
void AnalyserWindow::lifetimeEditStarted (Gtk::CellEditable* cell_editable, const Glib::ustring& path) {
  if(lifetimeValidated)
  {
    Gtk::CellEditable* celleditable_validated = cell_editable;
    Gtk::Entry* pEntry = dynamic_cast<Gtk::Entry*>(celleditable_validated);
    if(pEntry)
    {
      pEntry->set_text(invalid_text_for_retry);
      lifetimeValidated = false;
      invalid_text_for_retry.clear();
    }
  }

}


//------------------------------------------------------------------------------
// lifetimeEdited (const Glib::ustring&, const Glib::ustring&) : Triggered once
// the user has finished editing the level lifetime field. Checks that the input
// is valid, and, if it is, updates the Kurucz branching fraction information
// and the plotted data.
//
void AnalyserWindow::lifetimeEdited (const Glib::ustring& path_string, const Glib::ustring& new_text) {
    Gtk::TreePath path(path_string);

  istringstream iss;
  double NewValue;
  iss.str (new_text.c_str());
  iss >> NewValue;

  // Check to see if the new lifetime is invalid.
  if (NewValue <= 0.0)
  {
    // The lifetime is invalid. Display an error message and force the user to
    // try again.
    Gtk::MessageDialog dialog
      (*this, "The level lifetime must be a positive number in units of ns.", 
      false, Gtk::MESSAGE_ERROR);
    dialog.run();
    invalid_text_for_retry = new_text;
    lifetimeValidated = true;
    treeLevels.set_cursor(path, columnLifetime, textLifetime, true);

  } else {
  
    // The lifetime is valid so update all the branching fraction information
    // accordingly.
    Gtk::TreeModel::iterator iter = levelTreeModel -> get_iter(path);
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[levelCols.lifetime] = NewValue;
      row[levelCols.lifetime_colour] = Gdk::Color (AW_EQWIDTH_NORM_COLOUR);
      updateKuruczBF ();
      updatePlottedData ();
      projectHasChanged (true);
    }
  }
}


//------------------------------------------------------------------------------
// lifetimeErrorEdited (const Glib::ustring&, const Glib::ustring&) : Triggered
// once the user has finished editing the level lifetime field. Checks that the
// input is valid, and, if it is, updates the Kurucz branching fraction 
// information and the plotted data.
//
void AnalyserWindow::lifetimeErrorEdited (const Glib::ustring& path_string, const Glib::ustring& new_text) {
    Gtk::TreePath path(path_string);

  istringstream iss;
  double NewValue;
  iss.str (new_text.c_str());
  iss >> NewValue;

  // Check to see if the new lifetime is invalid.
  if (NewValue <= 0.0)
  {
    // The lifetime is invalid. Display an error message and force the user to
    // try again.
    Gtk::MessageDialog dialog
      (*this, "The level lifetime error must be a positive number in units of ns.", 
      false, Gtk::MESSAGE_ERROR);
    dialog.run();
    invalid_text_for_retry = new_text;
    lifetimeValidated = true;
    treeLevels.set_cursor(path, columnLifetimeError, textLifetimeError, true);

  } else {
  
    // The lifetime is valid so update all the branching fraction information
    // accordingly.
    Gtk::TreeModel::iterator iter = levelTreeModel -> get_iter(path);
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[levelCols.err_lifetime] = NewValue;
      row[levelCols.err_lifetime_colour] = Gdk::Color(AW_EQWIDTH_NORM_COLOUR);
      updateKuruczBF ();
      updatePlottedData ();
      projectHasChanged (true);
    }
  }
}


//------------------------------------------------------------------------------
// on_tools_options () : Shows the FAST options box
//
void AnalyserWindow::on_tools_options () {
  Options.set_modal (true);
  Gtk::Main::run(Options);
  updatePlottedData (true);
  updateKuruczCompleteness ();
}


//------------------------------------------------------------------------------
// on_help_about () : Shows the FAST about box
//
void AnalyserWindow::on_help_about () {
  AboutWindow About;
  About.set_modal (true);
  Gtk::Main::run(About);
}
