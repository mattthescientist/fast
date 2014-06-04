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
// Graph class (graph.cpp)
//==============================================================================

#include "graph.h"
#include <sstream>
#include <cmath>

//==============================================================================
// CONSTRUCTORS / DESTRUCTORS
//==============================================================================

Graph::Graph () {
  GraphMin.x = 0.0; GraphMin.y = 0.0;
  GraphMax.x = 0.0; GraphMax.y = 0.0;
  Selected = false;
  Disabled = false;
  Calibration = false;
  AutoLimits = false;
}


Graph::Graph (XgLine LineIn, vector <Coord> Points) {
  GraphMin.x = 0.0; GraphMin.y = 0.0;
  GraphMax.x = 0.0; GraphMax.y = 0.0;
  Selected = false;
  Disabled = false;
  Calibration = false;
  AutoLimits = false;
  addPlot (LineIn, Points);
}


Graph::~Graph () {

}


//==============================================================================
// SIGNAL HANDLERS
//==============================================================================

//------------------------------------------------------------------------------
// on_expose_event (GdkEventExpose*) : Draws the graph into the widget area each
// time an expose event is generated.
//
bool Graph::on_expose_event(GdkEventExpose* event)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window && Plots.size () > 0) {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
    cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
    cr->clip ();
    
    // Draw the graph border
    cr->set_line_width(BORDER_WIDTH);
    cr->save ();
    cr->rectangle (GRAPH_PAD_LEFT, GRAPH_PAD_TOP, 
      width - TOTAL_X_PAD, height - TOTAL_Y_PAD);
    if (Selected) {
      if (Disabled) cr -> set_source_rgb(COLOUR_DIS_SEL);
      if (Calibration) cr -> set_source_rgb(COLOUR_SEL_CAL);
      else cr -> set_source_rgb(COLOUR_SELECTED);
    } else {
      if (Disabled) cr -> set_source_rgb(COLOUR_DISABLED);
      if (Calibration) cr -> set_source_rgb(COLOUR_SELECTED);
      else cr -> set_source_rgb(COLOUR_BACK);
    }
    cr->fill_preserve ();
    cr->restore ();
    cr->stroke ();
    
    // Draw the tic marks on the graph
    drawXTicMarks (cr, height, width);
    drawYTicMarks (cr, height, width);
    
    // Plot the data
    for (unsigned int i = 0; i < Plots.size (); i ++) {
      cr->set_line_width(LineWidths[i]);
      cr->set_source_rgb(LineColours[i].r, LineColours[i].g, LineColours[i].b);
      cr->move_to ((Plots[i][0].x - GraphMin.x) / (GraphMax.x - GraphMin.x) * (width - TOTAL_X_PAD) + GRAPH_PAD_LEFT, 
        height - ((Plots[i][0].y - GraphMin.y) / (GraphMax.y - GraphMin.y) * (height - TOTAL_Y_PAD) + GRAPH_PAD_BOTTOM));
      
      for (unsigned int j = 0; j < Plots[i].size (); j ++) {
        cr->line_to ((Plots[i][j].x - GraphMin.x) / (GraphMax.x - GraphMin.x) * (width - TOTAL_X_PAD) + GRAPH_PAD_LEFT, 
          height - ((Plots[i][j].y - GraphMin.y) / (GraphMax.y - GraphMin.y) * (height - TOTAL_Y_PAD) + GRAPH_PAD_BOTTOM));
      }
      cr->stroke ();
    }
    drawText (cr, height, width);
  }
  
  return true;
}


