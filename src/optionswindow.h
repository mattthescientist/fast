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
// OptionsWindow class (optionswindow.h)
//==============================================================================
// Displays the FAST options window.
//
#ifndef LINE_ANALYSER_OPTIONS_WINDOW
#define LINE_ANALYSER_OPTIONS_WINDOW

// Include the GTK+ environment from GTKmm header files
#include <gtkmm/window.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/stock.h>
#include <gtkmm/frame.h>
#include <string>

using namespace::std;

class OptionsWindow : public Gtk::Window {
  private:
    bool CorrectSignalToNoise;

    // GTKmm widgets
    Gtk::ScrolledWindow Scroll;
    Gtk::VBox BaseVBox;
    Gtk::HBox BaseHBox;
    
    // Widgets for listing the data selection options
    Gtk::VBox BoxOptions;
    Gtk::Frame FrameCorrectSNR;
    Gtk::VBox BoxCorrectSNR;
    Gtk::RadioButton ButtonDoNotCorrectSNR;
    Gtk::RadioButton ButtonCorrectSNR;

    Gtk::HBox BoxOKCancel;
    Gtk::Button ButtonOK;
    Gtk::Button ButtonCancel;
    
    void on_button_ok ();
    void on_button_cancel ();
    
  public:

    OptionsWindow ();
    ~OptionsWindow () { /* Does nothing */ }

    bool correct_snr () { return CorrectSignalToNoise; }
};

#endif // LINE_ANALYSER_OPTIONS_WINDOW
