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
// plotFns.cpp
//==============================================================================
// Contains rotuines for interating with Gnuplot. These are used by ftsplot and
// ftslinefilter to generate plots of individual line profiles. Each plot
// contains the original line profile, as it was measured, the Voigt profile
// XGremlin fitted to it, and the Voigt fit residuals.
//
// In order to do this, an XGremlin 'writelines' line list is first needed to 
// identify the lines and provide their Voigt fit parameters. Then, two ASCII
// versions of the spectrum must be provided: one showing the original spectrum
// and a second containing the fit residuals. These can be generated in XGremlin
// with the 'writeasc' command before and after using the 'active' command.
//
#ifndef PLOT_FNS_CPP
#define PLOT_FNS_CPP

#define GNUPLOT_BIN "/usr/bin/gnuplot"
#define LATEX_BIN "/usr/bin/latex"
#define DVIPDF_BIN "/usr/bin/dvipdf"
//#define PLOT_HALF_WIDTH 1.0 /* cm^-1 */
//#define XGREMLIN_COMMENT '#'

// Filenames for the temporary line files created by plotLine() and plotLines().
// Make sure that the user has read/write access to the locations specified.
#define LINE_TEMP_FILE     ".temp_lines"
#define RESIDUAL_TEMP_FILE ".temp_res"
#define VOIGT_TEMP_FILE    ".temp_voigt"

// The number of data points contained in the synthetic Voigt profiles.
#define NUM_VOIGT_POINTS 200

#define PLOTS_PER_PAGE 12
#define PLOTS_PER_LINE 3

// Scaling factor for the residuals plot.
#define RES_SCALE 1.0

// Number of lines to skip at top of XGremlin HDR file to reach delw parameter.
#define XG_HDR_TOP 17

// Include the for generating Voigt profiles.
#include "voigtlsqfit.h"
#include "ErrDefs.h"

typedef struct latex_plot {
  vector <string> Name;
  string Caption;
} LatexPlot;