//------------------------------------------------------------------------------
// drawXTicMarks (Cairo::RefPtr<Cairo::Context>, const int, const int) :
// Calculates the optimal location of the X tic marks given the range of data to
// be plotted on the X-axis. Tic marks are then drawn at these locations and 
// labels added to each one.
//
void Graph::drawXTicMarks (Cairo::RefPtr<Cairo::Context> cr, 
  const int height, const int width) {
  
  vector <double> XTics;
  vector <string> Labels;
  int Order;
  double PlotXRange, XGraphPos, XGraphOffset, XPixelOffset;
  ostringstream oss;
  
  PlotXRange = GraphMax.x - GraphMin.x;
  if (log10 (PlotXRange) >= 0.0) {
    Order = int(log10 (PlotXRange));      // Round DOWN to the nearest int
  } else {
    Order = int(log10 (PlotXRange) - 1);  // Round UP to the nearest int
  }

  XGraphPos = int (Plots[0][Plots[0].size () * 0.5].x / pow (10.0, Order) + 0.5) * pow (10.0, Order);
  XGraphOffset = XGraphPos - GraphMin.x;
  XPixelOffset = XGraphOffset / (GraphMax.x - GraphMin.x) * (width - TOTAL_X_PAD);
  if (XPixelOffset < width - GRAPH_PAD_RIGHT) {
    XTics.push_back (XPixelOffset);
    oss.str (""); oss << XGraphPos;
    Labels.push_back (oss.str());
  }  
  
  XGraphPos = int (Plots[0][Plots[0].size () * 0.2].x / pow (10.0, Order) + 0.5) * pow (10.0, Order);
  XGraphOffset = XGraphPos - GraphMin.x;
  XPixelOffset = XGraphOffset / (GraphMax.x - GraphMin.x) * (width - TOTAL_X_PAD);
    XTics.push_back (XPixelOffset);
    oss.str (""); oss << XGraphPos;
    Labels.push_back (oss.str());
 
  XGraphPos = int (Plots[0][Plots[0].size () * 0.8].x / pow (10.0, Order) + 0.5) * pow (10.0, Order);
  XGraphOffset = XGraphPos - GraphMin.x;
  XPixelOffset = XGraphOffset / (GraphMax.x - GraphMin.x) * (width - TOTAL_X_PAD);
  if (XPixelOffset < width - TOTAL_X_PAD) {
    XTics.push_back (XPixelOffset);
    oss.str (""); oss << XGraphPos;
    Labels.push_back (oss.str());
  }

  // Draw the tic marks on the x-axis
  for (unsigned int i = 0; i < XTics.size (); i ++) {
    cr -> move_to (GRAPH_PAD_LEFT + XTics[i], height - GRAPH_PAD_BOTTOM - TIC_LENGTH);
    cr -> line_to (GRAPH_PAD_LEFT + XTics[i], height - GRAPH_PAD_BOTTOM);
  }
  cr -> stroke ();

  // Add the axis labels
  Glib::RefPtr<Pango::Context> lc = create_pango_context();
  Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create (lc);
  oss.str (""); oss << "normal " << FONT_SIZE;
  layout -> set_font_description(Pango::FontDescription (oss.str().c_str()));
  int TxtWidth, TxtHeight;

  for (unsigned int i = 0; i < Labels.size (); i ++) {
    layout -> set_text (Labels [i]);
    layout -> get_pixel_size (TxtWidth, TxtHeight);
    get_window () ->draw_layout(get_style()->get_text_gc(Gtk::STATE_NORMAL), 
      GRAPH_PAD_LEFT + XTics[i] - TxtWidth / 2.0, 
      height - GRAPH_PAD_BOTTOM, layout);
  }
}


