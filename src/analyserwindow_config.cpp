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
// AnalyserWindow class (analyserwindow.cpp)
//==============================================================================
// This file contains I/O routines for the FAST configuration file, which is
// stored in the user's HOME directory. The name of the FAST config file is
// defined in TypeDefs.h

//------------------------------------------------------------------------------
// readConfigFile () : Reads the current list of recent files from the FAST
// configuration file in the user's home directory.
//
void AnalyserWindow::readConfigFile () {
  ostringstream oss;
  ifstream ConfigFile;
  string UserHome = getenv ("HOME");
  string NextLine;

  oss << UserHome << "/" << FAST_CONFIG_FILE;
  ConfigFile.open (oss.str().c_str());
  if (ConfigFile.is_open ()) {
	getline (ConfigFile, NextLine);
	do {
	  RecentFiles.push_back (NextLine);
	  getline (ConfigFile, NextLine);
	} while (!ConfigFile.eof () && NextLine != "\0");
	ConfigFile.close ();
  }
}


//------------------------------------------------------------------------------
// writeConfigFile () : Writes the current list of recent files to the FAST
// configuration file in the user's home directory.
//
void AnalyserWindow::writeConfigFile () {
	ostringstream oss;
	ofstream ConfigFile;
	string UserHome = getenv ("HOME");
	string NextLine;

	// Only update the config file if the current file is different to the one
	// at the top of the list of recently loaded files.
	if (RecentFiles.size() == 0 || CurrentFilename != RecentFiles [0]) {

		// Insert the current file at the start of recent files
		RecentFiles.insert (RecentFiles.begin (), CurrentFilename);

		// Remove the current file from anywhere else in the list
		for (unsigned int i = 1; i < RecentFiles.size (); i ++) {
			if (RecentFiles [i] == CurrentFilename) {
				RecentFiles.erase (RecentFiles.begin () + i);
			}
		}

		// Remove the final recent file if there are now too many stored.
		if (RecentFiles.size () > NUM_RECENT_FILES) {
			RecentFiles.pop_back ();
		}

		// Write the FAST config file in the user's home directory and refresh
    	// the list of recent files in the AnalyserWindow 'File' menu.
		oss << UserHome << "/" << FAST_CONFIG_FILE;
		ConfigFile.open (oss.str().c_str());
		if (ConfigFile.is_open ()) {
			for (unsigned int i = 0; i < RecentFiles.size (); i ++) {
				ConfigFile << RecentFiles [i] << endl;
			}
			ConfigFile.close ();
			BaseBox.remove (*pMenubar);
			buildMenubarAndToolbar (false);
		}
	}

}


//------------------------------------------------------------------------------
// removeFromConfigFile () : Removes the file specified at arg1 from the list of
// recent files stored in the FAST config file.
//
void AnalyserWindow::removeFromConfigFile (string Filename) {
	ostringstream oss;
	ofstream ConfigFile;
	string UserHome = getenv ("HOME");
	string NextLine;

	// Search the list of recent files and remove the one specified by Filename.
	for (unsigned int i = 0; i < RecentFiles.size (); i ++) {
		if (RecentFiles [i] == Filename) {
			RecentFiles.erase (RecentFiles.begin () + i);
		}
	}

	// Write the FAST config file in the user's home directory and refresh
	// the list of recent files in the AnalyserWindow 'File' menu.
	oss << UserHome << "/" << FAST_CONFIG_FILE;
	ConfigFile.open (oss.str().c_str());
	if (ConfigFile.is_open ()) {
		for (unsigned int i = 0; i < RecentFiles.size (); i ++) {
			ConfigFile << RecentFiles [i] << endl;
		}
		ConfigFile.close ();
		BaseBox.remove (*pMenubar);
		buildMenubarAndToolbar (false);
	}
}







