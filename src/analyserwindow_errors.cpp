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
// AnalyserWindow class (analyserwindow_errors.cpp)
//==============================================================================
// This file contains AnalyserWindow error handlers.

//------------------------------------------------------------------------------
// on_data_load_line_list_error (Error) : Handles errors encountered when 
// reading lines from an XGremlin LIN file in on_data_load_line_list ()
//
void AnalyserWindow::display_error (Error Err, string StatusMsg) {
  Gtk::MessageDialog dialog(*this, Err.message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
  dialog.set_secondary_text(Err.subtext);
  dialog.run();
  Status.push (StatusMsg);
}
