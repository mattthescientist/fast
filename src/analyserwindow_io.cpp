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
// AnalyserWindow class (analyserwindow_io.cpp)
//==============================================================================
// This file contains I/O functions for saving FAST projects to disk and
// restoring them later on.

//==============================================================================
// ANALYSERWINDOW BINARY I/O FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
// Read/write a file version tag from/to the beginning of the file. This is
// currently unused, but may be important in the future if the FTS file format
// changes and backward compatibility is needed.
// 
void AnalyserWindow::writeFileVersion (ofstream *BinOut) {
  unsigned int Version = FTS_FILE_VERSION;
  BinOut->write ((char*)&Version, sizeof (unsigned int));
}

int AnalyserWindow::readFileVersion (ifstream *BinIn) {
  unsigned int Version;
  BinIn->read ((char*)&Version, sizeof (unsigned int));
  return Version;
}


//------------------------------------------------------------------------------
// saveExptSpectra (ofstream *) : Sends the loaded experimental spectra to the
// binary ofstream passed in at arg 1. If this function is changed, care must be
// taken to ensure that loadExptSpectra (ifstream *) is modified in a similar 
// way or loading saved data will fail due to binary bit mismatches.
//
void AnalyserWindow::saveExptSpectra (ofstream *BinOut) {
  unsigned int Size;
  vector <Coord> PointsToSave;
  vector <vector <XgLine> > LinesToSave;
  vector <vector <char> > LinHeaders;
  float NextPoint, PointSpacing, MinX;

  // Determine how many experimental spectra there are
  Size = ExptSpectra.size ();
  BinOut->write ((char*)&Size, sizeof (unsigned int));
  
  // Save each experimental spectrum in turn
  for (unsigned int i = 0; i < ExptSpectra.size (); i ++) {
  
    // Determine how many data points there are in the spectrum
    PointsToSave = ExptSpectra[i].data ();
    PointSpacing = ExptSpectra[i].get_point_spacing ();
    Size = PointsToSave.size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    MinX = PointsToSave[0].x;
    BinOut->write ((char*)&MinX, sizeof (float));
    BinOut->write ((char*)&PointSpacing, sizeof (float));
    
    // Write each data point in turn
    for (unsigned int j = 0; j < PointsToSave.size (); j ++) {
      NextPoint = (float) PointsToSave [j].y;
      BinOut->write ((char*) &NextPoint, sizeof (float));
    }
    
    // Save the file header
    vector <char> HeaderFile = ExptSpectra[i].headerFile ();
    Size = HeaderFile.size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    for (unsigned int j = 0; j < Size; j ++) {
      BinOut->write ((char*) &HeaderFile[j], sizeof (char));
    }
    
    // Determine how many line lists are attached to the spectrum
    LinesToSave = ExptSpectra[i].lines();
    LinHeaders = ExptSpectra[i].linHeaders ();
    Size = LinesToSave.size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    
    // Write each line list in turn
    for (unsigned int j = 0; j < LinesToSave.size (); j ++) {
      Size = LinHeaders [j].size ();
      BinOut->write ((char*)&Size, sizeof (unsigned int));
      for (unsigned int k = 0; k < LinHeaders [j].size (); k ++) {
        BinOut->write ((char*)&LinHeaders[j][k], sizeof (char));
      }
      Size = LinesToSave[j].size ();
      BinOut->write ((char*)&Size, sizeof (unsigned int));
      for (unsigned int k = 0; k < LinesToSave [j].size (); k ++) {
        LinesToSave [j][k].save (*BinOut);
      }
    }
    
    
    // Write the standard lamp spectrum, if one exists.
    PointsToSave = ExptSpectra[i].standard_lamp_spectrum();
    Size = PointsToSave.size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    for (unsigned int j = 0; j < PointsToSave.size (); j ++) {
      BinOut->write ((char*) &PointsToSave [j], sizeof (Coord));
    }
    
    // Write the standard lamp radiance data, if any exists.
    Size = ExptSpectra[i].radiance().size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    for (unsigned int j = 0; j < ExptSpectra[i].radiance().size (); j ++) {
      BinOut->write ((char*) &ExptSpectra[i].radiance() [j], sizeof (Coord));
    }
    
    // Write the standard lamp radiance uncertainties, if any exist.
    Size = ExptSpectra[i].radiance_error_ranges().size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    for (unsigned int j = 0; j < Size; j ++) {
      BinOut->write ((char*) &ExptSpectra[i].radiance_error_ranges()[j], sizeof (ErrRange));
    }
    
    // Finally, save the name, standard lamp name, index, and reference status 
    // of the spectrum
    Size = ExptSpectra[i].name().size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    char NameOut [Size];
    strcpy (NameOut, ExptSpectra[i].name().c_str());
    BinOut->write (NameOut, sizeof(char) * Size);

    Size = ExptSpectra[i].standard_lamp_file().size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    char StandardLampFileOut [Size];
    strcpy (StandardLampFileOut, ExptSpectra[i].standard_lamp_file().c_str());
    BinOut->write (StandardLampFileOut, sizeof(char) * Size);
    
    Size = ExptSpectra[i].radiance_file().size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    char RadianceFileOut [Size];
    strcpy (RadianceFileOut, ExptSpectra[i].radiance_file().c_str());
    BinOut->write (RadianceFileOut, sizeof(char) * Size);

    Size = ExptSpectra[i].index().size ();
    BinOut->write ((char*)&Size, sizeof (unsigned int));
    char IndexOut [Size];
    strcpy (IndexOut, ExptSpectra[i].index().c_str());
    BinOut->write (IndexOut, sizeof(char) * Size);

    bool Ref = ExptSpectra[i].isReference();
    BinOut->write ((char*)&Ref, sizeof(bool));
  }
  
  unsigned int LinkSize = LinkedSpectra.size ();
  BinOut->write ((char*)&LinkSize, sizeof (unsigned int));
  for (unsigned int i = 0; i < LinkedSpectra.size (); i ++) {
    BinOut->write ((char*)&LinkedSpectra [i].a, sizeof (unsigned int));
    BinOut->write ((char*)&LinkedSpectra [i].b, sizeof (unsigned int));
  }
}


