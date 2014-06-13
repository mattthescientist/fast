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
// OutputWindow class (outputwindow.cpp)
//==============================================================================
// Displays a window window containing the necessary functionality to output
// FAST results to Text/CSV or LaTeX file formats
//
#include "outputwindow.h"

//------------------------------------------------------------------------------
//
//
OutputWindow::OutputWindow () :
  ButtonSave (Gtk::Stock::SAVE), ButtonCancel (Gtk::Stock::CANCEL) {
   
  set_title("Output Results");
  set_default_size(640, 480);
  set_position(Gtk::WIN_POS_CENTER);

  create_image_buttons ();

  Scroll.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  ScrollAvailable.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  ScrollSelected.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  add (Scroll);
  Scroll.add (BaseVBox);
  BaseVBox.pack_start (BaseHBox, true, true, 0);

  BaseHBox.pack_start (LeftBox, false, false, 0);
  LeftBox.pack_start (FrameListFiles, false, false, 0);
  FrameListFiles.set_label ("Data Groups");
  FrameListFiles.add (BoxListFiles);
  BoxListFiles.pack_start (ButtonBFs, false, false, 2);
  BoxListFiles.pack_start (ButtonTargets, false, false, 2);
  BoxListFiles.pack_start (ButtonProfiles, false, false, 2);
  ButtonBFs.set_label ("Branching Fractions");
  ButtonBFs.set_active (true);
  ButtonTargets.set_label ("Target Lines");
  ButtonProfiles.set_label ("Fit Profiles");
  ButtonMoveUp.set_sensitive(false);
  ButtonMoveDown.set_sensitive(false);
  LeftBox.pack_start (ComboOutputType, false, false, 2);
  for (unsigned int i = 0; i < NUM_OUTPUTWINDOW_TYPES; i ++) {
    ComboOutputType.append_text (OUTPUTWINDOW_TYPES [i]);
  }
  ComboOutputType.set_active (0);
  LeftBox.pack_start (FrameListDelimiters, false, false, 2);
  FrameListDelimiters.set_label ("Delimiter");
  FrameListDelimiters.add (BoxListDelimiters);
  BoxListDelimiters.pack_start (ButtonDelimitComma);
  BoxListDelimiters.pack_start (ButtonDelimitSpace);
  BoxListDelimiters.pack_start (ButtonDelimitTab);
  BoxListDelimiters.pack_start (ButtonDelimitOther);
  BoxListDelimiters.pack_start (EntryDelimitOther);
  ButtonDelimitComma.set_label ("Comma");
  ButtonDelimitSpace.set_label ("Space");
  ButtonDelimitTab.set_label ("Tab");
  ButtonDelimitOther.set_label ("Other");
  Gtk::RadioButton::Group group = ButtonDelimitComma.get_group();
  ButtonDelimitSpace.set_group (group);
  ButtonDelimitTab.set_group (group);
  ButtonDelimitOther.set_group (group);
  EntryDelimitOther.set_sensitive (false);

  BaseHBox.pack_start (FrameFieldSelection, true, true, 5);
  FrameFieldSelection.set_label ("Select fields to output");
  FrameFieldSelection.add (BoxFieldSelection);
  ScrollAvailable.add (TreeAvailableFields);
  ScrollSelected.add (TreeSelectedFields);
  BoxFieldSelection.pack_start (ScrollAvailable, true, true, 2);
  BoxFieldSelection.pack_start (BoxFieldSelectButtons, false, false, 2);
  BoxFieldSelection.pack_start (ScrollSelected, true, true, 2);
  BoxFieldSelection.pack_start (BoxFieldMoveButtons, false, false, 2);
  BoxFieldSelectButtons.pack_start (BoxFieldSelectButtons2, Gtk::PACK_EXPAND_PADDING);
  BoxFieldSelectButtons2.pack_start (ButtonAddAll, false, false, 2);
  BoxFieldSelectButtons2.pack_start (ButtonAdd, false, false, 2);
  BoxFieldSelectButtons2.pack_start (ButtonRemove, false, false, 2);
  BoxFieldSelectButtons2.pack_start (ButtonRemoveAll, false, false, 2);
  BoxFieldMoveButtons.pack_start (BoxFieldMoveButtons2, Gtk::PACK_EXPAND_PADDING);
  BoxFieldMoveButtons2.pack_start (ButtonMoveUp, false, false, 2);
  BoxFieldMoveButtons2.pack_start (ButtonMoveDown, false, false, 2);

  BaseVBox.pack_start (FrameFileSelection, false, false, 5);
  FrameFileSelection.set_label ("Output file name");
  FrameFileSelection.add (BoxFileSelection);
  BoxFileSelection.pack_start (EntryFileSelection, true, true, 2);
  BoxFileSelection.pack_start (ButtonBrowse, false, false, 2);

  BaseVBox.pack_start (BoxSaveCancel, false, false, 10);
  BoxSaveCancel.pack_end (ButtonSave, false, false, 2);
  BoxSaveCancel.pack_end (ButtonCancel, false, false, 2);
  ButtonSave.set_size_request (75);
  ButtonCancel.set_size_request (75);
  ButtonSave.set_can_default(true);
  ButtonSave.grab_default();
  
  modelFieldsAvailable = Gtk::ListStore::create (fieldsCols);
  modelFieldsSelected = Gtk::ListStore::create (fieldsCols);
  TreeAvailableFields.set_model (modelFieldsAvailable);
  TreeSelectedFields.set_model (modelFieldsSelected);
  TreeAvailableFields.append_column ("Available Fields", fieldsCols.Name);
  TreeSelectedFields.append_column ("SelectedFields", fieldsCols.Name);
  Glib::RefPtr<Gtk::TreeSelection> treeDataSel = TreeAvailableFields.get_selection();
  treeDataSel->set_mode(Gtk::SELECTION_MULTIPLE);
  treeDataSel = TreeSelectedFields.get_selection();
  treeDataSel->set_mode(Gtk::SELECTION_MULTIPLE);
  
  set_button_signals ();
  show_all_children();
  DisplaySet = false;
}


