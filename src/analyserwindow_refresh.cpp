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
// AnalyserWindow class (analyserwindow_refresh.cpp)
//==============================================================================
// This file contains the functions for updating and refreshing the information
// displayed in the AnalyserWindow. They are called whenever any data is loaded
// or deleted, or when the user requests different data be shown.

#include "TypeDefs.h"
using namespace::std;

//------------------------------------------------------------------------------
// updateKuruczCompleteness () : Whenever any of the Kurucz line list parameters
// change, updateKuruczCompleteness is called to recalculate the fraction of
// significant Kurucz lines are present in the loaded experimental spectra.
//
void AnalyserWindow::updateKuruczCompleteness () {
  typedef Gtk::TreeModel::Children type_children;
  type_children children = levelTreeModel->children();
  double FractionFound;
  int Level;
  for (type_children::iterator iter = children.begin(); iter != children.end(); ++iter) {
    Level = (*iter)[levelCols.index];
    FractionFound = 0.0;
    for (unsigned int i = 0; i < ExptSpectra.size (); i ++) {
      for (unsigned int j = 0; j < LevelLines[Level][i].size (); j ++) {
        if (LevelLines[Level][i][j].plot->selected()) {
          if (LevelLines[Level][i][j].xgLine->wavenumber () > 0.0) {
            FractionFound += (LevelLines[Level][i][j].kzLine->brFrac () * 100.0);
          }
        }
      }
    }
    (*iter)[levelCols.fracFound] = FractionFound;
  }
  updateKuruczBF ();
}


//------------------------------------------------------------------------------
// updateKuruczBF () : Updates the branching fraction tab in the Kurucz Upper
// Levels section of the window
//
void AnalyserWindow::updateKuruczBF () {
  typedef Gtk::TreeModel::Children type_children;
  type_children children = levelTreeModel->children();
  double FractionFound, Ratio, EwTotal, ATotal, Lifetime;
  vector <double> Lifetimes;
  int Level, i;
  
  // Cycle through each of the upper levels listed in treeLevels and extract the
  // lines belonging to it from KuruczList. Pair these up with corresponding
  // XGremlin lines for each loaded spectrum using getLinePairs.
  children = levelTreeModel -> children();
  for (type_children::iterator iter = children.begin(); iter != children.end(); ++iter) {
    Level = (*iter)[levelCols.index];
    Lifetimes.push_back ((*iter)[levelCols.lifetime] * 1e-9);
    KuruczList.set_upper_level_lifetime (Level, (*iter)[levelCols.lifetime] * 1e-9);
    KuruczList.set_upper_level_lifetime_error (Level, (*iter)[levelCols.err_lifetime] * 1e-9);
    if ((*iter)[levelCols.lifetime] > 0.0 && (*iter)[levelCols.lifetime] != 1.0) {
      (*iter)[levelCols.lifetime_colour] = Gdk::Color (AW_EQWIDTH_NORM_COLOUR);
    }
    if ((*iter)[levelCols.err_lifetime] > 0.0) {
      (*iter)[levelCols.err_lifetime_colour] = Gdk::Color (AW_EQWIDTH_NORM_COLOUR);
    }
  }

  children = modelLevelsBF -> children(); 
  i = -1;
  for (type_children::iterator iter = children.begin(); iter != children.end(); ++iter) {
    Level = (*iter)[levelColsBF.index];
    i = i + 1; Lifetime = Lifetimes [i];
    EwTotal = 0.0;
    FractionFound = 0.0;

    // For each line pair, add its equivalent width and branching fraction to
    // a cumulative total. Combine this with the level lifetime to get A.
    for (unsigned int i = 0; i < ExptSpectra.size (); i ++) {
      for (unsigned int j = 0; j < LevelLines[Level][i].size (); j ++) {
        if (LevelLines[Level][i][j].plot->selected()) {
          if (LevelLines[Level][i][j].xgLine->wavenumber () > 0.0) {
            EwTotal += (LevelLines[Level][i][j].xgLine->eqwidth ()) 
              / ExptSpectra[i].response (LevelLines[Level][i][j].xgLine->wavenumber ());
            FractionFound += LevelLines[Level][i][j].kzLine->brFrac ();
          }
        }
      }
    }
    ATotal = FractionFound / (Lifetime);
    if (ATotal > 0) {
      Ratio = 1.0 / (Lifetime * ATotal);
    } else {
      Ratio = 0.0;
    }
    

    // If the current spectrum is the reference spectrum, display its 
    // branching fraction data in the "Br. Frac." tab.
    (*iter)[levelColsBF.a_total] = ATotal;
    (*iter)[levelColsBF.a_missing] = 100.0 * (1.0 - Lifetime * ATotal);
    (*iter)[levelColsBF.ew_total] = EwTotal;
    (*iter)[levelColsBF.ew_norm] = EwTotal * Ratio;

/*    // Finally, colour the branching fraction data according to whether or not
    // the spectra have been intensity calibrated.
    if (ExptSpectra[i].lines().size() > 0) {
      if (ExptSpectra[i].response (ExptSpectra[i].lines()[0][0].wavenumber ()) != 1.0) {
        (*iter)[levelColsBF.bg_colour] = Gdk::Color (AW_EQWIDTH_NORM_COLOUR);
      } else {
        (*iter)[levelColsBF.bg_colour] = Gdk::Color (AW_EQWIDTH_NO_NORM_COLOUR);
      }
    }*/
  }
}

