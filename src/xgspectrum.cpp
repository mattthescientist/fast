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
// XgSpectrum class (xgspectrum.cpp)
//==============================================================================

#include "xgspectrum.h"

//------------------------------------------------------------------------------
// Default constructor : Initialises class variables and prepares the GSL spline
// environment for use in calculating a radiance spline later on.
//
XgSpectrum::XgSpectrum () {
  Name = "";
  Index = "";
  Step = 0.0;
  IsReference = false;
  RadianceSplineCreated = false;
  B = 0; bw = 0; c = 0; r = 0; x = 0; y = 0; X = 0; cov = 0; mw = 0; w = 0;
  gsl_rng_env_setup();
}


//------------------------------------------------------------------------------
// Default destructor : Delete the GSL spline environment if it has been used to
// create a radiance spline
//
XgSpectrum::~XgSpectrum () {
  if (RadianceSplineCreated) {
    freeSplineEnvironment ();
  }
}


//------------------------------------------------------------------------------
// linesVector () : Returns a copy of the all the lines stored in the Lines
// vector. The individual sub-lists are merged into one long list.
//
vector <XgLine> XgSpectrum::linesVector () {
  vector <XgLine> RtnLines;
  for (unsigned int i = 0; i < Lines.size (); i ++) {
    for (unsigned int j = 0; j < Lines[i].size (); j ++) {
      RtnLines.push_back (Lines [i][j]);
    }
  }
  return RtnLines;
}


//------------------------------------------------------------------------------
// data (vector <Coord>) : Sets the spectrum's data using the input Coords
//
void XgSpectrum::data (vector <Coord> NewData) {
  Data = NewData;
  Step = (Data [Data.size () - 1].x - Data[0].x) / (Data.size () - 1);
}


//------------------------------------------------------------------------------
// data (vector <Coord>) : Adds a new data point to the end of the spectrum
//
void XgSpectrum::data_push_back (Coord a) { 
  Data.push_back (a);
  Step = (Data [Data.size () - 1].x - Data[0].x) / (Data.size () - 1);
}

//------------------------------------------------------------------------------
// Coord data () : 
//
Coord XgSpectrum::data (int Index) throw (Error) {
  if (Index >= 0 && Index < (int)Data.size ()) {
    return Data [Index];
  } else {
    throw (XGSPEC_OUT_OF_BOUNDS);
  }
}
  


//------------------------------------------------------------------------------
// clear () : Removes all data from the spectrum and resets class variables
//
void XgSpectrum::clear () {
  Data.clear(); 
  Lines.clear ();
  Plots.clear (); 
  LinHeaders.clear ();
  Response.clear ();
  StdLampSpectrum.clear ();
  Radiance.clear ();
  RadianceErrors.clear ();
  Name = "";
  Index = "";
  Step = 0.0;
  IsReference = false;
  RadianceFile = "";
  StandardLampFile = "";
}


//------------------------------------------------------------------------------
// data (uint a, uint b) : Returns all the data points between a and b
//
vector <Coord> XgSpectrum::data (int a, int b) throw (Error){
  vector <Coord> RtnPoints;

  if (a < 0 || a >= (int)Data.size ()) {
    throw (Error (XGSPEC_OUT_OF_BOUNDS, "At least one line is outside the spectrum range",
      "Check the contents of this LIN file in XGremlin and remove these invalid lines")); 
  }
  if (b < 0 || b >= (int)Data.size ()) {
    throw (Error (XGSPEC_OUT_OF_BOUNDS, "At least one line is outside the spectrum range",
      "Check the contents of this LIN file in XGremlin and remove these invalid lines")); 
  }

  for (int i = a; i < b; i ++) {
    RtnPoints.push_back (Data[i]);
  }
  return RtnPoints;
}


//------------------------------------------------------------------------------
// data (double, double) : Returns all the data points within a given Width
// around a stated Centre point. This is useful when obtaining data to plot in
// the vicinity of a line.
//
vector <Coord> XgSpectrum::data (double Centre, double Width) {
  if (Data.size () != 0) {
    int XStart, XEnd;
    XStart = int((Centre - Data[0].x - Width) / Step);
    XEnd = int ((Centre - Data[0].x + Width) / Step) + 1;
    return data (XStart, XEnd);
  } else {
    vector <Coord> RtnPoints;
    RtnPoints.push_back (Coord ());
    cout << "Warning: Data requested from an empty spectrum." << endl;
    return RtnPoints;
  }
}


