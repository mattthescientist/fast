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
// lineio.cpp
//==============================================================================
// Contains routines for reading from and writing to XGremlin data files, which
// are used by ftscommonlines, ftscalibrate, ftslinefilter, and ftsplot. The
// primary file format used is the XGremlin 'writelines' format, but additional
// write routines, writeSynLines(...), enable line data to be saved in the 'syn'
// format for use by XGremlin's 'readlines' command.
//
// On input, readLineList(...) extracts the lines from an XGremlin 'writelines'
// file and stores each in a Line object. Conversely, on output, a vector of 
// Line objects is passed to either writeLines(...) or writeSynLines(...) and 
// written in 'writelines' or 'syn' format respectively.
// 
#ifndef LINE_IO_CPP
#define LINE_IO_CPP

#define XG_WRITELINES_HEADER_LENGTH 4 /* rows */
#define XG_WAVCORR_OFFSET 33
#define LIN_HEADER_SIZE 320 /* bytes */
#define LIN_RECORD_SIZE 80 /* bytes */

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "ErrDefs.h"
#include "xgline.h"
#include "voigtlsqfit.h"

// A namespace to store the header from the XGremlin writelines file. This can
// then be used to copy the header to the output line list in writeLines().
namespace writelines_header {
  string WaveCorr;
  string AirCorr;
  string IntCal;
  string Columns;
}

// In XGremlin's lineio.f, the layout of a .lin file record is explained:
// 
//"* variable    type           size/bytes
// * --------    ----           ----------
// * sig         real*2         8
// * xint        real           4
// * width       real           4
// * dmping      real           4
// * itn         integer*2      2
// * ihold       integer*2      2
// * tags        character*4    4
// * epstot      real           4
// * epsevn      real           4
// * epsodd      real           4
// * epsran      real           4
// * spare       real           4
// * ident       character*32   32"
// 
// struct line_in replicates this record structure, allowing individual lines
// to be read directly from a .lin file in the readLinFile() function below.
//
typedef struct line_io {
  double wavenumber;
  float peak;
  float width;
  float dmp;
  short itn;
  short ihold;
  char tags [5];  // Add a char for '\0' terminator
  float epstot;
  float epsevn;
  float epsodd;
  float epsran;
  float spare;
  char id [33];   // Add a char for '\0' terminator
} LineIO;

double getWavCorr (string HeaderLine) throw (Error);
vector <XgLine> readLineList (string Filename) throw (Error);
vector <XgLine> readLinFile (string LinFile) throw (Error);
void readLinFileError (Error Err, int Line) throw (Error);
void writeLines (vector <XgLine> Lines, ostream &Output) throw (const char*);
void writeLines (vector <XgLine> Lines, string Filename) throw (int);
void writeSynLines (vector <XgLine> Lines, string Filename) throw (int);

//------------------------------------------------------------------------------
// getWavCorr (string) : Extracts the wavenumber scaling factor from an XGremlin
// 'writelines' header. If no scaling was applied to a line list, a value of
// zero is returned.
//
double getWavCorr (string HeaderLine) throw (Error) {
  istringstream iss;
  string NextField;
  char Rubbish [XG_WAVCORR_OFFSET];
  double WavCorr = 0.0;
  
  iss.str (HeaderLine);
  iss >> NextField;
  if (NextField == "NO") return 0.0;
  else if (NextField == "WAVENUMBER") iss.get (Rubbish, XG_WAVCORR_OFFSET);
  else { 
    throw (Error (FLT_FILE_READ_ERROR));
  }
  iss >> WavCorr;
  return WavCorr;
}
    