//------------------------------------------------------------------------------
// loadExptSpectra (ifstream *) : Reads experimental spectrum information from
// the ifstream at arg 1 that was previously saved with saveExptSpectra above.
//
void AnalyserWindow::loadExptSpectra (ifstream *BinIn) {
  unsigned int NumSpectra, DataSize, LinesSize, NumLists, StrSize, StdLampSize,
    RadianceSize, RadErrSize, HeaderSize, LinHeaderSize;
  XgLine NextLine;
  XgSpectrum NextSpectrum;
  Coord NextCoord;
  ErrRange NextError;
  vector <Coord> LoadedPoints;
  vector <XgLine> NextLineSet;
  vector <char> NextLinHeader;
  vector <ErrRange> LoadedErrors;
  float NextPoint, PointSpacing, MinX;
  char NextChar;
  
  // Determine how many experimental spectra there are and either load each in
  // turn. If there are no spectra, only the NumSpectra will be loaded.
  BinIn->read ((char*)&NumSpectra, sizeof(unsigned int));
  cout << "NumSpectra: " << NumSpectra << endl;
  // Load each experimental spectrum in turn
  for (unsigned int i = 0; i < NumSpectra; i ++) {
    NextSpectrum.clear ();
    
    // Determine how many data points there are in the spectrum
    BinIn->read ((char*)&DataSize, sizeof (unsigned int));
    BinIn->read ((char*)&MinX, sizeof (float));
    BinIn->read ((char*)&PointSpacing, sizeof (float));
    NextSpectrum.set_point_spacing (PointSpacing);
    
    // Read each data point into NextSpectrum
    LoadedPoints.clear ();
    for (unsigned int j = 0; j < DataSize; j ++) {
      BinIn->read ((char*)&NextPoint, sizeof (float));
      NextCoord.x = (double(j) * PointSpacing) + double(MinX);
      NextCoord.y = NextPoint;
      LoadedPoints.push_back (NextCoord);
    }
    NextSpectrum.data (LoadedPoints);
    
    // Load the file header
    vector <char> HeaderFile;
    BinIn->read ((char*)&HeaderSize, sizeof (unsigned int));
    for (unsigned int j = 0; j < HeaderSize; j ++) {
      BinIn->read ((char*) &NextChar, sizeof (char));
      HeaderFile.push_back (NextChar);
    }
    NextSpectrum.headerFile (HeaderFile);

    // Determine how many line lists are attached to the spectrum
    BinIn->read ((char*)&NumLists, sizeof (unsigned int));
    // Read each line list in turn into NextSpectrum
    for (unsigned int j = 0; j < NumLists; j ++) {
      BinIn->read ((char*)&LinHeaderSize, sizeof (unsigned int));
      NextLinHeader.clear ();
      for (unsigned int k = 0; k < LinHeaderSize; k ++) {
        BinIn->read ((char*)&NextChar, sizeof (char));
        NextLinHeader.push_back (NextChar);
      }
      BinIn->read ((char*)&LinesSize, sizeof (unsigned int));
      NextLineSet.clear ();
      for (unsigned int k = 0; k < LinesSize; k ++) {
        NextLine.load (*BinIn);
        NextLineSet.push_back (NextLine);
      }
      addNewLines (&NextSpectrum, NextLineSet);
      NextSpectrum.lin_headers_push_back (NextLinHeader);
    }
    
    // Read the standard lamp spectrum, if one exists
    LoadedPoints.clear ();
    BinIn->read ((char*)&StdLampSize, sizeof (unsigned int));
    for (unsigned int j = 0; j < StdLampSize; j ++) {
      BinIn->read ((char*)&NextCoord, sizeof (Coord));
      LoadedPoints.push_back (NextCoord);
    }      
    NextSpectrum.standard_lamp_spectrum (LoadedPoints);

    // Read the standard lamp radiance data, if any exists
    LoadedPoints.clear ();
    BinIn->read ((char*)&RadianceSize, sizeof (unsigned int));
    for (unsigned int j = 0; j < RadianceSize; j ++) {
      BinIn->read ((char*)&NextCoord, sizeof (Coord));
      LoadedPoints.push_back (NextCoord);
    }
    NextSpectrum.radiance (LoadedPoints);
    
    // Read the standard lamp radiance uncertainties, if any exist
    LoadedErrors.clear ();
    BinIn->read ((char*)&RadErrSize, sizeof (unsigned int));
    for (unsigned int j = 0; j < RadErrSize; j ++) {
      BinIn->read ((char*)&NextError, sizeof (ErrRange));
      LoadedErrors.push_back (NextError);
    }
    NextSpectrum.radiance_errors (LoadedErrors);
    
    // Finally, read the name, response file name, index, and reference status 
    // of the spectrum
    BinIn->read ((char*)&StrSize, sizeof (unsigned int));
    char NameIn [StrSize + 1];
    BinIn->read (NameIn, sizeof(char) * StrSize);
    NameIn [StrSize] = '\0';
    NextSpectrum.name (string (NameIn));

    BinIn->read ((char*)&StrSize, sizeof (unsigned int));
    char StdLampIn [StrSize + 1];
    BinIn->read (StdLampIn, sizeof(char) * StrSize);
    StdLampIn [StrSize] = '\0';
    NextSpectrum.standard_lamp_file (string (StdLampIn));

    BinIn->read ((char*)&StrSize, sizeof (unsigned int));
    char RadianceIn [StrSize + 1];
    BinIn->read (RadianceIn, sizeof(char) * StrSize);
    RadianceIn [StrSize] = '\0';
    NextSpectrum.radiance_file (string (RadianceIn));

    BinIn->read ((char*)&StrSize, sizeof (unsigned int));
    char IndexIn [StrSize + 1];
    BinIn->read (IndexIn, sizeof(char) * StrSize);
    IndexIn [StrSize] = '\0';
    NextSpectrum.index (string (IndexIn));

    bool Ref;
    BinIn->read ((char*)&Ref, sizeof(bool));
    NextSpectrum.isReference (Ref);
    ExptSpectra.push_back (NextSpectrum);
  }
  
  unsigned int LinkSize;
  TypeLinkSpectra NextLink;
  BinIn->read ((char*)&LinkSize, sizeof (unsigned int));
  for (unsigned int i = 0; i < LinkSize; i ++) {
    BinIn->read ((char*)&NextLink.a, sizeof (unsigned int));
    BinIn->read ((char*)&NextLink.b, sizeof (unsigned int));
    LinkedSpectra.push_back (NextLink);
  }
  
  // Finally, refresh the list of spectra to display the newly loaded data.
  refreshSpectraList ();
}


