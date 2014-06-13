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
// Kurucz List class (kzlist.cpp)
//==============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include "kzlist.h"

using namespace::std;

//==============================================================================
// CONSTRUCTORS AND DESTRUCTORS
//==============================================================================
//------------------------------------------------------------------------------
// Default constructor. Initialises class variables.
//
KzList::KzList () {
  Name = "";
  LevelPrecision = DEF_LIST_LEVEL_PRECISION;
}


//------------------------------------------------------------------------------
// KzList (std::string) constructor : Populates the new KzList object with lines
// from the Kurucz list in ListFile.
//
KzList::KzList (std::string ListFile) {
  LevelPrecision = DEF_LIST_LEVEL_PRECISION;
  std::ifstream LinesToRead (ListFile.c_str());
  read (LinesToRead);
  LinesToRead.close ();
  Name = ListFile;
}


//------------------------------------------------------------------------------
// List (std::string) constructor : Populates the new List object with lines
// from the input ifstream LinesToRead.
//
KzList::KzList (std::ifstream &LinesToRead) {
  LevelPrecision = DEF_LIST_LEVEL_PRECISION;
  read (LinesToRead);
  Name = "";
}


//------------------------------------------------------------------------------
// List (std::vector <Line>) constructor : Populates the new List object with 
// the lines in an existing vector of Line objects.
//
KzList::KzList (std::vector <KzLine> LinesIn) {
  Lines = LinesIn;
  LevelPrecision = DEF_LIST_LEVEL_PRECISION;
  setUpperLevels ();
  Name = "";
}


//==============================================================================
// PUBLIC GET FUNCTIONS
//==============================================================================
//------------------------------------------------------------------------------
// line (int) : Returns the KzLine in position Index of the Lines vector.
//
KzLine KzList::line (int Index) {
  if (Index < 0) throw ("Invalid line: index must be 0 or greater");
  if (Index > int(Lines.size()) - 1) throw ("Invalid line: index exceeds list size");
  return Lines[Index];
}


//------------------------------------------------------------------------------
// lines () : Returns the entire Lines Vector.
//
std::vector <KzLine> KzList::lines () {
  return Lines;
}


//------------------------------------------------------------------------------
// lines (double, char) : Returns all the KzLines in the Lines vector that have
// a log(gf) or branching fraction greater than or equal to Min.
//
std::vector <KzLine> KzList::lines (double Min, char Mode) throw (string) {
  std::vector <KzLine> FilteredLines;
  
  // If Min refers to a minimum log(gf) value
  if (Mode == 'g') {  
    for (unsigned int i = 0; i < Lines.size (); i ++) {
      if (Lines[i].loggf () >= Min) FilteredLines.push_back (Lines[i]);
    }
    
  // Else if Min refers to a minimum branching fraction value
  } else if (Mode == 'b') {
    for (unsigned int i = 0; i < Lines.size (); i ++) {
      if (Lines[i].brFrac () >= Min) FilteredLines.push_back (Lines[i]);
    }
  
  // Handle errors
  } else {
    throw ("Invalid mode for lines(): use 'g' for loggf filtering or 'b' for branching fractions.");
  }
  return FilteredLines;
}


//------------------------------------------------------------------------------
// name () : Returns the name of this KzList
//
std::string KzList::name () {
  return Name;
}


//------------------------------------------------------------------------------
// upperLevel (unsigned int) : Returns a sub-list that contains only the KzLines
// in the Lines vector that belong to upper level i.
//
KzList KzList::upperLevel (unsigned int i) throw (string) {
  std::vector <KzLine> RtnLines;
  
  if (i > UpperLevels.size () - 1) throw ("Error: Upper level out of bounds");
  for (unsigned int j = 0; j < UpperLevels [i].size (); j ++) {
    RtnLines.push_back (*UpperLevels [i][j]);
  }
  KzList RtnList (RtnLines);
  return RtnList;
}
  

//==============================================================================
// PUBLIC SET AND MODIFIER FUNCTIONS
//==============================================================================
//------------------------------------------------------------------------------
// name (std::string) : Sets the name of this KzList object.
//
void KzList::name (std::string NewName) {
  Name = NewName;
}


//------------------------------------------------------------------------------
// push_back (KzLine) : Appends KzLine NewLine to the end of the Lines vector.
// The upper levels are then recalculated so that they include this new line.
//
void KzList::push_back (KzLine NewLine) {
  Lines.push_back (NewLine);
  setUpperLevels ();
}