//------------------------------------------------------------------------------
// readLineList (string, vector <Line>) : Opens and reads an XGremlin writelines
// line list. The string from each individual row in the ascii file is passed to
// the Line object constructor, which extracts the line parameters. The
// resulting Line object is added to the Line vector at arg2, which, being 
// passed in by reference, is returned to the calling function.
//
vector <XgLine> readLineList (string Filename) throw (Error) {
  ostringstream oss, osssub;
  istringstream iss;
  string LineString;
  double WavCorr = 0.0;
  XgLine NewLine;
  vector <XgLine> Lines;
  double NextField;
  unsigned int LineCount = XG_WRITELINES_HEADER_LENGTH;
  bool ErrorFound = false;
  int DmpInt;
  float DmpFraction;
  VoigtLsqfit V;
  
  // Open the specified line list and abort if it cannot be read.
  ifstream ListFile (Filename.c_str(), ios::in);
  if (! ListFile.is_open()) {
    oss << "Error: Cannot read" << Filename;
    osssub << "Check the file exists and that you have read permission" << endl;
    throw Error (FLT_FILE_OPEN_ERROR, oss.str (), osssub.str ());
  }
  
  // Extract the data from the ASCII line list. First asssume that the input is
  // an XGremlin writelines file and search for the header information.
  try {
    getline (ListFile, writelines_header::WaveCorr); // wavenumber correction
    WavCorr = getWavCorr (writelines_header::WaveCorr);
    if (ListFile.fail()) throw(Error (FLT_FILE_READ_ERROR));
    getline (ListFile, writelines_header::AirCorr);  // air correction
    if (ListFile.fail()) throw(Error (FLT_FILE_READ_ERROR));
    getline (ListFile, writelines_header::IntCal);   // intensity calibration
    if (ListFile.fail()) throw(Error (FLT_FILE_READ_ERROR));
    getline (ListFile, writelines_header::Columns);  // column headers
    if (ListFile.fail()) throw(Error (FLT_FILE_READ_ERROR));

    while (!ListFile.eof ()) {
      LineCount ++;
      getline (ListFile, LineString);
      if (LineString[0] != '\0') {
        Lines.push_back (XgLine (LineString, WavCorr));
        Lines [Lines.size () - 1].name (Filename.substr (Filename.find_last_of ("/\\") + 1));
      }
    }
  
  // Failed to read an XGremlin header from the file, so assume it is a FAST
  // ASCII line list.
  } catch (Error *e) {
    ListFile.seekg (ios::beg);
    LineCount = 0;
    while (!ListFile.eof ()) {
      getline (ListFile, LineString);
      // Check to see if the next line is a comment. Ignore it if it is.
      if (LineString[0] != XGREMLIN_COMMENT && LineString[0] != IO_COMMENT &&
        LineString[0] != '\0') {
        iss.clear ();  // Clear any error flags
        LineCount ++;
        NewLine.line (LineCount);
        iss.str (LineString);
        iss >> NextField; if (iss.fail ()) { ErrorFound = true; break;}
        NewLine.wavenumber (NextField);
        iss >> NextField; if (iss.fail ()) { ErrorFound = true; break;}
        NewLine.peak (NextField);          
        iss >> NextField; if (iss.fail ()) { ErrorFound = true; break;}
        NewLine.width (NextField);
        iss >> NextField; if (iss.fail ()) { ErrorFound = true; break;}
        NewLine.dmp (NextField);
        if (iss.good ()) {
          NewLine.id (iss.str().substr(iss.tellg (), iss.str().size () - 1));
        }
        NewLine.name (Filename.substr (Filename.find_last_of ("/\\") + 1));
        // Calculate the equivalent width of the line using XGremlin's mystical
        // "p" array, as shown in subroutine wrtlin in lineio.f
        DmpInt = int ((NewLine.dmp () * 25.0) + 1);
        DmpFraction = (NewLine.dmp () * 25.0) + 1 - float (DmpInt);
        if (DmpInt == 26) {
          NewLine.eqwidth (V.P (26));
        } else {
          NewLine.eqwidth (V.P(DmpInt) + DmpFraction*(V.P(DmpInt+1)-V.P(DmpInt)));
        }
        NewLine.eqwidth(NewLine.eqwidth() * NewLine.width() * NewLine.peak());
        Lines.push_back (NewLine);
      }
    }
    if (ErrorFound) {
      oss << "Error: Cannot read line data from " << Filename;
      osssub << "Check the file is written in the correct format" << endl;
      ListFile.close ();
      throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
    }
  }
  ListFile.close ();
  return (Lines);
}


//------------------------------------------------------------------------------
// readLinFile (string) : Reads line data from an XGremlin LIN file. This is a
// binary file as opposed to an ASCII line list.
//
vector <XgLine> readLinFile (string LinFile) throw (Error) {
  ifstream LinIn;
  int NumLines; //FileSize, ResidualBytes;
  float Scale, SigCorrection;
  LineIO NextLineIn;
  XgLine NextLine;
  vector <XgLine> RtnLines;
  VoigtLsqfit V;
  string LinFileNoDirectory = LinFile.substr(LinFile.find_last_of ("/\\") + 1);
  LinIn.open (LinFile.c_str (), ios::in|ios::binary);
  if (!LinIn.is_open ()) {
    ostringstream oss, osssub;
    oss << "Error opening " << LinFileNoDirectory;
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_OPEN_ERROR, oss.str (), osssub.str ());
  }

  // Read the necessary information from the LIN file header.
  LinIn.read ((char*)&NumLines, sizeof (int));
  LinIn.seekg (sizeof (int) + sizeof (float), ios::cur);
  LinIn.read ((char*)&Scale, sizeof (float));
  LinIn.read ((char*)&SigCorrection, sizeof (float));
  if (!LinIn.good ()) {
    ostringstream oss, osssub;
    oss << "Error reading basic list details from " << LinFileNoDirectory;
    osssub << "The file may be corrupt. Try rewriting it with XGremlin.";
    throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
  }
  
  // Check the file size matches that expected for the given number of lines.
  // This will give a good indication of whether or not the file is corrupt, or
  // even if a non-LIN file has been selected.