//------------------------------------------------------------------------------
// updateKuruczList () : Updates the list of Kurucz lines displayed in the 
// "Kurucz Data" tab at the bottom right of the window.
//
void AnalyserWindow::updateKuruczList (KzList LineList) {
  Gtk::TreeModel::Row row;
  
  lineDataTreeModel->clear ();
  vector <KzLine> TargetLines = LineList.lines();
  for (unsigned int i = 0; i < TargetLines.size(); i ++) {
    row = *(lineDataTreeModel->append());
    if (TargetLines[i].brFrac () > MIN_SIGNIFICANT_BF) {
      row[targetsCols.colour] = Gdk::Color ("#000000");
      if (matchedXgLineExists (TargetLines[i], 
        KuruczList.levelPrecision ())) {
        row[targetsCols.bg_colour] = Gdk::Color ("#FFFFFF");
      } else {
        row[targetsCols.bg_colour] = Gdk::Color ("#FF9090");
      }
    } else {
      if (matchedXgLineExists (TargetLines[i], 
        KuruczList.levelPrecision ())) {
        row[targetsCols.colour] = Gdk::Color ("#606060");
      } else {
        row[targetsCols.colour] = Gdk::Color ("#D0D0D0");
      }
      row[targetsCols.bg_colour] = Gdk::Color ("#FFFFFF");
    }
    
    row[targetsCols.sigma] = TargetLines[i].sigma ();
    row[targetsCols.wavelength] = TargetLines[i].lambda ();
    row[targetsCols.loggf] = TargetLines[i].loggf ();
    row[targetsCols.bf] = TargetLines[i].brFrac ();
    row[targetsCols.elower] = TargetLines[i].eLower ();
    row[targetsCols.jlower] = TargetLines[i].jLower ();
    row[targetsCols.configlower] = TargetLines[i].configLower ();
    row[targetsCols.eupper] = TargetLines[i].eUpper ();
    row[targetsCols.jupper] = TargetLines[i].jUpper ();
    row[targetsCols.configupper] = TargetLines[i].configUpper ();
  }
}


//------------------------------------------------------------------------------
// updateXGremlinList () : Updates the list of XGremlin lines displayed in the
// "XGremlin Data" tab at the bottom right of the window. 
//
void AnalyserWindow::updateXGremlinList (
  vector < vector <LinePair *> > OrderedPairs, vector <string> SpectrumLabels,
  vector <unsigned int> SpectrumOrder) {

  Gtk::TreeModel::Row row, parentRow;
  bool LineFound;
  RatioAndError BestScalingFactor, Swap;
  
  // Cycle through all the lines in the current level with i
  for (unsigned int i = 0; i < OrderedPairs[0].size (); i ++) {
    LineFound = false;
    for (unsigned int j = 0; j < OrderedPairs.size (); j ++) {
      if (OrderedPairs[j][i] -> xgLine->wavenumber () > 0.0 && !OrderedPairs[j][i]->plot->hidden()) {
        if (!LineFound) {
          parentRow = *(modelDataXGr -> append ());
          row = parentRow;
        } else {
          row = *(modelDataXGr -> append (parentRow.children()));
        }

        // Add the XGremlin line data to the table
        row[colsDataXGr.spectrum] = SpectrumLabels[j];
        row[colsDataXGr.index] = OrderedPairs[j][i]->xgLine->line ();
        row[colsDataXGr.wavenumber] = OrderedPairs[j][i]->xgLine->wavenumber ();
        row[colsDataXGr.peak] = OrderedPairs[j][i]->xgLine->peak ();
        row[colsDataXGr.width] = OrderedPairs[j][i]->xgLine->width ();
        row[colsDataXGr.dmp] = OrderedPairs[j][i]->xgLine->dmp ();
        
        // Calculate the branching fraction data that needs to be shown
        try {
          BestScalingFactor = getBestScalingFactor (SpectrumOrder[0], SpectrumOrder[j], ScalingFactors);
        } catch (int e) {
          if (e == NO_SCALING_RATIO_FOUND) {
            try {
              BestScalingFactor = getBestScalingFactor (SpectrumOrder[j], SpectrumOrder[0], ScalingFactors);
              Swap = BestScalingFactor;
              BestScalingFactor.a = Swap.b;
              BestScalingFactor.b = Swap.a;
              BestScalingFactor.Ratio = 1.0 / Swap.Ratio;
            } catch (int e2) {
              if (e2 == NO_SCALING_RATIO_FOUND) {
                BestScalingFactor.a = 0;
                BestScalingFactor.b = 0;
                BestScalingFactor.Ratio = 1.0;
                BestScalingFactor.Error = 0.0;
//                cout << "Unable to link " << SpectrumLabels [j] << " and "
//                  << SpectrumLabels [0] << endl;
              }
            }
          }
        }        
        row[colsDataXGr.eqwidth] = OrderedPairs[j][i]->xgLine->eqwidth () 
          / ExptSpectra[SpectrumOrder[j]].response 
            (OrderedPairs[j][i]->xgLine->wavenumber ())
          * BestScalingFactor.Ratio;
        row[colsDataXGr.epstot] = OrderedPairs[j][i]->xgLine->epstot ();
        row[colsDataXGr.epsevn] = OrderedPairs[j][i]->xgLine->epsevn ();
        row[colsDataXGr.epsodd] = OrderedPairs[j][i]->xgLine->epsodd ();
        row[colsDataXGr.epsran] = OrderedPairs[j][i]->xgLine->epsran ();
        row[colsDataXGr.id] = OrderedPairs[j][i]->xgLine->id ();
        row[colsDataXGr.profile] = LineBoxes[j][i];
        row[colsDataXGr.bg_colour] = Gdk::Color (AW_PARENT_LINE_COLOUR);
        if (ExptSpectra[SpectrumOrder[j]].response 
          (OrderedPairs[j][i]->xgLine->wavenumber ()) == 1.0) {
          row[colsDataXGr.eq_width_colour] = Gdk::Color (AW_EQWIDTH_NO_NORM_COLOUR);
        } else {
          row[colsDataXGr.eq_width_colour] = Gdk::Color (AW_EQWIDTH_NORM_COLOUR);
        }
        LineFound = true;
      }
    }
  }
}