//------------------------------------------------------------------------------
// create_image_buttons () : Prepares the OutputWindow buttons that will display
// a stock image, but no text.
//
void OutputWindow::create_image_buttons () {
  // Create a Gtk::Image object for each button
  Gtk::Image *imageAddAll = Gtk::manage( new Gtk::Image() ); 
  Gtk::Image *imageAdd = Gtk::manage( new Gtk::Image() ); 
  Gtk::Image *imageRemove = Gtk::manage( new Gtk::Image() ); 
  Gtk::Image *imageRemoveAll = Gtk::manage( new Gtk::Image() );
  Gtk::Image *imageBrowse = Gtk::manage( new Gtk::Image() );  
  Gtk::Image *imageMoveUp = Gtk::manage( new Gtk::Image() );
  Gtk::Image *imageMoveDown = Gtk::manage( new Gtk::Image() );  
  
  // Populate each Gtk::Image with the appropriate stock icon
  Gtk::Stock::lookup( Gtk::Stock::ADD, Gtk::ICON_SIZE_SMALL_TOOLBAR, *imageAddAll); 
  Gtk::Stock::lookup( Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_SMALL_TOOLBAR, *imageAdd); 
  Gtk::Stock::lookup( Gtk::Stock::GO_BACK, Gtk::ICON_SIZE_SMALL_TOOLBAR, *imageRemove); 
  Gtk::Stock::lookup( Gtk::Stock::REMOVE, Gtk::ICON_SIZE_SMALL_TOOLBAR, *imageRemoveAll); 
  Gtk::Stock::lookup( Gtk::Stock::DIRECTORY, Gtk::ICON_SIZE_SMALL_TOOLBAR, *imageBrowse); 
  Gtk::Stock::lookup( Gtk::Stock::GO_UP, Gtk::ICON_SIZE_SMALL_TOOLBAR, *imageMoveUp); 
  Gtk::Stock::lookup( Gtk::Stock::GO_DOWN, Gtk::ICON_SIZE_SMALL_TOOLBAR, *imageMoveDown); 
  
  // Add the images to the class image buttons
  ButtonAddAll.add (*imageAddAll); 
  ButtonAdd.add (*imageAdd); 
  ButtonRemove.add (*imageRemove); 
  ButtonRemoveAll.add (*imageRemoveAll); 
  ButtonBrowse.add (*imageBrowse);
  ButtonMoveUp.add (*imageMoveUp);
  ButtonMoveDown.add (*imageMoveDown);
  
  // Add tooltip text to help the user
  ButtonAddAll.set_tooltip_text ("Add all fields");
  ButtonAdd.set_tooltip_text ("Add selected fields");
  ButtonRemove.set_tooltip_text ("Remove selected fields");
  ButtonRemoveAll.set_tooltip_text ("Remove all fields");
  ButtonMoveUp.set_tooltip_text ("Move selected fields up");
  ButtonMoveDown.set_tooltip_text ("Move selected fields down");
}


