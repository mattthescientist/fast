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
// VoigtLsqfit Class (voigtlsqfit.h)
//==============================================================================
// Fits a voigt profile to a set of data points from an atomic line spectrum.
// The algorithm (along with the mystical p, bgg, and vf) arrays were adapted
// from the XGremlin code. I have no idea how it works!
//
#ifndef VOIGT_LSQFIT_H
#define VOIGT_LSQFIT_H

#include <string>
#include "ErrDefs.h"

using namespace::std;

class VoigtLsqfit {
  public:
    VoigtLsqfit ();
    ~VoigtLsqfit ();
  
    void voigt(unsigned int n, double *x, double *y, double wd, 
      double a, double dmp, double xc);
    float P (int Index) throw (Error);
    
  private:
  
    void vstart ();
    float _voigt ();
//    void addlin (double *r, double xc, float xw, float a, float bt);
  
    float xpara, xparb, xparc;
  	float c, ca, cb, cc, cd, ce, ulim, lastv;
    int ni, nf, na, nb, nop;
    float xlast;

    float vt[50], ds[50], dm[50];
};

#endif // VOIGT_LSQFIT_H
