// -*- mode:C++ ; compile-command: "g++-3.4 -I.. -g -c gauss.cc -Wall" -*- 
#include "giacPCH.h"

/*
 *  Copyright (C) 2001,14 R. De Graeve, Institut Fourier, 38402 St Martin d'Heres
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

using namespace std;
#include <fstream>
#include "gauss.h"
#include "vecteur.h"
#include "derive.h"
#include "subst.h"
#include "usual.h"
#include "sym2poly.h"
#include "solve.h"
#include "ti89.h"
#include "plot.h"
#include "misc.h"
#include "ifactor.h"
#include "prog.h"
#include "giacintl.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  vecteur quad(int &b,const gen & q, const vecteur & x,GIAC_CONTEXT){
    //x=vecteur des variables, q=la fonction a tester,n=la dimension de x
    //b=2 si q est quadratique,=0,1 ou 3 si il y des termes d'ordre 0,1 ou 3
    //renvoie (la jacobienne de q)/2
    gen qs;
    gen dq;
    gen dqs;
    gen qdd;
    int n=int(x.size());
    
    vecteur A;
    //creation d'une matrice carree A d'ordre n
    for (int i=0;i<n;i++){
      vecteur li(n);
      A.push_back(li);
    }
    //A est un vecteur de vecteur=une matrice!
    //on met ds A :(la jacobienne de q)/2 
    for (int i=0;i<n;i++){
      for (int j=i;j<n;j++){
	qdd=derive(derive(q,x[i],contextptr),x[j],contextptr);
	qdd=recursive_normal(qdd,contextptr); 
	//cout<<i<<","<<j<<qdd<<endl;
	if (i==j){
	  (*A[i]._VECTptr)[i]=rdiv(qdd,2,contextptr);
	} 
	else {
	  (*A[i]._VECTptr)[j]=rdiv(qdd,2,contextptr);
	  (*A[j]._VECTptr)[i]=rdiv(qdd,2,contextptr);
	}
      }
    }
    //2*A=jacobienne de q
    //on calcule qs=q en zero
    //cout<<A<<endl;  
    qs=q;
    for (int i=0;i<n;i++){
      qs=subst(qs,x[i],0,false,contextptr);
    }
    //qs=la valeur de q en 0
    if (qs !=0){
      b=0;
      return(A);
    }
    //on regarde si il y des termes lineaires
    for (int j=0;j<n;j++){
      dq=derive(q,x[j],contextptr);
      dqs=dq;
      for (int i=0;i<n;i++){
	dqs=subst(dqs,x[i],0,false,contextptr);
      }
      //dqs=la diff de q en zero
      if (dqs!=0){
	b=1;
	return(A);
      }
    }
    for (int i=0;i<n;i++){
      for (int j=i;j<n;j++){
	for (int k=0;k<n;k++){
	  if (derive(A[i][j],x[k],contextptr)!=0){
	    b=3;
	    return(A);
	  }
	}
      }
    }
    b=2;
    //(*(A[1]._VECTptr))[0]=21;
    return(A);
  } 
  
  vecteur qxa(const gen &q,const vecteur & x,GIAC_CONTEXT){
    //transforme une forme quadratique en une matrice symetrique A
    //(les variables sont dans x)
    int d;
    //d nbre de variables
    d=int(x.size());
    // int da;
    //il faut verifier que q est quadratique
    vecteur A;
    int b;
    A=quad(b,q,x,contextptr);  
    if (b==2) {
      return(A);
    }
    else {
      return vecteur(1,gensizeerr(gettext("q is not quadratic")));
    }
    return 0;
  }

  static gen symb_q2a(const gen & args){
    return symbolic(at_q2a,args);
  }
  gen _q2a(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)
      return _q2a(makesequence(args,lidnt(args)),contextptr);
    int s=int(args._VECTptr->size());
    if (s!=2)
      return gendimerr(contextptr);
    if (args._VECTptr->back().type==_VECT)
      return qxa(args._VECTptr->front(),*args._VECTptr->back()._VECTptr,contextptr);
    return symb_q2a(args);
  }
  static const char _q2a_s []="q2a";
  static define_unary_function_eval (__q2a,&_q2a,_q2a_s);
  define_unary_function_ptr5( at_q2a ,alias_at_q2a,&__q2a,0,true);

  vecteur gauss(const gen & q, const vecteur & x, vecteur & D, vecteur & U, vecteur & P,GIAC_CONTEXT){
    int n=int(x.size()),b;
    gen u1,u2,q1,l1,l2;
    vecteur R(1);
    vecteur PR;
    for (int i=0;i<n-1;i++){
      vecteur li(n-1);
      PR.push_back(li);
    }
    vecteur PP;
    for (int i=0;i<n;i++){
      vecteur li(n);
      PP.push_back(li);
    } 
    vecteur I;
    if (n) I=midn(n);
    vecteur L;
  
    //si q n'est pas quadratique b<>2 et on retourne q
    vecteur A(quad(b,q,x,contextptr));
    if (b!=2){
      R[0]=q;
      D.clear();
      U.clear();
      return R;
    }
    //la forme q est quadratique de matrice A
    if (q==0) { 
      //R[0]=q;    
      vecteur vide(n);
      D=vide; 
      U=vide;    
      P=I;
      return vide;
    }
    if (n==1){
      gen q0=_factor(q,contextptr);
      R[0]=q0;
      vecteur un(1);
      un[0]=A[0][0];
      D=un;
      U=x;
      P=I;
      return(R);
    }
    int r;
    r=n;
    for (int i=n-1 ;i>=0;i--){
      if (A[i][i]!=0) {
	r=i;
      }
    }
    if (r!=n) {
      //il y a des termes carres
      u1=recursive_normal(rdiv(derive(q,x[r],contextptr),plus_two,contextptr),contextptr);
      q1=recursive_normal(q-rdiv(u1*u1,A[r][r],contextptr),contextptr);     
      vecteur y;
      //y contient les variables qui restent (on enleve x[r])	   
      for (int j=0;j<n;j++){
	if (j!=r){
	  y.push_back(x[j]);
	}
      }
      L=gauss(q1,y,D,U,PR,contextptr);
      //on rajoute 1/a_r_r sur la diagonale D
      R[0]=rdiv(1,A[r][r],contextptr);
      D=mergevecteur(R,D);
      //on rajoute u1 aux vecteurs constitue des formes lineaires
      //q= 1/a_r_r*(u1)^2+... 
      R[0]=u1; 
      U=mergevecteur(R,U);      
      //on complete la matrice PR de dim n-1 en la matrice PP de dim n
      //1iere ligne les coeff de u1 et rieme colonne doit avoir des 0
      for (int i=0;i<n;i++){
	(*PP[0]._VECTptr)[i]=recursive_normal(derive(u1,x[i],contextptr),contextptr); 
      }
      for (int i=1;i<n;i++){
	for (int j=0;j<r;j++){
	  (*PP[i]._VECTptr)[j]=PR[i-1][j];
	}
	for (int j=r+1;j<n;j++){
	  (*PP[i]._VECTptr)[j]=PR[i-1][j-1];
	}
      }
      P=PP;
      R[0]=rdiv(pow(u1,2),A[r][r],contextptr);
      return(mergevecteur(R,L));
    }
    //il n'y a pas de carres
    int r1;
    int r2;
    r1=0;
    r2=0;
    for (int i=n-2;i>=0;i--){
      for (int j=i+1;j>=1;j--){
	if (A[i][j]!=0) {
	  r1=i;
	  r2=j;
	}
      }
    }
    l1=rdiv(derive(q,x[r1],contextptr),2,contextptr);
    l2=rdiv(derive(q,x[r2],contextptr),2,contextptr);
    u1=recursive_normal(l1+l2,contextptr);
    u2=recursive_normal(l1-l2,contextptr);
    q1=recursive_normal(q-rdiv(plus_two*l1*l2,A[r1][r2],contextptr),contextptr);
    vecteur y;
    for (int j=0;j<n;j++){
      if ((j!=r1) && (j!=r2)) {
	y.push_back(x[j]);
      }
    }
    L=gauss(q1,y,D,U,PR,contextptr);
    //on rajoute 1/a_r1_r2 et -1/a_r1_r2 sur la diagonale D
    R[0]=rdiv(1,plus_two*A[r1][r2],contextptr);
    R.push_back(rdiv(-1,plus_two*A[r1][r2],contextptr));
    D=mergevecteur(R,D); 
    //on rajoute u1 et u2 au vecteur U constitue des formes lineaires
    //q= 1/a_r1_r2*(u1)^2 - 1/a_r1_r2*(u2)^2 + ... 
    R[0]=u1;
    R[1]=u2;
    U=mergevecteur(R,U);
    //on complete la matrice PR de dim n-2 en la matrice PP de dim n
    //1iere et 2ieme ligne les coeff de u1 et de u2 
    //r1ieme et r2ieme colonne doit avoir des 0
    for (int i=0;i<n;i++){
      (*PP[0]._VECTptr)[i]=recursive_normal(derive(u1,x[i],contextptr),contextptr);
      (*PP[1]._VECTptr)[i]=recursive_normal(derive(u2,x[i],contextptr),contextptr);
    }
    for (int i=2;i<n;i++){
      for (int j=0;j<r1;j++){
	(*PP[i]._VECTptr)[j]=PR[i-2][j];
      }
      for (int j=r1+1;j<r2;j++){
	(*PP[i]._VECTptr)[j]=PR[i-2][j-1];
      }
      for (int j=r2+1;j<n;j++){
	(*PP[i]._VECTptr)[j]=PR[i-2][j-2];
      }
    }	
    P=PP;
    R[0]=rdiv(pow(u1,2),plus_two*A[r1][r2],contextptr);
    R[1]=rdiv(-pow(u2,2),plus_two*A[r1][r2],contextptr);
    return(mergevecteur(R,L)); 
  } 
  gen _gauss(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)
      return _gauss(makesequence(args,lidnt(args)),contextptr);
    int s=int(args._VECTptr->size());
    if (s<2)
      return gendimerr(contextptr);
    const gen & arg1=(*args._VECTptr)[1];
    if (arg1.type==_VECT){
      const vecteur & v=*arg1._VECTptr;
      vecteur D,U,P;
      gen w=gauss(args._VECTptr->front(),v.empty()?lidnt(args):v,D,U,P,contextptr);
      w=_plus(w,contextptr);
      if (s>2|| v.empty())
	return makesequence(w,D,P);
      return w;
    }
    return _randNorm(args,contextptr);
  }
  static const char _gauss_s []="gauss";
  static define_unary_function_eval (__gauss,&_gauss,_gauss_s);
  define_unary_function_ptr5( at_gauss ,alias_at_gauss,&__gauss,0,true);

  gen axq(const vecteur &A,const vecteur & x,GIAC_CONTEXT){
    //transforme une matrice carree (symetrique) en la forme quadratique q
    //(les variables sont dans x)
    //d nbre de variables
    //il faut verifier que A est carree
    //A n'est pas forcement symetrique  
    int d=int(x.size());
    int da=int(A.size());
    if (!(is_squarematrix(A)) || (da!=d) )
      return gensizeerr(gettext("Invalid dimension"));
    vecteur Ax;
    multmatvecteur(A,x,Ax);
    return normal(dotvecteur(x,Ax),contextptr);
  }

  gen _a2q(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT || args._VECTptr->empty())
      return gensizeerr(contextptr);
    int s=int(args._VECTptr->size());
    if (args.subtype!=_SEQ__VECT && s<=6){
      vecteur vars(makevecteur(x__IDNT_e,y__IDNT_e,z__IDNT_e,t__IDNT_e,u__IDNT_e,v__IDNT_e));
      vars.erase(vars.begin()+s,vars.end());
      return _a2q(makesequence(args,vars),contextptr);
    }
    if (s!=2)
      return gendimerr(contextptr);
    const vecteur & v = *args._VECTptr;
    if (ckmatrix(v.front()) && v.back().type==_VECT)
      return axq(*v.front()._VECTptr,*v.back()._VECTptr,contextptr);
    return gensizeerr(contextptr);
  }
  static const char _a2q_s []="a2q";
  static define_unary_function_eval (__a2q,&_a2q,_a2q_s);
  define_unary_function_ptr5( at_a2q ,alias_at_a2q,&__a2q,0,true);



#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC

