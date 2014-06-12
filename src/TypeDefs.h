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
// TypeDefs.h
//==============================================================================
// This file contains definitions that are used throughout FAST.
#ifndef FAST_TYPEDEFS
#define FAST_TYPEDEFS

#include <string>
#include <gdkmm/color.h>

#include "xgline.h"
#include "kzline.h"
#include "linedata.h"

// The current version of FAST is named here. This is printed on startup and in
// the FAST help->about window.
#define FAST_VERSION "0.7.0"

// Define an FTS file version. This is stored in the FTS file and can be used
// for backward compatibility at a later date should a new file version be
// created.
#define FTS_FILE_VERSION 3
#define FTS_FILE_VERSION_UP_TO_0_6_5   2

// The number of data points contained in the synthetic Voigt profiles.
#define NUM_VOIGT_POINTS 200

// The minimum branching fraction considered to be significant.
#define MIN_SIGNIFICANT_BF 0.01

// Widget colour definitions for both Target and Experiment line lists.
#define AW_SPECTRUM_COLOUR "#FFFFFF"
#define AW_LINELIST_COLOUR "#FFFFAF"
#define AW_RESPONSE_COLOUR "#AFFFAF"
#define AW_LINK_COLOUR     "#AFAFFF"
#define AW_EQWIDTH_NORM_COLOUR "#CFFFCF"
#define AW_EQWIDTH_NO_NORM_COLOUR "#FFCFCF"
#define AW_EQWIDTH_SOME_NORM_COLOUR "#FFFFAF"
#define AW_CHILD_LINE_COLOUR "#DFDFDF"
#define AW_PARENT_LINE_COLOUR "#FFFFFF"
#define AW_CHILD_EQWIDTH_NORM_COLOUR "#ADDDAD"
#define AW_CHILD_EQWIDTH_NO_NORM_COLOUR "#DDADAD"
#define AW_CHILD_EQWIDTH_SOME_NORM_COLOUR "#DDDD8D"

// FAST configuration file in the user's home directory
#define FAST_CONFIG_FILE ".fastrc"
#define NUM_RECENT_FILES 4

// Default file save names for project exports
#define AW_DEF_TARGETS_NAME "targets.txt"

// Any line starting with IO_COMMENT will be ignored in text I/O routines
#define IO_COMMENT '#'

// Occasionally, a user may want to fit a line near the target line if it is partially blended.
// To prevent FAST using this line in its calculations, that line may be designed a FAKE LINE
// by giving it the following tag on loading.
#define FAKE_LINE_TAG "*Fake Line*                     "

// Fields to include in the outputwindow
#define NUM_BF_OUTPUT_FIELDS 15
#define NUM_TARGET_OUTPUT_FIELDS 9
#define NUM_FITTED_OUTPUT_FIELDS 9
const std::string FAST_BF_FIELD_NAMES [NUM_BF_OUTPUT_FIELDS] =  {"Spectrum Tag", 
  "Line Index", "Wavenumber", "Intensity", "U (S/N) / %", "U (Cal.) / %", "U (Trans.) / %",
  "U (Total) / %", "U (Int.)", "Branching Fraction", "U (Br. Frac.) / %",
  "Transition Probability x 10^{6}", "U (Tr. Prob.) / %", "log (gf)", "U (log(gf)) / dex" };
const std::string FAST_TARGET_FIELD_NAMES [NUM_TARGET_OUTPUT_FIELDS] = {
  "Wavelength", "Est. log(gf)", "Est. Branching Fraction", 
  "E Lower", "J Lower", "Config Lower", "E Upper", "J Upper", "Config Upper" };
const std::string FAST_FITTED_FIELD_NAMES [NUM_FITTED_OUTPUT_FIELDS] = {
  "Line", "Label", "Peak", "Width", "Damping",
  "U (Total)", "U (Even)", "U (Odd)", "U (Rand)" };

// Define a LinePair structure that permits target lines to be linked to their
// observed profiles. Include a plot of the observed line.
typedef struct line_pair {
  XgLine *xgLine;
  KzLine *kzLine;
  LineData *plot;
  int xgLineListIndex, xgLineLineIndex;
} LinePair;

// RatioAndError typedef, used for calculating the transfer ratios and errors
// needed to link a given intensity calibration across multiple spectra.
typedef struct ratio_and_error {
  unsigned int a, b;
  double Ratio;
  double Error;
} RatioAndError;

typedef struct type_data_bf {
  int index;
  double wavenumber, eqwidth, err_line, err_cal, err_trans, err_total, 
    err_eqwidth, br_frac, err_br_frac, a, err_a, loggf, dex;
  std::string spectrum;
  LineData *profile;
  Gdk::Color bg_colour, eq_width_colour, err_cal_colour;
} DataBF;


#endif // FAST_TYPEDEFS