//------------------------------------------------------------------------------
// drawYTicMarks (Cairo::RefPtr<Cairo::Context>, const int, const int) :
// Calculates the optimal location of the Y tic marks given the range of data to
// be plotted on the Y-axis. Tic marks are then drawn at these locations (with a
// solid line marking y = 0) and labels added to each one.
//
void Graph::drawYTicMarks (Cairo::RefPtr<Cairo::Context> cr, 
  const int height, const int width) {
  vector <double> YTics;
  
  // Set a nominal tic spacing based on the order of magnitude of PlotYRange
  double PlotYRange = GraphMax.y - GraphMin.y;
  double YTicSpacing = pow (10.0, int(log10 (PlotYRange)));
  
  // Now adjust the tic spacing by factors of 2 to ensure tic marks are neither
  // too closely nor too sparsely packed
  while (YTicSpacing / PlotYRange * (height - TOTAL_Y_PAD) < MIN_Y_TIC_SPACING) {
    YTicSpacing *= 2.0;
  }
  while (YTicSpacing / PlotYRange * (height - TOTAL_Y_PAD) > MAX_Y_TIC_SPACING) {
    YTicSpacing /= 2.0;
  }
  
  // Now the spacing has been established, create the first mark at y = 0
  double YZeroPosition = (0.0 - GraphMin.y) / PlotYRange;
  YTics.push_back (height - GRAPH_PAD_BOTTOM - (height - TOTAL_Y_PAD) * YZeroPosition);
  
  // Add as many positive y tic marks as will fit on the graph with the
  // current YTicSpacing
  unsigned int i = 1;
  while (YTics[i - 1] > GRAPH_PAD_TOP + YTicSpacing / PlotYRange * (height - TOTAL_Y_PAD)) {
    YTics.push_back (YTics[i - 1] - YTicSpacing / PlotYRange * (height - TOTAL_Y_PAD));
    i ++;
  }
  
  // Finally insert negative y tic marks at the start of the YTics array. This
  // will shift the y=0 mark away from the 0th array index, so keep track of it
  // with the YTicsZeroPos counter
  int YTicsZeroPos = 0;
  while (YTics[0] < height - GRAPH_PAD_BOTTOM - YTicSpacing / PlotYRange * (height - TOTAL_Y_PAD)) {
    YTics.insert (YTics.begin(), YTics[0] + YTicSpacing / PlotYRange * (height - TOTAL_Y_PAD));
    YTicsZeroPos ++;
  }

  // Draw the zero line on the y-axis
  cr -> set_source_rgba (0.4, 0.4, 1.0, 0.6);
  cr -> move_to (GRAPH_PAD_LEFT + TIC_LENGTH, YTics[YTicsZeroPos]);
  cr -> line_to (width - GRAPH_PAD_RIGHT - TIC_LENGTH, YTics[YTicsZeroPos]);
  cr -> stroke ();
  
  // Draw the tick marks on the y-axis
  cr -> set_source_rgba (0.0, 0.0, 0.0, 1.0);
  for (unsigned int i = 0; i < YTics.size (); i ++) {
    cr -> move_to (GRAPH_PAD_LEFT + TIC_LENGTH, YTics[i]);
    cr -> line_to (GRAPH_PAD_LEFT, YTics[i]);
    cr -> move_to (width - GRAPH_PAD_RIGHT - TIC_LENGTH, YTics[i]);
    cr -> line_to (width - GRAPH_PAD_RIGHT, YTics[i]);
  }
  cr -> stroke ();
  
  // Add the axis labels
  Glib::RefPtr<Pango::Context> lc = create_pango_context();
  Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create (lc);
  int TxtWidth, TxtHeight;
  ostringstream oss;
  oss << "normal " << FONT_SIZE;
  layout -> set_font_description(Pango::FontDescription (oss.str().c_str()));
  for (unsigned int i = 0; i < YTics.size (); i ++) {
    oss.str ("");
    oss << ((int) i - YTicsZeroPos) * YTicSpacing;
    layout -> set_text (oss.str());
    layout -> get_pixel_size (TxtWidth, TxtHeight);
    get_window () ->draw_layout(get_style()->get_text_gc(Gtk::STATE_NORMAL), 
      GRAPH_PAD_LEFT - TxtWidth - 5, YTics[i] - TxtHeight / 2.0, layout);
  }
}


void Graph::drawText (Cairo::RefPtr<Cairo::Context> cr,
		  const int height, const int width) {
	Glib::RefPtr<Pango::Context> lc = create_pango_context();
	Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create (lc);
	ostringstream oss;
	oss << "normal " << FONT_SIZE;
	layout -> set_font_description(Pango::FontDescription (oss.str().c_str()));
	for (unsigned int i = 0; i < Labels.size (); i ++) {
		layout -> set_text (Labels[i].text);
		get_window () -> draw_layout(get_style() -> get_text_gc(Gtk::STATE_NORMAL), Labels[i].x, Labels[i].y, layout);
	}
}


//==============================================================================
// GENERAL CLASS METHODS
//==============================================================================

//------------------------------------------------------------------------------
// addPlot (Line, string) : Takes the properties from the XgLine passed in at
// arg1 and then extracts suitable data from the coordinate vector at arg2 to
// plot the line.
//

void Graph::addPlot (XgLine LineIn, vector <Coord> AscLines, 
  bool IncludeInMinima, bool IncludeInMaxima) {
  unsigned int XStart = 0;
  unsigned int XEnd = 0;
  double PlotMin, PlotMax;
  vector <Coord> Plot;
  Coord Min, Max;
    
  // Find the first data point to plot.
  PlotMin = LineIn.wavenumber () - LineIn.width () * PLOT_WIDTH_RANGE;
  PlotMax = LineIn.wavenumber () + LineIn.width () * PLOT_WIDTH_RANGE;
  if (PlotMax < AscLines[0].x || PlotMin > AscLines[AscLines.size () - 1].x) {
    Plot.push_back (Coord (0, 0));
    Max.x = 0.0; Max.y = 0.0;
    Min.x = 0.0; Min.y = 0.0;
  } else {
    XStart = 0; XEnd = AscLines.size () - 1;
    while (XEnd - XStart > 1) {
      if (AscLines[XStart + (XEnd - XStart) / 2].x > PlotMin) {
        XEnd = XStart + (XEnd - XStart) / 2;
      } else {
        XStart = XStart + (XEnd - XStart) / 2;
      }
    }

    // Find the last data point to plot
    while (AscLines[XEnd].x < PlotMax) {
      XEnd ++;
      if (XEnd >= AscLines.size ()) {
        cout << "Error: Unable to find line " << LineIn.line()
          << ". Line plotting ABORTED." << endl;
        throw int (FLT_PLOT_LINE_MISSING);
      }
    }

    Min.x = AscLines[XStart].x; Min.y = AscLines[XStart].y;
    Max.x = AscLines[XEnd - 1].x; Max.y = AscLines[int((XStart + XEnd) / 2)].y;
    for (unsigned int i = XStart; i <= XEnd; i ++) {
      Plot.push_back(AscLines[i]);
      if (AscLines[i].y > Max.y) Max.y = AscLines[i].y;
      if (AscLines[i].y < Min.y) Min.y = AscLines[i].y;
    }
  }
  Plots.push_back (Plot);
  if (IncludeInMinima) Minima.push_back (Min);
  if (IncludeInMaxima) Maxima.push_back (Max);
  LineWidths.push_back (DEF_PLOT_WIDTH);
  LineColours.push_back (GraphColour (DEF_PLOT_COLOUR));
  setAutoLimits ();
}


