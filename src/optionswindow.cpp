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
// OptionsWindow class (optionswindow.cpp)
//==============================================================================
// Displays the FAST options window.
//
#include "optionswindow.h"

//------------------------------------------------------------------------------
//
//
OptionsWindow::OptionsWindow () :
  ButtonOK (Gtk::Stock::OK), ButtonCancel (Gtk::Stock::CANCEL) {
   
  // Set the basic window properties
  set_title("Options");
  set_default_size(370, 120);
  set_position(Gtk::WIN_POS_CENTER);
  Scroll.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  add (Scroll);
  Scroll.add (BaseVBox);
  BaseVBox.pack_start (BaseHBox, true, true, 0);
  BaseHBox.pack_start (BoxOptions, false, false, 0);

  // Add the option to correct line S/N ratios
  BoxOptions.pack_start (FrameCorrectSNR, false, false, 0);
  FrameCorrectSNR.set_label ("Correct line S/N Ratio?");
  FrameCorrectSNR.add (BoxCorrectSNR);
  BoxCorrectSNR.pack_start (ButtonDoNotCorrectSNR);
  BoxCorrectSNR.pack_start (ButtonCorrectSNR);
  ButtonDoNotCorrectSNR.set_label ("Noise level is set to unity. Do not adjust S/N ratios.");
  ButtonCorrectSNR.set_label      ("Noise level is not unity. Calculate S/N ratio in FAST.");
  Gtk::RadioButton::Group group = ButtonCorrectSNR.get_group();
  ButtonDoNotCorrectSNR.set_group (group);

  // Add the OK and Cancel buttons to the bottom of the window
  BaseVBox.pack_start (BoxOKCancel, false, false, 10);
  BoxOKCancel.pack_end (ButtonOK, false, false, 2);
  BoxOKCancel.pack_end (ButtonCancel, false, false, 2);
  ButtonOK.set_size_request (75);
  ButtonCancel.set_size_request (75);
  ButtonOK.signal_clicked().connect (sigc::mem_fun (*this,
  &OptionsWindow::on_button_ok));
  ButtonCancel.signal_clicked().connect (sigc::mem_fun (*this,
  &OptionsWindow::on_button_cancel));
  
  show_all_children();
  
  CorrectSignalToNoise = false;
  ButtonDoNotCorrectSNR.set_active (true);
}


//------------------------------------------------------------------------------
// on_button_save () :
//
void OptionsWindow::on_button_ok () {
  if (ButtonCorrectSNR.get_active ()) CorrectSignalToNoise = true;
  if (ButtonDoNotCorrectSNR.get_active ()) CorrectSignalToNoise = false;
  hide ();  
}


//------------------------------------------------------------------------------
// on_button_cancel () :
//
void OptionsWindow::on_button_cancel () {
  if (CorrectSignalToNoise == true) {
    ButtonCorrectSNR.set_active (true);
    ButtonDoNotCorrectSNR.set_active (false);
  } else {
    ButtonCorrectSNR.set_active (false);
    ButtonDoNotCorrectSNR.set_active (true);
  }
  hide ();  
}