//------------------------------------------------------------------------------
// response (double) : Returns the calculated response function at x. Linear
// interpolation is used when x does not lie exactly on a response function data
// point. Spline interpolation (as is used on the radiance data) is unnecessary
// as the response function and line spectrum should have a similar number of
// data points at similar spacing.
//
double XgSpectrum::response (double x) /*throw (string)*/ {
  if (Response.size () == 0) calculateResponseFunction ();
  if (Response.size () > 0) {
    if (x >= Response [0].x && x <= Response [Response.size() - 1].x) {
      int XStart, XEnd;
      XStart = 0; XEnd = Response.size () - 1;
      while (abs (XStart - XEnd) > 1) {
        if (x > Response [(XEnd - XStart) / 2 + XStart].x) {
          XStart = (XEnd - XStart) / 2 + XStart;
        } else {
          XEnd = (XEnd - XStart) / 2 + XStart;
        }
      }
      return (x - Response[XStart].x)/(Response[XEnd].x - Response[XStart].x) *
        (Response[XEnd].y - Response[XStart].y) + Response[XStart].y;
    } else {
      return 0.0;
    }
  }
  return 1.0;
}


//------------------------------------------------------------------------------
// standard_lamp_spectrum (double) : Returns the measured intensity of the 
// stored standard lamp spectrum at position x.
//
double XgSpectrum::standard_lamp_spectrum (double x) /*throw (string)*/ {
  if (StdLampSpectrum.size () > 0) {
    if (x >= StdLampSpectrum [0].x && x <= StdLampSpectrum [StdLampSpectrum.size() - 1].x) {
      int XStart, XEnd;
      XStart = 0; XEnd = StdLampSpectrum.size () - 1;
      while (abs (XStart - XEnd) > 1) {
        if (x > StdLampSpectrum [(XEnd - XStart) / 2 + XStart].x) {
          XStart = (XEnd - XStart) / 2 + XStart;
        } else {
          XEnd = (XEnd - XStart) / 2 + XStart;
        }
      }
      return (x - StdLampSpectrum[XStart].x)/(StdLampSpectrum[XEnd].x - StdLampSpectrum[XStart].x) *
        (StdLampSpectrum[XEnd].y - StdLampSpectrum[XStart].y) + StdLampSpectrum[XStart].y;      
    }
  }
  return 0.0;
}


//------------------------------------------------------------------------------
// response_error (double) : Returns the error in the response function at a
// given wavenumber.
//
double XgSpectrum::response_error (double Wavenumber) {
  double SNRError = standard_lamp_spectrum (Wavenumber);
  double CalError = radiance_error (Wavenumber);
  if (SNRError != 0.0) {
    SNRError = (1.0 / SNRError) * 100.0;
  }
  return sqrt (pow (SNRError, 2) + pow (CalError, 2));
}


//------------------------------------------------------------------------------
// radiance_error (double) : Returns the error in the stored radiance data at a
// given wavenumber. This will be determined from the information specified in
// a previously loaded RAD file.
//
double XgSpectrum::radiance_error (double Wavenumber) {
  double Wavelength = (1.0 / Wavenumber) * 1.0e7;
  for (unsigned int i = 0; i < RadianceErrors.size (); i ++) {
    if (Wavelength >= RadianceErrors[i].min 
      && Wavelength <= RadianceErrors[i].max) {
      return RadianceErrors [i].err;
    }
  }
  return 0.0;
}


