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
// Linedata class (linedata.cpp)
//==============================================================================

#include "linedata.h"
#include <sstream>

LineData::LineData () {
  doConstructor ();
}

LineData::LineData (XgLine LineIn) {
  doConstructor ();
  setLine (LineIn);
  prepareData();
}

LineData::~LineData () {
  delete Plot;
  delete Residual;
}

//------------------------------------------------------------------------------
// doConstructor () : Handles any LineData object preparation that is common
// to all class constructors.
//
void LineData::doConstructor () {
  Selected = false;
  Disabled = false;
  Hidden = false;
  Ready = false;
  ShowData = false;
  Plot = new Graph;
  Residual = new Graph;
  Plot -> set_size_request (200 * ZOOM_FACTOR, 200 * ZOOM_FACTOR);
  Residual -> set_size_request (200 * ZOOM_FACTOR, 100 * ZOOM_FACTOR);
  LineParams.set_size_request ((200 * ZOOM_FACTOR) - TOTAL_X_PAD, 200 * ZOOM_FACTOR);

  Box.set_homogeneous (false);
  Box.pack_start ((Gtk::Widget &) *Plot, false, false, 0);
  Box.pack_start ((Gtk::Widget &) *Residual, false, false, 0);
  Box.pack_start (ParamsBox, false, false, 0);
  ParamsBox.pack_end (DataFrame, false, false, 0);
  DataFrame.add (LineParams);
  
  //Create the Tree model
  m_refTreeModel = Gtk::TreeStore::create(m_Columns);
  LineParams.set_model(m_refTreeModel);
  
  //Fill the TreeView's model
  Gtk::TreeModel::Row row = *(m_refTreeModel->append());
  row[m_Columns.data] = "cm^{-1}";
  row = *(m_refTreeModel->append());
  row[m_Columns.data] = "";
  row = *(m_refTreeModel->append());
  row[m_Columns.data] = " mK";
  row = *(m_refTreeModel->append());
  row[m_Columns.data] = " mK";

  LineParams.append_column("Properties", m_Columns.data);
  LineParams.set_headers_visible (false);

  add (Box);
  show_all_children();
  signal_button_press_event().connect(sigc::mem_fun(*this,
              &LineData::on_button_press_event));
  
  // Construct the popup menus for a right click on a line profile
  {
	Gtk::Menu::MenuList& menulist = menuPlotDisablePopup.items();
	menulist.push_back(Gtk::Menu_Helpers::MenuElem("Disable Line",
	sigc::mem_fun(*this, &LineData::on_popup_disable_line)));
	menulist.push_back(Gtk::Menu_Helpers::MenuElem("Hide Line",
	sigc::mem_fun(*this, &LineData::on_popup_hide_line)));
  }
  {
	Gtk::Menu::MenuList& menulist = menuPlotEnablePopup.items();
	menulist.push_back(Gtk::Menu_Helpers::MenuElem("Enable Line",
	sigc::mem_fun(*this, &LineData::on_popup_enable_line)));
	menulist.push_back(Gtk::Menu_Helpers::MenuElem("Delete Line",
	sigc::mem_fun(*this, &LineData::on_popup_hide_line)));
  }

}

bool LineData::on_expose_event(GdkEventExpose* event) { 
  if (!Ready) prepareData (); 
  Plot -> select (Selected);
  Residual -> select (Selected);
  Plot -> disable (Disabled);
  Residual -> disable (Disabled);

  Gtk::EventBox::on_expose_event(event);
  return true;
}

