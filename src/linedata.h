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
// Linedata class (linedata.h)
//==============================================================================
// The Linedata class inherits the functionality of an XGremlin line (XgLine)
// and that of a Gtk::Eventbox, and combines the two to permit an XGremlin line
// to be plotted in a GTK widget. Two Graph objects are added to LineData: one
// to display the line profile, and a second to display XGremlin fit residuals.
//
#ifndef ANALYSER_GRAPH_H
#define ANALYSER_GRAPH_H

#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gdk/gdk.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <sigc++/sigc++.h>

#include <string>
#include <iomanip>
#include "xgline.h"
#include "graph.h"

typedef struct graph_limits {
  Coord min, max;
  
  graph_limits () {
    min.x = 0.0; max.x = 0.0; min.y = 0.0; max.y = 0.0;
  }
} GraphLimits;

class LineData : public XgLine, public Gtk::EventBox {

  public:
    LineData ();
    LineData (XgLine LineIn);
    ~LineData ();

    // Overload XgLine SET functions to allow GTK widgets to be refreshed
    virtual void line (int a) { Ready = false; XgLine::line (a); }
    virtual void itn (int a) { Ready = false; XgLine::itn (a); }
    virtual void h (int a) { Ready = false; XgLine::h (a); }
    virtual void wavenumber (double a) { Ready = false; XgLine::wavenumber (a); }
    virtual void peak (double a) { Ready = false; XgLine::peak (a); }
    virtual void width (double a) { Ready = false; XgLine::width (a); }
    virtual void dmp (double a) { Ready = false; XgLine::dmp (a); }
    virtual void eqwidth (double a) { Ready = false; XgLine::eqwidth (a); }
    virtual void epstot (double a) { Ready = false; XgLine::epstot (a); }
    virtual void epsevn (double a) { Ready = false; XgLine::epsevn (a); }
    virtual void epsodd (double a) { Ready = false; XgLine::epsodd (a); }
    virtual void epsran (double a) { Ready = false; XgLine::epsran (a); }
    virtual void wavelength (double a) { Ready = false; XgLine::wavelength (a); }
    virtual void tags (string a) { Ready = false; XgLine::tags (a); }
    virtual void id (string a) { Ready = false; XgLine::id (a); }
    virtual void wavCorr (double a) { Ready = false; XgLine::wavCorr (a); }
    virtual void airCorrection (double a) { Ready = false; XgLine::airCorrection (a); }
    virtual void intensityCalibration (double a) { Ready = false; XgLine::intensityCalibration (a); }
    virtual void createLine (string a) throw (const char*) { Ready = false; XgLine::createLine (a); }
    virtual void operator= (XgLine a) { Ready = false; XgLine::operator= (a); }

    void setLine (XgLine a) { Ready = false; XgLine::operator= (a); }
    void addPlot (XgLine l, vector<Coord> a, bool mi = true, bool ma = true) { Plot -> addPlot (l, a, mi, ma); }
    void addPlot (vector<Coord> a, bool mi = true, bool ma = true) { Plot -> addPlot (a, mi, ma); }
    void addText (double x, double y, string textIn) { Plot -> addText (x, y, textIn); }
    void clearText () { Plot -> clearText (); }
    void setPlotColour (int i, float r, float g, float b) { Plot -> setColour (i, r, g, b); }
    void setPlotWidth (int i, float w) { Plot -> setWidth (i, w); }
    void addResidual (vector<Coord> a) { Residual -> addPlot (a); }
    
    GraphLimits plotLimits ();
    GraphLimits resLimits (); 
    void plotLimits (GraphLimits NewLimits);
    void resLimits (GraphLimits NewLimits); 
    bool selected () { return Selected; }
    void selected (bool a) { Selected = a; Plot->select(a); Residual->select(a);}
    void disabled (bool a) { Selected = false; Disabled=a; Plot->disable(a); Residual->disable(a);}
    bool disabled () { return Disabled; }
    bool autoLimits () { return Plot -> autoLimits (); }
    void setAutoLimits () { Plot -> setAutoLimits (); }
    vector <Coord> getPlotData (int i) { return Plot -> getPlotData (i); }
    vector <Coord> getResidualData (int i) { return Residual -> getPlotData (i); }
    bool showParams () { return ShowData; }
    void showParams (bool Show) { ShowData = Show; Ready = false; }
    
    typedef sigc::signal<void, bool> type_signal_selected;
    type_signal_selected signal_selected() { return m_signal_selected; }
       
  protected:
    virtual bool on_expose_event(GdkEventExpose* event);
    virtual bool on_button_press_event(GdkEventButton* event);

    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
      public:
        Gtk::TreeModelColumn<string> data;
        ModelColumns() { add(data); }
    };

    ModelColumns m_Columns;

    type_signal_selected m_signal_selected;
      
  private:
    bool Ready, ShowData;
    Graph *Plot;
    Graph *Residual;
    Gtk::VBox Box;
    Gtk::HBox ParamsBox;
    Gtk::TreeView LineParams;
    Glib::RefPtr<Gtk::TreeStore> m_refTreeModel;

    Gtk::Frame DataFrame;
    
    bool Selected;
    bool Disabled;
    
    void prepareData ();
    void doConstructor ();

};
    
#endif // ANALYSER_GRAPH_H
