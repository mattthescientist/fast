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
// Main function
//==============================================================================
// Loads the main FAST GTK+ window and waits for it to be closed.
//
#include <gtkmm/main.h>
#include <iostream>
#include "analyserwindow.h"

using namespace::std;

//------------------------------------------------------------------------------
// main (int, char)
//
int main(int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);
  AnalyserWindow win;
  cout << "The FTS Atomic Spectrum Tool (FAST) v" << FAST_VERSION << " (built " << __DATE__ << ")" << endl;
  if (argc > 1) {
    cout << "Loading " << argv[1] << "..." << endl;
    win.fileOpen (argv[1]);
  }
  Gtk::Main::run(win);
  return 0;
}
