
/* Software SPAMS v2.1 - Copyright 2009-2011 Julien Mairal 
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
 */

#include <mex.h>
#include <mexutils.h>
#include <fista.h>

// alpha = mexFistaFlat(X,D,alpha0,param)

using namespace FISTA;

template <typename T>
inline void callFunction(mxArray* plhs[], const mxArray*prhs[],
      const int nlhs) {
   if (!mexCheckType<T>(prhs[0])) 
      mexErrMsgTxt("type of argument 1 is not consistent");
   if (mxIsSparse(prhs[0])) 
      mexErrMsgTxt("argument 1 should not be sparse");

   if (!mexCheckType<T>(prhs[1])) 
      mexErrMsgTxt("type of argument 2 is not consistent");

   if (!mexCheckType<T>(prhs[2])) 
      mexErrMsgTxt("type of argument 3 is not consistent");
   if (mxIsSparse(prhs[2])) 
      mexErrMsgTxt("argument 3 should not be sparse");

   if (!mxIsStruct(prhs[3])) 
      mexErrMsgTxt("argument 4 should be a struct");

   T* prX = reinterpret_cast<T*>(mxGetPr(prhs[0]));
   const mwSize* dimsX=mxGetDimensions(prhs[0]);
   INTM m=static_cast<INTM>(dimsX[0]);
   INTM n=static_cast<INTM>(dimsX[1]);
   Matrix<T> X(prX,m,n);

   const mwSize* dimsD=mxGetDimensions(prhs[1]);
   INTM mD=static_cast<INTM>(dimsD[0]);
   INTM p=static_cast<INTM>(dimsD[1]);
   AbstractMatrixB<T>* D;
   AbstractMatrixB<T>* D2 = NULL;
   AbstractMatrixB<T>* D3 = NULL;

   double* D_v;
   mwSize* D_r, *D_pB, *D_pE;
   INTM* D_r2, *D_pB2, *D_pE2;
   T* D_v2 = NULL;
   
   const int shifts = getScalarStructDef<int>(prhs[3],"shifts",1); // undocumented function

   if (mxIsSparse(prhs[1])) {
      D_v=static_cast<double*>(mxGetPr(prhs[1]));
      D_r=mxGetIr(prhs[1]);
      D_pB=mxGetJc(prhs[1]);
      D_pE=D_pB+1;
      createCopySparse<T>(D_v2,D_r2,D_pB2,D_pE2,
            D_v,D_r,D_pB,D_pE,p);
      D = new SpMatrix<T>(D_v2,D_r2,D_pB2,D_pE2,mD,p,D_pB2[p]);
   } else {
      T* prD = reinterpret_cast<T*>(mxGetPr(prhs[1]));
      D = new Matrix<T>(prD,mD,p);
   }
   const bool double_rows = getScalarStructDef<bool>(prhs[3],"double_rows",false); // undocumented function
   if (double_rows) {
      D2=D;
      D=new DoubleRowMatrix<T>(*D);
   }

   if (shifts > 1) {
      const bool center_shifts = getScalarStructDef<bool>(prhs[3],"center_shifts",false);
      D3=D;
      D=new ShiftMatrix<T>(*D,shifts,center_shifts);
   }

   T* pr_alpha0 = reinterpret_cast<T*>(mxGetPr(prhs[2]));
   const mwSize* dimsAlpha=mxGetDimensions(prhs[2]);
   INTM pAlpha=static_cast<INTM>(dimsAlpha[0]);
   INTM nAlpha=static_cast<INTM>(dimsAlpha[1]);
   Matrix<T> alpha0(pr_alpha0,pAlpha,nAlpha);

   plhs[0]=createMatrix<T>(pAlpha,nAlpha);
   T* pr_alpha=reinterpret_cast<T*>(mxGetPr(plhs[0]));
   Matrix<T> alpha(pr_alpha,pAlpha,nAlpha);

   FISTA::ParamFISTA<T> param;
   param.num_threads = getScalarStructDef<int>(prhs[3],"numThreads",-1);
   param.max_it = getScalarStructDef<int>(prhs[3],"max_it",1000);
   param.tol = getScalarStructDef<T>(prhs[3],"tol",0.000001);
   param.it0 = getScalarStructDef<int>(prhs[3],"it0",100);
   param.pos = getScalarStructDef<bool>(prhs[3],"pos",false);
   param.compute_gram = getScalarStructDef<bool>(prhs[3],"compute_gram",false);
   param.max_iter_backtracking = getScalarStructDef<int>(prhs[3],"max_iter_backtracking",1000);
   param.L0 = getScalarStructDef<T>(prhs[3],"L0",1.0);
   param.fixed_step = getScalarStructDef<bool>(prhs[3],"fixed_step",false);
   param.gamma = MAX(1.01,getScalarStructDef<T>(prhs[3],"gamma",1.5));
   param.c = getScalarStructDef<T>(prhs[3],"c",1.0);
   param.lambda= getScalarStructDef<T>(prhs[3],"lambda",1.0);
   param.delta = getScalarStructDef<T>(prhs[3],"delta",1.0);
   param.lambda2= getScalarStructDef<T>(prhs[3],"lambda2",0.0);
   param.lambda3= getScalarStructDef<T>(prhs[3],"lambda3",0.0);
   mxArray* ppr_groups = mxGetField(prhs[3],0,"groups");
   if (ppr_groups) {
      if (!mexCheckType<int>(ppr_groups))
         mexErrMsgTxt("param.groups should be int32 (starting group is 1)");
      int* pr_groups = reinterpret_cast<int*>(mxGetPr(ppr_groups));
      const mwSize* dims_groups =mxGetDimensions(ppr_groups);
      int num_groups=static_cast<int>(dims_groups[0])*static_cast<int>(dims_groups[1]);
      if (num_groups != pAlpha) mexErrMsgTxt("Wrong size of param.groups");
      param.ngroups=num_groups;
      param.groups=pr_groups;
   } else {
      param.size_group= getScalarStructDef<int>(prhs[3],"size_group",1);
   }

   param.admm = getScalarStructDef<bool>(prhs[3],"admm",false);
   param.lin_admm = getScalarStructDef<bool>(prhs[3],"lin_admm",false);
   param.sqrt_step = getScalarStructDef<bool>(prhs[3],"sqrt_step",true);
   param.is_inner_weights = getScalarStructDef<bool>(prhs[3],"is_inner_weights",false);
   param.transpose = getScalarStructDef<bool>(prhs[3],"transpose",false);
   if (param.is_inner_weights) {
      mxArray* ppr_inner_weights = mxGetField(prhs[4],0,"inner_weights");
      if (!ppr_inner_weights) mexErrMsgTxt("field inner_weights is not provided");
      if (!mexCheckType<T>(ppr_inner_weights)) 
         mexErrMsgTxt("type of inner_weights is not correct");
      param.inner_weights = reinterpret_cast<T*>(mxGetPr(ppr_inner_weights));
   }

   getStringStruct(prhs[3],"regul",param.name_regul,param.length_names);
   param.regul = regul_from_string(param.name_regul);
   if (param.regul==INCORRECT_REG)
      mexErrMsgTxt("Unknown regularization");
   getStringStruct(prhs[3],"loss",param.name_loss,param.length_names);
   param.loss = loss_from_string(param.name_loss);
   if (param.loss==INCORRECT_LOSS)
      mexErrMsgTxt("Unknown loss");

   param.intercept = getScalarStructDef<bool>(prhs[3],"intercept",false);
   param.resetflow = getScalarStructDef<bool>(prhs[3],"resetflow",false);
   param.verbose = getScalarStructDef<bool>(prhs[3],"verbose",false);
   param.clever = getScalarStructDef<bool>(prhs[3],"clever",false);
   param.ista= getScalarStructDef<bool>(prhs[3],"ista",false);
   param.linesearch_mode= getScalarStructDef<int>(prhs[3],"linesearch_mode",0);
   param.subgrad= getScalarStructDef<bool>(prhs[3],"subgrad",false);
   param.log= getScalarStructDef<bool>(prhs[3],"log",false);
   param.a= getScalarStructDef<T>(prhs[3],"a",T(1.0));
   param.b= getScalarStructDef<T>(prhs[3],"b",0);

   if (param.log) {
      mxArray *stringData = mxGetField(prhs[3],0,"logName");
      if (!stringData) 
         mexErrMsgTxt("Missing field logName");
      int stringLength = mxGetN(stringData)+1;
      param.logName= new char[stringLength];
      mxGetString(stringData,param.logName,stringLength);
   }

   if ((!double_rows && shifts==1 && param.loss != CUR && param.loss != MULTILOG) && (pAlpha != p || nAlpha != n || mD != m)) { 
      mexErrMsgTxt("Argument sizes are not consistent");
   } else if (param.loss == MULTILOG) {
      Vector<T> Xv;
      X.toVect(Xv);
      INTM maxval = static_cast<INTM>(Xv.maxval());
      INTM minval = static_cast<INTM>(Xv.minval());
      if (minval != 0)
         mexErrMsgTxt("smallest class should be 0");
      if (maxval*X.n() > nAlpha || mD != m) {
         cerr << "Number of classes: " << maxval << endl;
         //cerr << "Alpha: " << pAlpha << " x " << nAlpha << endl;
         //cerr << "X: " << X.m() << " x " << X.n() << endl;
         mexErrMsgTxt("Argument sizes are not consistent");
      }
   } else if (param.loss == CUR && (pAlpha != D->n() || nAlpha != D->m())) {
      mexErrMsgTxt("Argument sizes are not consistent");
   }

   if (param.num_threads == -1) {
      param.num_threads=1;
#ifdef _OPENMP
      param.num_threads =  MIN(MAX_THREADS,omp_get_num_procs());
#endif
   } 

   if (param.regul==GRAPH_PATH_L0 || param.regul==GRAPH_PATH_CONV) 
      mexErrMsgTxt("Error: mexFistaPathCoding should be used instead");
   if (param.regul==GRAPH || param.regul==GRAPHMULT) 
      mexErrMsgTxt("Error: mexFistaGraph should be used instead");
   if (param.regul==TREE_L0 || param.regul==TREEMULT || param.regul==TREE_L2 || param.regul==TREE_LINF) 
      mexErrMsgTxt("Error: mexFistaTree should be used instead");

   Matrix<T> duality_gap;
   FISTA::solver<T>(X,*D,alpha0,alpha,param,duality_gap);
   if (nlhs==2) {
      plhs[1]=createMatrix<T>(duality_gap.m(),duality_gap.n());
      T* pr_dualitygap=reinterpret_cast<T*>(mxGetPr(plhs[1]));
      for (int i = 0; i<duality_gap.n()*duality_gap.m(); ++i) pr_dualitygap[i]=duality_gap[i];
   }
   if (param.logName) delete[](param.logName);

   if (mxIsSparse(prhs[1])) {
      deleteCopySparse<T>(D_v2,D_r2,D_pB2,D_pE2,
            D_v,D_r);
   }
   delete(D);
   if (shifts > 1) {
      delete(D2);
   }
   if (double_rows) {
      delete(D3);
   }

}

   void mexFunction(int nlhs, mxArray *plhs[],int nrhs, const mxArray *prhs[]) {
      if (nrhs != 4)
         mexErrMsgTxt("Bad number of inputs arguments");

      if (nlhs != 1 && nlhs != 2) 
         mexErrMsgTxt("Bad number of output arguments");

      if (mxGetClassID(prhs[0]) == mxDOUBLE_CLASS) {
         callFunction<double>(plhs,prhs,nlhs);
      } else {
         callFunction<float>(plhs,prhs,nlhs);
      }
   }