//------------------------------------------------------------------------------
// plotLine (Line, string, string, string) : Uses Gnuplot to plot the Line at
// arg1 and its residual. This requires an two ASCII versions of the spectrum
// containing LineIn to have been previously saved in XGremlin: one showing the
// lines (before using "active" in XGremlin), and one showing the line fit
// residuals (after using "active" in XGremlin). The names of these files are
// passed in at arg2 and arg3 respectively. Arg4 specifies where the output
// postscipt plot should be saved.
//
// Optional args: arg5 allows the plotted wavenumber range to be modified. The
// specified number is the range to plot either side of the line centre. arg6
// allows a wavenumber correction factor to be applied to the line, producing
// wavenumber calibrated plots. 
// 
void plotLine (XgLine LineIn, string AscLine, string AscResidual, string Output, 
  double WaveCorr = 0.0) {
  FILE *GpPipe, *Voigt;
  double PlotMin = LineIn.wavenumber () - LineIn.width () * PLOT_WIDTH_RANGE;
  double PlotMax = LineIn.wavenumber () + LineIn.width () * PLOT_WIDTH_RANGE;
  double PlotRange = PlotMax - PlotMin;
  
  
  // Generate a Voigt profile for the current line. w[0] contains the Gaussian 
  // component of the Voigt profile width, and w[1] the Lorentzian component.
  // These are extracted from the XGremlin writelines data by using the "width"
  // and "dmp" colummn, where the latter is the fraction of the Voigt profile
  // width that arises from the Lorentzian. Don't forget that the writelines
  // widths are in mK.
  double x[NUM_VOIGT_POINTS], y[NUM_VOIGT_POINTS], xc;
  for (unsigned int i = 0; i < NUM_VOIGT_POINTS; i ++) {
    x[i] = (float(i) / float(NUM_VOIGT_POINTS)) * (PlotMax - PlotMin) + PlotMin;
    y[i] = 0.0;
  }
  xc = LineIn.wavenumber ();
//  w[0] = LineIn.width() / 1000.0 * (1.0 - LineIn.dmp());
//  w[1] = LineIn.width() / 1000.0 * LineIn.dmp();
  VoigtLsqfit voigtGen;

//  voigt(NUM_VOIGT_POINTS, x, y, w, xc);
  voigtGen.voigt(NUM_VOIGT_POINTS, x, y, 0.0005*LineIn.width()/(x[1]-x[0]), LineIn.peak(), LineIn.dmp(), xc);
  
  Voigt = fopen(VOIGT_TEMP_FILE, "w");
  if (! Voigt) {
    cout << "Error: Cannot create temp file " <<  VOIGT_TEMP_FILE
      << ". Graph plotting ABORTED." << endl;
    throw int (FLT_FILE_OPEN_ERROR);
  }
  
  // Create a synthetic Voigt profile using the input line parameters
  double VoigtPeak = y[int(NUM_VOIGT_POINTS / 2)];
  for (unsigned int i = 0; i < NUM_VOIGT_POINTS; i ++) {
    y[i] = y[i] / VoigtPeak * LineIn.peak ();
    fprintf(Voigt, "%11.5f %11.5f\n", x[i] * (1.0 + WaveCorr), y[i]);
  }
  fclose (Voigt);

  // Now plot the line, its fit residual, and its Voigt profile by piping the
  // data through to Gnuplot. 
  GpPipe = popen (GNUPLOT_BIN, "w");
  if (GpPipe) {
    fprintf (GpPipe, "set size 0.64, 0.87\n");
    fprintf (GpPipe, "set terminal postscript eps enhanced color solid lw 2 \"Times\" 21\n");
    fprintf (GpPipe, "set output \"%s\"\n", Output.c_str());
//    fprintf(GpPipe, "set title \"Line %i centered at %11.5fK\\nPeak = %9.4e, epstot = %5.3f, epsodd = %5.3f%% of peak height\"\n",
//      LineIn.line(), LineIn.wavenumber(), LineIn.peak(), LineIn.epstot(), LineIn.epsodd() / LineIn.peak() * 100);
    fprintf (GpPipe, "set lmargin 7\n");
    fprintf (GpPipe, "set rmargin 0.5\n");
    fprintf (GpPipe, "set tmargin 0.5\n");
    fprintf (GpPipe, "set bmargin 3.5\n");
    if (LineIn.peak () >= 1000.0) {
      fprintf (GpPipe, "set ylabel \"S / N Ratio\" offset 2, 0\n");
    } else if (LineIn.peak () >= 100.0) {
      fprintf (GpPipe, "set ylabel \"S / N Ratio\" offset 1.7, 0.0\n");
    } else if (LineIn.peak () >= 10.0) {
      fprintf (GpPipe, "set ylabel \"S / N Ratio\" offset 1.4, 0.0\n");
    } else {
      fprintf (GpPipe, "set ylabel \"S / N Ratio\" offset 1.0, 0.0\n");
    }
    fprintf (GpPipe, "set xlabel \"Wavenumber / cm^{-1}\"\n");
    fprintf (GpPipe, "set xrange [%f:%f]\n", PlotMin * (1.0 + WaveCorr), PlotMax * (1.0 + WaveCorr));
    fprintf(GpPipe, "set xtics %f\n", PlotRange / 4.0);
    fprintf(GpPipe, "set mxtics %f\n", PlotRange / 2.0);

    fprintf (GpPipe, "set nokey\n");
    fprintf (GpPipe, "set label \"%6.4f\" at graph 0.05, graph 0.92 font \"Times,24\"\n", LineIn.wavenumber ());
    fprintf (GpPipe, "set label \"Width %5.3f cm^{-1}\\nPeak %4.1f\" at graph 0.05, graph 0.85 font \"Times,19\"\n", LineIn.width () / 1000.0, LineIn.peak ());
    fprintf (GpPipe, "plot \"%s\" t \"Fit residual\" w lines lt rgb \"#D0D0D0\", \
      \"%s\" t \"Line data\" w lines lt 0 lw 1.0 lc rgb \"#000000\", \
      \"%s\" t \"Voigt profile\" w lines lt rgb \"#000000\"\n",
      AscResidual.c_str(), AscLine.c_str(), VOIGT_TEMP_FILE);
    fprintf (GpPipe, "exit\n");
    pclose(GpPipe);
    remove (VOIGT_TEMP_FILE);
  } else {
    cout << "Error: Unable to start Gnuplot for graph output" << endl;
    remove (VOIGT_TEMP_FILE);
    throw int (FLT_PLOT_NO_GNUPLOT);
  }
}