//------------------------------------------------------------------------------
// radiance (string) : Loads the radiance data specified in the RAD file passed
// in at arg1. This includes the loading of the associated radiance errors. Once
// the data has been loaded, calculateRadianceSpline() is called to create a
// spline representation of the radiance data for later use.
//
void XgSpectrum::radiance (string RadIn) throw (Error) {
  ostringstream oss, osssub;
  string RadFileNoDirectory = RadIn.substr(RadIn.find_last_of ("/\\") + 1);
  ifstream RadFile (RadIn.c_str (), ios::in);
  bool ErrorsLoaded = false;
  if (RadFile.is_open ()) {
    istringstream iss;
    string NextLine;
    Coord NextCoord;
    getline (RadFile, NextLine);
    Radiance.clear ();
    RadianceErrors.clear ();
    while (!RadFile.eof ()) {
      iss.clear ();
      iss.str (NextLine);
      if (iss.str()[0] == 'U' || iss.str()[0] == 'u') {
        try {
          radiance_errors (RadFile);
          ErrorsLoaded = true;
        } catch (Error *Err) {
          if (Err->code == FLT_FILE_READ_ERROR) {
            Radiance.clear ();
            RadianceErrors.clear ();
            oss << "Error reading radiance errors from " << RadFileNoDirectory;
            osssub << "Check the file is written in the correct RAD format and is not corrupt.";
            throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
          } else if (Err->code == XGSPEC_NO_RAD_UNCERTAINTIES) {
            ErrorsLoaded = false;
          }
        }
        break;
      }
      else if (iss.str()[0] != '#') {    // Skip comment lines in header
        iss >> NextCoord.x >> NextCoord.y;
        if (!iss.fail ()) {
          NextCoord.y = log (NextCoord.y);
          Radiance.push_back (NextCoord);
          iss.clear ();
        } else {
          Radiance.clear ();
          RadianceErrors.clear ();
          oss << "Error reading radiance data from " << RadFileNoDirectory;
          osssub << "Check the file is written in the correct RAD format and is not corrupt.";
          throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
        }
      }
      getline (RadFile, NextLine);
    }
    RadFile.close ();
    RadianceFile = RadFileNoDirectory;
    calculateRadianceSpline ();
    if (!ErrorsLoaded) {
      oss << "Warning: No calibration uncertainties were found in " << RadFileNoDirectory;
      osssub << "Check the file is written in the correct RAD format.";
      throw Error (XGSPEC_NO_RAD_UNCERTAINTIES, oss.str (), osssub.str ());
    }
  } else {
    oss << "Error opening " << RadFileNoDirectory;
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_OPEN_ERROR, oss.str (), osssub.str ());
  }

}


//------------------------------------------------------------------------------
// calculateRadianceSpline () : Uses the GNU GSL spline routines to fit a spline
// to the previously loaded standard lamp radiance data. This is needed in order
// to accurately interpolate those data later on.
//
void XgSpectrum::calculateRadianceSpline () {
  unsigned int n = Radiance.size ();
  double xmin = Radiance [0].x;
  double xmax = Radiance [n - 1].x;
  unsigned int i, j;
  
  double chisq; //, Rsq, dof, tss;

  if (RadianceSplineCreated) {
    freeSplineEnvironment ();
  }
  
  // Prepare the GSL environment
  r = gsl_rng_alloc(gsl_rng_default);

  bw = gsl_bspline_alloc(4, NUM_BREAK); // allocate a cubic bspline workspace (k=4)
  B = gsl_vector_alloc(NUM_COEFFS);

  x = gsl_vector_alloc(n);
  y = gsl_vector_alloc(n);
  X = gsl_matrix_alloc(n, NUM_COEFFS);
  c = gsl_vector_alloc(NUM_COEFFS);
  w = gsl_vector_alloc(n);
  cov = gsl_matrix_alloc(NUM_COEFFS, NUM_COEFFS);
  mw = gsl_multifit_linear_alloc(n, NUM_COEFFS);
  
  for (i = 0; i < n; i ++) {
    gsl_vector_set (x, i, Radiance [i].x);
    gsl_vector_set (y, i, Radiance [i].y);
    gsl_vector_set (w, i, 1.0);
  }

  // use uniform breakpoints between xmin and xmax
  gsl_bspline_knots_uniform(xmin, xmax, bw);

  // construct the fit matrix X
  for (i = 0; i < n; ++i)
   {
     double xi = gsl_vector_get(x, i);

     // compute B_j(xi) for all j
     gsl_bspline_eval(xi, B, bw);

     // fill in row i of X
     for (j = 0; j < NUM_COEFFS; ++j)
       {
         double Bj = gsl_vector_get(B, j);
         gsl_matrix_set(X, i, j, Bj);
       }
   }

  // do the spline fit
  gsl_multifit_wlinear(X, w, y, c, cov, &chisq, mw);
//  dof = n - NUM_COEFFS;
//  tss = gsl_stats_wtss(w->data, 1, y->data, 1, y->size);
//  Rsq = 1.0 - chisq / tss;
//  printf("chisq/dof = %e, Rsq = %f\n", chisq / dof, Rsq);
  RadianceSplineCreated = true;
}