/*  LinIn.seekg (0, ios::end);
  FileSize = LinIn.tellg ();
  ResidualBytes = FileSize - LIN_HEADER_SIZE - (NumLines * LIN_RECORD_SIZE);
  if (ResidualBytes != 0) {
    ostringstream oss, osssub;
    oss << "Cannot attach " << LinFileNoDirectory << " as it doesn't appear to be a LIN file";
    osssub << "If it is, the file may be corrupt. Try rewriting it with XGremlin.";
    throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
  }*/
  
  // Move to the location of the first line then extract all the line records.
  LinIn.seekg (LIN_HEADER_SIZE, ios::beg);
  for (int i = 0; i < NumLines; i ++) {
    LinIn.read ((char*)&NextLineIn.wavenumber, sizeof (double));
    LinIn.read ((char*)&NextLineIn.peak, sizeof (float));
    LinIn.read ((char*)&NextLineIn.width, sizeof (float));
    LinIn.read ((char*)&NextLineIn.dmp, sizeof (float));
    LinIn.read ((char*)&NextLineIn.itn, sizeof (short));
    LinIn.read ((char*)&NextLineIn.ihold, sizeof (short));
    LinIn.read ((char*)&NextLineIn.tags, sizeof (char) * 4);
    LinIn.read ((char*)&NextLineIn.epstot, sizeof (float));
    LinIn.read ((char*)&NextLineIn.epsevn, sizeof (float));
    LinIn.read ((char*)&NextLineIn.epsodd, sizeof (float));
    LinIn.read ((char*)&NextLineIn.epsran, sizeof (float));
    LinIn.read ((char*)&NextLineIn.spare, sizeof (float));
    LinIn.read ((char*)&NextLineIn.id, sizeof (char) * 32);
    NextLineIn.tags [4] = '\0';
    NextLineIn.id [32] = '\0';
    
    // Only continue to read this line if a) there was no read error and b) the
    // tags field is not set to "   F" to indicate that the XGremlin fit failed.
    if (LinIn.good ()) {
      if (string(NextLineIn.tags) != "   F") {
        NextLine.line (i + 1);
        NextLine.itn (NextLineIn.itn);
        NextLine.h (NextLineIn.ihold);
        try {
          NextLine.wavenumber (NextLineIn.wavenumber);
          NextLine.peak (NextLineIn.peak);
          NextLine.width (NextLineIn.width);
        } catch (Error *Err) {
          readLinFileError (*Err, i + 1);
        }
        NextLine.dmp ((NextLineIn.dmp - 1.0) / 25.0);
        NextLine.tags (NextLineIn.tags);
        NextLine.epstot (NextLineIn.epstot);
        NextLine.epsevn (NextLineIn.epsevn);
        NextLine.epsodd (NextLineIn.epsodd);
        NextLine.epsran (NextLineIn.epsran);
        NextLine.spare (NextLineIn.spare);
        NextLine.id (string (NextLineIn.id));
        NextLine.name (LinFile.substr (LinFile.find_last_of ("/\\") + 1));

        // Calculate the equivalent width of the line using XGremlin's mystical
        // "p" array, as shown in subroutine wrtlin in lineio.f
        int DmpInt = int (NextLineIn.dmp);
        float DmpFraction = NextLineIn.dmp - float (DmpInt);
        try {
          if (DmpInt == 26) {
            NextLine.eqwidth (V.P (26));
          } else {
            NextLine.eqwidth (V.P(DmpInt) + DmpFraction*(V.P(DmpInt+1)-V.P(DmpInt)));
          }
          NextLine.eqwidth(NextLine.eqwidth() * NextLine.width() * NextLine.peak());
        } catch (Error *Err) {
          readLinFileError (*Err, i + 1);
        }
        RtnLines.push_back (NextLine);
      }
    }
    else {
      ostringstream oss, osssub;
      oss << "Error extracting data from " << LinFileNoDirectory;
      osssub << "The file may be corrupt. Try rewriting it with XGremlin.";
      throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
    }    
  }
  LinIn.close ();
  return RtnLines;
}