//------------------------------------------------------------------------------
// push_back (KzList) : Appends all the lines in KzList NewLines to the end of 
// the Lines vector. The upper levels are then recalculated so that they include
// these new lines.
//
void KzList::push_back (KzList NewLines) {
  for (unsigned int i = 0; i < NewLines.size (); i ++) {
    Lines.push_back (NewLines.line(i));
  }
  setUpperLevels ();
}


//------------------------------------------------------------------------------
// push_back (std::vector <KzLine>) : Appends all the lines in the NewLines 
// vector to the end of the Lines vector. The upper levels are then recalculated
// so that they include these new lines.
//
void KzList::push_back (std::vector <KzLine> NewLines) {
  for (unsigned int i = 0; i < NewLines.size (); i ++) {
    Lines.push_back (NewLines[i]);
  }
  setUpperLevels ();
}


//------------------------------------------------------------------------------
// pop_back () : Removes the last line in the Lines vector. The upper levels are
// then recalculated so that they don't include this line.
//
void KzList::pop_back () {
  Lines.pop_back ();
  setUpperLevels ();
}


//------------------------------------------------------------------------------
// insert (KzLine, int) : Inserts the KzLine NewLine into the Lines vector at
// position Position. The upper levels are then recalculated so that they 
// include this new line.
//
void KzList::insert (KzLine NewLine, int Position) {
  Lines.insert (Lines.begin() + Position, NewLine);
  setUpperLevels ();
}


//------------------------------------------------------------------------------
// erase (int) : Removes the KzLine at position Position of the Lines vector.
// The upper levels are then recalculated so that they don't include this line.
//
void KzList::erase (int Position) {
  Lines.erase (Lines.begin() + Position);
  setUpperLevels ();
}


//------------------------------------------------------------------------------
// erase (int) : Removes all KzLines from Lines vector between index First and
// index Last. The upper levels are then recalculated so that they don't include
// these lines.
//
void KzList::erase (int First, int Last) {
  Lines.erase (Lines.begin() + First, Lines.begin() + Last);
  setUpperLevels ();
}


//------------------------------------------------------------------------------
// eraseUpperLevel (int) : Removes all KzLines from Lines vector that belong to
// upper level Index. The upper levels are then recalculated so that they don't 
// include these lines.
//
void KzList::eraseUpperLevel (int Index) {
  double UpperLevel = UpperLevels[Index][0] -> energyUpper();
  for (int i = Lines.size() - 1; i >= 0; i --) {
    if (Lines[i].energyUpper () == UpperLevel) {
      Lines.erase (Lines.begin() + i);
    }
  }
  if (Lines.size () > 0) {
    setUpperLevels ();
  }
}


//------------------------------------------------------------------------------
// levelPrecision (double) : Sets the LevelPrecision class variable, which is
// used to define an allowed tolerance on upper level energies when determining
// which lines belong to which upper level.
//
void KzList::levelPrecision (double NewPrecision) {
  LevelPrecision = NewPrecision;
}


//------------------------------------------------------------------------------
// set_upper_level_lifetime (double, double) : Sets the upper level lifetime of
// all lines belonging to upper level Index.
// 
void KzList::set_upper_level_lifetime (double Index, double Lifetime) {
  double UpperLevel = UpperLevels[Index][0] -> energyUpper();
  for (int i = Lines.size() - 1; i >= 0; i --) {
    if (Lines[i].energyUpper () == UpperLevel) {
      Lines[i].lifetime (Lifetime);
    }
  }  
}


//------------------------------------------------------------------------------
// set_upper_level_lifetime_error (double, double) : Sets the upper level 
// lifetime error of all lines belonging to upper level Index.
// 
void KzList::set_upper_level_lifetime_error (double Index, double ErrLifetime) {
  double UpperLevel = UpperLevels[Index][0] -> energyUpper();
  for (int i = Lines.size() - 1; i >= 0; i --) {
    if (Lines[i].energyUpper () == UpperLevel) {
      Lines[i].lifetime_error (ErrLifetime);
    }
  }  
}


//------------------------------------------------------------------------------
// clear () : Removes all lines from the current list object
//
void KzList::clear () {
  for (unsigned int i = 0; i < UpperLevels.size (); i ++) {
    UpperLevels[i].clear ();
  }
  UpperLevels.clear ();
  Lines.clear ();
  Name = "";
}


