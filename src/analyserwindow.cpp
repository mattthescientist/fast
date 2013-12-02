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
// This file contains various miscellaneous functions that do not belong in any
// of the other AnalyserWindow CPP files.

#include "analyserwindow.h"
#include "plotFns.cpp"
#include "lineio.cpp"

#include "analyserwindow_construct.cpp" // Constructor / destruction functions
#include "analyserwindow_signal.cpp"    // General signal handlers
#include "analyserwindow_signal_click.cpp" // Click event signal handlers
#include "analyserwindow_signal_file.cpp"  // File menu signal handlers
#include "analyserwindow_signal_data.cpp"  // Data menu signal handlers
#include "analyserwindow_signal_popup.cpp" // Popup menu signal handlers
#include "analyserwindow_io.cpp"        // I/O routines for ftslinetool projects
#include "analyserwindow_refresh.cpp"   // Functions to refresh window widgets
#include "analyserwindow_errors.cpp"    // Error handlers for AnalyserWindow
#include "analyserwindow_config.cpp"	// I/O routines for the FAST config file

using namespace::std;

void AnalyserWindow::projectHasChanged (bool ProjectHasChanged) {
  if (ProjectHasChanged) {
    ProjectChangedSinceSave = true;
    m_refActionGroup->get_action("FileSave")->set_sensitive(true);
  } else {
    ProjectChangedSinceSave = false;
    m_refActionGroup->get_action("FileSave")->set_sensitive(false);
  }
}