//------------------------------------------------------------------------------
// calculateBranchingFractions (vector < vector <LinePair *> >, vector <string>,
// vector <unsigned int>) : Calculates all the branching fraction data to be
// displayed in the "Br. Frac. Data" list. The contents of the list is based on
// which line plots are selected (highlighted) in the main window.
//
vector <DataBF> AnalyserWindow::calculateBranchingFractions (
  vector < vector <LinePair *> > OrderedPairs, vector <string> SpectrumLabels,
  vector <unsigned int> SpectrumOrder) {
  
  vector <DataBF> AllBrFracData;
  DataBF NextBrFracLine;
  RatioAndError BestScalingFactor;
  RatioAndError Swap;
  double gf;
  double TotalEqWidth = 0.0, TotalKuruczBrFrac = 0.0, TotalErrInEqWidth = 0.0;

  // Cycle through each of the lines in the target upper level
  for (unsigned int i = 0; i < OrderedPairs[0].size (); i ++) { 

    // Cycle through each of the loaded spectra with j
    for (unsigned int j = 0; j < OrderedPairs.size (); j ++) { 

      // Only consider a line if it has a valid wavenumber and is selected
      if (OrderedPairs[j][i] -> xgLine -> wavenumber () > 0.0) {
        if (OrderedPairs[j][i] -> plot -> selected ()) {
        
          // Calculate the branching fraction data that needs to be shown
          try {
            BestScalingFactor = getBestScalingFactor (SpectrumOrder[0], SpectrumOrder[j], ScalingFactors);
          } catch (int e) {
            if (e == NO_SCALING_RATIO_FOUND) {
              try {
                BestScalingFactor = getBestScalingFactor (SpectrumOrder[j], SpectrumOrder[0], ScalingFactors);
                Swap = BestScalingFactor;
                BestScalingFactor.a = Swap.b;
                BestScalingFactor.b = Swap.a;
                BestScalingFactor.Ratio = 1.0 / Swap.Ratio;
              } catch (int e2) {
                if (e2 == NO_SCALING_RATIO_FOUND) {
                  BestScalingFactor.a = 0;
                  BestScalingFactor.b = 0;
                  BestScalingFactor.Ratio = 1.0;
                  BestScalingFactor.Error = 0.0;
//                  cout << "Unable to link " << SpectrumLabels [j] << " and "
//                    << SpectrumLabels [0] << endl;
                }
              }
            }
          }
          NextBrFracLine.spectrum = SpectrumLabels[j];
          NextBrFracLine.index = OrderedPairs[j][i]->xgLine->line ();
          NextBrFracLine.wavenumber = OrderedPairs[j][i]->xgLine->wavenumber ();
          NextBrFracLine.eqwidth = OrderedPairs[j][i]->xgLine->eqwidth () 
            / ExptSpectra[SpectrumOrder[j]].response 
              (OrderedPairs[j][i]->xgLine->wavenumber ())
            * BestScalingFactor.Ratio;
          if (Options.correct_snr ()) {
            NextBrFracLine.err_line = OrderedPairs[j][i]->xgLine->noise() / OrderedPairs[j][i]->xgLine->snr() * 100;
          } else {
            NextBrFracLine.err_line = 1.0 / OrderedPairs[j][i]->xgLine->snr() * 100;
          }
          
          NextBrFracLine.err_cal = ExptSpectra[SpectrumOrder[j]].response_error
            (OrderedPairs[j][i]->xgLine->wavenumber ());
          NextBrFracLine.err_trans = BestScalingFactor.Error * 100;
          NextBrFracLine.err_total = sqrt (pow (NextBrFracLine.err_line, 2)
            + pow (NextBrFracLine.err_cal / sqrt (2.0), 2) 
            + pow (NextBrFracLine.err_trans, 2));
          NextBrFracLine.err_eqwidth = 
            NextBrFracLine.eqwidth * NextBrFracLine.err_total / 100;
          NextBrFracLine.profile = LineBoxes[j][i];

          // Keep a running total of the equivalent width and its error, and the
          // Kurucz branching fraction summed over all selected lines. These are
          // used for normalisation of parameters later on.
          TotalEqWidth += NextBrFracLine.eqwidth;
          TotalErrInEqWidth += 
            pow (NextBrFracLine.eqwidth * NextBrFracLine.err_total / 100, 2);
          TotalKuruczBrFrac += OrderedPairs[j][i]->kzLine->brFrac ();

          // The fields that depend upon the above level sum totals cannot be
          // fully calculated until those sum totals are known. Therefore, the
          // following fields contain only intermediate calculations, which are
          // completed when adding the data to the table, below.
          NextBrFracLine.a = 1.0 / OrderedPairs[j][i]->kzLine->lifetime ();
          NextBrFracLine.err_a = pow (100 * OrderedPairs[j][i]->kzLine->lifetime_error () 
              / OrderedPairs[j][i]->kzLine->lifetime (), 2);
          if (OrderedPairs[j][i]->kzLine->eUpper() > OrderedPairs[j][i]->kzLine->eLower()) {
            gf =  1.499e-14 * (2 * OrderedPairs[j][i] -> kzLine->jUpper () + 1)
              * pow (OrderedPairs[j][i] -> xgLine->airWavelength (), 2);
          } else { 
            gf =  1.499e-14 * (2 * OrderedPairs[j][i] -> kzLine->jLower () + 1)
              * pow (OrderedPairs[j][i] -> xgLine->airWavelength (), 2);
          }
          NextBrFracLine.loggf = gf;

          // Determine the background colour of fields that require additional
          // user data.
          NextBrFracLine.bg_colour = Gdk::Color (AW_PARENT_LINE_COLOUR);
          if (ExptSpectra[SpectrumOrder[j]].response 
            (OrderedPairs[j][i]->xgLine->wavenumber ()) == 1.0) {
            NextBrFracLine.eq_width_colour = Gdk::Color (AW_EQWIDTH_NO_NORM_COLOUR);
          } else {
            NextBrFracLine.eq_width_colour = Gdk::Color (AW_EQWIDTH_NORM_COLOUR);
          }
          if (ExptSpectra[SpectrumOrder[j]].response_error 
            (OrderedPairs[j][i]->xgLine->wavenumber ()) == 0.0) {
            NextBrFracLine.err_cal_colour = Gdk::Color (AW_EQWIDTH_NO_NORM_COLOUR);
          } else {
            NextBrFracLine.err_cal_colour = Gdk::Color (AW_EQWIDTH_NORM_COLOUR);
          }
          AllBrFracData.push_back (NextBrFracLine);
          
          // Only one line of each type can be considered in the branching
          // fraction calculation, so break out of the spectrum loop. This will
          // mean that if a given line is selected in more than one spectrum,
          // only the first selected instance will be used.
          break;
        }
      }
    }
  }
  TotalErrInEqWidth = sqrt (TotalErrInEqWidth);
  
  // Add all the branching fraction data to the "Br. Frac. Data" table.
  for (unsigned int j = 0; j < AllBrFracData.size (); j ++) {   
    // Now consider the fields that require normalisation to the level's total
    // eq. width, total error in eq. width, or total Kurucz branching fraction.
    // These normalisations could not be done above, so perform the calculations
    // here.
    AllBrFracData[j].br_frac = 
      AllBrFracData[j].eqwidth * TotalKuruczBrFrac / TotalEqWidth;
    
    // This calculation of err_br_frac is incorrect. Don't use it. It's here
    // simply because other publications have used it in the past, so it may
    // be necessary to use it for diagnostics in the future.
//    AllBrFracData[j].err_br_frac = sqrt(pow (AllBrFracData[j].err_total, 2) +
//      pow (100.0 * TotalErrInEqWidth / TotalEqWidth, 2));
  }

  // Calculate U (BF) based on Equation 7 in C.M.Sikstrom et al., JQSRT,
  // 74 pp. 355 (2002). Note that the coefficient (1- BF_k) should be squared as
  // it is in Equation 6!
  for (unsigned int j = 0; j < AllBrFracData.size (); j ++) {   
    AllBrFracData[j].err_br_frac = (1 - 2.0 * AllBrFracData[j].br_frac) *
      pow (AllBrFracData[j].err_total, 2);
    for (unsigned int k = 0; k < AllBrFracData.size (); k ++) {
      AllBrFracData[j].err_br_frac += pow (AllBrFracData[k].br_frac, 2) * 
        pow (AllBrFracData[k].err_total, 2);
    }
    AllBrFracData[j].err_br_frac = sqrt (AllBrFracData[j].err_br_frac);
  }

  for (unsigned int j = 0; j < AllBrFracData.size (); j ++) {   
    gf = AllBrFracData[j].loggf;
    AllBrFracData[j].a = AllBrFracData[j].br_frac * AllBrFracData[j].a;
    AllBrFracData[j].err_a = 
      sqrt (pow (AllBrFracData[j].err_br_frac, 2) + AllBrFracData[j].err_a);
    AllBrFracData[j].loggf = log10 (gf * AllBrFracData[j].a);
    AllBrFracData[j].dex = log10 (gf * (1 + AllBrFracData[j].err_a / 100))
      - log10 (gf);
  }
  return AllBrFracData;  
}