//------------------------------------------------------------------------------
// freeSplineEnvironment () : Clears all the GSL spline variables. This must be
// called by the class destructor when deleting an instance of XgSpectrum.
//
void XgSpectrum::freeSplineEnvironment () {
  gsl_rng_free(r);
  gsl_bspline_free(bw);
  gsl_vector_free(B);
  gsl_vector_free(x);
  gsl_vector_free(y);
  gsl_matrix_free(X);
  gsl_vector_free(c);
  gsl_vector_free(w);
  gsl_matrix_free(cov);
  gsl_multifit_linear_free(mw);
  RadianceSplineCreated = false;
}


//------------------------------------------------------------------------------
// radiance_errors (ifstream) : Loads the radiance errors from the RAD file
// ifstream specified at arg1.
//
void XgSpectrum::radiance_errors (ifstream &RadFile) throw (Error) {
  istringstream iss;
  string NextLine;
  ErrRange NextRange;
  getline (RadFile, NextLine);
  RadianceErrors.clear ();
  if (RadFile.eof ()) {
    throw Error (XGSPEC_NO_RAD_UNCERTAINTIES);
  }
  while (!RadFile.eof ()) {
    iss.clear ();
    iss.str (NextLine);
    if (iss.str()[0] != '#' && iss.str()[0] != '\0') {    // Skip comment lines in header
      iss >> NextRange.min >> NextRange.max >> NextRange.err;
      if (!iss.fail ()) {
        RadianceErrors.push_back (NextRange);
      } else {
        throw Error (FLT_FILE_READ_ERROR);
      }
    }
    getline (RadFile, NextLine);
  }
}


//------------------------------------------------------------------------------
// standard_lamp_spectrum (string) : Loads an XGremlin standard lamp spectrum
// from the ASCII file specified at arg1. This file must have been created with
// the XGremlin writeasc command.
//
void XgSpectrum::standard_lamp_spectrum (string StdLampIn) throw (Error) {
  ostringstream oss, osssub;
  string StdLampNoDirectory = StdLampIn.substr(StdLampIn.find_last_of ("/\\") + 1);
  ifstream StdLampFile (StdLampIn.c_str (), ios::in);
  if (StdLampFile.is_open ()) {
    istringstream iss;
    string NextLine;
    Coord NextCoord;
    getline (StdLampFile, NextLine);
    StdLampSpectrum.clear ();
    while (!StdLampFile.eof ()) {
      iss.clear ();
      iss.str (NextLine);
      if (iss.str()[0] != '#') {    // Skip comment lines in header
        iss >> NextCoord.x >> NextCoord.y;
        if (!iss.fail ()) {
          StdLampSpectrum.push_back (NextCoord);
        } else {
          StdLampSpectrum.clear ();
          oss << "Error reading standard lamp spectrum from " << StdLampNoDirectory;
          osssub << "Check the file is written in the correct format and is not corrupt.";
          throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
        }
      }
      getline (StdLampFile, NextLine);
    }
    StdLampFile.close ();
    size_t FilePos = StdLampIn.find_last_of ("/\\") + 1;
    StandardLampFile = StdLampIn.substr(FilePos);
  } else {
    oss << "Error opening " << StdLampNoDirectory;
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_OPEN_ERROR, oss.str (), osssub.str ());
  }
}


//------------------------------------------------------------------------------
// matchStandardLampResolution () : Lowers the resolution of the FTS measured
// standard lamp spectrum so that it matches that of the spectral radiance data.
//
vector <Coord> XgSpectrum::matchStandardLampResolution () {
  vector <Coord> RtnSpectrum;
  Coord NextPoint;
  if (Radiance.size () > 0 && StdLampSpectrum.size () > 0) {
    double RadStep = Radiance[1].x - Radiance[0].x;
    double SpecStep = 1e7 / StdLampSpectrum[1].x - 1e7 / StdLampSpectrum[0].x;
    int BoxcarSize = RadStep / SpecStep;
    double BoxcarAve;
    
    for (unsigned int i = 0; i < StdLampSpectrum.size () - BoxcarSize; i++) {
      BoxcarAve = 0.0;
      for (unsigned int j = i; j < i + BoxcarSize; j ++) {
        BoxcarAve += StdLampSpectrum[j].y;
      }
      NextPoint.x = 1e7 / StdLampSpectrum[i].x;
      NextPoint.y = BoxcarAve / BoxcarSize;
      RtnSpectrum.push_back (NextPoint);
    }
  }
  return RtnSpectrum;
}