//------------------------------------------------------------------------------
//
//
void OutputWindow::set_button_signals () {
  ButtonCancel.signal_clicked().connect (sigc::mem_fun (*this,
    &OutputWindow::on_button_cancel));
  ButtonAddAll.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_add_all));
  ButtonAdd.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_add));
  ButtonRemove.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_remove));
  ButtonRemoveAll.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_remove_all));
  ButtonMoveUp.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_move_up));
  ButtonMoveDown.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_move_down));

  ButtonBrowse.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_browse));
  ButtonSave.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_save));

  ButtonBFs.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::set_display_fields));
  ButtonTargets.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::set_display_fields));
  ButtonProfiles.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::set_display_fields));

  ComboOutputType.signal_changed().connect(sigc::mem_fun (*this,
    &OutputWindow::on_change_combo_output));

  ButtonDelimitOther.signal_clicked().connect(sigc::mem_fun (*this,
    &OutputWindow::on_button_delimit_other));
}


//------------------------------------------------------------------------------
// set_results (...) :
//
void OutputWindow::set_results (vector <vector <DataBF> > ResultsIn, 
  vector <vector <XgLine> > FitsIn, vector <vector <KzLine> > TargetsIn) {
  Fits.clear ();
  Targets.clear ();
  Results.clear ();
  ResultStrings.clear ();
  Fits = FitsIn;
  Targets = TargetsIn;
  Results = ResultsIn;
  set_result_strings ();
  
  if (!DisplaySet) {
    set_display_fields ();
  }
}


//------------------------------------------------------------------------------
// set_result_strings () :
//
void OutputWindow::set_result_strings () {
  OutputField NextField;
  vector <string> ResultsForNextLevel;
  ostringstream oss;

  ResultStrings.clear ();
  oss.setf( std::ios::fixed, std:: ios::floatfield);
  NextField.Active = false;
  NextField.Selected =false;
  for (unsigned int l = 0; l < NUM_BF_OUTPUT_FIELDS; l ++) {
    NextField.Name = FAST_BF_FIELD_NAMES [l];
    NextField.List = OUTPUT_BF_LIST;
    NextField.Value.clear ();
    for (unsigned int Level = 0; Level < Results.size (); Level ++) {
      ResultsForNextLevel.clear ();
      for (unsigned int Line = 0; Line < Results[Level].size (); Line ++) {
        switch (l) {
          case 0:
            NextField.List = OUTPUT_BF_LIST + OUTPUT_FITTED_LIST;
            ResultsForNextLevel.push_back(Results[Level][Line].spectrum);
            break;
          case 1:
            oss.str (""); oss << Results[Level][Line].index;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 2:
            NextField.List = OUTPUT_BF_LIST + OUTPUT_FITTED_LIST + OUTPUT_TARGET_LIST;
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(3) << Results[Level][Line].wavenumber;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 3:
            NextField.List = OUTPUT_BF_LIST + OUTPUT_FITTED_LIST;
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(0) << Results[Level][Line].eqwidth;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 4:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(2) << Results[Level][Line].err_line;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 5:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(2) << Results[Level][Line].err_cal;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 6:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(2) << Results[Level][Line].err_trans;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 7:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(2) << Results[Level][Line].err_total;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 8:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(0) << Results[Level][Line].err_eqwidth;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 9:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(4) << Results[Level][Line].br_frac;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 10:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(2) << Results[Level][Line].err_br_frac;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 11:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(3) << (Results[Level][Line].a / pow(10.0,6));
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 12:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(2) << Results[Level][Line].err_a;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 13:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(3) << Results[Level][Line].loggf;
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 14:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(3) << Results[Level][Line].dex;
            ResultsForNextLevel.push_back (oss.str ());
            break;
        }
      }
      oss.unsetf (ios::scientific);
      oss.unsetf (ios::fixed);
      NextField.Value.push_back (ResultsForNextLevel);
    }
    NextField.ResultIndex = l;
    ResultStrings.push_back (NextField);
  }
  
  for (unsigned int l = 0; l < NUM_FITTED_OUTPUT_FIELDS; l ++) {
    NextField.Name = FAST_FITTED_FIELD_NAMES [l];
    NextField.List = OUTPUT_FITTED_LIST;
    NextField.Value.clear ();
    for (unsigned int Level = 0; Level < Fits.size (); Level ++) {
      ResultsForNextLevel.clear ();
      for (unsigned int Line = 0; Line < Fits[Level].size (); Line ++) {
        switch (l) {
          case 0:
            oss.str (""); oss << Fits[Level][Line].line ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 1:
            ResultsForNextLevel.push_back(Fits[Level][Line].id ());
            break;
          case 2:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(1) << Fits[Level][Line].peak ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 3:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(2) << Fits[Level][Line].width ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 4:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(4) << Fits[Level][Line].dmp ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 5:
            oss.str (""); oss << setiosflags(ios::scientific) << setprecision(4) << Fits[Level][Line].epstot ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 6:
            oss.str (""); oss << setiosflags(ios::scientific) << setprecision(4) << Fits[Level][Line].epsevn ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 7:
            oss.str (""); oss << setiosflags(ios::scientific) << setprecision(4) << Fits[Level][Line].epsodd ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 8:
            oss.str (""); oss << setiosflags(ios::scientific) << setprecision(4) << Fits[Level][Line].epsran ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
        }
      }
      oss.unsetf (ios::scientific);
      oss.unsetf (ios::fixed);
      NextField.Value.push_back (ResultsForNextLevel); 
    }
    NextField.ResultIndex = NUM_BF_OUTPUT_FIELDS + l;
    ResultStrings.push_back (NextField);    
  }

  for (unsigned int l = 0; l < NUM_TARGET_OUTPUT_FIELDS; l ++) {
    NextField.Name = FAST_TARGET_FIELD_NAMES [l];
    NextField.List = OUTPUT_TARGET_LIST;
    NextField.Value.clear ();
    for (unsigned int Level = 0; Level < Fits.size (); Level ++) {
      ResultsForNextLevel.clear ();
      for (unsigned int Line = 0; Line < Fits[Level].size (); Line ++) {
        switch (l) {
          case 0:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(4) << Targets[Level][Line].lambda ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 1:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(3) << Targets[Level][Line].loggf ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 2:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(3) << Targets[Level][Line].brFrac ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 3:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(3) << Targets[Level][Line].eLower ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 4:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(1) << Targets[Level][Line].jLower ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 5:
            ResultsForNextLevel.push_back (Targets[Level][Line].configLower ());
            break;
          case 6:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(3) << Targets[Level][Line].eUpper ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 7:
            oss.str (""); oss << setiosflags(ios::fixed) << setprecision(1) << Targets[Level][Line].jUpper ();
            ResultsForNextLevel.push_back (oss.str ());
            break;
          case 8:
            ResultsForNextLevel.push_back (Targets[Level][Line].configUpper ());
            break;
        }
      }
      oss.unsetf (ios::scientific);
      oss.unsetf (ios::fixed);
      NextField.Value.push_back (ResultsForNextLevel);
    }
    NextField.ResultIndex = NUM_BF_OUTPUT_FIELDS + NUM_FITTED_OUTPUT_FIELDS + l;
    ResultStrings.push_back (NextField);
  }
}