void LineData::prepareData () {
  if (ShowData) {
    ostringstream oss;
    m_refTreeModel->clear ();
    Gtk::TreeModel::Row row = *(m_refTreeModel->append());
    oss << setprecision(6) << fixed << XgLine::wavenumber () << " K";
    row[m_Columns.data] = oss.str ();
    row = *(m_refTreeModel->append());
    oss.str (""); oss << setprecision (2) << XgLine::peak ();
    row[m_Columns.data] = oss.str ();
    row = *(m_refTreeModel->append());
    oss.str (""); oss << setprecision (2) << XgLine::width () << " mK";
    row[m_Columns.data] = oss.str ();
    row = *(m_refTreeModel->append());
    oss.str (""); oss << setprecision (3) << XgLine::eqwidth () << " mK";
    row[m_Columns.data] = oss.str ();
    row = *(m_refTreeModel->append());
    oss.str (""); oss << scientific << setprecision (4) << XgLine::epstot ();
    row[m_Columns.data] = oss.str ();
    row = *(m_refTreeModel->append());
    oss.str (""); oss << setprecision (4) << XgLine::epsevn ();
    row[m_Columns.data] = oss.str ();
    row = *(m_refTreeModel->append());
    oss.str (""); oss << setprecision (4) << XgLine::epsodd ();
    row[m_Columns.data] = oss.str ();
    row = *(m_refTreeModel->append());
    oss.str (""); oss << setprecision (4) << XgLine::epsran ();
    row[m_Columns.data] = oss.str ();
    ParamsBox.show ();
  } else {
    ParamsBox.hide ();
  }
  queue_draw ();
  Ready = true;
}

bool LineData::on_button_press_event(GdkEventButton* event) {
	if (!Hidden && Plot -> numPlots() > 0) {
		if ((event->type == GDK_BUTTON_PRESS)) {
			if (event->button == 1 && !Disabled) {
				Selected = !Selected;
				Plot -> select (Selected);
				Residual -> select (Selected);
				m_signal_selected.emit(Selected);
			}
			else if (event->button == 3) {
				if (Disabled) {
					menuPlotEnablePopup.popup(event->button, event->time);
				} else {
					menuPlotDisablePopup.popup(event->button, event->time);
				}
			}
		}
	}
	return true;
}

GraphLimits LineData::plotLimits () {
  GraphLimits Limits;
  Limits.min = Plot -> min ();
  Limits.max = Plot -> max ();
  return Limits;
}


GraphLimits LineData::resLimits () {
  GraphLimits Limits;
  Limits.min = Residual -> min ();
  Limits.max = Residual -> max ();
  return Limits;
}

void LineData::plotLimits (GraphLimits lim) {
  Plot -> min (lim.min);
  Plot -> max (lim.max);
}


void LineData::resLimits (GraphLimits lim) {
  Residual -> min (lim.min);
  Residual -> max (lim.max);
}
    
    
//------------------------------------------------------------------------------
// on_popup_enable_line () : Called when the user right clicks on a DISABLED
// line profile in the line plot area and chooses to reactive the line.
//
void LineData::on_popup_enable_line ()
{
	Disabled = false;
	Plot -> disable (false);
	Residual -> disable (false);
	m_signal_disabled.emit(false);
}


//------------------------------------------------------------------------------
// on_popup_disable_line () : Called when the user right clicks on an ENABLED
// line profile in the line plot area and chooses to deactivate the line.
//
void LineData::on_popup_disable_line ()
{
	Disabled = true;
	Plot -> disable (true);
	Residual -> disable (true);
	Selected = false;
	Plot -> select (false);
	Residual -> select (false);
	m_signal_disabled.emit(true);
}

//------------------------------------------------------------------------------
// on_popup_delete_line () : Called when the user right clicks on any line
// profile in the line plot area and chooses to delete the line profile.
//
void LineData::on_popup_hide_line ()
{
	Gtk::MessageDialog confirm("Are you sure you wish to hide this line?",
	  false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
	confirm.add_button ("Yes", Gtk::RESPONSE_YES);
	confirm.add_button ("No", Gtk::RESPONSE_NO);
	confirm.set_default_response(Gtk::RESPONSE_NO);
	int Result = confirm.run ();
	if (Result == Gtk::RESPONSE_YES) {
		Hidden = true;
		Plot -> hide (true);
		Residual -> hide (true);
		m_signal_hidden.emit();
	}
}
