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
// XgSpectrum class (xgspectrum.h)
//==============================================================================
// This class describes an XGremlin spectrum. Data points can be loaded from an
// XGremlin .dat/.hdr file pair using the loadDat (string) function, or from an
// ASCII created with the writeasc command using loadAscii (string). Lists of
// lines may be added to the spectrum using the lines () and lines_push_back ()
// functions. Plot widgets for use in the FAST interface may also be stored 
// using the plots () and plots_push_back () functions.
//
// A Standard lamp spectrum and set of radiance data may also be attached to the
// XgSpectrum object in preparation for the calculation of the spectrometer
// response function. This response function can then be accessed using 
// response (double).
//
#ifndef XG_SPECTRUM_H
#define XG_SPECTRUM_H

#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstdio>
#include "xgline.h"
#include "linedata.h"

// Include the GSL headers required for spline fitting
#include <gsl/gsl_bspline.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics.h>

// Default number of spline fit coefficients
#define NUM_COEFFS  40
#define NUM_BREAK   NUM_COEFFS - 2

// Number of lines to skip at top of XGremlin HDR file to reach delw parameter.
#define XGREMLIN_HDR_TOP 17

// XGremlin header tags for required variables
#define XMIN_TAG   "wstart"
#define DELTAX_TAG "delw"
#define NUM_PTS_TAG "npo"

using namespace::std;

// Define an error range structure for use with spectral radiance uncertainties.
// The error is 'err' between 'min' and 'max'.
typedef struct err_range {
  double min, max, err;
} ErrRange;

class XgSpectrum {

  private:
    vector <Coord> Data;                  // Experimental spectrum data points
    vector < vector <XgLine> > Lines;     // XGremlin lines for this spectrum
    vector < vector <char> > LinHeaders;
    vector < vector <LineData *> > Plots; // Plot objects; one for each line
    vector <Coord> Response;              // The spectrometer response function
    vector <Coord> StdLampSpectrum;       // Measured standard lamp spectrum
    vector <Coord> Radiance;              // Standard lamp radiance data  
    vector <ErrRange> RadianceErrors;     // Standard lamp radiance uncertainties
    vector <char> HeaderFile;             // Stores a copy of the HDR file
    string Name, Index, RadianceFile, StandardLampFile;
    bool IsReference;    // True if this spectrum is the FAST reference spectrum
    double Step;
    
    // Spline fitting environment variables that are used to interpolate the
    // standard lamp spectral radiance data
    gsl_bspline_workspace *bw;
    gsl_vector *B;
    gsl_rng *r;
    gsl_vector *c, *w;
    gsl_vector *x, *y;
    gsl_matrix *X, *cov;
    gsl_multifit_linear_workspace *mw;
    bool RadianceSplineCreated;

    // Internal functions for dealing with the spectrometer response function
    void calculateRadianceSpline ();
    void calculateResponseFunction ();
    void freeSplineEnvironment ();
    vector <Coord> matchStandardLampResolution ();
    
    // Internal function reading data from an XGremlin HDR file
    double getXGremlinHeaderField(string Filename, string FieldName) throw(Error);
    
    // Private function for reading errors from an already open RAD file
    void radiance_errors (ifstream &RadFile) throw (Error);
  
  public:
    XgSpectrum ();
    ~XgSpectrum ();
    
    // Load functions for reading spectrum data from an XGremlin spectrum file.
    void loadDat (string Filename) throw (Error);
    void loadAscii (string Filename) throw (Error);
    
    // Save functions for stored data
    void save (string Filename) throw (int);
//    void saveStdLamp (string Filename = StandardLampFile) throw (int);
//    void saveRadiance (string Filename = RadianceFile) throw (int);
    void storeHeader (string Filename) throw (Error);
    
    // GET functions for spectrum data. The linesPtr and linesPtr2 functions
    // provide direct access to the class Lines vector, and so should be used
    // with care. They are present simply to allow fast access to the spectrum's
    // lines that isn't possible when passing the Lines vector by value.
    Coord data (int Index) throw (Error);
    vector <Coord> data () { return Data; }
    vector <Coord> data (int Min, int Max) throw (Error);
    vector <Coord> data (double Centre, double Width);
    vector < vector <XgLine> > lines () { return Lines; }
    vector <XgLine> linesVector ();
    vector < vector <XgLine *> > linesPtr ();
    vector < vector <XgLine> >* linesPtr2 () { return &Lines; }
    vector < vector <LineData *> > plots () { return Plots; }
    LineData* plots (int i, int j) { return Plots[i][j]; }
    vector < vector <char> > linHeaders () { return LinHeaders; }
    vector <char> headerFile () { return HeaderFile; }
    string name () { return Name; }
    string index () { return Index; }
    bool isReference () { return IsReference; }
    unsigned int numDataPoints () { return Data.size (); }
    double get_point_spacing () { return Step; }

    // Functions for accessing response function related data
    double response (double x);
    vector <Coord> response () { return Response; }
    vector <Coord> radiance () { return Radiance; }
    double response_error (double Wavenumber);
    double radiance_error (double Wavenumber);
    double standard_lamp_spectrum (double Wavenumber);
    vector <Coord> standard_lamp_spectrum () { return StdLampSpectrum; }
    vector <ErrRange> radiance_error_ranges () { return RadianceErrors; }
    string radiance_file () { return RadianceFile; }
    string standard_lamp_file () { return StandardLampFile; }
    
    // SET functions
    void data (vector <Coord> a);
    void data_push_back (Coord a);
    void lines (vector < vector <XgLine> > a ) { Lines = a; }
    void lines_push_back (vector <XgLine> a) { Lines.push_back (a); }
    void plots (vector < vector <LineData *> > a) { Plots = a; }
    void plots_push_back (vector <LineData *> a) { Plots.push_back (a); }
    void lin_headers_push_back (vector <char> a) { LinHeaders.push_back (a); }
    void headerFile (vector <char> a) { HeaderFile = a; }
    void radiance (vector <Coord> a) { Radiance = a; }
    void radiance (string RadianceIn) throw (Error);
    void radiance_errors (vector <ErrRange> a) { RadianceErrors = a; }
    void standard_lamp_spectrum (vector <Coord> a) { StdLampSpectrum = a; }
    void standard_lamp_spectrum_push_back (Coord a) { StdLampSpectrum.push_back (a); }
    void standard_lamp_spectrum (string StdLampIn) throw (Error);
    void name (string a) { Name = a; }
    void index (string a) { Index = a; }
    void isReference (bool a) { IsReference = a; }
    void radiance_file (string a) { RadianceFile = a; }
    void standard_lamp_file (string a) { StandardLampFile = a; }
    void set_point_spacing (double a) { Step = a; }
    
    // Functions for the removal of data
    void remove_line (int ListIndex, int LineIndex);
    void remove_linelist (int Index);
    void remove_radiance () { Response.clear (); Radiance.clear (); RadianceErrors.clear (); RadianceFile = ""; }
    void remove_standard_lamp_spectrum () { Response.clear (); StdLampSpectrum.clear (); StandardLampFile = ""; }
    void clear ();
};

#endif // XG_SPECTRUM_H