//------------------------------------------------------------------------------
// set_display_fields () :
//
void OutputWindow::set_display_fields () {
  unsigned short int ActiveLists = 0x00;
  if (ButtonBFs.get_active ()) ActiveLists += OUTPUT_BF_LIST;
  if (ButtonTargets.get_active ()) ActiveLists += OUTPUT_TARGET_LIST;
  if (ButtonProfiles.get_active ()) ActiveLists += OUTPUT_FITTED_LIST;
  
  AvailableFields.clear ();
  SelectedFields.clear ();
  for (unsigned int Field = 0; Field < ResultStrings.size (); Field ++) {
    if (ResultStrings [Field].List & ActiveLists) {
      if (ResultStrings [Field].Selected) {
        SelectedFields.push_back (&ResultStrings[Field]);
      } else {
        AvailableFields.push_back (&ResultStrings[Field]);
      } 
    }
  }
  display_fields ();
  DisplaySet = true;
}


//------------------------------------------------------------------------------
// display_fields () :
//
void OutputWindow::display_fields () {
  Gtk::TreeModel::Row row;
  modelFieldsAvailable -> clear ();
  modelFieldsSelected -> clear ();   
  
  for (unsigned int Field = 0; Field < AvailableFields.size (); Field ++) {
    row = *(modelFieldsAvailable -> append ());
    row[fieldsCols.Name] = AvailableFields[Field]->Name;
    row[fieldsCols.ResultIndex] = AvailableFields[Field]->ResultIndex;
    row[fieldsCols.ListIndex] = Field;
  }
  for (unsigned int Field = 0; Field < SelectedFields.size (); Field ++) {
    row = *(modelFieldsSelected -> append ());
    row[fieldsCols.Name] = SelectedFields[Field]->Name;
    row[fieldsCols.ResultIndex] = SelectedFields[Field]->ResultIndex;
    row[fieldsCols.ListIndex] = Field;
  }
  if (SelectedFields.size() > 1) {
	  ButtonMoveUp.set_sensitive(true);
	  ButtonMoveDown.set_sensitive(true);
  } else {
	  ButtonMoveUp.set_sensitive(false);
	  ButtonMoveDown.set_sensitive(false);
  }
  ButtonSave.grab_focus();
}