//------------------------------------------------------------------------------
// saveKuruczList (ofstream *) : Sends the loaded Kurucz spectra to the binary
// ofstream passed in at arg 1. If this function is changed, care must be taken
// to ensure that loadKuruczList (ifstream *) is modified in a similar way or
// loading saved data will fail due to binary bit mismatches.
//
void AnalyserWindow::saveKuruczList (ofstream *BinOut) {
  unsigned int Size;
  KzLine NextLine;
  vector <KzLine> LinesToSave;
  
  // Determine how many lines are in the Kurucz list and save each in turn. If
  // there are no lines, write a zero value for "Size" then return.
  LinesToSave = KuruczList.lines ();
  Size = LinesToSave.size ();
  BinOut->write ((char*)&Size, sizeof (unsigned int));
  if (Size != 0) {
    for (unsigned int i = 0; i < Size; i ++) {
      LinesToSave [i].save (*BinOut);
    }
    
    // Save the name of the KuruczList
    Size = KuruczList.name ().size ();
    char NameOut [Size];
    strcpy (NameOut, KuruczList.name().c_str());
    BinOut->write ((char*)&Size, sizeof(unsigned int));
    BinOut->write (NameOut, sizeof(char) * Size);
    
    // Save the Kurucz list level precision variable
    double PrecisionOut = KuruczList.levelPrecision ();
    BinOut->write ((char*)&PrecisionOut, sizeof(double));
  }
}