//------------------------------------------------------------------------------
// on_delete_event (GdkEventAny*) : Overrides the default on_delete_event in
// order to check whether or not the current project has been saved. If it
// hasn't, tell the user and ask them if they still want to quit.
//
bool AnalyserWindow::on_delete_event (GdkEventAny* event) {
  if (ProjectChangedSinceSave) {
    Gtk::MessageDialog quit(*this, "Would you like to save the project before quitting?",
      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    quit.set_secondary_text(" Click cancel to return to the project.");
    quit.add_button ("Yes", Gtk::RESPONSE_YES);
    quit.add_button ("No", Gtk::RESPONSE_NO);
    quit.add_button ("Cancel", Gtk::RESPONSE_CANCEL);
    int Result = quit.run ();
    if (Result == Gtk::RESPONSE_YES) {
      try {
        if (CurrentFilename == "") { 
          on_file_save_as ();
          if (CurrentFilename == "") { throw Error (FLT_FILE_WRITE_ERROR); }
        } else {
          saveProject (CurrentFilename);
        }
        this -> hide ();
      } catch (Error *e) {
        // Do nothing. However, reaching this line will result in the quit being
        // aborted due to a file save error.
      }
    } else if (Result == Gtk::RESPONSE_NO) {
      this -> hide ();
    }
  } else {
    this -> hide ();
  }
  return true;
}

//------------------------------------------------------------------------------
// scaleGraphs () : Puts all the line profile plots on the same Y scale. This 
// makes it easier to see relative peak intensities, which is useful for 
// branching fraction work. This method is called when the Scale button is 
// clicked on the toolbar. 
//
void AnalyserWindow::scaleGraphs () {
  if (LineBoxes.size () == 0) return;
  if (!LineBoxes[0][0] -> autoLimits ()) {
    for (unsigned int i = 0; i < LineBoxes.size (); i ++) {
      for (unsigned int j = 0; j < LineBoxes[i].size (); j ++) {
        LineBoxes[i][j] -> setAutoLimits ();
        LineBoxes[i][j] -> queue_draw ();
      }
    }
  } else {
    double MinY = LineBoxes[0][0] -> plotLimits().min.y;
    double MaxY = LineBoxes[0][0] -> plotLimits().max.y;
    GraphLimits lim;
    for (unsigned int i = 0; i < LineBoxes.size (); i ++) {
      for (unsigned int j = 0; j < LineBoxes[i].size (); j ++) {
        if (LineBoxes[i][j] -> plotLimits().min.y < MinY) MinY = LineBoxes[i][j] -> plotLimits().min.y;
        if (LineBoxes[i][j] -> plotLimits().max.y > MaxY) MaxY = LineBoxes[i][j] -> plotLimits().max.y;
      }
    }
    for (unsigned int i = 0; i < LineBoxes.size (); i ++) {
      for (unsigned int j = 0; j < LineBoxes[i].size (); j ++) {
        lim = LineBoxes[i][j] -> plotLimits();
        lim.min.y = MinY; lim.max.y = MaxY;
        LineBoxes[i][j] -> plotLimits (lim);
        LineBoxes[i][j] -> queue_draw ();
      }
    }
  }
}


//------------------------------------------------------------------------------
// Generate a Voigt profile for the current line. w[0] contains the Gaussian 
// component of the Voigt profile width, and w[1] the Lorentzian component.
// These are extracted from the XGremlin writelines data by using the "width"
// and "dmp" colummn, where the latter is the fraction of the Voigt profile
// width that arises from the Lorentzian. Don't forget that the writelines
// widths are in mK.
//
vector <Coord> AnalyserWindow::voigtProfile (XgLine LineIn, vector <Coord> Points) {
  vector <Coord> RtnPlot (Points.size (), Coord ());
  double x[Points.size ()], y[Points.size ()];
  double xc = LineIn.wavenumber ();
  for (unsigned int i = 0; i < Points.size (); i ++) {
    x[i] = Points[i].x;
    y[i] = 0.0;
  }

  VoigtLsqfit voigtGen;
  voigtGen.voigt(Points.size (), x, y, 
    0.0005 * LineIn.width() / (x[1] - x[0]), LineIn.peak(), LineIn.dmp(), xc);
  
  // Create a synthetic Voigt profile using the input line parameters
  for (unsigned int i = 0; i < Points.size (); i ++) {
    RtnPlot[i].y = y[i];
    RtnPlot[i].x = x[i] /* * (1.0 + WaveCorr)*/;
  }
  return RtnPlot;
}


//------------------------------------------------------------------------------
// matchedXgLineExists (KzLine, double) : Determines whether or not an XGremlin
// line exists that matches the KzLine passed in at arg1. To find out exactly
// which line matches best, getLinePairs() should be used instead.
//
bool AnalyserWindow::matchedXgLineExists (KzLine LineIn, double Discrimintor) {
  for (unsigned int i = 0; i < ExptSpectra.size (); i ++) {
    for (unsigned int j = 0; j < ExptSpectra[i].linesPtr2() -> size (); j ++) {
      for (unsigned int k = 0; k < ExptSpectra[i].linesPtr2() -> at(j).size (); k ++) {
        if (abs(ExptSpectra[i].linesPtr2()->at (j)[k].wavenumber() - LineIn.sigma()) < Discrimintor) {
          return true;
        }
      }
    }
  }
  return false;
}


//------------------------------------------------------------------------------
// getLinePairs (*KzList, int) : Given a list of levels from the Kurucz
// database in KzLevel, getLinePairs will attempt to match each level with 
// an XGremlin line attached to spectrum Spec. When a match is found, a new 
// LinePair object is created to link the Kurucz level and XGremlin line, and 
// the level is removed from KzLevel.
//
vector <LinePair> AnalyserWindow::getLinePairs (vector <KzLine *> KzLevel, int Spec) {
  vector <LinePair> MatchedLines;
  vector <LinePair> AllXgLines;
  vector <LinePair> Candidates;
  vector <LineData *> CandidatePlots;
  vector <LineData *> AllPlots;
  vector <vector <XgLine *> > PtrLines;
  vector <int> CandidateIndicies;
  double MinDifference;
  int MinDiffIndex;
  LinePair NextPair;

  // Copy all the lines attached to ExptSpectra[Spec] into a single vector
  PtrLines = ExptSpectra[Spec].linesPtr();
  for (unsigned int j = 0; j < ExptSpectra[Spec].linesPtr2()->size (); j ++) {
    for (unsigned int k = 0; k < ExptSpectra[Spec].linesPtr2()->at(j).size (); k ++) {
      NextPair.xgLine = PtrLines[j][k];
      NextPair.xgLineListIndex = j;
      NextPair.xgLineLineIndex = k;
      AllXgLines.push_back (NextPair);
      AllPlots.push_back (ExptSpectra[Spec].plots()[j][k]);
    }
  }

  // Search through each level in KzLevel and compare the wavenumbers to the
  // lines in AllXgLines. If a line is found that matches the level wavenumber
  // to within a tolerance of KzLevel.levelPrecision (), add that line to a
  // list of candidates for the best match.
  for (unsigned int i = 0; i < KzLevel.size(); i ++) {
    for (unsigned int j = 0; j < AllXgLines.size (); j ++) {
      if (abs(AllXgLines[j].xgLine->wavenumber() - KzLevel[i]->sigma()) < KuruczList.levelPrecision ()) {
        if (AllXgLines[j].xgLine->id() != FAKE_LINE_TAG) {
          Candidates.push_back (AllXgLines[j]);
          CandidatePlots.push_back (AllPlots[j]);
          CandidateIndicies.push_back (j);
        }
      }
    }

    // Take all the candidate matches identified above and see which XGremlin
    // line BEST matches the wavenumber of the Kurucz level. When the best match
    // has been found, create a new LinePair object for it and add it to the
    // vector to be returned at the end of the method.
    if (Candidates.size () > 0) {
      MinDifference = abs (KzLevel[i]->sigma() - Candidates[0].xgLine->wavenumber());
      MinDiffIndex = 0;
      for (unsigned int j = 0; j < Candidates.size (); j ++) {
        if (abs (KzLevel[i]->sigma() - Candidates[j].xgLine->wavenumber()) < MinDifference) {
          MinDifference = abs(KzLevel[i]->sigma() - Candidates[j].xgLine->wavenumber());
          MinDiffIndex = j;
        }
      }
      NextPair = Candidates[MinDiffIndex];
      NextPair.kzLine = KzLevel[i];
      NextPair.plot = CandidatePlots[MinDiffIndex];
      MatchedLines.push_back (NextPair);

      // Remove the matched XGremlin line from AllXgLines and the level from
      // KzLevel so that neither are used again in subsequent matches.
      AllXgLines.erase (AllXgLines.begin() + CandidateIndicies[MinDiffIndex]);
      AllPlots.erase (AllPlots.begin() + CandidateIndicies[MinDiffIndex]);
      Candidates.clear ();
      CandidatePlots.clear ();
      CandidateIndicies.clear ();
    
    // If no candidate line was found, insert a blank line into the list of
    // matched lines so that the final vector is of a known size, and is the
    // same size for all loaded spectra.
    } else {
      NextPair.xgLine = new XgLine;
      NextPair.plot = new LineData (*NextPair.xgLine);
      NextPair.kzLine = KzLevel[i];
      NextPair.xgLineListIndex = -1;
      NextPair.xgLineLineIndex = -1;
      MatchedLines.push_back (NextPair);
    }
  }
  
  return MatchedLines;
}


//------------------------------------------------------------------------------
// getLinePairs (vector <KzLine *>) : Calls getLinePairs (KzList, int) above to
// actually obtain the Kurucz/XGremlin line pairs. These are then examined, and
// lines that have no corresponding experimental data in any of the loaded
// spectra are removed.
//
vector < vector <LinePair> > AnalyserWindow::getLinePairs (vector <KzLine *> KzLevel) {
  vector < vector <LinePair> > LinePairs;

  // Link the Kurucz list KzLevel to experimental XGremlin data 
  for (unsigned int i = 0; i < ExptSpectra.size (); i ++) {
    LinePairs.push_back (getLinePairs (KzLevel, i));
  }

  // Scan through the lines and remove any for which there is no loaded
  // experimental data in any of the XGremlin spectra
  if (LinePairs.size () > 0) {
    bool LineFound;
    for (int j = LinePairs[0].size () - 1; j >= 0; j --) {
      LineFound = false;
      for (unsigned int i = 0; i < LinePairs.size (); i ++) {
        if (LinePairs[i][j].xgLineLineIndex != -1) {
          LineFound = true;
          break;
        }
      }
      if (!LineFound) {
        for (unsigned int i = 0; i < LinePairs.size (); i ++) {
          delete LinePairs[i][j].plot;
          delete LinePairs[i][j].xgLine;
          LinePairs[i].erase (LinePairs[i].begin() + j);
        }
      }
    }
  }
  return LinePairs;
}


//------------------------------------------------------------------------------
// getLinePairs () : A wrapper function for getLinePairs (vector <KzLine *>),
// above. This function will get the Kurucz/XGremlin line pairs for all loaded
// upper levels and store them in LevelLines for later use.
//
void AnalyserWindow::getLinePairs () {
  vector < vector <LinePair> > NextPairSet;
  LevelLines.clear ();
  for (unsigned int i = 0; i < KuruczList.numUpperLevels (); i ++) {
    NextPairSet = getLinePairs (KuruczList.upperLevelLines (i));
    LevelLines.push_back (NextPairSet);
  }
}


//------------------------------------------------------------------------------
// plotLines (vector < vector <LinePair>) : Plots all the XGremlin lines passed
// in at arg1. All lines from a given spectrum are attached to the Profiles 
// table in a single row. A new row created for the next spectrum.
//
void AnalyserWindow::plotLines (vector < vector <LinePair *> > PlotLines, 
  vector <unsigned int> PlotOrder) {
  vector <Coord> LineCoords, VoigtCoords, ResCoords;
  
  // First clear any plots that are currently displayed in the AnalyserWindow.
  clearDisplayedPlots ();
  // Now create LineData objects for all the remaining lines and add the
  // appropriate plots and line data to them.  
  generatePlots (PlotLines);
  for (unsigned int i = 0; i < LineBoxes.size (); i ++) {
    frameSpectrumPlots.push_back (new Gtk::Frame);
    hboxSpectrumPlots.push_back (new Gtk::HBox);
    frameSpectrumPlots[i] -> set_label (ExptSpectra[PlotOrder[i]].name());
    frameSpectrumPlots[i] -> add ((Gtk::Widget &) *hboxSpectrumPlots [i]);
    frameSpectrumPlots[i] -> show ();
    hboxSpectrumPlots[i] -> show ();
    for (unsigned int j = 0; j < LineBoxes[i].size (); j ++) {
      hboxSpectrumPlots[i] -> pack_start ((Gtk::Widget &) *LineBoxes[i][j], false, false, 0);
    }
    Profiles.pack_start ((Gtk::Widget &) *frameSpectrumPlots [i], false, false, 0);
  }
}

void AnalyserWindow::generatePlots (vector < vector <LinePair *> > PlotLines) {
  vector <LineData *> Plots;
  for (unsigned int i = 0; i < PlotLines.size (); i ++) {
    for (unsigned int j = 0; j < PlotLines[i].size (); j ++) {
      Plots.push_back (PlotLines[i][j]->plot);
      Plots[j] -> show ();
    }
    LineBoxes.push_back (Plots);
    Plots.clear ();
  }
}


//------------------------------------------------------------------------------
// addNewLines : Once on_data_load_line_list has loaded a new set of XGremlin
// addNewLines is called to 1) store these lines in the ExptSpectra vector, and
// 2) more importantly, generate plots for each of the new lines that can be
// quickly called up when requested by the user.
//
void AnalyserWindow::addNewLines (XgSpectrum *Spectrum, vector <XgLine> NewLines){
  vector <Coord> LineCoords, VoigtCoords, ResCoords;
  vector <vector <Coord> > Neighbours;
  vector <LineData *> Plots;
  Coord NextPoint;
  double ResidualRMS;

  // Create a plot for each object
  for (unsigned int i = 0; i < NewLines.size (); i ++) {
    Plots.push_back (new LineData (NewLines[i]));
    Plots[i]->signal_selected().connect
      (sigc::mem_fun(*this, &AnalyserWindow::on_click_plot));
    
    Plots[i] -> showParams (ViewLineParams);
    Plots[i] -> addPlot (Spectrum -> data(NewLines[i].wavenumber(),
      NewLines[i].width() * PLOT_WIDTH_RANGE), true, false);
    LineCoords = Plots[i] -> getPlotData (0);
    VoigtCoords = voigtProfile (NewLines[i], LineCoords);
    Plots[i] -> addPlot (VoigtCoords);
    
    // Find neighbouring lines that might be sufficiently close to the current
    // line to interfere with the residual RMS calculation.
    Neighbours.clear ();
    for (unsigned int j = (((int)i - 5 > 0) ? ((int)i - 5) : 0); 
      j < ((i + 5 < NewLines.size ()) ? (i + 5) : NewLines.size ()); j ++) {
      if (j != i && abs (NewLines[j].wavenumber () - NewLines[i].wavenumber ()) 
        < 3 * DEF_PLOT_WIDTH) {
        Neighbours.push_back (voigtProfile (NewLines[j], LineCoords));
      }
    }
    // Calculate the residuals from the difference between the experimental line
    // data and the Voigt profile generated from the XGremlin fit.
    ResCoords.clear ();
    ResidualRMS = 0.0;
    for (unsigned int k = 0; k < VoigtCoords.size (); k ++) {
      NextPoint = Coord (LineCoords[k].x, LineCoords[k].y - VoigtCoords[k].y);
      for (unsigned int j = 0; j < Neighbours.size (); j ++) {
        NextPoint.y -= Neighbours [j][k].y;
      }
      ResCoords.push_back (NextPoint);
      ResidualRMS += ResCoords[k].y * ResCoords[k].y;
    }
    ResidualRMS = sqrt (ResidualRMS / VoigtCoords.size ());
    NewLines[i].noise (ResidualRMS);
    
    Plots[i] -> setPlotColour (0, 0.5, 0.5, 0.5);
    Plots[i] -> setPlotWidth (1, 1.0);
    Plots[i] -> addResidual (ResCoords);
  }
  
  // Done generating new plots. Store the new lines and plots in the Spectrum.
  Spectrum -> lines_push_back (NewLines);
  Spectrum -> plots_push_back (Plots);
}
  

//------------------------------------------------------------------------------
// plotLines (XgSpectrum, int) : Plots all the XGremlin lines passed in at arg1.
//
void AnalyserWindow::plotLines (XgSpectrum XgData, int Index) {
  vector <Coord> LineCoords, VoigtCoords, ResCoords;
  Gtk::TreeModel::Row row;
  
  clearDisplayedPlots ();
  LineBoxes.push_back (XgData.plots()[Index]);
  
  modelDataXGr -> clear ();
  modelDataBF -> clear ();
  lineDataTreeModel -> clear ();
  for (unsigned int i = 0; i < XgData.lines()[Index].size (); i ++) {
    row = *(modelDataXGr -> append ());

    // Add the XGremlin line data to the table
    row[colsDataXGr.spectrum] = "";
    row[colsDataXGr.index] = XgData.lines()[Index][i].line ();
    row[colsDataXGr.wavenumber] = XgData.lines()[Index][i].wavenumber ();
    row[colsDataXGr.peak] = XgData.lines()[Index][i].peak ();
    row[colsDataXGr.width] = XgData.lines()[Index][i].width ();
    row[colsDataXGr.dmp] = XgData.lines()[Index][i].dmp ();
    row[colsDataXGr.eqwidth] = XgData.lines()[Index][i].eqwidth () 
      / XgData.response (XgData.lines()[Index][i].wavenumber ());
    row[colsDataXGr.epstot] = XgData.lines()[Index][i].epstot ();
    row[colsDataXGr.epsevn] = XgData.lines()[Index][i].epsevn ();
    row[colsDataXGr.epsodd] = XgData.lines()[Index][i].epsodd ();
    row[colsDataXGr.epsran] = XgData.lines()[Index][i].epsran ();
    row[colsDataXGr.id] = XgData.lines()[Index][i].id ();
    row[colsDataXGr.profile] = LineBoxes[0][i];
    if (XgData.response (XgData.lines()[Index][i].wavenumber ()) == 1.0) {
      row[colsDataXGr.eq_width_colour] = Gdk::Color (AW_EQWIDTH_NO_NORM_COLOUR);
      row[colsDataXGr.bg_colour] = Gdk::Color (AW_PARENT_LINE_COLOUR);
    } else {
      row[colsDataXGr.eq_width_colour] = Gdk::Color (AW_EQWIDTH_NORM_COLOUR);
      row[colsDataXGr.bg_colour] = Gdk::Color (AW_PARENT_LINE_COLOUR);
    }
  }
  
  for (unsigned int i = 0; i < LineBoxes.size (); i ++) {
    frameSpectrumPlots.push_back (new Gtk::Frame);
    hboxSpectrumPlots.push_back (new Gtk::HBox);
    frameSpectrumPlots[i] -> set_label ("Name");
    frameSpectrumPlots[i] -> add ((Gtk::Widget &) *hboxSpectrumPlots [i]);
    frameSpectrumPlots[i] -> show ();
    hboxSpectrumPlots[i] -> show ();
    for (unsigned int j = 0; j < LineBoxes[i].size (); j ++) {
      hboxSpectrumPlots[i] -> pack_start ((Gtk::Widget &) *LineBoxes[i][j], false, false, 0);
      LineBoxes[i][j] -> show ();
    }
    Profiles.pack_start ((Gtk::Widget &) *frameSpectrumPlots [i], false, false, 0);
  }
}


//------------------------------------------------------------------------------
// compareLinkedSpectra (unsigned int, unsigned int, Gtk::TreeModel::Row) : If
// the user has explicitly linked the two spectra specified at arg1 and arg2, it
// is assumed that they were measured simultaneously, and have identical 
// spectrometer response functions (or have been pre-processed to remove 
// differences in response function). This will typically be the case when
// linking spectra from FTS channel A and channel B. By definition, this link,
// assets that the relative intensity of all lines in both spectra should be
// identical. As a result, ANY line common to both spectra be used to calculate
// a scaling factor rather than just the lines from the currently selected upper
// level.
//
RatioAndError AnalyserWindow::compareLinkedSpectra (unsigned int a, unsigned int b, 
  Gtk::TreeModel::Row parentRow) throw (int) {
  
  Gtk::TreeModel::Row row;
  vector <XgLine> List1 = ExptSpectra[a].linesVector ();
  vector <XgLine> List2 = ExptSpectra[b].linesVector ();
  double Ratio = 0.0, Error = 0.0, AveRatio = 0.0, AveError = 0.0, RatioDenom = 0.0;
  double SNR1, SNR2;
  RatioAndError RtnRatioAndError;
  ostringstream oss;
  unsigned int NumLinesCompared = 0;
  
  for (unsigned int i = 0; i < List1.size (); i ++) {
    for (unsigned int j = 0; j < List2.size (); j ++) {
      if (abs (List1[i].wavenumber () - List2[j].wavenumber ()) < KuruczList.levelPrecision ()) {
        // Add a new child row for the comparison of individual lines and
        // fill it with the required information
        row = *(modelDataComp -> append (parentRow->children()));
        row[colsDataComp.ref] = ExptSpectra [a].index ();
        row[colsDataComp.comparison] = ExptSpectra [b].index ();
        row[colsDataComp.wavenumber] = "";

        Ratio = List1[i].eqwidth () / List2[j].eqwidth ();
        if (Options.correct_snr ()) {
          SNR1 = List1[i].snr ();
          SNR2 = List2[j].snr ();
        } else {
          SNR1 = List1[i].snr () / List1[i].noise();
          SNR2 = List2[j].snr () / List2[j].noise();
        } 
        Error = sqrt(pow (SNR1, -2.0) + pow (SNR2, -2.0));
        AveRatio += Ratio / pow (Error, 2.0);
        RatioDenom += pow (Error, -2.0);
        AveError += pow (Error, 2.0);
        oss.str (""); oss << Ratio << " +/- " << Ratio * Error;
        row[colsDataComp.ratio] = oss.str ();
        oss.str (""); oss << List1[i].wavenumber();
        row[colsDataComp.wavenumber] = oss.str ();
        NumLinesCompared ++;
      }
    }
  }
  
  if (NumLinesCompared == 0) {
    throw (NO_COMPARISON_LINES_FOUNDS);
  }
  // Update the root node with the overall comparison information
  AveRatio /= RatioDenom;
  AveError = pow (AveError, 0.5) / NumLinesCompared;
  parentRow[colsDataComp.ref] = ExptSpectra [a].index ();
  parentRow[colsDataComp.comparison] = ExptSpectra [b].index ();
  parentRow[colsDataComp.wavenumber] = "";
  oss.str (""); oss << AveRatio << " +/- " << AveError;
  parentRow[colsDataComp.ratio] = oss.str ();
  RtnRatioAndError.Ratio = AveRatio;
  RtnRatioAndError.Error = AveError;
  RtnRatioAndError.a = a;
  RtnRatioAndError.b = b;
  return RtnRatioAndError;
}    

 
//------------------------------------------------------------------------------
// getBestScalingFactor (unsigned int, unsigned int, vector <RatioAndError>) :
// Finds the best equivalent width scaling factor to use when overlapping the
// two spectra specified at arg1 and arg2. If the two spectra do not overlap, or
// if there are no common lines between them, this function will attempt to
// calculate a compound scaling factor using intermediate spectra. Be careful:
// this is a RECURSIVE function.
//
RatioAndError AnalyserWindow::getBestScalingFactor (unsigned int Start, 
  unsigned int End, vector <RatioAndError> ScalingFactors) throw (int) {
  
  RatioAndError RtnFactor, NextFactor, Swap;
  vector <RatioAndError> Subset;
  RtnFactor.Error = -1;
  
  // Search through all available ScalingFactors to find a link from spectrum
  // 'Start', A, to spectrum 'End', B.  
  for (unsigned int i = 0; i < ScalingFactors.size (); i ++) {
    if (ScalingFactors [i].a == Start) {
      if (ScalingFactors [i].b == End) {
        // Success: The factor linking A and B has been found. Return it.
        return ScalingFactors [i];
      } else {
        // The current scaling factor starts at spectrum A, but does not link to
        // B. It instead links to a third spectrum, C.
        Subset = ScalingFactors;
        Subset.erase (Subset.begin () + i);
        try {
          // Try to find to find another scaling factor that links C to B. This
          // search is RECURSIVE, and so is capable of finding a suitable link
          // from A to B with more than one intermediate spectrum.
          NextFactor = getBestScalingFactor (ScalingFactors [i].b, End, Subset);
        } catch (int e) {
          try {
            // No factor was found linking C to B, but one might exist that
            // links B to C. If so, this can be inverted.
            NextFactor = getBestScalingFactor (End, ScalingFactors [i].b, Subset);
            Swap = NextFactor;
            NextFactor.a = Swap.b;
            NextFactor.b = Swap.a;
            NextFactor.Ratio = 1.0 / Swap.Ratio;
          } catch (int e) {
            // No factor was found linking either C to B or B to C.
            NextFactor.Error = -1;
          }
        }
        // If a scaling factor was found linking C to B, combine it with the
        // factor linking A to C to obtain the required link from A to B.
        if (NextFactor.Error != -1) {
          NextFactor.Ratio *= ScalingFactors [i].Ratio;
          NextFactor.Error = 
            sqrt (pow (NextFactor.Error, 2) + pow (ScalingFactors[i].Error, 2));
          // Depending on which spectra are loaded in the current project, it
          // may be possible to link A to B through different intermediary
          // spectra. If this is the case, keep only the scaling factor that has
          // the lowest associated uncertainty.
          if (RtnFactor.Error == -1 || NextFactor.Error < RtnFactor.Error) {
            RtnFactor = NextFactor;
          }
        }
      }
    }
  }
  // If no suitable link was found from A to B, throw an error
  if (RtnFactor.Error == -1) throw (NO_SCALING_RATIO_FOUND);
  return RtnFactor;
}
  
  
  

