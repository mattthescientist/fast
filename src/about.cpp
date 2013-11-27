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
// AboutWindow class (about.cpp)
//==============================================================================

#include "about.h"
#include "TypeDefs.h"
AboutWindow::AboutWindow() : ButtonOK (Gtk::Stock::OK)
{
  set_title("About FAST");
  set_border_width(5);
  set_default_size(500, 370);
  set_position(Gtk::WIN_POS_CENTER);

  add(BaseBox);

  // Add the Textbox, inside a ScrolledWindow. Only show the scrollbars
  // when they are necessary
  Scroll.add(TextLicence);
  Scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  BaseBox.add(Scroll);

  // Add buttons and connect signal
  ButtonBox.pack_start(ButtonOK, Gtk::PACK_EXPAND_PADDING);
  BaseBox.pack_start(ButtonBox, Gtk::PACK_SHRINK);
  ButtonOK.set_size_request (75);
  ButtonOK.signal_clicked().connect(sigc::mem_fun(*this,
              &AboutWindow::on_button_ok) );
  TextLicence.set_justify(Gtk::JUSTIFY_LEFT);
  TextLicence.set_use_markup (true);
  TextLicence.set_markup(licenceText ());
  show_all_children();
}

string AboutWindow::licenceText ()
{
  ostringstream oss;
  istringstream iss;
  iss.str (__DATE__);
  string Month, Day, Year;
  iss >> Month >> Day >> Year;
  
  oss << "\
<b>The FTS Atomic Spectrum Tool (FAST) v" << FAST_VERSION << " (built " << __DATE__ << ")</b>\n \
Copyright (C) 2011-" << Year << " M. P. Ruffoni\n \
\n \
For news on FAST, or to download the latest version, please visit\n \
\"http://sp.ph.ic.ac.uk/~mruffoni/fast/\"\n \
\n \
This program is free software: you can redistribute it and/or modify\n \
it under the terms of the GNU General Public License as published by\n \
the Free Software Foundation, either version 3 of the License, or\n \
(at your option) any later version.\n \
\n \
This program is distributed in the hope that it will be useful,\n \
but WITHOUT ANY WARRANTY; without even the implied warranty of\n \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n \
GNU General Public License for more details.\n \
\n \
You should have received a copy of the GNU General Public License\n \
along with this program.  If not, see \"http://www.gnu.org/licenses/\".";
  return oss.str ();
}

