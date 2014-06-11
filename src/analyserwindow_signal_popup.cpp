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
// AnalyserWindow class (analyserwindow_signal_popup.cpp
//==============================================================================
// This file contains the signal handlers for popup menu actions that do not
// exist in the main menubar.

//------------------------------------------------------------------------------
// on_popup_link_spectrum () : Begins the process of creating a link between two
// experimental spectra listed in treeSpectra in the bottom-left pane of the 
// FAST window. The task is finished by do_link_spectrum () when the user 
// selects a second spectrum in the list.
//
void AnalyserWindow::on_popup_link_spectrum () {
  if (ExptSpectra.size () > 1) {
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeSpectra.get_selection();
    if(refSelection) {
      Gtk::TreeModel::iterator iter = refSelection->get_selected();
      if(iter) {
        ostringstream oss;
        oss << "Click on the spectrum you want to link to " 
          << (string)(*iter)[m_Columns.name];
        Status.push (oss.str());
        TypeLinkSpectra NewLink;
        NewLink.a = (*iter)[m_Columns.index];
        NewLink.b = -1;
        LinkedSpectra.push_back (NewLink);
        LinkConnection = treeSpectra.signal_button_press_event().connect_notify
          (sigc::mem_fun(*this, &AnalyserWindow::do_link_spectrum));
        AbortLinkConnection = this->signal_button_press_event().connect_notify
          (sigc::mem_fun(*this, &AnalyserWindow::abort_link_spectrum));
      }
    }
  }
}


//------------------------------------------------------------------------------
// do_link_spectrum () : Finishes the process of creating a link between two
// experimental spectra listed in treeSpectra in the bottom-left pane of the 
// FAST window. This function is called when the user selects the second 
// spectrum to link. The link will then be active and will affect how the ratio
// of the intensity scales of the two spectra is calculated in 
// updateComparisonList ().
//
void AnalyserWindow::do_link_spectrum (GdkEventButton* event) {
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeSpectra.get_selection();
  if(refSelection) {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter) {
      Gtk::TreeModel::Row SelectedRow = *(iter);
      LinkConnection.disconnect ();
      AbortLinkConnection.disconnect ();
      TypeLinkSpectra *NewLink = &LinkedSpectra[LinkedSpectra.size ()-1];
      if (NewLink -> a != (unsigned int)(*iter)[m_Columns.index]){
        NewLink -> b = (unsigned int)(*iter)[m_Columns.index];
        ostringstream oss;
        oss << "Spectrum " << ExptSpectra[NewLink -> a].name () << " and "
          << (string)(*iter)[m_Columns.name] << " have been linked";
        Status.push (oss.str());
        
        // Add details of the link to treeSpectra as a child of the selected
        // experimental spectrum.
        Gtk::TreeModel::Row row = *(m_refTreeModel->append(SelectedRow->children()));
        int Index = (*iter)[m_Columns.index];
        bool Ref = (*iter)[m_Columns.ref];
        oss.str ("");
        oss << "Linked to " << ExptSpectra[NewLink -> a].name ();
        row[m_Columns.index] = Index;
        row[m_Columns.line_index] = -1;
        row[m_Columns.ref] = Ref;
        row[m_Columns.emin] = 0;
        row[m_Columns.emax] = 0;
        row[m_Columns.name] = oss.str().c_str();
        row[m_Columns.bg_colour] = Gdk::Color (AW_LINK_COLOUR);
        
      } else {
        LinkedSpectra.pop_back ();
        ostringstream oss;
        oss << "You cannot link a spectrum to itself. Linking aborted.";
        Status.push (oss.str());
      }
    }
  }
}


//------------------------------------------------------------------------------
// abort_link_spectrum (GdkEventButton *) : If on_popup_link_spectrum () has
// been called, but the user fails to click on a second spectrum to create a
// link, this function is called to abort the link process.
//
void AnalyserWindow::abort_link_spectrum (GdkEventButton* event) {
  LinkConnection.disconnect ();
  LinkConnection.disconnect ();
  LinkedSpectra.pop_back ();
  ostringstream oss;
  oss << "Aborted spectrum linking";
  Status.push (oss.str());
}


