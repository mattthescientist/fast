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
#include <fstream>
#include "analyserwindow.h"
#include <string>

using namespace::std;

#define NUM_CMD_LINE_ARGS 2
#define ERROR_INVALID_ARGS    1
#define NO_ERROR              0

void showHelp () {
	cout << "fast : The FTS Atomic Spectrum Tool, for analysis of atomic line spectra" << endl;
	cout << "------------------------------------------------------------------------" << endl;
	cout << "Syntax : fast [<fast file> | -h]" << endl << endl;
	cout << "<fast file>  : A previously saved FAST project to be opened." << endl;
	cout << "-h | --help  : Displays this help message." << endl << endl;;
}


//------------------------------------------------------------------------------
// main (int, char) : This is where it all starts. Output the current version
// and build date to standard output for basic diagnosis purposes. Process the
// command line arguments if any exist
//
int main(int argc, char *argv[])
{
    string CurrentDir = getenv ("PWD");
	string FileName;
	string Argument;

	// First check to see if too many arguments have been specified. If so,
	// display an error and showHelp (). Also check to see if the user
	// has explicitly requested help.
	if (argc > NUM_CMD_LINE_ARGS) {
	  cout << "Invalid command line arguments." << endl << endl;
	  showHelp ();
	  return ERROR_INVALID_ARGS;
	}
	if (argc == NUM_CMD_LINE_ARGS) {
		Argument = argv[1];
		if (Argument == "-h" || Argument == "--help") {
			showHelp ();
			return NO_ERROR;
		}
	}

	// The command line arguments are OK, so start FAST. If a file has been
	// specified at argv[1], try and open it on startup.
	cout << "The FTS Atomic Spectrum Tool (FAST) v" << FAST_VERSION << " (built " << __DATE__ << ")" << endl;
	Gtk::Main kit(argc, argv);
	AnalyserWindow win;
	if (argc == NUM_CMD_LINE_ARGS) {
		Argument = argv[1];
		cout << "Loading " << Argument << "..." << endl;

		// First try opening the file at argv[1] under the assumption that it is in
		// the current working directory or that the user has specified a RELATIVE
		// path to the file.
		FileName = CurrentDir + "/" + Argument;
		ifstream BinIn (FileName.c_str (), ios::in|ios::binary);
		if (BinIn.is_open()) {
			BinIn.close ();
			win.fileOpen (FileName);
		} else {

			// If the file could not be found in the current directory, assume the
			// user has specified an ABSOLUTE path to the file. Send argv[1]
			// straight to win.fileOpen this time so that the user is given an
			// error message in the main FAST window if the file is still absent or
			// unreadable.
			FileName = Argument;
			win.fileOpen (FileName);
		}
  }

  // Show the main FAST project window
  Gtk::Main::run(win);
  return NO_ERROR;
}