//------------------------------------------------------------------------------
// calculateResponseFunction () : Calculates the spectrometer response function
// from the standard lamp data stored within the XgSpectrum object. This is
// called on the first attempt to access the response function in response ().
//
void XgSpectrum::calculateResponseFunction () {
  double ySpline, yerr, wlen, ymax, xi, yi;
  vector <Coord> ResMatchedStdLamp;
  Coord NextPoint;
  ymax = 0.0;
  if (Radiance.size () > 0 && StdLampSpectrum.size () > 0) {
    if (!RadianceSplineCreated) calculateRadianceSpline ();
    //ResMatchedStdLamp = matchStandardLampResolution ();
    ResMatchedStdLamp = StdLampSpectrum;
    Response.clear ();
    for (unsigned int i = 0; i < ResMatchedStdLamp.size (); i ++) {
      xi = ResMatchedStdLamp [i].x;
      yi = ResMatchedStdLamp [i].y;
      wlen = 1e7 / xi;    // Convert wavenumber to vacuum wavelength

      // Only proceed if xi is within the valid spline interpolation range
      NextPoint.x = xi;
      if (wlen >= Radiance [0].x && wlen <= Radiance [Radiance.size () - 1].x) {
        gsl_bspline_eval(wlen, B, bw);
        gsl_multifit_linear_est(B, c, cov, &ySpline, &yerr);
          
        // Calculate the response function based on 'photon' in XGremlin
        NextPoint.y = xi * xi * xi * yi / exp(ySpline);
//        cout << NextPoint.y << " = " << xi << "^3 * " << yi << " / exp(" << ySpline << ")" << endl;
        if (NextPoint.y > ymax) ymax = NextPoint.y;
        Response.push_back (NextPoint);
      } else {
        NextPoint.y = 0.0;
        Response.push_back (NextPoint);
      }
    }
    for (unsigned int i = 0; i < Response.size (); i ++) {
//      cout << ResMatchedStdLamp [i].y << "  " << Response[i].y << "  " << ymax << endl;
      Response[i].y /= ymax;
    }
  }
}  
  

//------------------------------------------------------------------------------
// remove_linelist (int) : Removes the line list and plots at the given Index in
// the Lines and Plots vectors.
//
void XgSpectrum::remove_linelist (int Index) {
  Lines.erase (Lines.begin () + Index); 
  for (unsigned int i = 0; i < Plots[Index].size (); i ++) {
    delete (Plots[Index][i]);
  }
  Plots.erase (Plots.begin () + Index);
}


//------------------------------------------------------------------------------
// remove_line (int, int) : Removes the line and plot at the given index (arg2)
// in the list specified at arg1.
//
void XgSpectrum::remove_line (int ListIndex, int LineIndex) {
  if (ListIndex >= 0 && ListIndex < (int)Lines.size ()) {
    if (LineIndex >= 0 && LineIndex < (int)Lines[ListIndex].size ()) {
      Lines[ListIndex].erase (Lines[ListIndex].begin () + LineIndex);
      delete (Plots[ListIndex][LineIndex]);
      Plots[ListIndex].erase (Plots[ListIndex].begin () + LineIndex);
    } else {
      cout << "ERROR: XgSpectrum::remove_line LineIndex out of bounds. No line removed." << endl;
    }
  } else {
    cout << "ERROR: XgSpectrum::remove_line ListIndex out of bounds. No line removed." << endl;
  }
}