//------------------------------------------------------------------------------
// loadKuruczList (ifstream *) : Reads Kurucz list information from the ifstream
// at arg 1 that was previously saved with saveKuruczList above.
//
void AnalyserWindow::loadKuruczList (ifstream *BinIn) {
  unsigned int Size, NameSize;
  KzLine NextLine;
  double PrecisionIn;
  vector <KzLine> Lines;

  // Determine how many lines are in the Kurucz list and either load each in
  // turn or abort if there are no Kurucz lists available.
  BinIn->read ((char*)&Size, sizeof (unsigned int));
  if (Size == 0) return;  
  for (unsigned int i = 0; i < Size; i ++) {
    NextLine.load (*BinIn);
    Lines.push_back (NextLine);
  }
  KuruczList.push_back (Lines);

  // Load the name of the KuruczList
  BinIn->read ((char*)&NameSize, sizeof (unsigned int));
  char NameIn [NameSize + 1];
  BinIn->read (NameIn, sizeof(char) * NameSize);
  NameIn [NameSize + 1] = '\0';
  KuruczList.name (NameIn);

  // Load the Kurucz list level precision variable
  BinIn->read ((char*)&PrecisionIn, sizeof(double));
  KuruczList.levelPrecision (PrecisionIn);
  
  // Finally, refresh the list of Kurucz lines to display the newly loaded data
  getLinePairs ();
  refreshKuruczList ();
}


//------------------------------------------------------------------------------
// saveInterface (ofstream) : Saves interface settings to the project file
// attached to the ofstream at arg1.
void AnalyserWindow::saveInterface (ofstream *BinOut) {
  bool Selected, CorrectSignalToNoise;
  for (unsigned int Level = 0; Level < LevelLines.size (); Level ++) {
    for (unsigned int i = 0; i < LevelLines[Level].size (); i ++) {
      for (unsigned int j = 0; j < LevelLines[Level][i].size (); j ++) {
        if (LevelLines[Level][i][j].xgLine->wavenumber () > 0.0) {
          Selected = LevelLines[Level][i][j].plot -> selected ();
          BinOut->write ((char*) &Selected, sizeof (bool));
        }
      }
    }
  }
  CorrectSignalToNoise = Options.correct_snr ();
  BinOut->write ((char*) &CorrectSignalToNoise, sizeof (bool));
}


//------------------------------------------------------------------------------
// loadInterface (ifstream) : Loads interface settings from the project file
// attached to the ifstream at arg1.
//
void AnalyserWindow::loadInterface (ifstream *BinIn, int FileVersion) {
  bool Selected, CorrectSignalToNoise;
  for (unsigned int Level = 0; Level < LevelLines.size (); Level ++) {
    for (unsigned int i = 0; i < LevelLines[Level].size (); i ++) {
      for (unsigned int j = 0; j < LevelLines[Level][i].size (); j ++) {
        if (LevelLines[Level][i][j].xgLine->wavenumber () > 0.0) {
          BinIn->read ((char*)&Selected, sizeof(bool));
          LevelLines[Level][i][j].plot -> selected (Selected);
        }
      }
    }
  }
  // FTS_FILE_VERSION 1 did not save the CorrectSignalToNoise boolean, so only
  // attempt to load this variable if the current file version is greater than 1.
  if (FileVersion > 1) {
	BinIn->read ((char*)&CorrectSignalToNoise, sizeof(bool));
	Options.set_correct_snr (CorrectSignalToNoise);
  }
  updateKuruczCompleteness ();
}