//==============================================================================
// GENERAL FUNCTIONS
//==============================================================================
//------------------------------------------------------------------------------
// List reading functions : Extracts the lines listed in either a given file
// (as specified by its name) or a file stream. 
//
void KzList::read (std::string ListFile) throw (Error) {
  ostringstream oss, osssub;
  string ListNoDirectory = ListFile.substr(ListFile.find_last_of ("/\\") + 1);
  std::ifstream LinesToRead (ListFile.c_str());
  if (LinesToRead.is_open ()) {
    try {
      read (LinesToRead);
    } catch (Error &Err) {
      LinesToRead.close ();
      oss << "Error reading Kurucz list from " << ListNoDirectory;
      osssub << "Check the file is written in the correct format and is not corrupt.";
      throw Error (FLT_FILE_READ_ERROR, oss.str (), osssub.str ());
    }    
    LinesToRead.close ();
  } else {
    oss << "Error opening " << ListNoDirectory;
    osssub << "Check the file exists and that you have read permission.";
    throw Error (FLT_FILE_OPEN_ERROR, oss.str (), osssub.str ());
  }
}

void KzList::read (std::ifstream &LinesToRead) throw (Error) {
  std::string NextLine;
  KzLine l;
  vector <KzLine *> NewLevel;
  
  while (!LinesToRead.eof()) {
    getline(LinesToRead, NextLine);
    if (NextLine[0] != '\0') {
      try {
        l.readLine (NextLine);
        Lines.push_back (l);
      } catch (Error &Err) {
        Lines.clear ();
        throw Err;
      }
    }
  }
  setUpperLevels ();
}

//------------------------------------------------------------------------------
// setUpperLevels () : Groups the lines in vector <Line> Lines by their upper
// level energy. Once all the upper levels have been obtained, theoretical 
// branching fractions are calculated for each of the lines contained within 
// them. 
//
void KzList::setUpperLevels () {
  vector <KzLine *> LinesLeft;
  vector <KzLine *> NewLevel;
  for (unsigned int i = 0; i < Lines.size (); i ++) {
    LinesLeft.push_back (&Lines[i]);
  }
  
  UpperLevels.clear ();
  while (LinesLeft.size () > 0) {
    NewLevel.clear ();
    for (unsigned int i = LinesLeft.size () - 1; i > 0; i --) {
      if (std::abs (LinesLeft[0]->energyUpper () - LinesLeft[i]->energyUpper ())
        < LevelPrecision) {
        if (noDuplicateExists (NewLevel, LinesLeft[i])) {
          NewLevel.push_back (LinesLeft[i]);
        }
        LinesLeft.erase (LinesLeft.begin() + i);
      }
    }
    if (noDuplicateExists (NewLevel, LinesLeft[0])) {
      NewLevel.push_back (LinesLeft [0]);
    }
    LinesLeft.erase (LinesLeft.begin());
    UpperLevels.push_back (NewLevel);
  }
  sortUpperLevels ();
  calcBranchingFractions ();
}


//------------------------------------------------------------------------------
// noDuplicateExists (vector <KzLine *>, KzLine) : This function checks whether 
// or not the line at arg2 exists in the vector at arg1, returning false if it
// does or true otherwise.
//
bool KzList::noDuplicateExists (vector <KzLine *> NewLevel, KzLine *LineIn) {
  for (unsigned int i = 0; i < NewLevel.size (); i ++) {
    if (std::abs (NewLevel[i] -> energyLower () - LineIn -> energyLower ()) 
      < MAX_LEVEL_PRECISION_ERROR) {
      return false;
    }
  }
  return true;
}


//------------------------------------------------------------------------------
// sortUpperLevels () : Sorts the upperlevels found in setUpperLevels () in
// order of ascending upper level energy. A simple bubble sort is used since
// the number of levels handled at any one time is expected to be small.
//
void KzList::sortUpperLevels () {
  bool ElementsSwapped;
  std::vector <KzLine *> Temp;
  do {
    ElementsSwapped = false;
    for (unsigned int i = 0; i < UpperLevels.size () - 1; i ++) {
      if (UpperLevels[i][0] -> energyUpper () 
        > UpperLevels[i + 1][0] -> energyUpper ()) {
        Temp = UpperLevels [i + 1];
        UpperLevels [i + 1] = UpperLevels [i];
        UpperLevels [i] = Temp;
        ElementsSwapped = true;
      }
    }
  } while (ElementsSwapped);
}