//------------------------------------------------------------------------------
// on_button_add_all () :
//
void OutputWindow::on_button_add_all () {
  for (unsigned int i = 0; i < AvailableFields.size (); i ++) {
    SelectedFields.push_back (AvailableFields[i]);
    AvailableFields[i]->Selected = true;
  }
  AvailableFields.clear ();

  // Refresh the display of selected and available fields
  display_fields ();
}


//------------------------------------------------------------------------------
// on_button_add () :
//
void OutputWindow::on_button_add () {
  Glib::RefPtr<Gtk::TreeView::Selection> Selected = TreeAvailableFields.get_selection();
  if(Selected) {
    // Add the newly selected items to the list of "Selected Fields"
    Selected -> selected_foreach_iter(
      sigc::mem_fun(*this, &OutputWindow::row_callback_button_add));
    
    // Remove the newly selected items from the list of "Available Fields". Do
    // this in reverse order to avoid invalid vector array references.
    for (int i = AvailableFields.size () - 1; i >= 0; i --) {
      if (AvailableFields[i]->Active) {
        AvailableFields[i]->Active = false;
        AvailableFields[i]->Selected = true;
        AvailableFields.erase (AvailableFields.begin () + i);
      }
    }
    
    // Refresh the display of selected and available fields
    display_fields ();
  }
}

void OutputWindow::row_callback_button_add (const Gtk::TreeModel::iterator& iter) {
  if(iter) {
    SelectedFields.push_back (&ResultStrings [(*iter)[fieldsCols.ResultIndex]]);
    ResultStrings[(*iter)[fieldsCols.ResultIndex]].Active = true;
  }
}


//------------------------------------------------------------------------------
// on_button_remove () :
//
void OutputWindow::on_button_remove () {
  Glib::RefPtr<Gtk::TreeView::Selection> Selected = TreeSelectedFields.get_selection();
  if(Selected) {
    // Add the newly selected items to the list of "Selected Fields"
    Selected -> selected_foreach_iter(
      sigc::mem_fun(*this, &OutputWindow::row_callback_button_remove));
    
    // Remove the newly selected items from the list of "Available Fields". Do
    // this in reverse order to avoid invalid vector array references.
    for (int i = SelectedFields.size () - 1; i >= 0; i --) {
      if (SelectedFields[i]->Active) {
        SelectedFields[i]->Active = false;
        AvailableFields[i]->Selected = false;
        SelectedFields.erase (SelectedFields.begin () + i);
      }
    }
    // Refresh the display of selected and available fields
    display_fields ();
  }
}

void OutputWindow::row_callback_button_remove (const Gtk::TreeModel::iterator& iter) {
  if(iter) {
    AvailableFields.push_back (&ResultStrings [(*iter)[fieldsCols.ResultIndex]]);
    ResultStrings[(*iter)[fieldsCols.ResultIndex]].Active = true;
  }
}


//------------------------------------------------------------------------------
// on_button_remove_all () :
//
void OutputWindow::on_button_remove_all () {
  for (unsigned int i = 0; i < SelectedFields.size (); i ++) {
    AvailableFields.push_back (SelectedFields[i]);
    SelectedFields[i] -> Selected = false;
  }
  SelectedFields.clear ();

  // Refresh the display of selected and available fields
  display_fields ();
}

void OutputWindow::on_button_move_up () {
  Glib::RefPtr<Gtk::TreeView::Selection> Selected = TreeSelectedFields.get_selection();
  OutputField *TempField;
  Gtk::TreeModel::iterator iter;
  
  if(Selected) {
    // Use a callback function to set the Selected flag to true for each of the
    // SelectedFields elements that are currently selected.
    Selected -> selected_foreach_iter(
      sigc::mem_fun(*this, &OutputWindow::row_callback_button_move));
    
    // Don't move the fields up if the first field is selected. Just deselect
    // the target array elements in SelectedFields.
    if (SelectedFields[0] -> Active) {
      for (unsigned int i = 0; i < SelectedFields.size (); i ++) {
        if (SelectedFields[i] -> Active) {
          SelectedFields[i] -> Active = false;
        }
      }
    
    // The first row is not selected, so shift all the selected fields up one
    // place in the SelectedFields array.
    } else { 
      for (unsigned int i = 0; i < SelectedFields.size (); i ++) {
        if (SelectedFields[i] -> Active) {
          SelectedFields[i] -> Active = false;
          TempField = SelectedFields[i - 1];
          SelectedFields[i - 1] = SelectedFields [i];
          SelectedFields[i] = TempField;
        }
      }
      
      // Shift the selected TreeView entries up one place as well. This approach
      // is better than refreshing the list with display_fields () as it
      // preserves the selection for subsequent move operations.
      std::vector<Gtk::TreeModel::Path> PathList = Selected->get_selected_rows();
      for (unsigned int i = 0; i < PathList.size (); i ++) {
        iter = modelFieldsSelected -> get_iter(PathList[i]);
        if (modelFieldsSelected -> children().begin() != iter) {
          modelFieldsSelected -> iter_swap(iter, iter--);
        }
      }
    }
    ButtonSave.grab_focus();
  }
}

