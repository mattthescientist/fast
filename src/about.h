// The FTS Atomic Spectrum Tool (FAST)
// Copyright (C) 2011-2012 M. P. Ruffoni
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
// AboutWindow class (about.h)
//==============================================================================
// Displays a small window containing information about FAST, including the
// GPL licence extract shown above.
//
#ifndef LINE_ANALYSER_ABOUT_WINDOW
#define LINE_ANALYSER_ABOUT_WINDOW

// Include the GTK+ environment from GTKmm header files
#include <gtkmm/window.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/stock.h>
#include <gtkmm/frame.h>
#include <string>

using namespace::std;

class AboutWindow : public Gtk::Window {

  private:

    // GTKmm VBox to hold all the widgets in the AnalyserWindow
    Gtk::VBox BaseBox;
    Gtk::HBox ButtonBox;
    Gtk::ScrolledWindow Scroll;
    Gtk::Label TextLicence;
    Gtk::Button ButtonOK;
    
    void on_button_ok () { hide (); }
    string licenceText();

  public:

    AboutWindow ();
    ~AboutWindow () { /* Does nothing */ }
};

#endif // LINE_ANALYSER_ABOUT_WINDOW
