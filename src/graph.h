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
// Graph class (graph.h)
//==============================================================================
// GtkGraph inherits Gtk::DrawingArea and uses its functionality to display
// simple 2-D line plots. 

#ifndef GRAPH_H
#define GRAPH_H

#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/drawingarea.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "xgline.h"

using namespace::std;

// Colour definitions for different graph states
#define DEF_PLOT_COLOUR  0.0, 0.0, 0.0  /* Black       */
#define COLOUR_SELECTED  0.8, 0.8, 1.0  /* Light blue  */
#define COLOUR_BACK      1.0, 1.0, 1.0  /* White       */
#define COLOUR_DISABLED  0.6, 0.6, 0.6  /* Medium grey */
#define COLOUR_DIS_SEL   0.8, 0.8, 0.8  /* Light grey  */

// Graph dimension definitions
#define ZOOM_FACTOR      0.8
#define GRAPH_FRACTION   0.85 /* Fraction of DrawingArea height used by graph */
#define TIC_SPACING      0.25
#define LABEL_SPACING    0.5
#define DEF_PLOT_WIDTH   1
#define TIC_LENGTH       (4 * ZOOM_FACTOR)  /* pixels */
#define BORDER_WIDTH     (2 * ZOOM_FACTOR)  /* pixels */
#define X_AXIS_HEIGHT    (20 * ZOOM_FACTOR) /* pixels */
#define Y_AXIS_WIDTH     (40 * ZOOM_FACTOR) /* pixels */
#define GRAPH_PAD_RIGHT  (5 * ZOOM_FACTOR)  /* pixels */
#define GRAPH_PAD_TOP    (10 * ZOOM_FACTOR) /* pixels */
#define GRAPH_PAD_LEFT   (GRAPH_PAD_RIGHT + Y_AXIS_WIDTH)
#define GRAPH_PAD_BOTTOM (GRAPH_PAD_TOP + X_AXIS_HEIGHT)
#define TOTAL_X_PAD      (GRAPH_PAD_LEFT + GRAPH_PAD_RIGHT)
#define TOTAL_Y_PAD      (GRAPH_PAD_TOP + GRAPH_PAD_BOTTOM)
#define MIN_Y_TIC_SPACING 15 /* pixels */
#define MAX_Y_TIC_SPACING 40 /* pixels */
#define MIN_X_TIC_SPACING 30 /* pixels */
#define MAX_X_TIC_SPACING 50 /* pixels */
#define Y_GRAPH_ZOOM     0.94
#define FONT_SIZE        (int)(12 * ZOOM_FACTOR)

#define PLOT_WIDTH_RANGE 0.004
#define XGREMLIN_COMMENT '!'

#define ERR_GRAPH_INDEX_TOO_LOW  -1

//------------------------------------------------------------------------------
// Type definitions
//
typedef struct td_coord {
  double x;
  double y;
  
  td_coord () { x = 0.0; y = 0.0; }
  td_coord (double nx, double ny) { x = nx; y = ny; }
  
} Coord;

typedef struct graph_colour {
  float r, g, b;
  
  graph_colour () { r = 0.0; g = 0.0; b = 0.0; }
  graph_colour (float nr, float ng, float nb) { r = nr; g = ng; b = nb; }
} GraphColour;

//------------------------------------------------------------------------------
// Class definition
//
class Graph : public Gtk::DrawingArea {

private:
  vector < vector <Coord> > Plots;
  vector <Coord> Minima, Maxima;
  vector <int> LineWidths;
  vector <GraphColour> LineColours;
  Coord GraphMin, GraphMax;
  string XLabel, YLabel;
  bool Selected;    // true if the graph is currently selected
  bool Disabled;    // true if the graph is currently disabled
  bool AutoLimits;  // true if limits were last set by setAutoLimits()
  
  void drawXTicMarks (Cairo::RefPtr<Cairo::Context> cr, const int height, const int width);
  void drawYTicMarks (Cairo::RefPtr<Cairo::Context> cr, const int height, const int width);

protected:
  
  virtual bool on_expose_event(GdkEventExpose* event);

public:
  Graph ();
  Graph (XgLine LineIn, vector <Coord> Points);
  virtual ~Graph ();

  void addPlot (XgLine LineIn, vector <Coord> AscLines, 
    bool IncludeInMinima = true, bool IncludeInMaxima = true);
  void addPlot (vector <Coord> NewPlot, bool IncludeInMinima = true, 
    bool IncludeInMaxima = true);
  void max (Coord NewMax) { GraphMax = NewMax; AutoLimits = false; }
  void min (Coord NewMin) { GraphMin = NewMin; AutoLimits = false; }
  Coord max () { return GraphMax; }
  Coord min () { return GraphMin; }
  void select (bool a) { Selected = a; queue_draw (); }
  void disable (bool a) { Selected = false; Disabled = a; queue_draw (); }
  void setWidth (int Index, float nWidth);
  void setColour (int Index, float nr, float ng, float nb);
  void setAutoLimits ();
  bool autoLimits () { return AutoLimits; }
  vector <Coord> getPlotData (int i) throw (int);
};

#endif // GRAPH_H