//------------------------------------------------------------------------------
// plotLines (vector <Line>, string, string, string) : Uses plotLine(...) to 
// generate plots of all the lines in the vector <Line> at arg1. arg2 and arg3
// specify the names of the XGremlin ASCII files where the lines and their
// residuals may be found. arg4 gives the first part of the name for the output
// files. Each line index and a ".ps" extension are appended to this.
//
// Note: The lines MUST be sorted in order of ascending wavenumber, as output by
// XGremlin! 
//
// Each line is extracted in turn from the larger XGremin ASCII files and saved
// in temp files for use by Gnuplot. This greatly increases the speed at which
// Gnuplot runs, espeially when using large ASCII files.
//
// Optional args: arg5 allows the plotted wavenumber range to be modified. The
// specified number is the range to plot either side of the line centre. arg6
// allows a wavenumber correction factor to be applied to the line, producing
// wavenumber calibrated plots. 
//
void plotLine (XgLine Line, vector <Coord> LinePlot, 
  vector <Coord> ResPlot, string OutputName, double WaveCorr = 0.0) {
  FILE *tempLines, *tempResiduals;

  // Create two temp files - one for the line data, another for the residual -
  // that contain only the information for the current line. These will be
  // read by Gnuplot in plotLine (...).
  tempLines = fopen(LINE_TEMP_FILE, "w");
  tempResiduals = fopen(RESIDUAL_TEMP_FILE, "w");
  if (! tempLines) {
    cout << "Error: Cannot create line temp file " << LINE_TEMP_FILE 
    << ". Line plotting ABORTED." << endl;
    throw int (FLT_FILE_OPEN_ERROR);
  }
  if (! tempResiduals) {
    cout << "Error: Cannot create residual temp file " << RESIDUAL_TEMP_FILE 
    << ". Line plotting ABORTED." << endl;
    throw int (FLT_FILE_OPEN_ERROR);
  }
  for (unsigned int i = 0; i < LinePlot.size (); i ++) {
    fprintf(tempLines, "%11.5f %13.6e\n", LinePlot[i].x * (1.0 + WaveCorr), LinePlot[i].y);
  }
  for (unsigned int i = 0; i < ResPlot.size (); i ++) {
    fprintf(tempResiduals, "%11.5f %13.6e\n", ResPlot[i].x * (1.0 + WaveCorr), ResPlot[i].y);
  }
  fclose (tempLines);
  fclose (tempResiduals);
  plotLine (Line, LINE_TEMP_FILE, RESIDUAL_TEMP_FILE, OutputName, WaveCorr);

  remove (LINE_TEMP_FILE);
  remove (RESIDUAL_TEMP_FILE);
}


/*void combinePlotsWithLatex (vector <LatexPlot> Plots, string Name){
  ostringstream oss;
  FILE *tempLatex = fopen (Name.c_str(), "w");
  
  // Prepare the LaTeX environment
  fprintf (tempLatex, "%s", "\\documentclass[a4paper,12pt]{article}\n");
  fprintf (tempLatex, "%s", "\\usepackage[left=1cm,top=0.5cm,right=1.5cm,nohead,nofoot]{geometry}\n");
  fprintf (tempLatex, "%s", "\\usepackage{graphicx}\n");
  fprintf (tempLatex, "%s", "\\begin{document}\n\n");
  
  // Process each plot
  for (unsigned int k = 0; k < Plots.size (); k ++) {
    for (unsigned int i = 0; i < Plots[k].Name.size (); i += PLOTS_PER_PAGE) {
      fprintf (tempLatex, "%s", "\\begin{figure*}\n");
      oss.str (""); oss << "\\caption{" << Plots[k].Caption << "}\n";
      fprintf (tempLatex, "%s", oss.str().c_str());
      for (unsigned int j = i; j < Plots[k].Name.size () && j < i + PLOTS_PER_PAGE; j ++){
        oss.str ("");
        oss << "\\includegraphics[angle=0,width=0.333\\textwidth]{" 
          << Plots[k].Name[j] << "}";
        if (j % PLOTS_PER_LINE == PLOTS_PER_LINE - 1) oss << "\n";
        fprintf (tempLatex, "%s", oss.str().c_str());
      }
      fprintf (tempLatex, "%s", "\n\\end{figure*}\n\n");
    }
  }
  fprintf (tempLatex, "%s", "\\end{document}\n");
  fclose (tempLatex);

  oss.str ("");
  oss << LATEX_BIN << " " << Name;
  system (oss.str().c_str());
  oss.str ("");
  oss << DVIPDF_BIN << " " << Name << ".dvi";
  system (oss.str().c_str());

  oss.str ("");
  oss << Name << ".aux";
  remove (oss.str().c_str());
  oss.str ("");
  oss << Name << ".log";
  remove (oss.str().c_str());
  oss.str ("");
  oss << Name << ".dvi";
  remove (oss.str().c_str());
  remove (Name.c_str ());
}*/

#endif // PLOT_FNS_CPP