//------------------------------------------------------------------------------
// readLinFileError (Error, int) : Process any error code generated while
// reading an XGremlin LIN file in the readLinFile (string) function above. The
// result is two strings containing error messages that can be displated in the
// FAST interface.
//
void readLinFileError (Error Err, int Line) throw (Error) {
  ostringstream oss, osssub;
  switch (Err.code) {
    case (LINE_NEGATIVE_WAVENUMBER):
      oss << "Negative wavenumber found for line " << Line;
      osssub << "Line wavenumbers must be positive. Refit this line." << endl;
      break;
    case (LINE_NEGATIVE_PEAK):
      oss << "Negative peak found for line " << Line;
      osssub << "Line peak amplitudes must be positive. Refit this line." << endl;
      break;
    case (LINE_NEGATIVE_WIDTH):
      oss << "Negative width found for line " << Line;
      osssub << "Line widths must be positive. Refit this line." << endl;
      break;
    case (LINE_NEGATIVE_EQWIDTH):
      oss << "Negative integrated intensity (eq. width) found for line " << Line;
      osssub << "Integrated line intensities must be positive. Refit this line." << endl;
      break;
    case (LINE_NEGATIVE_WAVELENGTH):
      oss << "Negative wavelength found for line " << Line;
      osssub << "Line wavelengths must be positive. Refit this line." << endl;
      break;
    case (LINE_INVALID_DAMPING):
      oss << "Invalid line damping parameter found for line " << Line;
      osssub << "Check the terminal output for more details." << endl;
      break;
    default:
      oss << "An unexpected error has been encountered (code " << Err.code << ")";
      break;
  }
  throw Error (Err.code, oss.str (), osssub.str ());
}


//------------------------------------------------------------------------------
// readLinHeader (string) : Reads the header from a the LIN file specified at
// arg1 and returns it.
//
vector <char> readLinFileHeader (string LinFile) throw (Error) {
  ifstream LinIn;
  vector <char> LinHeader;
  char c;
  
  LinIn.open (LinFile.c_str (), ios::in|ios::binary);
  if (!LinIn.is_open ()) {
    ostringstream oss, osssub;
    oss << "Error opening " << LinFile;
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_OPEN_ERROR, oss.str (), osssub.str ());
  }
  for (unsigned int i = 0; i < LIN_HEADER_SIZE; i ++) {
    LinIn.read ((char*)&c, sizeof (char));
    LinHeader.push_back (c);
  }
  return LinHeader;
}


//------------------------------------------------------------------------------
// writeLinFile (string) : Reads the header from a the LIN file specified at
// arg1 and returns it.
//
void writeLinFile (string LinFile, vector <char> LinHeader, 
  vector <XgLine> Lines) throw (Error) {
  ofstream LinOut;
  LinOut.open (LinFile.c_str (), ios::out|ios::binary);
  if (!LinOut.is_open ()) {
    ostringstream oss, osssub;
    oss << "Error opening " << LinFile << " for output";
    osssub << "Check you have permission to write to this location";
    throw Error (FLT_FILE_WRITE_ERROR, oss.str (), osssub.str ());
  }
  for (unsigned int i = 0; i < LinHeader.size (); i ++) {
    LinOut.write ((char*)&LinHeader[i], sizeof (char));
  }
  
  LineIO NextLineOut;
  for (unsigned int i = 0; i < Lines.size (); i ++) {
    NextLineOut.itn = Lines[i].itn ();
    NextLineOut.ihold = Lines[i].h ();
    NextLineOut.wavenumber = Lines[i].wavenumber ();
    NextLineOut.peak = Lines[i].peak ();
    NextLineOut.width = Lines[i].width ();
    NextLineOut.dmp = (Lines[i].dmp () * 25.0) + 1;
    strncpy (NextLineOut.tags, Lines[i].tags ().c_str(), 4);
    NextLineOut.epstot = Lines[i].epstot ();
    NextLineOut.epsevn = Lines[i].epsevn ();
    NextLineOut.epsodd = Lines[i].epsodd ();
    NextLineOut.epsran = Lines[i].epsran ();
    NextLineOut.spare = Lines[i].spare ();
    strncpy (NextLineOut.id, Lines[i].id ().c_str(), 32);
    LinOut.write ((char*)&NextLineOut.wavenumber, sizeof (double));
    LinOut.write ((char*)&NextLineOut.peak, sizeof (float));
    LinOut.write ((char*)&NextLineOut.width, sizeof (float));
    LinOut.write ((char*)&NextLineOut.dmp, sizeof (float));
    LinOut.write ((char*)&NextLineOut.itn, sizeof (short));
    LinOut.write ((char*)&NextLineOut.ihold, sizeof (short));
    LinOut.write ((char*)&NextLineOut.tags, sizeof (char) * 4);
    LinOut.write ((char*)&NextLineOut.epstot, sizeof (float));
    LinOut.write ((char*)&NextLineOut.epsevn, sizeof (float));
    LinOut.write ((char*)&NextLineOut.epsodd, sizeof (float));
    LinOut.write ((char*)&NextLineOut.epsran, sizeof (float));
    LinOut.write ((char*)&NextLineOut.spare, sizeof (float));
    LinOut.write ((char*)&NextLineOut.id, sizeof (char) * 32);
  }
  LinOut.close ();
}