//------------------------------------------------------------------------------
// on_popup_export_linelist () : Called when the user right clicks a line list
// in treeSpectra and selects "Export Line List". The selected line list is
// exported to a file specificed by the user.
//
void AnalyserWindow::on_popup_export_linelist ()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeSpectra.get_selection();
  if(refSelection) {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter) { 
      int Index = (*iter)[m_Columns.index];
      int LineIndex = (*iter)[m_Columns.line_index];
      vector <XgLine> LinesToExport = ExptSpectra[Index].lines ()[LineIndex];
      vector <char> LinHeader = ExptSpectra[Index].linHeaders ()[LineIndex];
      
      Gtk::FileChooserDialog dialog("Export the selected line list as",
        Gtk::FILE_CHOOSER_ACTION_SAVE);
      dialog.set_transient_for(*this);

      // Add response buttons the the dialog
      dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

      // Add filters, so that only certain file types can be selected
      Gtk::FileFilter filter_dat, filter_all;
      filter_dat.set_name("XGremlin LIN files");
      filter_dat.add_pattern("*.lin");
      dialog.add_filter(filter_dat);
      filter_all.set_name("All files");
      filter_all.add_pattern("*");
      dialog.add_filter(filter_all);

      dialog.set_current_folder (DefaultFolder);
      int result = dialog.run ();

      // Handle the response 
      switch(result)
      {
        case(Gtk::RESPONSE_OK):
          string Filename = dialog.get_filename();
          size_t FilePos = dialog.get_filename().find_last_of ("/\\") + 1;
          DefaultFolder = dialog.get_filename().substr (0, FilePos);
          if (Filename.substr (Filename.size () - 4, 4) != ".lin") {
            Filename = Filename + ".lin";
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
            int result = message.run();
            if (result == Gtk::RESPONSE_NO) { return; }
          }
          
          writeLinFile (Filename, LinHeader, LinesToExport);
          break;
      }
    }
  }
}
        
      
//------------------------------------------------------------------------------
// on_popup_remove_linelist () : Called when the user right clicks a line list
// in treeSpectra and selects "Remove Line List". The selected line list is
// deleted here, then update functions called to refresh the remaining data.
//
void AnalyserWindow::on_popup_remove_linelist ()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeSpectra.get_selection();
  if(refSelection) {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter) {
      int Index = (*iter)[m_Columns.index];
      clearDisplayedPlots ();
      ExptSpectra[Index].remove_linelist ((*iter)[m_Columns.line_index]);
      Gtk::TreePath Path = treeSpectra.get_model()->get_path(iter);
      Path.up ();

      // Adjust the m_Columns.line_index property of the remaining line lists to
      // remove the gap left by the deleted list.
      Gtk::TreeModel::Row ParentRow = *(m_refTreeModel -> get_iter (Path));
      for (Gtk::TreeModel::iterator iter2 = ParentRow.children().begin(); 
        iter2 != ParentRow.children().end(); ++iter2) {
        if ((*iter2)[m_Columns.line_index] > (*iter)[m_Columns.line_index] &&
          (*iter2)[m_Columns.line_index] != -1) {
          (*iter2)[m_Columns.line_index] = (*iter2)[m_Columns.line_index] - 1;
        }
      }       

      // Finally, erase the list's entry in treeSpectra and update anything that
      // would have referenced the deleted line list.
      m_refTreeModel->erase (iter);
      projectHasChanged (true);
      getLinePairs ();
      updatePlottedData ();
      updateKuruczCompleteness ();
    }
  }
}


//------------------------------------------------------------------------------
// on_popup_remove_standard_lamp_spectrum () : Called when the user right clicks
// a response function in treeSpectra and selects "Remove Response Function". 
// The function is deleted here. Update routines are then called to refresh 
// anything that would have referenced the deleted response function.
//
void AnalyserWindow::on_popup_remove_standard_lamp_spectrum ()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeSpectra.get_selection();
  if(refSelection) {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter) {
      int Index = (*iter)[m_Columns.index];
      ExptSpectra[Index].remove_standard_lamp_spectrum ();
      m_refTreeModel->erase (iter);
      projectHasChanged (true);
      updateKuruczCompleteness();
      updatePlottedData();
    }
  }
}


//------------------------------------------------------------------------------
// on_popup_remove_radiance () : Called when the user right clicks a 
// response function calibration error file in treeSpectra and selects "Remove 
// Response Calibration Errors". The errors are deleted here. Update routines
// are then called to refresh  anything that would have referenced the deleted
// calibration errors.
//
void AnalyserWindow::on_popup_remove_radiance ()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeSpectra.get_selection();
  if(refSelection) {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter) {
      int Index = (*iter)[m_Columns.index];
      ExptSpectra[Index].remove_radiance ();
      m_refTreeModel->erase (iter);
      projectHasChanged (true);
      updateKuruczCompleteness();
      updatePlottedData();
    }
  }
}


//------------------------------------------------------------------------------
// on_popup_remove_linelist () : Called when the user right clicks a Kurucz 
// level in treeLevels and selects "Remove Level". The selected level is then
// deleted and any information displayed about it cleared from the window.
//
void AnalyserWindow::on_popup_remove_level()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeLevels.get_selection();
  if(refSelection) {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter) {
      int Level = (*iter)[levelCols.index];
      clearDisplayedPlots (); 
      KuruczList.eraseUpperLevel (Level);
      modelDataXGr -> clear ();
      modelDataBF -> clear ();
      lineDataTreeModel -> clear ();
      modelDataComp -> clear ();
      if (KuruczList.size () > 0) {
        levelTreeModel->erase (iter);
        getLinePairs ();
        refreshKuruczList ();
      } else {
        levelTreeModel -> clear ();
        modelLevelsBF -> clear ();
      }
      projectHasChanged (true);
    }
  }
}


//------------------------------------------------------------------------------
// on_popup_disable_line () : Called when the user right clicks on a line
// profile in the line plot area and chooses to enable or disable the line.
//
void AnalyserWindow::on_popup_disable_line (bool Disable)
{
	updatePlottedData (true);
	updateKuruczCompleteness ();
	projectHasChanged (true);
}

//------------------------------------------------------------------------------
// on_popup_delete_line () : Called when the user right clicks on any line
// profile in the line plot area and chooses to delete the line profile.
//
void AnalyserWindow::on_popup_hide_line ()
{
	updatePlottedData (true);
	updateKuruczCompleteness ();
	projectHasChanged (true);
}
