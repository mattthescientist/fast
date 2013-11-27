#ifndef FAST_TYPEDEFS
#define FAST_TYPEDEFS

#include <string>
#include <gdkmm/color.h>

#include "xgline.h"
#include "kzline.h"
#include "linedata.h"

#define FAST_VERSION "0.6.3"

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

// Default file save names for project exports
#define AW_DEF_TARGETS_NAME "targets.txt"

#define IO_COMMENT '#'

#define FAKE_LINE_TAG "*Fake Line*                     "

// Fields to include in the outputwindow
#define NUM_BF_OUTPUT_FIELDS 15
#define NUM_TARGET_OUTPUT_FIELDS 9
#define NUM_FITTED_OUTPUT_FIELDS 9
const std::string FAST_BF_FIELD_NAMES [NUM_BF_OUTPUT_FIELDS] =  {"Spectrum Tag", 
  "Line Index", "Wavenumber", "Intensity", "U (S/N)", "U (Cal.)", "U (Trans.)",
  "U (Total)", "U (Int.)", "Branching Fraction", "U (Br. Frac.)",
  "Transition Probability", "U (Tr. Prob.)", "log (gf)", "U (log(gf))" };
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