//------------------------------------------------------------------------------
// linesPtr : Returns a vector that contains pointers to each line stored in
// Lines. This vector is arranged as a single list of lines rather than using
// the sub-list structure of the Lines vector itself. This function should be
// used with care as it gives direct access to a class variable.
//
// To return the Lines vector as it is (i.e. with sub-lists intact) use the 
// linesPtr2 () function.
//
vector < vector <XgLine *> > XgSpectrum::linesPtr () {
  vector <XgLine *> NextSet;
  vector < vector <XgLine *> > PtrLines;
  for (unsigned int i = 0; i < Lines.size (); i ++) {
    NextSet.clear ();
    for (unsigned int j = 0; j < Lines[i].size (); j ++) {
      NextSet.push_back (&Lines [i][j]);
    }
    PtrLines.push_back (NextSet);
  }
  return PtrLines;
}


//------------------------------------------------------------------------------
// loadAscii (string) : Loads an XGremlin spectrum ASCII file that has
// previously been saved with the "writeasc" command. The contents of this file
// is returned a vector <Coord>.
//
void XgSpectrum::loadAscii (string Filename) throw (Error) {
  ostringstream oss, osssub;
  string LineString;
  Coord NewPoint;
  istringstream iss;
  vector <Coord> Coords;
  int LineCounter = 0;

  string FilenameNoDirectory = Filename.substr(Filename.find_last_of ("/\\") + 1);
  ifstream XgAscii (Filename.c_str(), ios::in);
  if (!XgAscii.is_open ()) {
    oss << "Error opening " << FilenameNoDirectory;
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_OPEN_ERROR, oss.str (), osssub.str ());
  }
  Coords.clear ();
  while (!XgAscii.eof ()) {
    LineCounter ++;
    getline (XgAscii, LineString);
    if (XgAscii.fail()) {
      if (XgAscii.eof ()) break;
      oss << "Error extracting data from " << FilenameNoDirectory;
      osssub << "Ensure the file is of the correct format and try again";
      throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
    }
    if (LineString[0] != XGREMLIN_COMMENT) {
      iss.clear ();
      iss.str (LineString);
      iss >> NewPoint.x >> NewPoint.y;
      if (iss.fail()) {
        oss << "Error extracting data from " << FilenameNoDirectory << ". File loading aborted.";
        osssub << "Ensure the file is of the correct format and try again";
        throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
      }
      Coords.push_back (NewPoint);
    }
  }
  if (Coords.size () < 2) {
    oss << "Fewer than 2 data points were found in " << FilenameNoDirectory << ".";
    osssub << "A spectrum must have more data points. Please check the file and try again.";
    throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
  }
  XgAscii.close ();
  data (Coords);
  Step = (Coords[Coords.size () - 1].x - Coords[0].x) / (Coords.size () - 1);
}


//------------------------------------------------------------------------------
// loadDat (string) : Opens an XGremlin line spectrum DAT file, the path
// to which is given at arg1, and returns a vector<Coord> containing the data points contained within i
//
void XgSpectrum::loadDat (string Filename) throw (Error) {
  ostringstream oss, osssub;
  ifstream DataIn;
  float NextPoint;
  Coord NextCoord;
  vector <Coord> RtnCoords;
  int FileSize, NumDataPoints;
  double MinX;

  string FilenameNoDirectory = Filename.substr(Filename.find_last_of ("/\\") + 1);
  DataIn.open (Filename.c_str (), ios::in|ios::binary);
  if (!DataIn.is_open ()) {
    oss << "Error opening " << FilenameNoDirectory;
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_OPEN_ERROR, oss.str (), osssub.str ());
  }
  Step = getXGremlinHeaderField (Filename, DELTAX_TAG);
  MinX = getXGremlinHeaderField (Filename, XMIN_TAG);
  storeHeader (Filename);
  
  // Determine how many data points are in the file then set the get pointer to
  // the location of the first data point.
  DataIn.seekg (0, ios::end);
  FileSize = DataIn.tellg ();
  NumDataPoints = FileSize / sizeof (float);
  DataIn.seekg (0, ios::beg);
  
  // Now read in all the points from the DAT file and store them in RtnCoords.
  for (int i = 0; i < NumDataPoints; i ++) {
    DataIn.read ((char*)&NextPoint, sizeof (float));
    if (DataIn.good ()) {
      NextCoord.y = double(NextPoint);
      NextCoord.x = double(i) * Step + MinX;
      RtnCoords.push_back (NextCoord);
    }
    else {
      oss << "Error extracting data from " << Filename << ". File loading aborted.";
      osssub << "Ensure the file is of the correct format and try again";
      throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
    }    
  }
  
  // Success. Close the DAT file and return the spectrum coordinates.
  DataIn.close ();
  data (RtnCoords);
}