//------------------------------------------------------------------------------
// writeLines (vector <Line>, ostream) : Requests the XGremlin writelines string
// from each Line in the vector at arg1 and sends this string to the stream at
// arg2.
//
void writeLines (vector <XgLine> Lines, ostream &Output = std::cout) throw (const char*) {
  if (Lines[0].wavCorr () != 0.0) {
    Output << "  WAVENUMBER CORRECTION APPLIED: wavcorr =   " 
      << Lines[0].wavCorr () << endl;
  }
  else {
    Output << writelines_header::WaveCorr << endl;
  }
  Output << writelines_header::AirCorr << endl;
  Output << writelines_header::IntCal << endl;
  Output << "  line    wavenumber      peak    width      dmp   eq width   itn   H tags     epstot     epsevn     epsodd     epsran  identification" << endl;

  if (Output.fail()) throw "the file header";
  for (unsigned int i = 0; i < Lines.size (); i ++) {
    Output << Lines[i].getLineString() << endl;
    if (Output.fail ()) {
      ostringstream oss;
      oss << "line " << Lines[i].line ();
      throw oss.str().c_str();
    }
  }
}

//------------------------------------------------------------------------------
// writeLines (vector <Line>, string) : Creates an output file stream from
// the filename specified at arg2, then calls writeLines (vector <Line>,
// ostream) to output the XGremlin writelines data to this file.
//
void writeLines (vector <XgLine> Lines, string Filename) throw (int) {
  ofstream ListFile (Filename.c_str(), ios::out);
  if (! ListFile.is_open()) {
    cout << "Error: Cannot open " << Filename 
      << " for output. List writing ABORTED." << endl;
    throw int (FLT_FILE_OPEN_ERROR);
  }
  try {
    writeLines (Lines, ListFile);
  } catch (const char *Err) {
    cout << "Error writing " << Err << " to " << Filename << 
      ". List writing ABORTED." << endl;
    throw int (FLT_FILE_WRITE_ERROR);
  }
}


//------------------------------------------------------------------------------
// writeSynLines (vector <Line>, ostream) : Requests the XGremlin 'syn' string
// from each Line in the vector at arg1 and sends this string to the stream at
// arg2.
//
void writeSynLines (vector <XgLine> Lines, ostream &Output = std::cout) throw (const char*) {
  for (unsigned int i = 0; i < Lines.size (); i ++) {
    Output << Lines[i].getLineSynString() << endl;
    if (Output.fail ()) {
      ostringstream oss;
      oss << "line " << Lines[i].line ();
      throw oss.str().c_str();
    }
  }
}

//------------------------------------------------------------------------------
// writeSynLines (vector <Line>, string) : Creates an output file stream from
// the filename specified at arg2, then calls writeSynLines (vector <Line>,
// ostream) to output the XGremlin 'syn' data to this file.
//
void writeSynLines (vector <XgLine> Lines, string Filename) throw (int) {
  ofstream ListFile (Filename.c_str(), ios::out);
  if (! ListFile.is_open()) {
    cout << "Error: Cannot open " << Filename 
      << " for output. List writing ABORTED." << endl;
    throw int (FLT_FILE_OPEN_ERROR);
  }
  try { 
    writeSynLines (Lines, ListFile);
  } catch (const char *Err) {
    cout << "Error writing " << Err << " to " << Filename << 
      ". List writing ABORTED." << endl;
    throw int (FLT_FILE_WRITE_ERROR);
  }
}

#endif // LINE_IO_CPP