//------------------------------------------------------------------------------
// updateBranchingFractions (...) :
//
void AnalyserWindow::updateBranchingFractions (
  vector < vector <LinePair *> > OrderedPairs, vector <string> SpectrumLabels,
  vector <unsigned int> SpectrumOrder) {
  Gtk::TreeModel::Row row;
  vector <DataBF> AllBrFracData =
    calculateBranchingFractions (OrderedPairs, SpectrumLabels, SpectrumOrder);
  
  // Add all the branching fraction data to the "Br. Frac. Data" table.
  for (unsigned int j = 0; j < AllBrFracData.size (); j ++) { 
    row = *(modelDataBF -> append ());;
    
    // First copy across the fields that were calculated in their entirety above.
    row[colsDataBF.spectrum] = AllBrFracData[j].spectrum;
    row[colsDataBF.index] = AllBrFracData[j].index;
    row[colsDataBF.wavenumber] = AllBrFracData[j].wavenumber;
    row[colsDataBF.eqwidth] = AllBrFracData[j].eqwidth;
    row[colsDataBF.err_line] = AllBrFracData[j].err_line;
    row[colsDataBF.err_cal] = AllBrFracData[j].err_cal;
    row[colsDataBF.err_trans] = AllBrFracData[j].err_trans;
    row[colsDataBF.err_total] = AllBrFracData[j].err_total;
    row[colsDataBF.err_eqwidth] = AllBrFracData[j].err_eqwidth;
    row[colsDataBF.profile] = AllBrFracData[j].profile;
    row[colsDataBF.bg_colour] = AllBrFracData[j].bg_colour;
    row[colsDataBF.eq_width_colour] = AllBrFracData[j].eq_width_colour;
    row[colsDataBF.err_cal_colour] = AllBrFracData[j].err_cal_colour;
    row[colsDataBF.br_frac] = AllBrFracData[j].br_frac;
    row[colsDataBF.err_br_frac] = AllBrFracData[j].err_br_frac;
    row[colsDataBF.a] = AllBrFracData[j].a;
    row[colsDataBF.err_a] = AllBrFracData[j].err_a;
    row[colsDataBF.loggf] = AllBrFracData[j].loggf;
    row[colsDataBF.dex] = AllBrFracData[j].dex; 
  }
}