//------------------------------------------------------------------------------
// getXGremlinHeaderField (ifstream &, string) : Searches the XGremlin header
// file attached to the ifstream at arg1 for the variable specified at arg2.
// If found, it's value is extracted and returned as a double.
//
double XgSpectrum::getXGremlinHeaderField (string Filename, string FieldName) throw (Error) {
  ostringstream oss, osssub;
  string LineString, NextField;
  double ReturnValue;
  ifstream Header;
  istringstream iss;

  string HeaderName = Filename.substr (0, Filename.length () - 4) + string(".hdr");
  string HeaderNoDirectory = HeaderName.substr(HeaderName.find_last_of ("/\\") + 1);
  Header.open (HeaderName.c_str (), ios::in);
  if (!Header.is_open ()) {
    oss << "Error opening " << HeaderNoDirectory;
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_HEAD_ERROR, oss.str (), osssub.str ());
  }

  // Search the XGremlin header for FieldName
  do {
    getline (Header, LineString);
    iss.str (LineString);
    iss >> NextField;
    iss.clear ();
  } while (NextField != FieldName && !Header.eof ());
  
  // See if the end of file has been reached. If not, extract the variable.
  if (!Header.eof ()) {
    iss.str (LineString.substr (9, 23));
    iss >> ReturnValue;
  } else {
    oss << "Error reading '" << FieldName << "' from " << HeaderNoDirectory;
    osssub << "Check that the file is actually an XGremlin HDR file and is not corrupt.";
    throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
  }
  
  // Return the extracted XGremlin header variable.
  return ReturnValue;
}


//------------------------------------------------------------------------------
// storeHeader (string) : Stores a copy of the spectrum's HDR file.
//
void XgSpectrum::storeHeader (string Filename) throw (Error) {
  ostringstream oss, osssub;
  ifstream Header;
  char NextChar;
  string HeaderName = Filename.substr (0, Filename.length () - 4) + string(".hdr");
  Header.open (HeaderName.c_str (), ios::in|ios::binary);
  if (!Header.is_open ()) {
    oss << "Error opening '" << HeaderName << ". File loading aborted";
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
  }
  while (!Header.eof ()) {
    Header.read ((char*)&NextChar, sizeof(char));
    HeaderFile.push_back (NextChar);
  }
  Header.close ();
}


//------------------------------------------------------------------------------
//
//
void XgSpectrum::save (string Filename) throw (int) {
  ofstream BinOut;
  float NextPoint;
  ostringstream oss;
  FILE *AscOut;
  
  // If an XGremlin HDR file has previously been saved, the data must have been
  // loaded from XGremlin DAT and HDR files. Save them back to file in this 
  // format
  if (HeaderFile.size () > 0) {
    BinOut.open (Filename.c_str(), ios::out|ios::binary);
    if (!BinOut.is_open ()) throw (FLT_FILE_WRITE_ERROR);
    for (unsigned int i = 0; i < Data.size (); i ++) {
      NextPoint = Data [i].y;
      BinOut.write ((char*)&NextPoint, sizeof (float));
    }
    BinOut.close ();
    
    oss << Filename.substr (0, Filename.size () - 4) << ".hdr";
    BinOut.open (oss.str().c_str(), ios::out|ios::binary);
    if (!BinOut.is_open ()) throw (FLT_FILE_WRITE_ERROR);
    for (unsigned int i = 0; i < HeaderFile.size (); i ++) {
      BinOut.write ((char*)&HeaderFile[i], sizeof (char));
    }
    BinOut.close ();
  
  // If no XGremlin HDR file is present, the data must have been loaded from an
  // ASCII file. Therefore, output the data to file in ASCII format.
  } else {
    AscOut = fopen (Filename.c_str(), "w");
    if (AscOut == 0) throw (FLT_FILE_WRITE_ERROR);
    fprintf (AscOut, "# %s saved by FAST\n", Name.c_str ());
    for (unsigned int i = 0; i < Data.size (); i ++) {
      fprintf (AscOut, "%13.5f  %13.6e\n", Data[i].x, Data[i].y);
    }
    fclose (AscOut);
  }
}