void OutputWindow::row_callback_button_move (const Gtk::TreeModel::iterator& iter) {
  if(iter) {
    ResultStrings[(*iter)[fieldsCols.ResultIndex]].Active = true;
    
  }
}


//------------------------------------------------------------------------------
// on_button_move_down () :
//
void OutputWindow::on_button_move_down () {
  Glib::RefPtr<Gtk::TreeView::Selection> Selected = TreeSelectedFields.get_selection();
  OutputField *TempField;
  Gtk::TreeModel::iterator iter;
  
  if(Selected) {
    // Use a callback function to set the Selected flag to true for each of the
    // SelectedFields elements that are currently selected.
    Selected -> selected_foreach_iter(
      sigc::mem_fun(*this, &OutputWindow::row_callback_button_move));
    
    // Don't move the fields down if the last field is selected. Just deselect
    // the target array elements in SelectedFields.
    if (SelectedFields[SelectedFields.size () - 1] -> Active) {
      for (unsigned int i = 0; i < SelectedFields.size (); i ++) {
        if (SelectedFields[i] -> Active) {
          SelectedFields[i] -> Active = false;
        }
      }
    
    // The first row is not selected, so shift all the selected fields down one
    // place in the SelectedFields array.
    } else { 
      for (int i = SelectedFields.size () - 1; i >= 0; i --) {
        if (SelectedFields[i] -> Active) {
          SelectedFields[i] -> Active = false;
          TempField = SelectedFields[i + 1];
          SelectedFields[i + 1] = SelectedFields [i];
          SelectedFields[i] = TempField;
        }
      }
      
      // Shift the selected TreeView entries down one place as well. This 
      // approach is better than refreshing the list with display_fields () as 
      // it preserves the selection for subsequent move operations.
      std::vector<Gtk::TreeModel::Path> PathList = Selected->get_selected_rows();
      for (int i = PathList.size () - 1; i >= 0; i --) {
        iter = modelFieldsSelected -> get_iter(PathList[i]);
        if (modelFieldsSelected -> children().end() != iter) {
          modelFieldsSelected -> iter_swap(iter, iter++);
        }
      }
    }
    ButtonSave.grab_focus();
  }
}


