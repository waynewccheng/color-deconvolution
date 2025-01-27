/*!
 * Software SPAMS v2.5 - Copyright 2009-2014 Julien Mairal 
 *
 * This file is part of SPAMS.
 *
 * SPAMS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SPAMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SPAMS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * \file
 *                toolbox projsplx
 *
 *                by Yuansi Chen 
 *                yuansi.chen@berkeley.edu
 *
 *                File projsplx.h
 * \brief Contains projection on simplex algorithms 
 * It requires the toolbox linalg */

#ifndef PROJSPLX_H
#define PROJSPLX_H

#include <utils.h>

/*
 * Projection y = (y1, .., yn) to the simplex
 * Dn = {x: x = (x1, .., xn), x>=0, sum(x) = 1}
 */

template <typename T>
void projsplx(const Vector<T>& y, Vector<T>& b) {
  int n = y.n();
  bool getOpt = false;

  b.copy(y);
  b.sort(false); // decreasing
  T tmpSum = 0.0;
  T rhoMax = 0.0;
  
  // main loop to find the rho* s.t. yj = 0 for j > rho*
  for(int i = 0; i <n-1; ++i) {
    tmpSum += b[i];
    rhoMax = (tmpSum - 1)/(i+1);
    if(rhoMax >= b[i+1]) {
      getOpt = true;
      break;
    }
  }

  if(!getOpt) {
    rhoMax = (tmpSum + b[n-1]-1)/n;
  }
  
  b.copy(y);
  b.add(-rhoMax);
  b.thrsmax(0.0);
}

/* **************************
 * Projection of each column of matrix 
 * **************************/

template <typename T>
void projsplxMatrix(const Matrix<T>& Beta, Matrix<T>& BetaOut) {
  const int n = Beta.m();
  const int p = Beta.n();
  BetaOut.resize(n,p);
  Vector<T> refColBeta;
  Vector<T> refColBetaOut;
  for(int i =0; i<p; ++i) {
    Beta.refCol(i, refColBeta);
    BetaOut.refCol(i, refColBetaOut);
    projsplx(refColBeta, refColBetaOut);
  }
}

/* **************************
 * Inplace Version Matrix Projection 
 * **************************/

template <typename T>
void projsplxMatrixOn(const Matrix<T>& Beta) {
  const int p = Beta.n();
  Vector<T> refColBeta;
  Vector<T> b;
  for(int i =0; i<p; ++i) {
    Beta.refCol(i, refColBeta);
    projsplx(refColBeta, b);
    refColBeta.copy(b);
  }
}

#endif
