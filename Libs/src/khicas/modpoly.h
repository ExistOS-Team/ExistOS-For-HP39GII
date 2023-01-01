// -*- mode:C++ ; compile-command: "g++ -I.. -g -c modpoly.cc" -*-
/*
 *  Copyright (C) 2000,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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

#ifndef _GIAC_MODPOLY_H_
#define _GIAC_MODPOLY_H_
#include "first.h"
#include "global.h"
#include "fraction.h"
#ifdef HAVE_LIBNTL
#include <NTL/ZZXFactoring.h>
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/GF2X.h>
#include <NTL/pair_GF2X_long.h>
#include <NTL/GF2XFactoring.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZX.h>
#include <NTL/pair_ZZ_pX_long.h>
#include <NTL/ZZ_pXFactoring.h>
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#endif // HAVE_LIBNTL

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC
  // Previous Fourier prime if fourier_for_n!=0, or previous prime <= maxp
  int prevprimep1p2p3(int p,int maxp,int fourier_for_n=0);
  template<class T> class tensor;
  typedef tensor<gen> polynome;
  typedef vecteur modpoly;
  typedef vecteur dense_POLY1; // same internal rep but assumes non modular op

  // set env->pn, returns true if size of field <=2^31
  bool normalize_env(environment * env);
  // modular or GF inverse
  gen invenv(const gen & g,environment * env);

  // ***********************
  // Building and converting
  // ***********************
  // Conversion from univariate to multivariate polynomials
  modpoly modularize(const polynome & p,const gen & n,environment * env);
  modpoly modularize(const dense_POLY1 & p,const gen & n,environment * env);
  polynome unmodularize(const modpoly & a);
  bool is_one(const modpoly & p);
  int is_cyclotomic(const modpoly & p,GIAC_CONTEXT);
  int is_cyclotomic(const modpoly & p,double eps);
  modpoly one(); // 1
  modpoly xpower1(); // x
  modpoly xpowerpn(environment * env); // x^modulo
  vecteur x_to_xp(const vecteur & v, int p); // x -> x^p non modular
  gen nrandom(environment * env); // random modular integer
  modpoly random(int i,environment * env); // random univariate polynomial of degree i
  void shiftmodpoly(modpoly & a,int n);   // multiply by x^n
  // high = high*x^n + low, size of low must be < n
  void mergemodpoly(modpoly & high,const modpoly & low,int n); 
  modpoly trim(const modpoly & p,environment * env);
  void trim_inplace(modpoly & p);
  void fast_trim_inplace(std::vector<int> & p,int modulo,int maxsize=-1);
  bool trim(modpoly & v); // true if v is empty after trimming
  void rrdm(modpoly & p, int n);   // right redimension poly to degree n

  // ***************
  // Arithmetic
  // ***************
  // !! Do not call Addmodpoly with modpoly slices if new_coord and th/other overlapp
  void Addmodpoly(modpoly::const_iterator th_it,modpoly::const_iterator th_itend,modpoly::const_iterator other_it,modpoly::const_iterator other_itend,environment * env, modpoly & new_coord);
  void addmodpoly(const modpoly & th, const modpoly & other, modpoly & new_coord);
  void addmodpoly(const modpoly & th, const modpoly & other, environment * env,modpoly & new_coord);
  modpoly operator_plus (const modpoly & th,const modpoly & other,environment * env);
  modpoly operator + (const modpoly & th,const modpoly & other);
  void Submodpoly(modpoly::const_iterator th_it,modpoly::const_iterator th_itend,modpoly::const_iterator other_it,modpoly::const_iterator other_itend,environment * env,modpoly & new_coord);
  void submodpoly(const modpoly & th, const modpoly & other, environment * env,modpoly & new_coord);
  void submodpoly(const modpoly & th, const modpoly & other, modpoly & new_coord);
  modpoly operator_minus (const modpoly & th,const modpoly & other,environment * env);
  modpoly operator - (const modpoly & th,const modpoly & other);
  void negmodpoly(const modpoly & th, modpoly & new_coord);
  modpoly operator - (const modpoly & th) ;

  // Warning: mulmodpoly assumes that coeff are integers. 
  // Use operator_times unless you know what you do
  void mulmodpoly(const modpoly & th, const gen & fact, modpoly & new_coord);
  void mulmodpoly(const modpoly & th, const gen & fact, environment * env, modpoly & new_coord);
  void mulmodpoly_naive(modpoly::const_iterator ita,modpoly::const_iterator ita_end,modpoly::const_iterator itb,modpoly::const_iterator itb_end,environment * env,modpoly & new_coord);
  void mulmodpoly_kara_naive(const modpoly & a, const modpoly & b,environment * env,modpoly & new_coord,int seuil_kara);
  // new_coord += a*b in place
  void add_mulmodpoly(const modpoly::const_iterator & ita0,const modpoly::const_iterator & ita_end,const modpoly::const_iterator & itb0,const modpoly::const_iterator & itb_end,environment * env,modpoly & new_coord);
  // modpoly operator * (const modpoly & th, const gen & fact);
  void make_positive(std::vector<int> & f,int p);
  modpoly operator_times (const modpoly & th, const gen & fact,environment * env);
  // commented otherwise int * gen might be interpreted as
  // make a modpoly of size the int and multiply
  modpoly operator_times (const gen & fact,const modpoly & th,environment * env);
  modpoly operator * (const gen & fact,const modpoly & th);
  void mulmodpoly(const modpoly & a, const modpoly & b, environment * env,modpoly & new_coord,int maxdeg=RAND_MAX);
  modpoly operator * (const modpoly & th, const modpoly & other) ;
  modpoly operator_times (const modpoly & th, const modpoly & other,environment * env) ;
  bool operator_times(const std::vector<int> & a,const std::vector<int> & b,int m,std::vector<int> & ab);
  // ichinrem reconstruct in resp1 from resp1/resp2/resp3
  void ichinremp1p2p3(const std::vector<int> & resp1,const std::vector<int> & resp2,const std::vector<int> & resp3,int n,std::vector<int> & res,int modulo);

  void operator_times (const modpoly & a, const modpoly & b,environment * env,modpoly & new_coord,int maxdeg=RAND_MAX);
  // res=(*it) * ... (*(it_end-1))
  void mulmodpoly(std::vector<modpoly>::const_iterator it,std::vector<modpoly>::const_iterator it_end,environment * env,modpoly & new_coord);
  void mulmodpoly(std::vector<modpoly>::const_iterator * it,int debut,int fin,environment * env,modpoly & pi);
  void mulmodpoly(const std::vector<modpoly *> & v,environment * env,modpoly & res,int dbg);

  void divmodpoly(const modpoly & th, const gen & fact, modpoly & new_coord);
  void divmodpoly(const modpoly & th, const gen & fact, environment * env,modpoly & new_coord);
  modpoly operator / (const modpoly & th,const modpoly & other) ;
  int coefftype(const modpoly & v,gen & coefft);
  bool DivRem(const modpoly & th, const modpoly & other, environment * env,modpoly & quo, modpoly & rem,bool allowrational=true);
  bool DenseDivRem(const modpoly & th, const modpoly & other,modpoly & quo, modpoly & rem,bool fastfalsetest=false);
  // Pseudo division a*th = other*quo + rem
  void PseudoDivRem(const dense_POLY1 & th, const dense_POLY1 & other, dense_POLY1 & quo, dense_POLY1 & rem, gen & a);
  modpoly operator / (const modpoly & th,const gen & fact ) ;
  modpoly operator_div (const modpoly & th,const modpoly & other,environment * env) ;
  modpoly operator % (const modpoly & th,const modpoly & other) ;
  modpoly operator_mod (const modpoly & th,const modpoly & other,environment * env) ;
  // division by ascending power (used for power series)
  dense_POLY1 AscPowDivRem(const dense_POLY1 & num, const dense_POLY1 & den,int order);

  modpoly powmod(const modpoly & p,const gen & n,const modpoly & pmod,environment * env);
  gen cstcoeff(const modpoly & q);
  // p=(X-x)q+p(x), first horner returns p(x), second one compute q as well
  gen horner(const gen & g,const gen & x);
  gen horner(const gen & g,const gen & x);
  gen hornermod(const vecteur & v,const gen & alpha,const gen & modulo);
  int hornermod(const vecteur & v,int alpha,int modulo);

  extern const unary_function_ptr * const  at_horner ;
  gen horner(const modpoly & p,const gen & x,environment * env,bool simp=true);
  gen horner(const modpoly & p,const gen & x);
  gen horner(const modpoly & p,const gen & x,environment * env,modpoly & q);
  gen horner(const modpoly & p,const fraction & f,bool simp);
  gen _horner(const gen & args,GIAC_CONTEXT);
  std::complex<double> horner_newton(const vecteur & p,const std::complex<double> &x,GIAC_CONTEXT); // x-p(x)/p'(x)

  void hornerfrac(const modpoly & p,const gen &num, const gen &den,gen & res,gen & d); // res/d=p(num/den)
  // find bounds for p(interval[l,r]) with p, l and r real and exact
  vecteur horner_interval(const modpoly & p,const gen & l,const gen & r);

  gen symb_horner(const modpoly & p,const gen & x,int d);
  gen symb_horner(const modpoly & p,const gen & x);
  // shift polynomial, if P!=0, *P is a list of pascal n-th lines
  modpoly taylor(const modpoly & p,const gen & x,environment * env=0,matrice * P=0);
  // split P=Pp-Pn in two parts, Pp positive coeffs and Pn negative coeffs
  void splitP(const vecteur &P,vecteur &Pp,vecteur &Pn);

  gen norm(const dense_POLY1 & q,GIAC_CONTEXT); // max of |coeff|
  modpoly derivative(const modpoly & p);
  modpoly derivative(const modpoly & p,environment * env);
  // integration of p with coeff in *ascending* order, does not add 0
  // if you use it with usual modpoly, you must reverse(p.begin(),p.end())
  // before and after usage, set shift_coeff to 1, and push_back a zero
  modpoly integrate(const modpoly & p,const gen & shift_coeff);

  // ***************
  // GCD and related. Works currently only if modular aritmetic
  // ***************
  modpoly simplify(modpoly & a, modpoly & b,environment * env);
  gen ppz(dense_POLY1 & q);
  gen lgcd(const dense_POLY1 & p); // gcd of coeffs
  gen lgcd(const dense_POLY1 & p,const gen & g);
  // modular gcd
  void modpoly2smallmodpoly(const modpoly & p,std::vector<int> & v,int m);

  bool gcdmodpoly(const modpoly &p,const modpoly & q,environment * env,modpoly &a); 
  bool gcd_modular_algo(const modpoly &p,const modpoly &q,modpoly &d,modpoly * p_simp,modpoly * q_simp); // p and q must have coeffs in Z or Z[i]

  // half-gcd: a0.size() must be > a1.size(), returns [[A,B],[C,D]]
  bool hgcd(const modpoly & a0,const modpoly & a1,const gen & modulo,modpoly &A,modpoly &B,modpoly &C,modpoly &D,modpoly & a,modpoly & b,modpoly & tmp1,modpoly & tmp2); // a0 is A in Yap, a1 is B

  // fast modular inverse: f*g=1 mod x^l, f must be invertible (f.back()!=0)
  bool invmod(const modpoly & f,int l,environment * env,modpoly & g);
  // for p prime such that p-1 is divisible by 2^N, compute a 2^N-th root of 1
  // otherwise return 0
  unsigned nthroot(unsigned p,unsigned N);
  // euclidean quotient using modular inverse
  // returns 0 on failure, 1 on success, 2 if division is exact
  int DivQuo(const modpoly & a, const modpoly & b, environment * env,modpoly & q);
  // 1-d modular for small modulus<sqrt(RAND_MAX)
  bool gcdsmallmodpoly(const polynome &p,const polynome & q,int m,polynome & d,polynome & dp,polynome & dq,bool compute_cof); 
  void smallmodpoly2modpoly(const std::vector<int> & v,modpoly & p,int m);
  void gcdsmallmodpoly(const std::vector<int> &p,const std::vector<int> & q,int m,std::vector<int> & d);
  void gcdsmallmodpoly(const std::vector<int> &p,const std::vector<int> & q,int m,std::vector<int> & d,std::vector<int> * pcof,std::vector<int> * qcof);
  void gcdsmallmodpoly(const std::vector<int> &p,const std::vector<int> & q,int m,std::vector<int> & d);
  void gcdsmallmodpoly(const modpoly &p,const modpoly & q,int m,modpoly & d);
  void DivRem(const std::vector<int> & th, const std::vector<int> & other,int m,std::vector<int> & quo, std::vector<int> & rem,bool ck_exactquo=false);
  modpoly gcd(const modpoly & a,const modpoly &b,environment * env,bool call_ntl=false); 
  void euclide_gcd(const modpoly &p,const modpoly & q,environment * env,modpoly &a);
  // n-var modular gcd
  bool gcd_modular(const polynome &p_orig, const polynome & q_orig, polynome & pgcd,polynome & pcofactor,polynome & qcofactor,bool compute_cofactors);

  bool convert(const polynome &p_orig, const polynome & q_orig,index_t & d,std::vector<hashgcd_U> & vars,std::vector< T_unsigned<gen,hashgcd_U> > & p,std::vector< T_unsigned<gen,hashgcd_U> > & q);
  bool convert(const polynome &p_orig, const polynome & q_orig,index_t & d,std::vector<hashgcd_U> & vars,std::vector< T_unsigned<int,hashgcd_U> > & p,std::vector< T_unsigned<int,hashgcd_U> > & q,int modulo);
  bool modgcd(const polynome &p_orig, const polynome & q_orig, const gen & modulo, polynome & d,polynome & pcofactor,polynome & qcofactor,bool compute_cofactors);
  bool mod_gcd_c(const polynome &p_orig, const polynome & q_orig, const gen & modulo, polynome & d,polynome & pcofactor,polynome & qcofactor,bool compute_cofactors);
  bool mod_gcd(const polynome &p_orig, const polynome & q_orig, const gen & modulo, polynome & pgcd,polynome & pcofactor,polynome & qcofactor,bool compute_cofactors);
  modpoly lcm(const modpoly & a,const modpoly &b,environment * env); 
  bool gcd_modular_algo1(polynome &p,polynome &q,polynome &d,bool compute_cof);

  // check if a (normally a fraction) modulo m is equal to p
  bool chk_equal_mod(const gen & a,longlong p,int m);
  // p1*u+p2*v=d
  void egcd(const modpoly &p1, const modpoly & p2, environment * env,modpoly & u,modpoly & v,modpoly & d,bool deterministic=true);
  // a*u+b*v=d optimized for GMP, if u_ptr and v_ptr are non 0 compute previous line of Euclide algorithm, stops as soon as degree<degstop
  bool egcd_mpz(const modpoly & a,const modpoly &b,int degstop,const gen & m,modpoly & u,modpoly &v,modpoly & d,modpoly * u_ptr,modpoly * v_ptr,modpoly * r_ptr);
  bool egcd_pade(const modpoly & n,const modpoly & x,int l,modpoly & a,modpoly &b,environment * env,bool psron=true);
  // alg extension norme for p_y, alg extension defined by pmini
  bool algnorme(const polynome & p_y,const polynome & pmini,polynome & n);
  void subresultant(const modpoly & P,const modpoly & Q,gen & res);
  int sizeinbase2(const gen & g);
  int sizeinbase2(const vecteur & v);
  // multinomial power by Miller pure recurrence 
  bool miller_pow(const modpoly & p_,unsigned m,modpoly & res);

  // Given [v_0 ... v_(2n-1)] (begin of the recurrence sequence) 
  // return [b_n...b_0] such that b_n*v_{n+k}+...+b_0*v_k=0
  // Example [1,-1,3,3] -> [1,-3,-6]
  vecteur reverse_rsolve(const vecteur & v,bool psron=true);
  // given a and c, find u such that 
  // a[0]*...a[n-1]*u[n]+a[0]*...*a[n-2]*a[n]*u[n-1]+...+a[1]*...*a[n-1]*u[0]=1
  bool egcd(const std::vector<modpoly> & a, environment * env,std::vector<modpoly> & u);
  // same as above
  std::vector<modpoly> egcd(const std::vector<modpoly> & a, environment * env);
  // assuming pmod and qmod are prime together, find r such that
  // r = p mod pmod  and r = q mod qmod
  // hence r = p + A*pmod = q + B*qmod
  // or A*pmod -B*qmod = q - p
  // assuming u*pmod+v*pmod=d we get
  // A=u*(q-p)/d
  dense_POLY1 ichinrem(const dense_POLY1 &p,const dense_POLY1 & q,const gen & pmod,const gen & qmod);
  modpoly chinrem(const modpoly & p,const modpoly & q, const modpoly & pmod, const modpoly & qmod,environment * env);
  int ichinrem_inplace(dense_POLY1 &p,const std::vector<int> & q,const gen & pmod,int qmodval,int reserve_mem=0); // 0 error, 1 p changed, 2 p unchangedb
  bool ichinrem_inplace(dense_POLY1 &p,const dense_POLY1 & q,const gen & pmod,int qmodval);
  void divided_differences(const vecteur & x,const vecteur & y,vecteur & res,environment * env);
  // in-place modification and exact division if divexact==true
  void divided_differences(const vecteur & x,vecteur & res,environment * env,bool divexact);
  void interpolate(const vecteur & x,const vecteur & y,modpoly & res,environment * env);
  void interpolate_inplace(const vecteur & x,modpoly & res,environment * env);
  void mulpoly_interpolate(const polynome & p,const polynome & q,polynome & res,environment * env);
  bool dotvecteur_interp(const vecteur & a,const vecteur &b,gen & res);
  bool mmult_interp(const matrice & a,const matrice &b,matrice & res);
  bool poly_pcar_interp(const matrice & a,vecteur & p,bool compute_pmin,GIAC_CONTEXT);
  void polymat2matpoly(const vecteur & R,vecteur & res);

  void makepositive(int * p,int n,int modulo);
  // Fast Fourier Transform, f the poly sum_{j<n} f_j x^j, 
  // and w=[1,omega,...,omega^[m-1]] with m a multiple of n
  // return [f(1),f(omega),...,f(omega^[n-1])
  // WARNING f is given in ascending power
  void fft(const modpoly & f,const modpoly & w ,modpoly & res,environment * env);
  // void fft(std::vector< std::complex<double> >& f,const std::vector< std::complex<double> > & w ,std::vector<std::complex< double> > & res);
  void fft(std::complex<double> * f,int n,const std::complex<double> * w,int m,std::complex< double> * t);
  void fft(const std::vector<int> & f,const std::vector<int> & w ,std::vector<int> & res,int modulo);
  // res=a .* b mod p
  void fft_ab_p(const std::vector<int> &a,const std::vector<int> &b,std::vector<int> & res,int p);
  // res=a ./ b mod p, returns false if a division fails
  bool fft_aoverb_p(const std::vector<int> &a,const std::vector<int> &b,std::vector<int> & res,int p);
  // reverse the table of root of unity^k for inverse fft
  void fft_reverse(std::vector<int> & W,int p);

  // res=a*b mod p
  bool fft2mult(int ablinfnorm,const std::vector<int> & a,const std::vector<int> & b,std::vector<int> & res,int modulo,std::vector<int> & W,std::vector<int> & fftmult_p,std::vector<int> & fftmult_q,bool reverseatend,bool dividebyn,bool makeplus);
  bool fftmultp1234(const modpoly & p,const modpoly & q,const gen &P,const gen &Q,modpoly & pq,int modulo, std::vector<int> & a,std::vector<int>&b,std::vector<int> &resp1,std::vector<int>&resp2,std::vector<int> & resp3, std::vector<int> & Wp1,std::vector<int> & Wp2, std::vector<int> & Wp3,std::vector<int> & Wp4,std::vector<int> &tmp_p,std::vector<int> &tmp_q,bool compute_pq);
  // FFT mod 2^{r*2^l}+1, tmp1, tmp2 temporary gen must be _ZINT
  void fft2rl(gen * f,long n,int r,int l,gen * t,bool direct,gen & tmp1, gen & tmp2,mpz_t & tmpqz);
  // alpha[i] *= beta[i] mod 2^(expoN)+1
  void fft2rltimes(modpoly & alpha,const modpoly & beta,unsigned long expoN,mpz_t & tmp,mpz_t & tmpqz);
  void fft2rltimes(const modpoly & alpha,const modpoly & beta,modpoly & res,unsigned long expoN,mpz_t & tmp,mpz_t & tmpqz);
  // pq *= -2^shift mod N=2^(expoN+1) where -2^shift is the inverse of n mod N
  void fft2rldiv(modpoly & pq,unsigned long expoN,unsigned long shift,mpz_t & tmp,mpz_t & tmpqz);
  gen intnorm(const dense_POLY1 & p,GIAC_CONTEXT);

  // FFT representation is a triplet of FFT mod p1, p2, p3
  struct fft_rep {
    int modulo;   
    // modulo=0 means we make the computation for the 3 primes p1, p2 and p3
    // modulo!=0 means we reconstruct mod modulo with p1, p2, p3
    std::vector<int> modp1,modp2,modp3;
  };

  // p -> f it's FFT representation, p is reversed before FFT is called
  // a is a temporary vector = A mod modulo after call
  // Wp1, Wp2, Wp3 is a vector of powers of the n-th root of unity mod p1,p2,p3
  // if size is not n or Wp[0]==0, Wp is computed
  // do not share Wp1/p2/p3 between different threads
  void to_fft(const vecteur & A,int modulo,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,std::vector<int> & a,int n,fft_rep & f,bool reverse,bool makeplus);
  void to_fft(const std::vector<int> & a,int modulo,int w,std::vector<int> & Wp,int n,std::vector<int> & f,int reverse,bool makeplus,bool makemod=true);
  void to_fft(const std::vector<int> & a,int modulo,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,int n,fft_rep & f,bool reverse,bool makeplus,bool makemod=true);
  // FFT representation f -> res
  // Wp1,p2,p3 should be computed with to_fft
  // division by n=size of f.modp1/p2/p3 is done
  // result should normally be reversed at end
  // tmp1/p2/p3 are temporary vectors
  // do not share Wp1/p2/p3 between different threads
  void from_fft(const fft_rep & f,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,std::vector<int> & res,std::vector<int> & tmp1,std::vector<int> & tmp2,std::vector<int> & tmp3,bool reverseatend,bool revw);
  void from_fft(const std::vector<int> & f,int p,std::vector<int> & Wp,std::vector<int> & res,bool reverseatend,bool revw);

  struct multi_fft_rep {
    gen modulo;
    fft_rep p1p2p3; // for p1, p2 and p3
    std::vector<fft_rep> v; // one fft_rep for each prime < p1, !=p2 and !=p3
  };
  // convert A to multi fft representation, with a number of primes
  // suitable for ichinrem reconstruction mod modulo
  void to_multi_fft(const vecteur & A,const gen & modulo,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,unsigned long n,multi_fft_rep & f,bool reverse,bool makeplus);
  void from_multi_fft(const multi_fft_rep & f,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,vecteur & res,bool reverseatend);
  void multi_fft_ab_cd(const multi_fft_rep & a,const multi_fft_rep & b,const multi_fft_rep & c,const multi_fft_rep & d,multi_fft_rep & res);

  // Convolution of p and q, omega a n-th root of unity, n=2^k
  // WARNING p0 and q0 are given in ascending power
  void fftconv(const modpoly & p,const modpoly & q,unsigned long k,unsigned long n,const gen & omega,modpoly & pq,environment * env);
  // Convolution of p and q, omega a n-th root of unity, n=2^k
  // p and q are given in descending power order
  void fftconv(const modpoly & p0,const modpoly & q0,unsigned long k,const gen & omega,modpoly & pq,environment * env);
  bool fftmult(const modpoly & p,const modpoly & q,modpoly & pq,int modulo,int maxdeg=RAND_MAX);
  modpoly fftmult(const modpoly & p,const modpoly & q);
  // input A with positive int, output fft in A
  // w a 2^n-th root of unity mod p
  void fft2( int *A, int n, int w, int p ,bool permute=true);
  // float fft, theta should be +/-2*M_PI/n
  void fft2( std::complex<double> * A, int n, double theta );
  // resultant on Z, if eps!=0 might be non deterministic
  gen mod_resultant(const modpoly & P,const modpoly & Q,double eps);
  modpoly unmod(const modpoly & a,const gen & m);
  // resultant of P and Q modulo m, modifies P and Q, 
  int resultant_int(std::vector<int> & P,std::vector<int> & Q,std::vector<int> & tmp1,std::vector<int> & tmp2,int m,int w=0);
#ifndef USE_GMP_REPLACEMENTS
  void vecteur2vector_ll(const vecteur & v,longlong m,std::vector<longlong> & res);
  longlong resultantll(std::vector<longlong> & P,std::vector<longlong> & Q,std::vector<longlong> & tmp1,std::vector<longlong> & tmp2,longlong m);
#endif

  bool ntlresultant(const modpoly &p,const modpoly &q,const gen & modulo,gen & res,bool ntl_on_check=true);
  bool ntlxgcd(const modpoly &a,const modpoly &b,const gen & modulo,modpoly & reu,modpoly &v,modpoly & d,bool ntl_on_check=true);
  bool ntlgcd(const modpoly &a,const modpoly &b,const gen & modulo,modpoly & d,bool ntl_on_check=true);
 #ifdef HAVE_LIBNTL
#ifdef HAVE_LIBPTHREAD
  extern pthread_mutex_t ntl_mutex;
#endif
  typedef gen inttype; 

  bool polynome2tab(const polynome & p,int deg,inttype * tab);
  polynome tab2polynome(const inttype * tab,int deg);
  bool ininttype2ZZ(const inttype & temp,const inttype & step,NTL::ZZ & z,const NTL::ZZ & zzstep);
  NTL::ZZ inttype2ZZ(const inttype & i);
  void inZZ2inttype(const NTL::ZZ & zztemp,int pow2,inttype & temp);
  inttype ZZ2inttype(const NTL::ZZ & z);
  NTL::ZZX tab2ZZX(const inttype * tab,int degree);
  void ZZX2tab(const NTL::ZZX & f,int & degree,inttype * & tab);

  // Z/nZ[X] conversions
  NTL::GF2X modpoly2GF2X(const modpoly & p);
  modpoly GF2X2modpoly(const NTL::GF2X & f);
  NTL::ZZ_pX modpoly2ZZ_pX(const modpoly & p);
  modpoly ZZ_pX2modpoly(const NTL::ZZ_pX & f);

#endif

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // NO_NAMESPACE_GIAC

#endif // _GIAC_MODPOLY_H