//------------------------------------------------------------------------------
// on_button_browse () :
//
void OutputWindow::on_button_browse () {
  
  Gtk::FileChooserDialog dialog("Export the current project",
    Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_transient_for(*this);

  // Add response buttons the the dialog
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  // Add filters, so that only certain file types can be selected
  Gtk::FileFilter filter_csv, filter_all, filter_tex;
  filter_csv.set_name("Text/CSV files");
  filter_csv.add_pattern("*.csv");
  dialog.add_filter(filter_csv);
  filter_tex.set_name("LaTeX files");
  filter_tex.add_pattern("*.tex");
  dialog.add_filter(filter_tex);
  filter_all.set_name("All files");
  filter_all.add_pattern("*");
  dialog.add_filter(filter_all);

  int result = dialog.run ();
  string Filename;

  // Handle the response 
  if (result == Gtk::RESPONSE_OK) {
    Filename = dialog.get_filename();
    EntryFileSelection.set_text (Filename);
    ButtonSave.grab_focus();
  }
}


//------------------------------------------------------------------------------
// on_button_delimit_other () :
//
void OutputWindow::on_button_delimit_other () {
  if (ButtonDelimitOther.get_active ()) {
    EntryDelimitOther.set_sensitive (true);
  } else {
    EntryDelimitOther.set_sensitive (false);
  }
  ButtonSave.grab_focus();
}


//------------------------------------------------------------------------------
// on_change_combo_output () :
//
void OutputWindow::on_change_combo_output () {
  if (ComboOutputType.get_active_row_number () == 0) {
    FrameListDelimiters.show ();
  } else {
    FrameListDelimiters.hide ();
  }
  ButtonSave.grab_focus();
}

//------------------------------------------------------------------------------
// on_button_save () :
//
void OutputWindow::on_button_save () {
  string Filename = EntryFileSelection.get_text ();
  int FileType = ComboOutputType.get_active_row_number ();
  try {
    if (SelectedFields.size () == 0)
      throw (Error (FLT_SYNTAX_ERROR, "Please specify some fields to output", "The list of 'Selected Fields' is currently empty"));
    if (Filename == "")
      throw (Error (FLT_SYNTAX_ERROR, "Please specify an output file name", "The 'Output file name' box is currently empty"));
    switch (FileType) {
      case 0: writeCSV (Filename); break;
      case 1: writeLaTeX (Filename); break;
      case 2: writeAASTeX (Filename); break;
      default: 
        throw (Error (FLT_SYNTAX_ERROR, 
          "Please select an output file type", 
          "Use the drop down box on the left of the window"));
    }
    hide ();
  } catch (Error &Err) {
    if (Err.code != FLT_SAVE_ABORTED) {
      Gtk::MessageDialog dialog(*this, Err.message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dialog.set_secondary_text(Err.subtext);
      dialog.run();
    }
  }
}


//------------------------------------------------------------------------------
// confirm_file_overwrite (string) : Check whether or not the file specified at
// arg1 already exists. If it does, ask the user if they want to overwrite it.
// If the file does not already exist or if the user is happy to overwrite it,
// this function will return normally. If not, an FLT_SAVE_ABORTED Error will be
// thrown.
//
void OutputWindow::confirm_file_overwrite (string Filename) throw (Error) {
  ifstream CheckFile (Filename.c_str ());
  if (CheckFile.is_open ()) {
    CheckFile.close ();
    ostringstream oss;
    size_t FilePos = Filename.find_last_of ("/\\") + 1;
    oss << Filename.substr(FilePos).c_str() << " already exists";
    Gtk::MessageDialog message(*this, oss.str(),
      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
    message.set_secondary_text("Do you want to overwrite it?");
    int result = message.run();
    if (result == Gtk::RESPONSE_NO) { throw (Error (FLT_SAVE_ABORTED)); }
  }
}


//------------------------------------------------------------------------------
// writeCSV (string, string) : 
//
void OutputWindow::writeCSV (string Filename) throw (Error) {
  ostringstream oss;
  string Delimiter;
  
  // Ensure the filename has a ".csv" or ".txt" extension then use it to open an
  // output file stream
  if (Filename.substr (Filename.size () - 4, 4) != ".csv" && 
    Filename.substr (Filename.size () - 4, 4) != ".txt") {
    Filename = Filename + ".csv";
  }
  confirm_file_overwrite (Filename);
  ofstream Output (Filename.c_str(), ios::out);
  
  if (ButtonDelimitComma.get_active ()) Delimiter = ",";
  if (ButtonDelimitSpace.get_active ()) Delimiter = " ";
  if (ButtonDelimitTab.get_active ()) Delimiter = "\t";
  if (ButtonDelimitOther.get_active ()) Delimiter = EntryDelimitOther.get_text ();
  
  // Only proceed with the output if the file has been opened correctly
  if (Output.is_open ()) {
  
    // Output the column headers
    for (unsigned int Field = 0; Field < SelectedFields.size (); Field ++) {
      oss << SelectedFields[Field] -> Name << Delimiter;
    }
    Output << oss.str().substr (0, oss.str().size() - Delimiter.size ()) << endl;
    oss.str("");

    // Construct the delimited output strings, one spectral line at a time, and
    // output them to file. Close the output file once finished.
    for (unsigned int Level = 0; Level < SelectedFields[0]->Value.size (); Level ++) {
      for (unsigned int Line = 0; Line < SelectedFields[0]->Value[Level].size (); Line ++) {
        for (unsigned int Field = 0; Field < SelectedFields.size (); Field ++) {
//          cout << Level << " " << Line << " " << Field << ": " << flush << SelectedFields[Field] -> Value[Level][Line] << endl;
          oss << SelectedFields[Field] -> Value[Level][Line] << Delimiter;
        }
        Output << oss.str().substr (0, oss.str().size() - Delimiter.size ()) << endl;
        oss.str("");
      }
      Output << endl;
    }
    Output.close ();
    
  // If an error was encountered on trying to open the output file, generate an
  // Error to inform the user of the problem.
  } else {
    oss.str ("");
    oss << "Unable to open " << Filename;
    throw (Error (FLT_FILE_OPEN_ERROR, oss.str (), "Check you have write permission for this file"));
  }
}


//------------------------------------------------------------------------------
// writeLaTeX (string) : 
//
void OutputWindow::writeLaTeX (string Filename) throw (Error) {
  ostringstream oss;
  
  // Ensure the filename has a ".tex" extension then use it to open an output
  // file stream
  if (Filename.substr (Filename.size () - 4, 4) != ".tex") {
    Filename = Filename + ".tex";
  }
  confirm_file_overwrite (Filename);
  ofstream Output (Filename.c_str(), ios::out);
  
  // Only proceed with the output if the file has been opened correctly
  if (Output.is_open ()) {
  
    // Generate the LaTeX table header
    Output << "\\begin{table}" << endl;
    Output << "\\centering" << endl;
    Output << "\\begin{tabular}{";
    for (unsigned int Field = 0; Field < SelectedFields.size (); Field ++) {
      Output << "l";
    }
    Output << "}" << endl;
    for (unsigned int Field = 0; Field < SelectedFields.size (); Field ++) {
      oss << SelectedFields[Field] -> Name << " & ";
    }
    Output << oss.str().substr (0, oss.str().size() - 2) << "\\\\" << endl;
    Output << "\\hline \\hline" << endl;
    
    // Write the FAST results to file
    Output << getLaTeXRows () << endl;

    // Finish off the table and close the output file
    Output << "\\end{tabular}" << endl;
    Output << "\\caption{Your caption goes here}" << endl;
    Output << "\\label{table:fastresults}" << endl;
    Output << "\\end{table}" << endl;
    Output.close ();
  
  // If an error was encountered on trying to open the output file, generate an
  // Error to inform the user of the problem.
  } else {
    oss.str ("");
    oss << "Unable to open " << Filename;
    throw (Error (FLT_FILE_OPEN_ERROR, oss.str (), "Check you have write permission for this file"));
  }
}


//------------------------------------------------------------------------------
// writeAASTeX (string) : 
//
void OutputWindow::writeAASTeX (string Filename) throw (Error) {
  ostringstream oss;
  
  // Ensure the filename has a ".tex" extension then use it to open an output
  // file stream
  if (Filename.substr (Filename.size () - 4, 4) != ".tex") {
    Filename = Filename + ".tex";
  }
  confirm_file_overwrite (Filename);
  ofstream Output (Filename.c_str(), ios::out);
  
  // Only proceed with the output if the file has been opened correctly
  if (Output.is_open ()) {
  
    // Generate the AASTeX deluxe table header
    Output << "\\begin{deluxetable}{";
    for (unsigned int Field = 0; Field < SelectedFields.size (); Field ++) {
      Output << "l";
    }
    Output << "}" << endl;
    Output << "\\tablewidth{0pt}" << endl;
    Output << "\\tabletypesize{\\scriptsize}" << endl;
    Output << "\\tablecaption{Your caption goes here}" << endl;
    Output << "\\tablehead{" << endl << "  ";
    for (unsigned int Field = 0; Field < SelectedFields.size (); Field ++) {
      oss << "\\colhead{" << SelectedFields[Field] -> Name << "} & ";
    }
    Output << oss.str().substr (0, oss.str().size() - 3) << endl << "}" << endl;
    
    // Output the data to file.
    Output << "\\startdata" << endl;
    Output << getLaTeXRows () << endl;
    Output << "\\enddata" << endl;

    // Finish off the table and close the output file
    Output << "\\label{table:fastresults}" << endl;
    Output << "\\end{deluxetable}" << endl;
    Output.close ();
  
  // If an error was encountered on trying to open the output file, generate an
  // Error to inform the user of the problem.
  } else {
    oss.str ("");
    oss << "Unable to open " << Filename;
    throw (Error (FLT_FILE_OPEN_ERROR, oss.str (), "Check you have write permission for this file"));
  }
}


//------------------------------------------------------------------------------
// getLaTeXRows () :Construct a stringstream containing the output data in LaTeX
// table format. Take care to use "\\" at the end of all lines rather than a
// field delimiter. Return the stringstream string for output in the calling
// function.
//
string OutputWindow::getLaTeXRows () {
  ostringstream oss;
  for (unsigned int Level = 0; Level < SelectedFields[0]->Value.size (); Level ++) {
    for (unsigned int Line = 0; Line < SelectedFields[0]->Value[Level].size (); Line ++) {
      for (unsigned int Field = 0; Field < SelectedFields.size (); Field ++) {
        oss << "$" << SelectedFields[Field] -> Value[Level][Line] << "$ & ";
      }
      oss.seekp ((long)oss.tellp () - 2);
      oss << "\\\\\n";
    }
    oss << endl;
  }
  return oss.str().substr (0, oss.str().size() - 3);
}