//------------------------------------------------------------------------------
// addPlot (vector <Coord>) : Adds a plot containing ALL the data points listed
// in the coordinates vector at arg1.
//
void Graph::addPlot (vector <Coord> NewPlot, bool IncludeInMinima, 
  bool IncludeInMaxima) {
  Coord Min, Max;

  Plots.push_back (NewPlot);
  Min.x = NewPlot[0].x; 
  Min.y = NewPlot[0].y;
  Max.x = NewPlot[NewPlot.size () - 1].x; 
  Max.y = NewPlot[int((NewPlot.size () - 1) / 2)].y;
  for (unsigned int i = 0; i < NewPlot.size (); i ++) {
    if (NewPlot[i].x > Max.x) Max.x = NewPlot[i].x;
    if (NewPlot[i].y > Max.y) Max.y = NewPlot[i].y;
    if (NewPlot[i].x < Min.x) Min.x = NewPlot[i].x;
    if (NewPlot[i].y < Min.y) Min.y = NewPlot[i].y;
  }
  if (IncludeInMinima) Minima.push_back (Min);
  if (IncludeInMaxima) Maxima.push_back (Max);
  LineWidths.push_back (DEF_PLOT_WIDTH);
  LineColours.push_back (GraphColour (DEF_PLOT_COLOUR));
  setAutoLimits ();
}


void Graph::addText (double x, double y, string textIn) {
	Label newLabel;
	newLabel.x = x;
	newLabel.y = y;
	newLabel.text = textIn;
	Labels.push_back (newLabel);
}


vector <Coord> Graph::getPlotData (int i) throw (int) { 
  if (i >= 0) {
    if (i < (int) Plots.size ()) {
      return Plots [i]; 
    } else {
      throw ((int) Plots.size ());
    } 
  } else {
    throw (ERR_GRAPH_INDEX_TOO_LOW);
  }
}

//------------------------------------------------------------------------------
// setAutoLimits ()
//
void Graph::setAutoLimits () {
  if (Minima.size () > 0 && Maxima.size () > 0) {
    GraphMin = Minima[0];
    for (unsigned int i = 0; i < Minima.size (); i ++) {
      if (Minima[i].x < GraphMin.x) GraphMin.x = Minima[i].x;
      if (Minima[i].y < GraphMin.y) GraphMin.y = Minima[i].y;
    }
    GraphMax = Maxima[0];
    for (unsigned int i = 0; i < Maxima.size (); i ++) {
      if (Maxima[i].x > GraphMax.x) GraphMax.x = Maxima[i].x;
      if (Maxima[i].y > GraphMax.y) GraphMax.y = Maxima[i].y;
    }
    GraphMax.y /= Y_GRAPH_ZOOM;
    GraphMin.y /= Y_GRAPH_ZOOM;
    AutoLimits = true;
  }
}


//------------------------------------------------------------------------------
// setWidth (int, float) : Sets the brush width of the ith plot to the value
// passed in at arg2.
//
void Graph::setWidth (int Index, float nWidth) {
  if (Index >= 0 && (unsigned int) Index < LineWidths.size ()) {
    if (nWidth > 0.0) {
      LineWidths[Index] = nWidth;
    }
  }
}


//------------------------------------------------------------------------------
// setColour () : Sets the colour of the ith plot to the r,g,b values passed in
// at args 2 to 4.
//
void Graph::setColour (int Index, float nr, float ng, float nb) {
  if (Index >= 0 && (unsigned int) Index < LineColours.size ()) {
    if (nr >=0.0 && nr <=1.0 && ng >=0.0 && ng <=1.0 && nb >=0.0 && nb <=1.0) {
      LineColours[Index] = GraphColour (nr, ng, nb);
    }
  }
}