//------------------------------------------------------------------------------
// updateComparisonList () : Update the "Compare Spectra" tab at the bottom
// right of the window. This tab displays information allowing the loaded
// experimental spectra to be compared against one another.
//
vector <RatioAndError> AnalyserWindow::updateComparisonList (vector < vector <LinePair *> > 
  OrderedPairs, vector <string> SpectrumLabels, vector <unsigned int> SpectrumOrder) {
  Gtk::TreeModel::Row row, parentRow;
  double Ratio, Error, AveRatio, AveError, RatioDenom, SNRa, SNRb;
  ostringstream oss;
  RatioAndError NextRatioAndError;
  vector <RatioAndError> RtnData;
  bool LinkFound, NoAppend = false;
  unsigned int NumLinesCompared = 0;

  // Compare each of the loaded experimental spectra to the others. First add
  // null scaling for the reference spectrum. Then assess what factors are
  // needed to put the other spectra on the same scale as the reference spectrum.
  NextRatioAndError.Ratio = 1.0;
  NextRatioAndError.Error = 0.0;
  NextRatioAndError.a = SpectrumOrder[0];
  NextRatioAndError.b = SpectrumOrder[0];
  RtnData.push_back (NextRatioAndError);
  
  if (OrderedPairs.size () > 1) {
    for (unsigned int k = 0; k < OrderedPairs.size () - 1; k ++) {
      for (unsigned int j = 1; j < OrderedPairs.size (); j ++) {
        if (j > k) {
        
          // Add a new row to the comparison list and initialise it
          if (!NoAppend) {
            parentRow = *(modelDataComp -> append ());
          }
          NoAppend = false;
          AveRatio = 0.0;
          AveError = 0.0;
          RatioDenom = 0.0;
          LinkFound = false;
          // First check to see if the spectra have been explicitly linked together by the user.
          // In such cases ALL lines common to both spectra may be used to calculate the transfer
          // ratio rather than only the lines belonging to the current upper level.
          for (unsigned int i = 0; i < LinkedSpectra.size (); i ++) {
            try {
              if (LinkedSpectra [i].a == SpectrumOrder[k] &&
                LinkedSpectra [i].b == SpectrumOrder[j]) {
                LinkFound = true;
                NextRatioAndError = 
                  compareLinkedSpectra (LinkedSpectra [i].a, LinkedSpectra [i].b, parentRow);
              } else if (LinkedSpectra [i].b == SpectrumOrder[k] &&
                LinkedSpectra [i].a == SpectrumOrder[j]){
                LinkFound = true;
                NextRatioAndError = 
                  compareLinkedSpectra (LinkedSpectra [i].b, LinkedSpectra [i].a, parentRow);
              }
              if (LinkFound) {
                parentRow[colsDataComp.ref] = SpectrumLabels[k];
                parentRow[colsDataComp.comparison] = SpectrumLabels[j];
                parentRow[colsDataComp.wavenumber] = "";
                oss.str (""); 
                oss << NextRatioAndError.Ratio << " +/- " << NextRatioAndError.Error;
                parentRow[colsDataComp.ratio] = oss.str ();
                RtnData.push_back (NextRatioAndError);
              }
              break;
            } catch (int e) {
              if (e == NO_COMPARISON_LINES_FOUNDS) {
                NoAppend = true;
                break;
              }
            }
          }
          // If the spectra have not been linked by the user only the common lines belonging to the
          // current upper level may be used to calculate the transfer ratio.
          if (!LinkFound) {
            NumLinesCompared = 0;
            // Compare matching lines in each of the two spectra
            for (unsigned int i = 0; i < OrderedPairs[k].size (); i ++) {
              if (OrderedPairs[k][i]->xgLine->wavenumber () > 0.0 && !OrderedPairs[k][i]->plot->disabled()
                && OrderedPairs[j][i]->xgLine->wavenumber () > 0.0 && !OrderedPairs[j][i]->plot->disabled()) {
                
                // Add a new child row for the comparison of individual lines and
                // fill it with the required information
                row = *(modelDataComp -> append (parentRow->children()));
                row[colsDataComp.ref] = SpectrumLabels[k];
                row[colsDataComp.comparison] = SpectrumLabels[j];
                row[colsDataComp.wavenumber] = "";

                Ratio = (OrderedPairs [k][i]->xgLine->eqwidth () 
                  / ExptSpectra[SpectrumOrder[k]].response (OrderedPairs[k][i]->xgLine->wavenumber ()))
                  / (OrderedPairs [j][i]->xgLine->eqwidth ()
                  / ExptSpectra[SpectrumOrder[j]].response (OrderedPairs[j][i]->xgLine->wavenumber ()));
                
                if (Options.correct_snr ()) {
                  SNRa = OrderedPairs [k][i]->xgLine->snr () / OrderedPairs [k][i]->xgLine->noise();
                  SNRb = OrderedPairs [j][i]->xgLine->snr () / OrderedPairs [j][i]->xgLine->noise();
                } else {
                  SNRa = OrderedPairs [k][i]->xgLine->snr ();
                  SNRb = OrderedPairs [j][i]->xgLine->snr ();
                }

                // Calculate the weighted average of the transfer ratio of each common line
                Error = sqrt(pow (SNRa, -2.0) + pow (SNRb, -2.0));
                AveRatio += Ratio / pow (Error, 2.0);
                RatioDenom += pow (Error, -2.0);
                AveError += 1.0 / Error;
                oss.str (""); oss << Ratio << " +/- " << Ratio * Error;
                row[colsDataComp.ratio] = oss.str ();
                oss.str (""); oss << OrderedPairs [k][i]->xgLine->wavenumber();
                row[colsDataComp.wavenumber] = oss.str ();
                NumLinesCompared ++;
              }
            }

            if (NumLinesCompared > 0) {
              // Update the root node with the overall comparison information
              AveRatio /= RatioDenom;
              AveError /= RatioDenom;
              parentRow[colsDataComp.ref] = SpectrumLabels[k];
              parentRow[colsDataComp.comparison] = SpectrumLabels[j];
              parentRow[colsDataComp.wavenumber] = "";
              oss.str (""); oss << AveRatio << " +/- " << AveRatio * AveError;
              parentRow[colsDataComp.ratio] = oss.str ();
              NextRatioAndError.Ratio = AveRatio;
              NextRatioAndError.Error = AveError;
              NextRatioAndError.a = SpectrumOrder[k];
              NextRatioAndError.b = SpectrumOrder[j];
              RtnData.push_back (NextRatioAndError);
            } else {
              NoAppend = true;
            }
          }
        }
      }
    }
  }
  if (NoAppend) { modelDataComp -> erase (parentRow); }
  return RtnData;
}


//------------------------------------------------------------------------------
// updatePlottedData () : 
//
void AnalyserWindow::updatePlottedData (bool CalcScaleFactors) {
  ostringstream oss;
  oss.precision (2);
  oss << fixed;
  // Check the current selection is valid. Only proceed if it is.
  Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeLevelsBF.get_selection();
  if (treeSelection) {
    Gtk::TreeModel::iterator iter = treeLevelsBF.get_selection()->get_selected();
    if (iter) {

      // Update the list of XGremlin lines and the list of br. frac. data
      int Level = (*iter)[levelCols.index];
      modelDataXGr -> clear ();
      modelDataBF -> clear ();
      if (CalcScaleFactors) {
        modelDataComp -> clear ();
      }
      if (LevelLines[Level].size () > 0) {
        vector < vector <LinePair *> > OrderedPairs;
        vector <LinePair *> NextPairSet;
        vector <string> SpectrumLabels;
        vector <unsigned int> SpectrumOrder;
        vector <unsigned int> LinesToPlot;
        unsigned int RefIndex = 0;
        

        // Look at each line in turn and see if it is plotted in at least one spectrum.
		// If so, add its index to LinesToPlot. This is effectively scanning through each
		// COLUMN in the line profile plot area to make sure something is visible.
		for (unsigned int j = 0; j < LevelLines[Level][0].size (); j ++) {
			for (unsigned int i = 0; i < LevelLines[Level].size (); i ++) {
				if (LevelLines[Level][i][j].xgLineLineIndex != -1 && !LevelLines[Level][i][j].plot->hidden()) {
					LinesToPlot.push_back (j);
					break;
				}
			}
		}

        // Find the reference spectrum. This will be plotted first. Add it's details to the beginning of the
        // SpectrumLabels and SpectrumOrder vectors so that it is always referenced first.
        for (unsigned int i = 0; i < ExptSpectra.size (); i ++) {
          if (ExptSpectra[i].isReference()) {
            RefIndex = i;
            break;
          }
        }
        SpectrumLabels.push_back (ExptSpectra [RefIndex].index());
        SpectrumOrder.push_back (RefIndex);

        // Now copy all of the lines belonging to the currently selected level in the reference spectrum to
        // NextPairSet.
        for (unsigned int i = 0; i < LinesToPlot.size (); i ++){
          // Create NextPairSet from the LevelLines matrix
          NextPairSet.push_back (&LevelLines[Level][RefIndex][LinesToPlot[i]]);

          // Add a note to the plot to say what the response function value is at the wavenumber of the
          // current line. This will be a number between 0 and 1.
          oss << "C=" << ExptSpectra [RefIndex].response (LevelLines[Level][RefIndex][LinesToPlot[i]].xgLine->wavenumber ());
          LevelLines[Level][RefIndex][LinesToPlot[i]].plot -> clearText ();
          LevelLines[Level][RefIndex][LinesToPlot[i]].plot -> addText (45, 15, oss.str ());
		  oss.str ("");
        }

		// Finally, add NextPairSet to OrderedPairs for future use.
        OrderedPairs.push_back (NextPairSet);

        // Now repeat the above steps to add the lines from all the other spectra to OrderedPairs
        for (unsigned int i = 0; i < LevelLines[Level].size (); i ++) {
          NextPairSet.clear ();
          if (i != RefIndex) {
            for (unsigned int j = 0; j < LinesToPlot.size (); j ++) {
              // Create NextPairSet from the LevelLines matrix
              NextPairSet.push_back (&LevelLines[Level][i][LinesToPlot[j]]);

			  // Add a note to the plot to say what the response function value is at the wavenumber of the
			  // current line. This will be a number between 0 and 1.
              oss << "C=" << ExptSpectra [i].response (LevelLines[Level][i][LinesToPlot[j]].xgLine->wavenumber ());
              LevelLines[Level][i][LinesToPlot[j]].plot -> clearText ();
              LevelLines[Level][i][LinesToPlot[j]].plot -> addText (45, 15, oss.str ());
              oss.str ("");
            }

            // Add the details for the current spectrum to the end of the SpectrumLabels, SpectrumOrder and OrderedPairs vectors.
            SpectrumLabels.push_back (ExptSpectra [i].index());
            SpectrumOrder.push_back (i);
            OrderedPairs.push_back (NextPairSet);
          }
        }

        plotLines (OrderedPairs, SpectrumOrder);
        if (CalcScaleFactors) {
          ScalingFactors = updateComparisonList (OrderedPairs, SpectrumLabels, SpectrumOrder);
        }
        updateXGremlinList (OrderedPairs, SpectrumLabels, SpectrumOrder);
        updateBranchingFractions (OrderedPairs, SpectrumLabels, SpectrumOrder);
      }

      // Update the list of Kurucz lines
      updateKuruczList (KuruczList.upperLevel(Level));
    }
  }
}


//------------------------------------------------------------------------------
// refreshKuruczList () : Clears the information currently displayed in the two
// "Kurucz Upper Levels" tabs and refreshes them to reflect any recent changes
// to the loaded Kurucz data.
//
void AnalyserWindow::refreshKuruczList () {
  KzList LineList;
  Gtk::TreeModel::Row row, rowBF;

  // Clear the data currently in the "Levels" and "Br. Frac." tabs
  levelTreeModel -> clear ();
  modelLevelsBF -> clear ();
  
  // Refresh each upper level in turn
  for (unsigned int i = 0; i < KuruczList.numUpperLevels (); i ++) {

    // Append new rows for the current level  
    row = *(levelTreeModel->append());
    rowBF = *(modelLevelsBF->append());

    // Insert the general Kurucz data into the "Levels" tab    
    LineList = KuruczList.upperLevel(i);    
    row[levelCols.index] = i;
    row[levelCols.name] = LineList.name ();
    row[levelCols.lifetime] = LineList.lines()[0].lifetime () * 1e9;
    row[levelCols.err_lifetime] = LineList.lines()[0].lifetime_error () * 1e9;    
    row[levelCols.fracFound] = 0.0;
    row[levelCols.lifetime_colour] = Gdk::Color (AW_EQWIDTH_NO_NORM_COLOUR);
    row[levelCols.err_lifetime_colour] = Gdk::Color (AW_EQWIDTH_NO_NORM_COLOUR);
    
    // Insert generic data in the "Br. Frac." tab. This will be updated in
    // updateKuruczCompleteness () later on
    rowBF[levelColsBF.index] = i;
    rowBF[levelColsBF.branches] = LineList.lines().size ();
    rowBF[levelColsBF.a_total] = 0.0;
    rowBF[levelColsBF.a_missing] = 0.0;
    rowBF[levelColsBF.ew_total] = 0.0;
    rowBF[levelColsBF.ew_norm] = 0.0;

    // Determine which Kurucz list column contains the upper level data then
    // add it to each of the two tabs
    if (KuruczList.upperLevel(i).lines()[0].eUpper () > 
      KuruczList.upperLevel(i).lines()[0].eLower ()) {
      row[levelCols.jupper] = LineList.lines()[0].jUpper ();
      row[levelCols.eupper] = LineList.lines()[0].eUpper ();
      row[levelCols.config] = LineList.lines()[0].configUpper ();
      rowBF[levelColsBF.jupper] = LineList.lines()[0].jUpper ();
      rowBF[levelColsBF.eupper] = LineList.lines()[0].eUpper ();
      rowBF[levelColsBF.config] = LineList.lines()[0].configUpper ();
    } else {
      row[levelCols.jupper] = LineList.lines()[0].jLower ();
      row[levelCols.eupper] = LineList.lines()[0].eLower ();
      row[levelCols.config] = LineList.lines()[0].configLower ();
      rowBF[levelColsBF.jupper] = LineList.lines()[0].jLower ();
      rowBF[levelColsBF.eupper] = LineList.lines()[0].eLower ();
      rowBF[levelColsBF.config] = LineList.lines()[0].configLower ();
    }
  }
  
  // Perform the calculations to determine how much of each upper level
  // is seen, and what the associated branching fraction parameters are.
  updateKuruczCompleteness ();
  
  // Finally, force the treeview objects on both tabs to be shown and drawn.
  // This enables subsequent set_cursor call on click events to be handled by
  // both the main tab and the branching fraction tab.
  bookLevels.set_current_page (1);
  bookLevels.set_current_page (0);
}


//------------------------------------------------------------------------------
// refreshSpectraList () : Clears the information currently displayed in the  
// "Experimental Spectra" list and then uses refreshSpectraList (XgSpectrum, 
// int, bool, bool) below to refresh it. By default, the first item in the list
// is selected as the reference spectrum.
//
void AnalyserWindow::refreshSpectraList () {
  if (ExptSpectra.size () > 0) {
    m_refTreeModel -> clear ();
    addToSpectraList (ExptSpectra [0], 0, ExptSpectra[0].isReference (), true);
    for (unsigned int i = 1; i < ExptSpectra.size (); i ++) {
      addToSpectraList (ExptSpectra [i], i, ExptSpectra[i].isReference (), false);
    }

    // Finally, perform the calculations to determine how much of each upper level
    // is seen, and what the associated level branching fraction parameters are.
    getLinePairs ();
    updateKuruczCompleteness ();
  }
}


//------------------------------------------------------------------------------
// refreshSpectraList (XgSpectrum, int, bool, bool) : Adds data from the
// XgSpectrum passed in at arg 1 to the "Experimental Spectra" list. The Index
// at arg 2 is the spectrum's array index in ExptSpectra, and is stored in the
// list to allow data to be extracted from ExptSpectra later on. If Ref is true
// the spectrum's reference box will be ticked. If Select is true, the spectrum
// will be highlighted.
//
void AnalyserWindow::addToSpectraList (XgSpectrum NewSpectrum, 
  int Index, bool Ref = false, bool Select = true) 
{

  // Add a new row to the spectra list and fills in basic spectrum information
  Gtk::TreeModel::Row parentRow = *(m_refTreeModel->append());
  parentRow[m_Columns.name] = NewSpectrum.name();
  parentRow[m_Columns.emin] = int(NewSpectrum.data()[0].x + 0.5);
  parentRow[m_Columns.emax] = int(NewSpectrum.data()[NewSpectrum.data().size () - 1].x + 0.5);
  parentRow[m_Columns.index] = Index;
  parentRow[m_Columns.label] = NewSpectrum.index();  
  
  // Set line_index to -1 to indicate that the current row does not contain a
  // line list.
  parentRow[m_Columns.line_index] = -1;
  
  // parentRow currently points to a root spectrum line, so set it's colour to
  // AW_SPECTRUM_COLOUR
  parentRow[m_Columns.bg_colour] = Gdk::Color (AW_SPECTRUM_COLOUR);
  
  // If Ref is true, tick the "Ref" tick box.
  if (Ref) {
    parentRow[m_Columns.ref] = true;
  } else {
    parentRow[m_Columns.ref] = false;
  }

  // If Select is true, force the current row to be selected
  if (Select) {
    Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeSpectra.get_selection();
    if (treeSelection) {
      treeSelection->select(parentRow);
    }
  }

  // Now append child rows to the "Experimental Spectra" list for each of the
  // XGremlin line lists associated with the current spectrum. Colour them with
  // AW_LINELIST_COLOUR and use the line_list value to store the array index of
  // the current line list within the current XgSpectrum object.
  for (unsigned int i = 0; i < NewSpectrum.lines().size (); i ++) {
    Gtk::TreeModel::Row row = *(m_refTreeModel->append(parentRow->children()));
    row[m_Columns.name] = string(ExptSpectra[Index].lines()[i][0].name ());
    bool Ref = parentRow[m_Columns.ref];
    row[m_Columns.index] = Index;
    row[m_Columns.line_index] = i;
    row[m_Columns.ref] = Ref;
    row[m_Columns.emin] = ExptSpectra[Index].lines()[i][0].wavenumber ();
    row[m_Columns.emax] = ExptSpectra[Index].lines()[i][ExptSpectra[Index].lines()[i].size () - 1].wavenumber ();
    row[m_Columns.bg_colour] = Gdk::Color (AW_LINELIST_COLOUR);
  }

  // After all the line lists have been displayed, add a child entry for the
  // standard lamp spectrum (if one exists). Colour it with AW_RESPONSE_COLOUR
  // and set its line_index to -1 as it isn't a line list.
  if (NewSpectrum.standard_lamp_spectrum ().size () > 0) {
    Gtk::TreeModel::Row row = *(m_refTreeModel->append(parentRow->children()));
    row[m_Columns.name] = string(ExptSpectra[Index].standard_lamp_file ());
    bool Ref = parentRow[m_Columns.ref];
    row[m_Columns.index] = Index;
    row[m_Columns.line_index] = -1;
    row[m_Columns.ref] = Ref;
    row[m_Columns.emin] = ExptSpectra[Index].standard_lamp_spectrum()[0].x;
    row[m_Columns.emax] = ExptSpectra[Index].standard_lamp_spectrum()
      [ExptSpectra[Index].standard_lamp_spectrum().size () - 1].x;
    row[m_Columns.bg_colour] = Gdk::Color (AW_RESPONSE_COLOUR);
  }

  // Next, add a child entry for the standard lamp radiance (if one exists). 
  // Colour it with AW_RESPONSE_COLOUR and set its line_index to -1 as it isn't
  // a line list.
  if (NewSpectrum.radiance ().size () > 0) {
    Gtk::TreeModel::Row row = *(m_refTreeModel->append(parentRow->children()));
    row[m_Columns.name] = string(ExptSpectra[Index].radiance_file ());
    bool Ref = parentRow[m_Columns.ref];
    row[m_Columns.index] = Index;
    row[m_Columns.line_index] = -1;
    row[m_Columns.ref] = Ref;
    row[m_Columns.emin] = ExptSpectra[Index].radiance()[0].x;
    row[m_Columns.emax] = ExptSpectra[Index].radiance()[ExptSpectra[Index].radiance().size () - 1].x;
    row[m_Columns.bg_colour] = Gdk::Color (AW_RESPONSE_COLOUR);
  }
  
  // Finally, add a child entry for any links this spectrum has to other loaded
  // spctra. Colour it with AW_LINK_COLOUR and set its line_index to -1 as it
  // isn't a line list.
  for (unsigned int i = 0; i < LinkedSpectra.size (); i ++) {
    if (LinkedSpectra [i].b == (unsigned int)Index) {
      Gtk::TreeModel::Row row = *(m_refTreeModel->append(parentRow->children()));
      ostringstream oss;
      oss << "Linked to " << ExptSpectra [LinkedSpectra [i].a].name ();
      row[m_Columns.name] = oss.str();
      bool Ref = parentRow[m_Columns.ref];
      row[m_Columns.index] = Index;
      row[m_Columns.line_index] = -1;
      row[m_Columns.ref] = Ref;
      row[m_Columns.emin] = 0;
      row[m_Columns.emax] = 0;
      row[m_Columns.bg_colour] = Gdk::Color (AW_LINK_COLOUR);
    }
  }
}
