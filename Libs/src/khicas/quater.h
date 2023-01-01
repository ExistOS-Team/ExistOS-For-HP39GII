// -*- mode:C++ ; compile-command: "g++ -I.. -I../include -g -c quater.cc" -*-
/*
 *  Copyright (C) 2001,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#ifndef _GIAC_QUATER_H
#define _GIAC_QUATER_H
#include "first.h"
#include "gen.h"
#include <string>


#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC
  
  extern const unary_function_ptr * const  at_quaternion; // user-level quaternion constructor
#ifndef NO_RTTI
  class quaternion : public gen_user {
  public:
    gen r,i,j,k;
    virtual gen_user * memory_alloc() const { 
      quaternion * ptr= new quaternion(*this);
      return dynamic_cast<gen_user *>(ptr); 
    }
    quaternion(const quaternion & q):r(q.r),i(q.i),j(q.j),k(q.k){};
    quaternion(const gen & myr,const gen & myi,const gen & myj,const gen & myk):r(myr),i(myi),j(myj),k(myk) {};
    quaternion():r(zero),i(zero),j(zero),k(zero) {};
    quaternion(const gen & g);
    virtual gen operator + (const gen & g) const { 
      quaternion q(g);
      return quaternion(r+q.r,i+q.i,j+q.j,k+q.k);
    }
    virtual gen operator - (const gen & g) const { 
      quaternion q(g);
      return quaternion(r-q.r,i-q.i,j-q.j,k-q.k);
    }
    virtual std::string print (GIAC_CONTEXT) const ;
  };
  gen _quaternion(const gen & args,GIAC_CONTEXT);

  gen char2_uncoerce(const gen & a);
  class galois_field : public gen_user {
  public:
    gen p; // F_p^m, characteristic of the field
    gen P; // minimal irreducible polynomial of degree m, as vector
    gen x; // the name of the variable for construction
    gen a; // value as a vector polynomial or undef (whole field)
    virtual gen_user * memory_alloc() const { 
      galois_field * ptr= new galois_field(*this,false);
      // if (a != smod(a,p) && smod(a,p))  CERR << "not reduced" << '\n';
      return ptr; 
    }
    galois_field(const galois_field & q,bool doreduce=true);
    galois_field(const gen p_,const gen & P_,const gen & x_,const gen & a_,bool doreduce=true);
    galois_field(const gen & g,bool primitive,GIAC_CONTEXT);
    void reduce(); // reduce a
    virtual gen operator + (const gen & g) const;
    virtual gen operator - (const gen & g) const;
    virtual gen operator - () const;
    virtual gen operator * (const gen & g) const;
    virtual gen operator / (const gen & g) const;
    virtual gen inv () const ;
    virtual std::string print (GIAC_CONTEXT) const ;
    virtual std::string texprint (GIAC_CONTEXT) const ;
    virtual gen giac_constructor (GIAC_CONTEXT) const ;
    virtual bool operator == (const gen &) const ;
    virtual bool is_zero() const;
    virtual bool is_one() const;
    virtual bool is_minus_one() const;
    virtual gen operator () (const gen &,GIAC_CONTEXT) const;
    virtual gen operator [] (const gen &) ;
    virtual gen operator > (const gen & g) const;
    virtual gen operator < (const gen & g) const;
    virtual gen operator >= (const gen & g) const;
    virtual gen operator <= (const gen & g) const;
    virtual gen gcd (const gen &) const { return plus_one;}    
    virtual gen gcd (const gen_user & a) const { return plus_one; }
    virtual gen polygcd (const polynome &,const polynome &,polynome &) const ;
    virtual gen makegen(int i) const ;
    virtual gen polyfactor (const polynome & p,factorization & f) const ;
    virtual gen conj(GIAC_CONTEXT) const { return *this;}
    virtual gen re(GIAC_CONTEXT) const { return *this;}
    virtual gen im(GIAC_CONTEXT) const {  return 0;}
    virtual gen sqrt(GIAC_CONTEXT) const;
    virtual gen rand(GIAC_CONTEXT) const;
    polynome poly_reduce(const polynome & p) const ;
  };

  // Is the polynomial v irreducible and primitive modulo p?
  // If it is only irreducible, returns 2 and sets vmin if primitive==1
  // if primitive==0 or 2 does not compute vmin 
  // issues a warning if primitive==0
  int is_irreducible_primitive(const vecteur & v,const gen & p,vecteur & vmin,int primitive,GIAC_CONTEXT);
  int is_irreducible(const vecteur &v,const gen &g);
  vecteur find_irreducible_primitive(const gen & p,int m,int primitive,GIAC_CONTEXT);
  gen _galois_field(const gen & args,GIAC_CONTEXT);

  struct gen_context_t {
    gen g;
    context * ptr ;
  };
  // All Galois field in a map[p^m]=generator of GF(p,m)
  // the generator might be replaced by some polynomial of a GF(p,m*m2)
  // if a binary operation on two elements of different GF(p,.) happens
  typedef std::map<gen,gen_context_t,comparegen > gfmap; 
  gfmap & gf_list();
  int gfsize(const gen & P);
  bool has_gf_coeff(const gen & e,gen & p, gen & pmin);
  bool has_gf_coeff(const vecteur & v,gen & p, gen & pmin);
  bool has_gf_coeff(const gen & e);
  bool has_gf_coeff(const vecteur & v);
  // convert v in char 2, returns minimal polynomial or 0 (unknown) or -1 (unable to convert), set x to the generator of the field
  int gf_char2_vecteur2vectorint(const vecteur & v,std::vector<int> & V,gen & x);
  // convert m in char 2, returns minimal polynomial or 0 (unknown) or -1 (unable to convert), set x to the generator of the field
  int gf_char2_matrice2vectorvectorint(const matrice & m,std::vector< std::vector<int> > & M,gen & x);
  void gf_char2_vectorint2vecteur(const std::vector<int> & source,vecteur & target,int M,const gen & x);
  void gf_char2_vectorvectorint2mat(const std::vector< std::vector<int> > & source,matrice & target,int M,const gen & x);
  int dotgf_char2(const std::vector<int> & v,const std::vector<int> & w,int M);
  bool gf_char2_mmult_atranb(const std::vector< std::vector<int> > & A,const std::vector< std::vector<int> > & tranB,std::vector< std::vector<int> > & C,int M);
  bool gf_char2_rref(std::vector< std::vector<int> > & N,const gen & x,int M,vecteur & pivots,std::vector<int> & permutation,std::vector<int> & maxrankcols,gen & det,int l, int lmax, int c,int cmax,int fullreduction,int dont_swap_below,int rref_or_det_or_lu);
  bool gf_char2_multpoly(const std::vector<int> & a,const std::vector<int> & b,std::vector<int> & res,int M);
  bool gf_multpoly(const std::vector< std::vector<int> > & a,const std::vector< std::vector<int> > & b,std::vector< std::vector<int> > & res,const std::vector<int> & pmin,int modulo);
  int gf_vecteur2vectorvectorint(const vecteur & v,std::vector< std::vector<int> > & V,gen & x,std::vector<int> & pmin);
  bool gf_multpoly(const std::vector< std::vector<int> > & a,const std::vector< std::vector<int> > & b,std::vector< std::vector<int> > & res,const std::vector<int> & pmin,int modulo);
  void gf_vectorvectorint2vecteur(const std::vector< std::vector<int> > & source,vecteur & target,const gen & carac,const vecteur & pmin,const gen & x);
  void gf_vectorvectorint2vecteur(const std::vector< std::vector<int> > & source,vecteur & target,int carac,const std::vector<int> & pmin,const gen & x);

#else // NO_RTTI
  inline bool has_gf_coeff(const gen & e,gen & p, gen & pmin){ return false; }
  inline bool has_gf_coeff(const vecteur & v,gen & p, gen & pmin){ return false; }
  inline bool has_gf_coeff(const gen & e){ return false; }
  inline bool has_gf_coeff(const vecteur & v){ return false; }
  inline int gf_char2_vecteur2vectorint(const vecteur & v,std::vector<int> & V,gen & x){ return -1; }
  inline int gf_char2_matrice2vectorvectorint(const matrice & m,std::vector< std::vector<int> > & M,gen & x){ return -1;}
  inline int dotgf_char2(const std::vector<int> & v,const std::vector<int> & w,int M){ return 0; }
  inline void gf_char2_vectorint2vecteur(const std::vector<int> & source,vecteur & target,int M,const gen & x){};
  inline void gf_char2_vectorvectorint2mat(const std::vector< std::vector<int> > & source,matrice & target,int M,const gen & x){};
  inline   bool gf_char2_mmult_atranb(const std::vector< std::vector<int> > & A,const std::vector< std::vector<int> > & tranB,std::vector< std::vector<int> > & C,int M){ return false;};
  inline bool gf_char2_rref(std::vector< std::vector<int> > & N,const gen & x,int M,vecteur & pivots,std::vector<int> & permutation,std::vector<int> & maxrankcols,gen & det,int l, int lmax, int c,int cmax,int fullreduction,int dont_swap_below,int rref_or_det_or_lu){ return false; };
  inline bool gf_char2_multpoly(const std::vector<int> & a,const std::vector<int> & b,std::vector<int> & res,int M){ return false; }
  inline int gf_vecteur2vectorvectorint(const vecteur & v,std::vector< std::vector<int> > & V,gen & x,std::vector<int> & pmin){ return 0; }
  inline bool gf_multpoly(const std::vector< std::vector<int> > & a,const std::vector< std::vector<int> > & b,std::vector< std::vector<int> > & res,const std::vector<int> & pmin,int modulo){ return false; }
  inline void gf_vectorvectorint2vecteur(const std::vector< std::vector<int> > & source,vecteur & target,const gen & carac,const vecteur & pmin,const gen & x){}
  inline void gf_vectorvectorint2vecteur(const std::vector< std::vector<int> > & source,vecteur & target,int carac,const std::vector<int> & pmin,const gen & x){};

#endif // NO_RTTI

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC


#endif // _GIAC_QUATER_H
