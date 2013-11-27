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
// Kurucz List class (kzlist.h)
//==============================================================================
// Assembles a list of lines from the Kurucz database and, in addition to giving
// standard I/O routines, provides some additional list processing functions.
//
// The list of lines are stored in std::vector <KzLine> Lines. Any function that
// creates or modifies this vector then calls setUpperLevels (), which divides
// the list into groups of lines with the same upper level. A tolerance on 
// matching energy levels is given by double LevelPrecision, which defaults to
// DEF_LIST_LEVEL_PRECISION or can be modified by levelPrecision(double).
// setUpperLevels () also calculates theoretical branching fractions for the 
// lines in each group using calcBranchingFractions ().
//
// Lines can then be accessed by their index in the full list with line(int), or
// by their upper level with upperLevel(int). Lines may also be extracted from
// the List based on a minimum log(gf) value or branching fraction with
// lines (double, char).
//
// Finally, two lists may be compared, and common lines extracted, using the
// getCommonLines (List) function.
// 
#ifndef LIST_H
#define LIST_H

#include "kzline.h"
#include <vector>
#include <string>

#define DEF_LIST_LEVEL_PRECISION 9.0e-2
#define MAX_LEVEL_PRECISION_ERROR 1e-6
#define TR_PROB_CONST 1.49919          /* For converting A values to log(gf)s */

class KzList {

  private:
    std::vector <KzLine> Lines;
    std::string Name;
    std::vector < std::vector <KzLine *> > UpperLevels;
    double LevelPrecision;
    
    void setUpperLevels ();
    void sortUpperLevels ();
    void calcBranchingFractions ();
    bool noDuplicateExists (vector <KzLine *> NewLevel, KzLine *LineIn);
    
    void read (std::ifstream &ListToRead) throw (Error);
    void save (std::ofstream &Output);
  
  public:
  
    // Constructors and destructor
    KzList ();
    KzList (std::string ListFile);
    KzList (std::ifstream &ListToRead);
    KzList (std::vector <KzLine> LinesIn);
    ~KzList () { /* Does nothing */ }
    
    // I/O functions for both text and binary read/save operations.
    void read (std::string ListFile) throw (Error);
    void save (std::string OutFile) throw (Error);
    
    // Public GET functions
    KzLine line (int Index);
    std::vector <KzLine> lines ();
    std::vector <KzLine> lines (double Min, char Mode) throw (string);
    std::string name ();
    unsigned int size () { return Lines.size(); }
    KzList upperLevel (unsigned int i) throw (string);
    std::vector <KzLine *> upperLevelLines (unsigned int i) { return UpperLevels[i]; }
    unsigned int numUpperLevels () { return UpperLevels.size (); }
    double levelPrecision () { return LevelPrecision; }

    // Public SET and modifier functions. These largely mirror the functions 
    // found in a std::vector. 
    void name (std::string NewName);
    void push_back (KzLine NewLine);
    void push_back (KzList NewLines);
    void push_back (std::vector <KzLine> NewLines);
    void pop_back ();
    void insert (KzLine NewLine, int Position);
    void erase (int Position);
    void erase (int First, int Last);
    void eraseUpperLevel (int Index);
    void levelPrecision (double NewPrecision);
    void clear ();
    
    void set_upper_level_lifetime (double Index, double Lifetime);
    void set_upper_level_lifetime_error (double Index, double Lifetime);
    
    KzList getCommonLines (KzList Comparison);
};

#endif // LIST_H