//------------------------------------------------------------------------------
// calcBranchingFraction () : Calculates theoretical branching fractions for all
// the upper levels currently loaded from a Kurucz line list. First, a
// transition probability is calculated for each branch present in a given upper
// level using the prescription in Spectrophysics pp. 173, Table 7.1 and 7.2.
// These are then normalised to the sum of transition probabilities for the
// level to give the branching fraction of each transition.
//
void KzList::calcBranchingFractions () {
  double SumTrProb;
  
  // First cycle through all the upper levels loaded from the Kurucz database
  for (unsigned int i = 0; i < UpperLevels.size (); i ++) {
    SumTrProb = 0.0;
    
    // Then cycle over all the branches in the i th upper level
    for (unsigned int j = 0; j < UpperLevels[i].size (); j ++) {
    
      // Calculate the transition probability for each branch in the i th upper
      // level.
      //
      // t = ( 10^[log(gf)] * sigma^2 ) / ( 1.49919 * [2 * Jupper + 1])
      //
      UpperLevels[i][j]->trProb (pow(10, UpperLevels[i][j]->loggf())
        * pow(UpperLevels[i][j]->sigma(), 2)
        / (TR_PROB_CONST * (UpperLevels[i][j]->jUpper () * 2 + 1)));
      
      // Keep a record of the total transition probabilty calculated so far.
      SumTrProb += UpperLevels[i][j]->trProb ();
    }
    
    // Now that we have transition probabilities for all the branches in the
    // i th upper level, normalise them to give the branching fractions.
    for (unsigned int j = 0; j < UpperLevels[i].size (); j ++) {
      UpperLevels[i][j]->brFrac (UpperLevels[i][j]->trProb () /SumTrProb);
    }
  }
}


//------------------------------------------------------------------------------
// getCommonLines (KzList) : Compares this KzList object with the list passed in
// at arg1. All common lines are extracted and returned.
//
KzList KzList::getCommonLines (KzList Comparison) {
  KzList CommonLines;
  std::vector <KzLine> Candidates;
  std::vector <int> CandidateIndicies;
  double MinDifference;
  int MinDiffIndex;
  for (int j = Lines.size () - 1; j >= 0; j --) {
  
    // Find all the lines in this object that could match the i th Comparison
    // line. Store them as candidates for a line match.
    for (unsigned int i = 0; i < Comparison.size (); i ++) {
      if (abs(Comparison.line(i).sigma() - Lines[j].sigma()) < LevelPrecision) {
        Candidates.push_back (Lines [j]);
        CandidateIndicies.push_back (j);
        break;
      }
    }
    // Now go through each candidate line and find the one that best matches the
    // i th Comparison line. Store that line in CommonLines and then delete it
    // from the Comparison list so that it isn't erroneously matched again.
    if (Candidates.size () > 0) {
      MinDifference = abs (Lines[j].sigma() - Candidates[0].sigma());
      MinDiffIndex = 0;
      for (unsigned int i = 0; i < Candidates.size (); i ++) {
        if (abs (Lines[j].sigma() - Candidates[i].sigma()) < MinDifference) {
          MinDifference = abs(Lines[j].sigma() - Candidates[i].sigma());
          MinDiffIndex = i;
        }
      }
      CommonLines.push_back (Candidates[MinDiffIndex]);
      Comparison.erase (CandidateIndicies[MinDiffIndex]);
      Candidates.clear ();
      CandidateIndicies.clear ();
    }
  }
  return CommonLines;
}
      

//------------------------------------------------------------------------------
// save (std::string) : Saves the current KzList as a text file using the normal
// Kurucz database file format.
//
void KzList::save (std::string OutFile) throw (Error) {
  ostringstream oss, osssub;
  string FileNoDirectory = OutFile.substr(OutFile.find_last_of ("/\\") + 1);
  std::ofstream Output (OutFile.c_str());
  if (Output.is_open ()) {
    save (Output);
    Output.close ();
  } else {
    Output.close ();
    oss << "Error opening " << FileNoDirectory << " for output";
    osssub << "Check that you have write permission for this file.";
    throw Error (FLT_FILE_WRITE_ERROR, oss.str (), osssub.str ());
  }
}


//------------------------------------------------------------------------------
// save (std::string) : Saves the current KzList as a text file using the normal
// Kurucz database file format.
//
void KzList::save (std::ofstream &Output) {
  for (unsigned int i = 0; i < numUpperLevels (); i ++) {
    for (unsigned int j = 0; j < upperLevel (i).size (); j ++) {
      Output << upperLevel(i).line(j).lineString () << std::endl;
    }
    Output << std::endl;
  }
}
    





