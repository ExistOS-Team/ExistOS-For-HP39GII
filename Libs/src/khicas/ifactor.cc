// -*- mode:C++ ; compile-command: "g++-3.4 -I.. -g -c ifactor.cc -DHAVE_CONFIG_H -DIN_GIAC" -*-
#include "giacPCH.h"
#if defined NSPIRE_NEWLIB || !defined KHICAS
#define GIAC_MPQS // define if you want to use giac for sieving 
#endif

#ifdef BF2GMP_H
#undef HAVE_LIBECM
#undef HAVE_LIBBERNMM
#endif

#if defined HAVE_LIBECM 
#include <ecm.h>
#endif

#if defined HAVE_LIBBERNMM 
#include <bern_modp.h>
#include <bern_rat.h>
#endif

#include "path.h"
/*
 *  Copyright (C) 2003,14 R. De Graeve & B. Parisse, 
 *  Institut Fourier, 38402 St Martin d'Heres
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
#ifdef GIAC_HAS_STO_38
//#undef clock
//#undef clock_t
#else
#include <fstream>
//#include <unistd.h> // For reading arguments from file
#include "ifactor.h"
#include "pari.h"
#include "usual.h"
#include "sym2poly.h"
#include "rpn.h"
#include "prog.h"
#include "misc.h"
#include "giacintl.h"
#endif

#ifdef GIAC_HAS_STO_38
#define BESTA_OS
#endif
// Trying to make ifactor(2^128+1) work on ARM
#if defined(RTOS_THREADX) || defined(BESTA_OS) || defined NSPIRE
//#define OLD_AFACT
#define GIAC_ADDITIONAL_PRIMES 16// if defined, additional primes are used in sieve
#else
#define GIAC_ADDITIONAL_PRIMES 32// if defined, additional primes are used in sieve
#endif


#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

#if 0
  struct int256 {
    longlong a;
    ulonglong b,c,d;
  };
  struct int128 {
    longlong a;
    ulonglong b;
  };

  void sub(int256 A,int b,int256 & C){
    bool Apos=A.a>=0;
    if (Apos){
      if (b<0){
	add(A,b,C);
	return;
      }
      bool carry=A.d<b;
      A.d -= b;
      if (carry){
	carry=A.c<1;
	A.c--;
	if (carry){
	  carry=A.b<1;
	  A.b--;
	  if (carry)
	    A.a--;
	}
      }
      C=A;
      return;
    }
    A.a = -A.a;
    sub(A,-b,C);
    C.a = -C.a;
  }

  void add(const int192 & A,int b,int192 & C){
    bool Apos=A.a>=0;
    if (Apos){
      if (b<0){
	sub(A,-b,C);
	return;
      }
      A.d += b;
      if (A.d<b){
	A.c++;
	if (A.c==0){
	  A.b++;
	  if (A.n==0)
	    A.a++;
	}
      }
      C=A;
      return;
    }
    A.a = -A.a;
    sub(A,-b,C);
    C.a = -C.a;
  }

  inline void int128tounsigned(const int128 & A,unsigned & Aa,unsigned & Ab,unsigned & Ac,unsigned & Ad){
    Aa = (* ((unsigned *) (&A.a)+1)) & 0x7fff;
    Ab = *(unsigned *) A.a;
    Ac = *( (unsigned *) &A.b)+1;
    Ad = *(unsigned *) A.d;
  }

  void mul(int128 A,int128 B,int256 & C){
    bool neg=(A.a<0) ^ (B.a<0);
    unsigned A3,A2,A1,A0,B3,B2,B1,B0,C6,C5,C4,C3,C2,C1,C0;
    int128tounsigned(A,A3,A2,A1,A0);
    int128tounsigned(B,B3,B2,B1,B0);
    ulonglong p1,p2,p3,p;
    p1=A0; p2=B0;
    p3=p1*p2;
    C0=p3;
    C1=p3>>32;
    p1=A1;
    p = p1*p2;
    p += C1;
    unsigned short carry = (p<C1);
    p1=A0; p2=B1;
    p3=p1*p2;
    p += p3;
    carry += (p<p3);
    C1 = p;
    C2 = p>>32;
    C2 += carry;
    carry = (C2<carry);
  }

#endif

  const short int giac_primes[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,1163,1171,1181,1187,1193,1201,1213,1217,1223,1229,1231,1237,1249,1259,1277,1279,1283,1289,1291,1297,1301,1303,1307,1319,1321,1327,1361,1367,1373,1381,1399,1409,1423,1427,1429,1433,1439,1447,1451,1453,1459,1471,1481,1483,1487,1489,1493,1499,1511,1523,1531,1543,1549,1553,1559,1567,1571,1579,1583,1597,1601,1607,1609,1613,1619,1621,1627,1637,1657,1663,1667,1669,1693,1697,1699,1709,1721,1723,1733,1741,1747,1753,1759,1777,1783,1787,1789,1801,1811,1823,1831,1847,1861,1867,1871,1873,1877,1879,1889,1901,1907,1913,1931,1933,1949,1951,1973,1979,1987,1993,1997,1999,2003,2011,2017,2027,2029,2039,2053,2063,2069,2081,2083,2087,2089,2099,2111,2113,2129,2131,2137,2141,2143,2153,2161,2179,2203,2207,2213,2221,2237,2239,2243,2251,2267,2269,2273,2281,2287,2293,2297,2309,2311,2333,2339,2341,2347,2351,2357,2371,2377,2381,2383,2389,2393,2399,2411,2417,2423,2437,2441,2447,2459,2467,2473,2477,2503,2521,2531,2539,2543,2549,2551,2557,2579,2591,2593,2609,2617,2621,2633,2647,2657,2659,2663,2671,2677,2683,2687,2689,2693,2699,2707,2711,2713,2719,2729,2731,2741,2749,2753,2767,2777,2789,2791,2797,2801,2803,2819,2833,2837,2843,2851,2857,2861,2879,2887,2897,2903,2909,2917,2927,2939,2953,2957,2963,2969,2971,2999,3001,3011,3019,3023,3037,3041,3049,3061,3067,3079,3083,3089,3109,3119,3121,3137,3163,3167,3169,3181,3187,3191,3203,3209,3217,3221,3229,3251,3253,3257,3259,3271,3299,3301,3307,3313,3319,3323,3329,3331,3343,3347,3359,3361,3371,3373,3389,3391,3407,3413,3433,3449,3457,3461,3463,3467,3469,3491,3499,3511,3517,3527,3529,3533,3539,3541,3547,3557,3559,3571,3581,3583,3593,3607,3613,3617,3623,3631,3637,3643,3659,3671,3673,3677,3691,3697,3701,3709,3719,3727,3733,3739,3761,3767,3769,3779,3793,3797,3803,3821,3823,3833,3847,3851,3853,3863,3877,3881,3889,3907,3911,3917,3919,3923,3929,3931,3943,3947,3967,3989,4001,4003,4007,4013,4019,4021,4027,4049,4051,4057,4073,4079,4091,4093,4099,4111,4127,4129,4133,4139,4153,4157,4159,4177,4201,4211,4217,4219,4229,4231,4241,4243,4253,4259,4261,4271,4273,4283,4289,4297,4327,4337,4339,4349,4357,4363,4373,4391,4397,4409,4421,4423,4441,4447,4451,4457,4463,4481,4483,4493,4507,4513,4517,4519,4523,4547,4549,4561,4567,4583,4591,4597,4603,4621,4637,4639,4643,4649,4651,4657,4663,4673,4679,4691,4703,4721,4723,4729,4733,4751,4759,4783,4787,4789,4793,4799,4801,4813,4817,4831,4861,4871,4877,4889,4903,4909,4919,4931,4933,4937,4943,4951,4957,4967,4969,4973,4987,4993,4999,5003,5009,5011,5021,5023,5039,5051,5059,5077,5081,5087,5099,5101,5107,5113,5119,5147,5153,5167,5171,5179,5189,5197,5209,5227,5231,5233,5237,5261,5273,5279,5281,5297,5303,5309,5323,5333,5347,5351,5381,5387,5393,5399,5407,5413,5417,5419,5431,5437,5441,5443,5449,5471,5477,5479,5483,5501,5503,5507,5519,5521,5527,5531,5557,5563,5569,5573,5581,5591,5623,5639,5641,5647,5651,5653,5657,5659,5669,5683,5689,5693,5701,5711,5717,5737,5741,5743,5749,5779,5783,5791,5801,5807,5813,5821,5827,5839,5843,5849,5851,5857,5861,5867,5869,5879,5881,5897,5903,5923,5927,5939,5953,5981,5987,6007,6011,6029,6037,6043,6047,6053,6067,6073,6079,6089,6091,6101,6113,6121,6131,6133,6143,6151,6163,6173,6197,6199,6203,6211,6217,6221,6229,6247,6257,6263,6269,6271,6277,6287,6299,6301,6311,6317,6323,6329,6337,6343,6353,6359,6361,6367,6373,6379,6389,6397,6421,6427,6449,6451,6469,6473,6481,6491,6521,6529,6547,6551,6553,6563,6569,6571,6577,6581,6599,6607,6619,6637,6653,6659,6661,6673,6679,6689,6691,6701,6703,6709,6719,6733,6737,6761,6763,6779,6781,6791,6793,6803,6823,6827,6829,6833,6841,6857,6863,6869,6871,6883,6899,6907,6911,6917,6947,6949,6959,6961,6967,6971,6977,6983,6991,6997,7001,7013,7019,7027,7039,7043,7057,7069,7079,7103,7109,7121,7127,7129,7151,7159,7177,7187,7193,7207,7211,7213,7219,7229,7237,7243,7247,7253,7283,7297,7307,7309,7321,7331,7333,7349,7351,7369,7393,7411,7417,7433,7451,7457,7459,7477,7481,7487,7489,7499,7507,7517,7523,7529,7537,7541,7547,7549,7559,7561,7573,7577,7583,7589,7591,7603,7607,7621,7639,7643,7649,7669,7673,7681,7687,7691,7699,7703,7717,7723,7727,7741,7753,7757,7759,7789,7793,7817,7823,7829,7841,7853,7867,7873,7877,7879,7883,7901,7907,7919,7927,7933,7937,7949,7951,7963,7993,8009,8011,8017,8039,8053,8059,8069,8081,8087,8089,8093,8101,8111,8117,8123,8147,8161,8167,8171,8179,8191,8209,8219,8221,8231,8233,8237,8243,8263,8269,8273,8287,8291,8293,8297,8311,8317,8329,8353,8363,8369,8377,8387,8389,8419,8423,8429,8431,8443,8447,8461,8467,8501,8513,8521,8527,8537,8539,8543,8563,8573,8581,8597,8599,8609,8623,8627,8629,8641,8647,8663,8669,8677,8681,8689,8693,8699,8707,8713,8719,8731,8737,8741,8747,8753,8761,8779,8783,8803,8807,8819,8821,8831,8837,8839,8849,8861,8863,8867,8887,8893,8923,8929,8933,8941,8951,8963,8969,8971,8999,9001,9007,9011,9013,9029,9041,9043,9049,9059,9067,9091,9103,9109,9127,9133,9137,9151,9157,9161,9173,9181,9187,9199,9203,9209,9221,9227,9239,9241,9257,9277,9281,9283,9293,9311,9319,9323,9337,9341,9343,9349,9371,9377,9391,9397,9403,9413,9419,9421,9431,9433,9437,9439,9461,9463,9467,9473,9479,9491,9497,9511,9521,9533,9539,9547,9551,9587,9601,9613,9619,9623,9629,9631,9643,9649,9661,9677,9679,9689,9697,9719,9721,9733,9739,9743,9749,9767,9769,9781,9787,9791,9803,9811,9817,9829,9833,9839,9851,9857,9859,9871,9883,9887,9901,9907,9923,9929,9931,9941,9949,9967,9973};

  // #define PRIME_SIEVE
#ifdef PRIME_SIEVE  
  // fill Erathosthene sieve crible for searching primes up to 2*crible.size()*32+1
  // crible is a (packed) bit array, crible[i] is true if 2*i+1 is a prime
  // crible must be set to true at startup
  void fill_crible(vector<unsigned> & crible,unsigned p){
    crible.resize((p-1)/64+1);
    unsigned cs=crible.size();
    unsigned lastnum=64*cs;
    unsigned lastsieve=int(std::sqrt(double(lastnum)));
    unsigned primesieved=1;
    crible[0] = 0xfffffffe; // 1 is not prime and not sieved (2 is not sieved)
    for (unsigned i=1;i<cs;++i)
      crible[i]=0xffffffff;
    for (;primesieved<=lastsieve;primesieved+=2){
      // find next prime
      unsigned pos=primesieved/2;
      for (;pos<cs;pos++){
	if (crible[pos/32] & (1 << (pos %32)))
	  break;
      }
      // set mutiples of (2*pos+1) to false
      primesieved=2*pos+1;
      unsigned n=3*primesieved;
      for (;n<lastnum;n+=2*primesieved){
	pos=(n-1)/2;
	crible[(pos/32)] &= ~(1<<(pos %32));
      }
    }
  }
  unsigned nextprime(vector<unsigned> & crible,unsigned p){
    // assumes crible has been filled
    ++p;
    if (p%2==0)
      ++p;
    unsigned pos=(p-1)/2,cs=crible.size()*32; 
    if (2*cs+1<=p)
      return nextprime(int(p)).val;
    for (;pos<cs;++pos){
      if (crible[pos/32] & (1<<(pos%32))){
	pos=2*pos+1;
	// if (pos!=nextprime(int(p)).val) CERR << "error " << p << '\n';
	return pos;
      }
    }
    return nextprime(int(p)).val; // not found
  }
#endif

  static const int giac_last_prime=giac_primes[sizeof(giac_primes)/sizeof(short)-1];
#if defined RTOS_THREADX || defined BESTA_OS || defined NSPIRE
  const unsigned QS_SIZE=65536; // number of slicetype in a sieve slice
  typedef unsigned char slicetype; // define to unsigned char if not enough
#else
  const unsigned QS_SIZE=65536; // number of slicetype in a sieve slice
  typedef unsigned char slicetype; // define to unsigned char if not enough
#endif

#ifdef NSPIRE
  template<class T>
  static void printbool(nio::ios_base<T> & os,const vector<unsigned> & v,int C=1){
    if (C)
      C=giacmin(C,int(v.size()));
    else
      C=v.size();
    for (int c=0;c<C;++c){
      for (int s=0;s<32;++s){
	os << (((v[c] >> s & 1)==1)?1:0) << " ";
      }
    }
    os << '\n';
  }

  template<class T>
  void printbool(nio::ios_base<T> & os,const vector< vector<unsigned> > & m,int L=32){
    if (L)
      L=giacmin(L,int(m.size()));
    else
      L=m.size();
    for (int l=0;l<L;++l){
      printbool(os,m[l]);
    }
  }
#else
  static void printbool(ostream & os,const vector<unsigned> & v,int C=1){
    if (C)
      C=giacmin(C,int(v.size()));
    else
      C=int(v.size());
    for (int c=0;c<C;++c){
      for (int s=0;s<32;++s){
	os << (((v[c] >> s & 1)==1)?1:0) << " ";
      }
    }
    os << '\n';
  }

  void printbool(ostream & os,const vector< vector<unsigned> > & m,int L=32){
    if (L)
      L=giacmin(L,int(m.size()));
    else
      L=int(m.size());
    for (int l=0;l<L;++l){
      printbool(os,m[l]);
    }
  }
#endif

  template <class T>
  inline void swap(T * & ptr1, T * & ptr2){
    T * tmp=ptr1;
    ptr1=ptr2;
    ptr2=tmp;
  }

#ifdef x86_64
#define GIAC_RREF_UNROLL 4
#else
#define GIAC_RREF_UNROLL 4
#endif

  // #define RREF_SORT
#ifdef RREF_SORT
  struct line_t {
    unsigned * tab;
    unsigned count;
  };

  bool operator < (const line_t & l1,const line_t & l2){
    if (!l1.count)
      return false;
    if (!l2.count)
      return true;
    return l1.count<l2.count;
  }

  unsigned count_ones(unsigned * tab,int C32){
    register unsigned r=0;
    register unsigned * tabend=tab+C32;
    for (;tab!=tabend;++tab){
      register unsigned u=*tab;
      while (u){
	r += u & 1;
	u >>= 1;
      }
    }
    return r;
  }

#else
  struct line_t {
    unsigned * tab;
  };
#endif
  

  // mode=0: full reduction, 1 subreduction, 2 finish full reduction from subreduction
  void rref(vector< line_t > & m,int L,int C32,int mode){
    int i,l=0,c=0,C=C32*32;
    for (;l<L && c<C;){
      // printbool(CERR,m);
      int c1=c/32,c2=c%32;
      // find first non-0 pivot in col c starting at row l
      for (i=l;i<L;++i){
	if ((m[i].tab[c1] >> c2) & 1)
	  break;
      }
      if (i==L){ // none found in this column
	++c;
	continue;
      }
      if (i!=l) 
	swap(m[i].tab,m[l].tab); // don't care about count...
      int start=mode==1?l+1:0, end=mode==2?l:L;
#ifdef x86_64
      ulonglong * pivend, * pivbeg;
      pivbeg = (ulonglong *) (m[l].tab+(c1/GIAC_RREF_UNROLL)*GIAC_RREF_UNROLL);
      pivend = (ulonglong *) (m[l].tab+C32);
#else
      unsigned * pivbeg = m[l].tab+(c1/GIAC_RREF_UNROLL)*GIAC_RREF_UNROLL, * pivend = m[l].tab+C32;
#endif
      for (i=start;i<end;++i){
	if (i==l || ( (m[i].tab[c1] >> c2) & 1)!=1) 
	  continue;
	// line combination l and i
#ifdef x86_64
	ulonglong * curptr=(ulonglong *) (m[i].tab+(c1/GIAC_RREF_UNROLL)*GIAC_RREF_UNROLL);
	for (ulonglong * pivptr=pivbeg;pivptr!=pivend;curptr += GIAC_RREF_UNROLL/2,pivptr += GIAC_RREF_UNROLL/2){
	  // small optimization (loop unroll), assumes mult of 4(*32) columns
	  // PREFETCH(curptr+8);
	  *curptr ^= *pivptr;
	  curptr[1] ^= pivptr[1];
#if GIAC_RREF_UNROLL==8
	  curptr[2] ^= pivptr[2];
	  curptr[3] ^= pivptr[3];
#endif
	}
#else
	unsigned * curptr=m[i].tab+(c1/GIAC_RREF_UNROLL)*GIAC_RREF_UNROLL;
	for (unsigned * pivptr=pivbeg;pivptr!=pivend;curptr += GIAC_RREF_UNROLL,pivptr += GIAC_RREF_UNROLL){
	  // small optimization (loop unroll), assumes mult of 4(*32) columns
	  // PREFETCH(curptr+16);
	  *curptr ^= *pivptr;
	  curptr[1] ^= pivptr[1];
	  curptr[2] ^= pivptr[2];
	  curptr[3] ^= pivptr[3];
#if GIAC_RREF_UNROLL==8
	  curptr[4] ^= pivptr[4];
	  curptr[5] ^= pivptr[5];
	  curptr[6] ^= pivptr[6];
	  curptr[7] ^= pivptr[7];
#endif
	}
#endif
      }
      ++l;
      ++c;
    }
  }

  template <class T>
  void release_memory(vector<T> & slice){
    // release memory from slice
    vector<T> tmp;
    swap(slice,tmp);
  }

#ifdef USE_GMP_REPLACEMENTS
  int modulo(const mpz_t & a,unsigned b){
    if (mpz_cmp_ui(a,0)<0){
      mpz_neg(*(mpz_t *)&a,a);
      int res=modulo(a,b);
      mpz_neg(*(mpz_t *)&a,a);
      return b-res;
    }
    mp_digit C; 
    mp_mod_d((mp_int *)&a,b,&C);
    return C;
  }
#else
  int modulo(const mpz_t & a,unsigned b){
    return mpz_fdiv_ui(a,b);
  }
#endif

  int modulo(const gen & a,unsigned b){
    if (a.type==_INT_)
      return a.val % b;
    return modulo(*a._ZINTptr,b);
  }

#if defined RTOS_THREADX || defined BESTA_OS || defined NSPIRE
  typedef unsigned short pui_t ;
  typedef unsigned short ushort_t;
  typedef short short_t;
#else
  typedef unsigned pui_t ;
  // #ifndef USE_GMP_REPLACEMENTS // uncomment for Aspen debugging
#define PRIMES32
  // #endif
#ifdef PRIMES32
  typedef unsigned ushort_t;
  typedef int short_t;
#else
  typedef unsigned short ushort_t;
  typedef unsigned short int short_t;
#endif

#if defined(EMCC) || defined(EMCC2)
#include <map>
#endif
#if (defined EMCC || defined EMCC2 || defined(HASH_MAP_NAMESPACE)) && defined(PRIMES32) && !defined(ADDITIONAL_PRIMES_HASHMAP)
#define ADDITIONAL_PRIMES_HASHMAP
#endif
#endif // RTOS_THREADX || BESTA_OS

  struct axbinv {
#if 0
    unsigned short aindex;
    unsigned short bindex;
#else
    unsigned aindex;
    unsigned bindex;
#endif
    int shiftpos;
    pui_t first,second; // indexes in the "puissancestab" table
    axbinv(ushort_t a_,int shiftpos_,ushort_t b_,pui_t f_,pui_t s_):aindex(a_),bindex(b_),shiftpos(shiftpos_),first(f_),second(s_) {};
    axbinv() {};
  };

#ifdef ADDITIONAL_PRIMES_HASHMAP
  unsigned largep(const axbinv & A,ushort_t * puissancestab) { 
    // return A.largeprime;
    if (A.second-A.first<3) return 0;
#ifdef PRIMES32
    if (*(puissancestab+A.second-2)!=1)
      return 0;
    return *(puissancestab+A.second-1);
#else
    if (*(puissancestab+A.second-3)!=1)
      return 0;
    return (unsigned(*(puissancestab+A.second-2)) << 16)  + *(puissancestab+A.second-1);
#endif      
  }
#endif

#ifdef ADDITIONAL_PRIMES_HASHMAP
#if defined(EMCC) || defined(EMCC2) // container does not seem to be important for <= 70 digits
  typedef map<unsigned,axbinv> additional_map_t;
#else
  typedef HASH_MAP_NAMESPACE::hash_map<unsigned,axbinv,hash_function_unsigned_object > additional_map_t ;
#endif
#endif

#if !defined(RTOS_THREADX) && !defined(BESTA_OS) && !defined NSPIRE
  // #define WITH_INVA
#if defined(__APPLE__) || defined(x86_64)
#define LP_TAB_SIZE 15 // slice size will be 2^LP_TAB_SIZE
  //  #define LP_SMALL_PRIMES
#define LP_TAB_TOGETHER
#define USE_MORE_PRIMES
#else
#define LP_TAB_SIZE 15 // slice size will be 2^LP_TAB_SIZE
#endif // APPLE or 64 bits
#endif // !defined RTOS_THREADX and BESTA_OS

#ifdef LP_TAB_SIZE
#define LP_MASK ((1<<LP_TAB_SIZE)-1)
  struct lp_entry_t {
    ushort_t pos;
    ushort_t p;
    lp_entry_t():pos(0),p(0) {};
    lp_entry_t(ushort_t pos_,ushort_t p_):pos(pos_),p(p_) {};
  };
  typedef vector<lp_entry_t> lp_tab_t;
#endif

#ifdef LP_TAB_SIZE
#define LP_BIT_LIMIT 15
#else
#define LP_BIT_LIMIT 15
#endif

#if GIAC_ADDITIONAL_PRIMES==16
  typedef unsigned short additional_t;
#else
  typedef int additional_t;
#endif

  inline int _equalposcomp(const std::vector<additional_t> & v, additional_t w){
    int n=1;
    for (std::vector<additional_t>::const_iterator it=v.begin(),itend=v.end();it!=itend;++it){
      if ((*it)==w)
	return n;
      else
	n++;
    }
    return 0;
  }

  // #define SQRTMOD_OUTSIDE
#define WITH_LOGP // if defined primes should not exceed 2^24 (perhaps 2^25, choice of sqrt)

  struct small_basis_t {
    unsigned short root1;
    unsigned short root2;
    unsigned short p;
    unsigned short logp;
  };

#ifdef SQRTMOD_OUTSIDE
  struct basis_t {
    unsigned root1; // first root position in slice
    unsigned root2; // second root position
    ushort_t p:24; // the prime p
#ifdef WITH_LOGP
    unsigned char logp:8; // could be unsigned char
#endif
    basis_t():root1(0),root2(0),p(2) {
#ifdef WITH_LOGP
      logp=sizeinbase2(p);
#endif
    }
    basis_t(ushort_t _p):root1(0),root2(0),p(_p) {
#ifdef WITH_LOGP
      logp=sizeinbase2(p);
#endif
    }
  } ;

#else // SQRTMOD_OUTSIDE
  struct basis_t {
    unsigned root1; // first root position in slice
    unsigned root2; // second root position
    ushort_t p; // the prime p
    unsigned sqrtmod:24;
#ifdef WITH_LOGP
    unsigned char logp:8; // could be unsigned char
#endif
    basis_t():root1(0),root2(0),p(2),sqrtmod(0) {
#ifdef WITH_LOGP
      logp=sizeinbase2(p);
#endif
    }
    basis_t(ushort_t _p):root1(0),root2(0),p(_p),sqrtmod(0) {
#ifdef WITH_LOGP
      logp=sizeinbase2(p);
#endif
    }
    basis_t(ushort_t _p,ushort_t _sqrtmod):root1(0),root2(0),p(_p),sqrtmod(_sqrtmod) {
#ifdef WITH_LOGP
      logp=sizeinbase2(p);
#endif
}
  } ;
#endif // SQRTMOD_OUTSIDE

#ifdef LP_SMALL_PRIMES 
  static inline void core_sieve(slicetype * slice,small_basis_t * bit,small_basis_t * bitend)  {
    for (;bit!=bitend;++bit){
      // first root is at bit->root1
      register unsigned p=bit->p;
      register unsigned char nbits=bit->logp;
      register unsigned pos=bit->root1,pos2=bit->root2;
      if (pos==pos2){
	for (;pos<32768; pos += p){
	  slice[pos] -= nbits;
	}
	bit->root2=bit->root1 = pos-32768; // save for next slice
      }
      else {
	for (;pos<32768; pos += p){
	  slice[pos] -= nbits;
	}
	bit->root1 = pos-32768; // save for next slice
	// second root, polynomial has 2 distinct roots
	for (;pos2<32768;pos2 += p){
	  slice[pos2] -= nbits;
	}
	bit->root2 = pos2-32768;
      }
    }
  }

#else // LP_SMALL_PRIMES

#ifdef LP_TAB_SIZE
#define SLICEEND (1<<LP_TAB_SIZE)
#else
#define SLICEEND ss
#endif

  // return position of last prime sieved (useful when large prime hashtable is enabled
  static inline basis_t * core_sieve(slicetype * slice,int ss,basis_t * bit,basis_t * bitend)  {
    register unsigned char nbits=sizeinbase2(bit->p);
    // int next=1 << nbits;
    for (;bit!=bitend;++bit){
      // first root is at bit->root1
      register ushort_t p=bit->p;
#ifdef WITH_LOGP
      nbits=bit->logp;
#else
      if (p>next){
	++nbits;
#if !defined(BESTA_OS) && !defined(RTOS_THREADX) && !defined NSPIRE
	if (nbits==LP_BIT_LIMIT+1)
	  break;
#endif
	next *=2;
      }
#endif
      register unsigned pos=bit->root1,pos2=bit->root2;
      if (pos==pos2){
	for (;int(pos)<SLICEEND; pos += p){
	  slice[pos] -= nbits;
	}
	bit->root2=bit->root1 = pos-SLICEEND; // save for next slice
      }
      else {
	for (;int(pos)<SLICEEND; pos += p){
	  slice[pos] -= nbits;
	}
	bit->root1 = pos-SLICEEND; // save for next slice
	// second root, polynomial has 2 distinct roots
	for (;int(pos2)<SLICEEND;pos2 += p){
	  slice[pos2] -= nbits;
	}
	bit->root2 = pos2-SLICEEND;
      }
    }
#if !defined(RTOS_THREADX) && !defined(BESTA_OS) && !defined NSPIRE
#ifndef LP_TAB_SIZE
    for (;bit!=bitend;++bit){
      // same as above but we are sieving with primes >2^15, no need to check for nbits increase
      register ushort_t p=bit->p;
      register unsigned pos=bit->root1;
      for (;pos<ss; pos += p){
	slice[pos] -= LP_BIT_LIMIT+1;
      }
      bit->root1 = pos-ss; // save for next slice
      // if (sameroot) continue;
      pos=bit->root2;
      for (;pos<ss;pos += p){
	slice[pos] -= LP_BIT_LIMIT+1;
      }
      bit->root2 = pos-ss;
    }
#endif
#endif
    return bit;
  }
#endif // LP_SMALL_PRIMES

  // sieve in [sqrtN+shift,sqrtN+shift+slice.size()-1]
  // return -1 if memory problem, or the number of relations
  int msieve(const gen & a,const vecteur & sqrtavals,
	     const vecteur &bvals,const mpz_t& c,
	     vector<basis_t> & basis,unsigned lp_basis_pos,
#ifdef LP_SMALL_PRIMES
	     vector<small_basis_t> & small_basis,
#endif
	     unsigned maxadditional,
#ifdef ADDITIONAL_PRIMES_HASHMAP
	     additional_map_t & additional_primes_map,
#else
	     vector<additional_t> & additional_primes,vector<bool> & additional_primes_twice,
#endif
	     const gen & N,const gen & isqrtN,
	     slicetype * slice,int ss,int shift,
	     ushort_t * puissancesbegin,ushort_t* & puissancesptr,ushort_t * puissancesend,    
	     vector<ushort_t> & curpuissances,vector<ushort_t> &recheck,
	     vector<axbinv> & axbmodn,
	     mpz_t & z1,mpz_t & z2,mpz_t & z3,mpz_t & alloc1,mpz_t & alloc2,mpz_t & alloc3,mpz_t & alloc4,mpz_t & alloc5,
#ifdef LP_TAB_SIZE
	     const lp_tab_t & lp_tab,
#endif
	     GIAC_CONTEXT){
    int nrelations=0;
    // first fill slice with expected number of bits of 
    // (isqrtN+shift)^2-N = 2*shift*isqrtN + negl.
    // -> log(2*isqrtN)+log(shift)
    int shiftss=absint(shift+ss),absshift=absint(shift);
    int nbits=mpz_sizeinbase(*isqrtN._ZINTptr,2)+sizeinbase2(absshift>shiftss?absshift:shiftss);
    // int nbits1=int(0.5+std::log(evalf_double(isqrtN,1,context0)._DOUBLE_val/2.*(absshift>shiftss?absshift:shiftss))/std::log(2.));
    // int curbits=0;
    int bs=int(basis.size());
    double up_to=1.5;
    if (nbits>70)
      up_to += (0.8*(nbits-70))/70;
    if (debug_infolevel>7)
      *logptr(contextptr) << CLOCK() << gettext("Sieve tolerance factor ") << up_to << '\n';
    unsigned char logB=(unsigned char) (nbits-int(up_to*sizeinbase2(basis.back().p)+.5));
    // unsigned char logB=(unsigned char) (nbits-int(up_to*std::log(double(basis.back().p))/std::log(2.0)+.5));
    if (debug_infolevel>6)
      *logptr(contextptr) << CLOCK() << gettext(" reset") << '\n';
    // assumes slice type is size 1 byte and multiple of 32
#ifdef x86_64
    ulonglong * ptr=(ulonglong *) &slice[0];
    ulonglong * ptrend=ptr+ss/8;
    ulonglong pattern=(logB <<24)|(logB<<16)|(logB<<8) | logB;
    pattern = (pattern << 32) | pattern;
    for (;ptr!=ptrend;++ptr){
      *ptr=pattern;
    }
#else
    unsigned * ptr=(unsigned *) &slice[0];
    unsigned * ptrend=ptr+ss/4;
    unsigned pattern=(logB <<24)|(logB<<16)|(logB<<8) | logB;
    for (;ptr!=ptrend;++ptr){
      *ptr=pattern;
    }
#endif
    if (debug_infolevel>8)
      *logptr(contextptr) << CLOCK() << gettext(" end reset, nbits ") << nbits << '\n';
    // now for all primes p in basis move in slice from p to p
    // decrease slice[] by number of bits in p
    // determines the first prime used in basis
#if 0 // def WITH_LOGP
    nbits=2*mpz_sizeinbase(*isqrtN._ZINTptr,2);
    int next=50; 
    // note that msieve leaves 20 to 22 primes for normal range, and 15 for large 
    nbits = sizeinbase2(next);
#else
    if (nbits>120)
      nbits = 7;
    else { 
      if (nbits>90)
	nbits = 6;
      else {
	if (nbits>78)
	  nbits=5;
	else
	  nbits = 4;
      }
    }
    int next = 1 << (nbits-1);
#endif
    unsigned bstart;
    for (bstart=0;bstart<basis.size();++bstart){
      int p=basis[bstart].p;
      if (p>next){
	if (debug_infolevel>7)
	  *logptr(contextptr) << gettext("Sieve first prime ") << p << " nbits " << nbits << '\n';
	break;
      }
#ifdef LP_SMALL_PRIMES 
      int pos=small_basis[bstart].root1;
      pos=(pos-ss)%p;
      if (pos<0)
	pos+=p;
      small_basis[bstart].root1=pos;
      pos=small_basis[bstart].root2;
      pos=(pos-ss)%p;
      if (pos<0)
	pos+=p;
      small_basis[bstart].root2=pos;
#else
      // update pos_root_mod for later check
      int pos=basis[bstart].root1;
      pos=(pos-ss)%p;
      if (pos<0)
	pos+=p;
      basis[bstart].root1=pos;
      pos=basis[bstart].root2;
      pos=(pos-ss)%p;
      if (pos<0)
	pos+=p;
      basis[bstart].root2=pos;
#endif
    }
    next *= 2;
    if (debug_infolevel>8)
      *logptr(contextptr) << CLOCK() << gettext(" sieve begin ") << '\n';
    // bool sameroot; // Should be there to avoid counting twice the same root but it's faster to ignore it..;
#ifdef LP_SMALL_PRIMES
    small_basis_t * bit=&small_basis[bstart], * bitend=&small_basis[0]+small_basis.size();
    core_sieve(slice,bit,bitend);
#else
    basis_t * bit=&basis[bstart], * bitend=&basis[0]+bs;
#ifdef LP_TAB_SIZE
    bitend=core_sieve(slice,ss,bit,&basis[0]+lp_basis_pos);
#else
    bitend=core_sieve(slice,ss,bit,bitend);
#endif
#endif
    slicetype * st=slice, * stend=slice+ss;
#ifdef LP_TAB_SIZE
    // sieve for large prime using saved position
    if (!lp_tab.empty()){
      const lp_entry_t * lpit=&lp_tab[0],*lpitend=lpit+lp_tab.size(),*lpitend1=lpitend-8;
      if (lpitend-lpit>8){
	for (;lpit<lpitend1;lpit+=8){
	  //PREFETCH(lpit + 16);
	  slice[lpit->pos] -= 16;
	  slice[lpit[1].pos] -= 16;
	  slice[lpit[2].pos] -= 16;
	  slice[lpit[3].pos] -= 16;
	  slice[lpit[4].pos] -= 16;
	  slice[lpit[5].pos] -= 16;
	  slice[lpit[6].pos] -= 16;
	  slice[lpit[7].pos] -= 16;
	}
      }
      for (;lpit<lpitend;++lpit)
	slice[lpit->pos] -= 16;
    }
#endif
    unsigned cl=0;
    if (debug_infolevel>6)
      cl=CLOCK();
    if (debug_infolevel>8)
      *logptr(contextptr) << cl << gettext("relations ") << '\n';
    // now find relations
    st=slice; stend=slice+ss;
#ifdef x86_64
    ulonglong * st8=(ulonglong *) &slice[0],*st8end=st8+ss/8;
#else
    unsigned * st4=(unsigned *) &slice[0],*st4end=st4+ss/4;
#endif
    for (
#ifdef x86_64
	 ;st8!=st8end;st8+=4
#else
	 ;st4<st4end;st4+=8
#endif
	 ){
      // compare slice[pos] to boundary
#ifdef x86_64
      if ( !( (*st8  | st8[1] | st8[2] | st8[3] ) & 0x8080808080808080) )
	continue;
      int pos=int(((slicetype*)st8)-slice);
#else
      if ( !( (*st4  | st4[1] | st4[2] | st4[3] | st4[4] | st4[5] | st4[6] | st4[7]) & 0x80808080) )
	continue;
      int pos=((slicetype*)st4)-slice;
#endif
      st = slice+pos;
      for (int stpos=0;stpos<32;++st,++pos,++stpos){
	if (!(*st&0x80))
	  continue;
	// factor (isqrtN+shift+pos)^2-N on basis
	curpuissances.clear(); recheck.clear();
	int shiftpos=shift+pos;
#if 0
	gen tmp=shiftpos;
	tmp=(a*tmp+2*bvals.back())*tmp+c;
	tmp.uncoerce(); mpz_set(z1,*tmp._ZINTptr);
	*logptr(contextptr) << tmp << '\n';
#else 
	mpz_set_si(z1,shiftpos);
	mpz_mul(z2,z1,*a._ZINTptr);
	mpz_mul_2exp(z3,*bvals.back()._ZINTptr,1);
	mpz_add(z2,z2,z3);
	mpz_mul(z3,z1,z2);
	mpz_add(z1,z3,c);
#endif
	if (mpz_cmp_si(z1,0)<0){ // if (is_positive(-tmp,context0))
	  curpuissances.push_back(0xffff);
	  mpz_neg(z1,z1); // tmp=-tmp;
	}
	bool done=false;
#ifdef LP_TAB_SIZE
#ifdef LP_SMALL_PRIMES
	small_basis_t * basisptr=&small_basis[0], * basisend=basisptr+(bitend-bit);
#else
	basis_t * basisptr=&basis[0], * basisend=basisptr+(bitend-bit);
#endif
#else // LP_TAB_SIZE
	basis_t * basisptr=&basis[0], * basisend=basisptr+bs;
#endif
	// we have modified pos_root_mod1 and pos_root_mod2 -> posss
	int posss=ss-pos; // always positive
	for (;basisptr!=basisend;++basisptr){
	  register int bi=basisptr->p;
	  // check if we have a root 
	  register int check=bi-(posss%bi); 
	  if (check!=bi && check!=int(basisptr->root1) && check!=int(basisptr->root2))
	    continue;
	  if (check==bi && basisptr->root1 && basisptr->root2)
	    continue;
	  recheck.push_back(bi);
	} // end for on (small) primes
#ifdef LP_TAB_SIZE
	// add primes from large prime hashtable
	lp_tab_t::const_iterator lpit=lp_tab.begin(),lpend=lp_tab.end();
	int hash_pos=recheck.size();
	for (;lpit!=lpend;++lpit){
	  if (pos==int(lpit->pos)){
	    recheck.push_back(lpit->p);
	  }
	}
	if (int(recheck.size())>hash_pos+1)
	  sort(recheck.begin(),recheck.end());
#endif
	// now divide first by product of elements of recheck
	double prod=1,nextprod=1;
	for (unsigned k=0;k<recheck.size();++k){
	  nextprod=prod*recheck[k];
	  if (nextprod< 2147483648. )
	    prod=nextprod;
	  else {
	    // mpz_fdiv_q_ui(z1,z1,prod);
	    mpz_set_si(z2,int(prod));
#ifdef USE_GMP_REPLACEMENTS
	    mp_grow(&alloc1,z1.used+2);
	    mpz_set_ui(alloc1,0);
	    alloc1.used = z1.used +2 ;
	    mpz_set(alloc2,z1);
	    mpz_set(alloc3,z2);
	    alloc_mp_div(&z1,&z2,&z1,&z3,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
#else
	    mpz_divexact(z3,z1,z2);
	    mpz_swap(z1,z3);
#endif
	    prod=recheck[k];
	  }
	}
	if (prod!=1){
	  // mpz_fdiv_q_ui(z1,z1,prod);
	  mpz_set_si(z2,int(prod));
#ifdef USE_GMP_REPLACEMENTS
	  mp_grow(&alloc1,z1.used+2);
	  mpz_set_ui(alloc1,0);
	  alloc1.used = z1.used +2 ;
	  mpz_set(alloc2,z1);
	  mpz_set(alloc3,z2);
	  alloc_mp_div(&z1,&z2,&z1,&z3,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
#else
	  mpz_divexact(z3,z1,z2);
	  mpz_swap(z1,z3);
#endif
	}
	// then set curpuissances
	bool small_=false;
	int Z1=0;
	for (unsigned k=0;k<recheck.size();++k){
	  int j=0;
	  int bi=recheck[k];
#ifdef USE_GMP_REPLACEMENTS
	  if (!small_){
	    small_=mpz_sizeinbase(z1,2)<32;
	    if (small_)
	      Z1=mpz_get_si(z1);
	  }
	  if (small_){
	    for (++j;;++j){
#if 0 // def FXCG
	      if (Z1 % bi)
		break;
	      Z1 /= bi;
#else	      
	      div_t qr;
	      qr=div(Z1,bi);
	      if (qr.rem)
		break;
	      Z1=qr.quot;
#endif
	    }
	  }
	  else {
	    for (++j;;++j){
#if 1
	      mpz_set_ui(z2,bi);
	      mp_grow(&alloc1,z1.used+2);
	      mpz_set_ui(alloc1,0);
	      alloc1.used = z1.used +2 ;
	      mpz_set(alloc2,z1);
	      mpz_set(alloc3,z2);
	      alloc_mp_div(&z1,&z2,&z2,&z3,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
#else
	      mpz_fdiv_qr_ui(z2,z3,z1,bi);
#endif
	      if (mpz_cmp_si(z3,0))
		break;
	      mpz_set(z1,z2);
	    }
	  }
#else
	  for (++j;;++j){
	    mpz_fdiv_qr_ui(z2,z3,z1,bi);
	    if (mpz_cmp_si(z3,0))
	      break;
	    mpz_set(z1,z2);
	  }
#endif
	  /*
	    while (is_zero(smod(tmp,bi))){
	    tmp=tmp/bi;
	    ++j;
	    }
	  */
	  if (!done && bi>255){
	    curpuissances.push_back(0);
	    done=true;
	  }
	  if (done){
	    for (;j;--j)
	      curpuissances.push_back(bi);
	  }
	  else {
	    for (;j>=256;j-=256)
	      curpuissances.push_back(bi<<8);
	    if (j)
	      curpuissances.push_back( (bi << 8) | j);
	  }
	}
	if (small_) 
	  mpz_set_si(z1,Z1);
	if (mpz_cmp_si(z1,1)==0){ // is_one(tmp)){
	  ++nrelations;
	  if (debug_infolevel>6)
	    *logptr(contextptr) << CLOCK() << gettext(" true relation ") << '\n';
	  axbmodn.push_back(axbinv(int(sqrtavals.size())-1,shiftpos,int(bvals.size())-1,int(puissancesptr-puissancesbegin),int(puissancesptr-puissancesbegin)+int(curpuissances.size())));	
	  for (unsigned i=0;i<curpuissances.size();++puissancesptr,++i){
	    if (puissancesptr>=puissancesend)
	      return -1;
	    *puissancesptr=curpuissances[i];
	  }
	}
	else {
	  unsigned param2;
#if (GIAC_ADDITIONAL_PRIMES==16)
	  param2=0xffff;
#else
	  param2=maxadditional;
#endif
	  if (mpz_cmp_ui(z1,param2)>0){
	    if (debug_infolevel>6)
	      *logptr(contextptr) << gen(z1) << gettext(" Sieve large remainder:") << '\n';
	  }
	  else {
#ifdef GIAC_ADDITIONAL_PRIMES
	    additional_t P=mpz_get_ui(z1);
	    // if (int(P)>2*int(basis.back())) continue;
	    // if (debug_infolevel>5)
	    if (debug_infolevel>6)
	      *logptr(contextptr) << CLOCK() << " " << P << " remain " << '\n';
#ifdef ADDITIONAL_PRIMES_HASHMAP
	    // add relation
	    ++nrelations;
	    curpuissances.push_back(1); // marker
#if (GIAC_ADDITIONAL_PRIMES==32) 
#ifndef PRIMES32
	    curpuissances.push_back(P >> 16);
#endif
#endif
	    curpuissances.push_back(P);
	    for (unsigned i=0;i<curpuissances.size();++puissancesptr,++i){
	      if (puissancesptr>=puissancesend)
		return -1;
	      *puissancesptr=curpuissances[i];
	    }
	    additional_map_t::iterator it=additional_primes_map.find(P),itend=additional_primes_map.end();
	    if (it!=itend) // build a large prime relation (P is the large prime)
	      axbmodn.push_back(axbinv(sqrtavals.size()-1,shiftpos,bvals.size()-1,(puissancesptr-puissancesbegin)-curpuissances.size(),(puissancesptr-puissancesbegin)));
	    else // record a partial relation
	      additional_primes_map[P]=axbinv(sqrtavals.size()-1,shiftpos,bvals.size()-1,(puissancesptr-puissancesbegin)-curpuissances.size(),(puissancesptr-puissancesbegin));
#else
	    int Ppos=_equalposcomp(additional_primes,P); // this is in O(additional^2)=o(B^3)
	    if (Ppos){
	      if (debug_infolevel>6)
		*logptr(contextptr) << P << gettext(" already additional") << '\n';
	      --Ppos;
	      additional_primes_twice[Ppos]=true;
	    } else {
	      // add a prime in additional_primes if <=QS_B_BOUND
	      if (int(additional_primes.size())>=4*bs 
#if defined(RTOS_THREADX) || defined(BESTA_OS) || defined NSPIRE
		  || bs+additional_primes.size()>700
#endif
		  )
		continue;
	      additional_primes.push_back(P);
	      additional_primes_twice.push_back(false);
	      Ppos=int(additional_primes.size())-1;
	    }
	    // add relation
	    curpuissances.push_back(1); // marker
#if GIAC_ADDITIONAL_PRIMES==32 
#ifndef PRIMES32
	    curpuissances.push_back(P >> 16);
#endif
#endif
	    curpuissances.push_back(P);
	    axbmodn.push_back(axbinv(int(sqrtavals.size())-1,shiftpos,int(bvals.size())-1,int(puissancesptr-puissancesbegin),int(puissancesptr-puissancesbegin)+int(curpuissances.size())));
	    for (unsigned i=0;i<curpuissances.size();++puissancesptr,++i){
	      if (puissancesptr>=puissancesend)
		return -1;
	      *puissancesptr=curpuissances[i];
	    }
#endif // ADDITIONAL_PRIMES_HASHMAP
#endif // GIAC_ADDITIONAL_PRIMES
	  }
	}
      }
    } // end for loop on slice array
    if (debug_infolevel>6){
      unsigned cl2=CLOCK();
      *logptr(contextptr) << cl2 << gettext(" end relations ") << cl2-cl << '\n';
    }
    return nrelations;
  }

  // #define MP_MODINV_1
#ifdef MP_MODINV_1
  static inline unsigned mp_modinv_1(unsigned a, unsigned p) {
  
    unsigned ps1, ps2, dividend, divisor, rem, q, t;
    unsigned parity;
    
    q = 1; rem = a; dividend = p; divisor = a;
    ps1 = 1; ps2 = 0; parity = 0;
    
    while (divisor > 1) {
      rem = dividend - divisor;
      t = rem - divisor;
      if (rem >= divisor) { q += ps1; rem = t; t -= divisor;
	if (rem >= divisor) { q += ps1; rem = t; t -= divisor;
	  if (rem >= divisor) { q += ps1; rem = t; t -= divisor;
	    if (rem >= divisor) { q += ps1; rem = t; t -= divisor;
	      if (rem >= divisor) { q += ps1; rem = t; t -= divisor;
		if (rem >= divisor) { q += ps1; rem = t; t -= divisor;
		  if (rem >= divisor) { q += ps1; rem = t; t -= divisor;
		    if (rem >= divisor) { q += ps1; rem = t;
		      if (rem >= divisor) {
			q = dividend / divisor;
			rem = dividend % divisor;
			q *= ps1;
		      } 
		    } 
		  } 
		} 
	      } 
	    } 
	  } 
	} 
      }
      
      q += ps2;
      parity = ~parity;
      dividend = divisor;
      divisor = rem;
      ps2 = ps1;
      ps1 = q;
    }
  
    if (parity == 0)
      return ps1;
    else
      return p - ps1;
  }
#endif

#if (defined __i386__ || defined __x86_64__) && !defined PIC && !defined _I386_ && !defined __APPLE__ && !defined VISUALC && !defined(FIR_LINUX) && !defined(FIR_ANDROID)
  #define _I386_
#endif

#ifdef _I386_
  // a->a+b*c mod m
  inline void addmultmod(int & a,int b,int c,int m){
    asm volatile("testl %%ebx,%%ebx\n\t" /* sign bit=1 if negative */
		 "jns .Lok%=\n\t"
		 "addl %%edi,%%ebx\n" /* a+=m*/
		 ".Lok%=:\t"
		 "imull %%ecx; \n\t" /* b*c in edx:eax */
		 "addl %%ebx,%%eax; \n\t" /* b*c+a */
		 "adcl $0x0,%%edx; \n\t" /* b*c+a carry */
		 "idivl %%edi; \n\t"
		 :"=d"(a)
		 :"a"(b),"b"(a),"c"(c),"D"(m)
		 );
  }
#endif

  inline 
  int modmult(int a,int b,unsigned p){
#ifdef _I386_
    register int res;
    asm volatile("imull %%edx\n\t" /* a*b-> edx:eax */ 
		 "idivl %%ecx\n\t" /* edx:eax div p -> quotient=eax, remainder=edx */
		 :"=d"(res)
		 :"a"(a),"d"(b),"c"(p)
		 :
		 );
    return res;
#else
    return a*longlong(b) % p;
#endif
  }

  // assumes b>0 and |a|<b
  int invmodnoerr(int a,int b){
    if (a==1 || a==-1 || a==0)
      return a;
    if (a<0) // insure a>0 so that all remainders below are >=0
      a+=b;
#ifdef _I386_ // works only for ushort_t == unsigned short
    // int res=mp_modinv_1(a,b),p=b;
    /* GDB: si will step in assembly, info registers show register content, x/i $pc show next ins */
    asm volatile("movl $0,%%edi\n\t" 
		 "movl $1,%%ecx\n\t"
		 "movl $0,%%edx\n\t" 
		 ".Lloop%=:\t"
		 "movl %%esi,%%eax\n\t" 
		 "andl $0x80000000,%%esi\n\t"
		 "xorl $0x80000000,%%esi\n\t" /* parity indicator for sign */
		 "andl $0x7fffffff,%%eax\n\t" /* clear high bit of ax */
		 "divl %%ebx\n\t" /* divide si by bx, ax=quotient, dx=rem */
		 "orl %%ebx,%%esi\n\t"  /* copy bx in si but keep high bit of si */
		 "movl %%edx,%%ebx\n\t" /* si now contains bx and bx the remainder */
		 "mull %%ecx\n\t" /* quotient*cx is in ax (dx=0) */
		 "addl %%eax,%%edi\n\t" /* di <- di+q*cx*/
		 "xchgl %%edi,%%ecx\n\t" /* cx <- origi di+q*cx, di <- orig cx */
		 "testl %%ebx,%%ebx\n\t"
		 "jne .Lloop%=\n\t" 
		 :"=D"(a),"=S"(b)
		 :"S"(b),"b"(a)
		 :"%eax","%ecx","%edx"
		 );
    if (b<0)
      b=b&0x7fffffff;
    else
      a=-a;
    a=(b==1)?a:0;
    // if ((a-res)%p)
    //  CERR << "error" << '\n';
    return a;
#else // i386

#ifdef MP_MODINV_1
    return mp_modinv_1(a,b);
#endif
    // r0=b=ab*a+1*b
    // r1=a=aa*a+0*b
    int aa(1),ab(0),ar(0);
#if 0 // def FXCG
    ushort_t q,r;
    while (a){
      q=b/a;
      ar=ab-q*aa;
      r=b-q*a;
      if (!r)
	return a==1?aa:0;
      q=a/r;
      ab=aa-q*ar;
      b=a-q*r;
      if (!b)
	return r==1?ar:0;
      q=r/b;
      aa=ar-q*ab;
      a=r-q*b;
    }
    return b==1?ab:0;
#else
    div_t qr;
    while (a){
      qr=div(b,a);
      ar=ab-qr.quot*aa;
      b=a;
      a=qr.rem;
      ab=aa;
      aa=ar;
    }
    if (b==1)
      return ab;
    return 0;
#endif
#endif // i386
  }

#if 0 // def PRIMES32
  // assumes |a|<b
  longlong invmodnoerr(longlong a,longlong b){
    if (a==1 || a==-1 || a==0)
      return a;
    if (a<0) // insure a>0 so that all remainders below are >=0
      a+=b;
    // r0=b=ab*a+1*b
    // r1=a=aa*a+0*b
    longlong aa(1),ab(0),ar(0);
    longlong q,r;
    lldiv_t qr;
    while (a){
      qr=lldiv(b,a);
      ar=ab-qr.quot*aa;
      b=a;
      a=qr.rem;
      ab=aa;
      aa=ar;
    }
    if (b==1)
      return ab;
    return 0;
  }
#endif

  static int find_multiplier(const gen & n,double & delta,GIAC_CONTEXT){
    delta=0;
    if (n.type!=_ZINT)
      return 1;
    static const unsigned char mult[] =
      { 1, 3, 5, 7, 11, 13, 15, 17, 19, 
	21, 23, 29, 31, 33, 35, 37, 39, 41, 43, 47}; // only odd values for multiplier
    unsigned nmult=sizeof(mult)/sizeof(unsigned char);
    double scores[50];
    int nmodp=modulo(*n._ZINTptr,8),knmodp;
    // init scores and set value for 2
    double ln2=std::log(2.0);
    for (unsigned i=0;i<nmult;++i){
      knmodp=(mult[i]*nmodp)%8;
      scores[i]=0.5*std::log(double(mult[i]));
      switch(knmodp){
      case 1:
	scores[i] -= 2*ln2;
	break;
      case 5:
	scores[i] -= ln2;
	break;
      case 3: case 7:
	scores[i] -= 0.5 * ln2;
	break;
      }
    }
    // now compute contribution for giac_primes[1..300]
    for (unsigned i=1;i<=300;++i){
      int p=giac_primes[i];
      double contrib=std::log(double(p))/(p-1);
      nmodp=modulo(*n._ZINTptr,p);
      for (unsigned j=0;j<nmult;++j){
	knmodp=(nmodp*mult[j])%p;
	if (knmodp==0)
	  scores[j] -= contrib;
	else {
	  if (!is_undef(sqrt_mod(knmodp,p,true,context0)))
	    scores[j] -= 2*contrib;
	}
      }
    }
    // select the smallest scores
    int pos=0; 
    double minscore=scores[0]-0.1;
    for (unsigned i=1;i<nmult;++i){
      if (scores[i]<minscore){
	minscore=scores[i];
	pos=i;
      }
    }
    if (debug_infolevel>6){
      for (unsigned i=0;i<nmult;++i){
	*logptr(contextptr) << gettext("multiplier ") << int(mult[i]) << " score " << scores[i] << '\n';
      }
    }
    if (pos){
      delta=minscore-scores[0];
      if (debug_infolevel)
	*logptr(contextptr) << gettext("Using multiplier ") << int(mult[pos]) << " delta-score " << delta << '\n';
    }
    return mult[pos];
  }

  void add_relation(vector<line_t> relations,unsigned j,ushort_t * curpui,ushort_t * curpuiend,const vector<basis_t> & basis,const vector<additional_t> & additional_primes){
    unsigned curpuisize=unsigned(curpuiend-curpui);
    bool done=false;
    unsigned i=0; // position in basis
    unsigned k=0; // position in curpui
    additional_t p=0; // prime
    unsigned bs=unsigned(basis.size());
    for (;k<curpuisize;++k){
      p=curpui[k];
      if (p==0xffff){
	relations[0].tab[j/32] ^= (1 << (j%32));
	continue;
      }
      if (p==0){
	done=true;
	continue;
      }
      if (p==1){
#ifndef ADDITIONAL_PRIMES_HASHMAP
	p=curpui[k+1];
#if GIAC_ADDITIONAL_PRIMES==32
#ifndef PRIMES32
	p <<= 16;
	p += curpui[k+2];
#endif
#endif
	// k must be == curpui.size()-1
	// find p in additional_primes and position
	int Ppos=_equalposcomp(additional_primes,p);
	// *logptr(contextptr) << p << " " << Ppos+bs << " " << relations.size() << '\n';
	relations[bs+Ppos].tab[j/32] |= (1 << (j %32));	  
#endif
	break;
      }
      if (!done){
	if (p%2==0) // even exponent?
	  continue;
	p >>= 8;
      }
      else {
	int c=1;
	for (;k+1<curpuisize;c++){
	  if (curpui[k+1]==unsigned(p))
	    ++k;
	  else
	    break;
	}
	if (c%2==0)
	  continue;
      }
      // advance to next i in basis
      for (;i<bs;++i){
	if (basis[i].p==unsigned(p))
	  break;
      }
      if (i<bs){
	++i;
	relations[i].tab[j/32] ^= (1 << (j %32));
      }
      else {
	// ERROR
      }
    } // end loop on k in curpui
  }

  void update_xy(axbinv & A,mpz_t & zx,mpz_t & zy,vector<short_t> & p,vector<short_t> & add_p,const gen & N,const vector<basis_t> & basis,const vector<additional_t> & additional_primes,const vecteur & sqrtavals,const vecteur & bvals,ushort_t * puissancestab,mpz_t & zq,mpz_t & zr,mpz_t & alloc1, mpz_t & alloc2,mpz_t & alloc3,mpz_t & alloc4, mpz_t & alloc5){
    // x=x*(a*shiftpos+b), y =y*sqrta;
    mpz_set_si(alloc2,A.shiftpos);
    if (sqrtavals[A.aindex].type==_INT_){
      mpz_mul_ui(alloc1,alloc2,sqrtavals[A.aindex].val);
      mpz_mul_ui(alloc2,alloc1,sqrtavals[A.aindex].val);
      mpz_mul_ui(zy,zy,sqrtavals[A.aindex].val);
    }
    else {
      mpz_mul(alloc1,alloc2,*sqrtavals[A.aindex]._ZINTptr);
      mpz_mul(alloc2,alloc1,*sqrtavals[A.aindex]._ZINTptr);
      mpz_mul(zy,zy,*sqrtavals[A.aindex]._ZINTptr);
    }
    mpz_add(alloc1,alloc2,*bvals[A.bindex]._ZINTptr);
    // mpz_mul(alloc2,alloc1,*invsqrtamodnvals[A.aindex]._ZINTptr);
    mpz_mul(zr,zx,alloc1);
#ifdef USE_GMP_REPLACEMENTS
    mp_grow(&alloc1,zr.used+2);
    mpz_set_ui(alloc1,0);
    alloc1.used = zr.used +2 ;
    mpz_set(alloc2,zr);
    mpz_set(alloc3,*N._ZINTptr);
    // mpz_set_si(alloc4,0);
    // mpz_set_si(alloc5,0);
    alloc_mp_div(&zr,N._ZINTptr,&zq,&zx,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
    mp_grow(&alloc1,zy.used+2);
    mpz_set_ui(alloc1,0);
    alloc1.used = zy.used +2 ;
    mpz_set(alloc2,zy);
    mpz_set(alloc3,*N._ZINTptr);
    // mpz_set_si(alloc4,0);
    // mpz_set_si(alloc5,0);
    alloc_mp_div(&zy,N._ZINTptr,&zq,&zy,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
#else
    mpz_tdiv_r(zx,zr,*N._ZINTptr);
    mpz_tdiv_r(zy,zy,*N._ZINTptr);
#endif	 
    bool done=false;
    unsigned bi=0;
    ushort_t * it=puissancestab+A.first,* itend=puissancestab+A.second;
    for (;it!=itend;++it){
      if (*it==0xffff)
	continue;
      if (*it==1){
	++it;
	additional_t p=*it;
#if GIAC_ADDITIONAL_PRIMES==32 
#ifndef PRIMES32
	p <<= 16;
	++it;
	p += *it;
#endif
#endif
	int pos=_equalposcomp(additional_primes,p);
	if (pos) 
	  ++add_p[pos-1];
	else {
	  // otherwise ERROR!!!
	}
	break;
      }
      if (!*it){
	done=true;
	continue;
      }
      if (done){
	while (bi<basis.size() && basis[bi].p!=*it)
	  ++bi;
	if (bi<basis.size())
	  p[bi]++;
	else {
	  // ERROR
	}
      }
      else {
	while (basis[bi].p!=(*it>>8))
	  ++bi;
	p[bi]+=(*it&0xff);
      }
    }
  }

  void find_bv_be(int tmp,int & bv,int &be){
    bv=1; be=-1;
    while (tmp%2==0){
      ++bv;
      tmp /= 2;
    }
    tmp /= 2;
    if (tmp%2)
      be=1;
    else
      be=-1;
  }


#ifdef PRIMES32
  // Change b coeff of polynomial: update roots for small primes
  // for large primes do it depending on LP_TAB_TOGETHER
#ifdef LP_SMALL_PRIMES
  void copy(vector<basis_t> & basis,vector<small_basis_t> & small_basis){
    small_basis_t * small_basisptr=&small_basis[0], * small_basisend=small_basisptr+small_basis.size();
    basis_t * basisptr=&basis[0];
    unsigned next=2,logp=1;
    if (small_basis[0].p==0){
      for (;small_basisptr<small_basisend;++basisptr,++small_basisptr){
	small_basisptr->root1=basisptr->root1;
	small_basisptr->root2=basisptr->root2;
	register unsigned short p =basisptr->p;
	small_basisptr->p = p;
	small_basisptr->logp=logp;
	if (p>next){
	  ++logp;
	  next *= 2;
	}
      }
    }
    else {
      for (;small_basisptr<small_basisend;++basisptr,++small_basisptr){
	small_basisptr->root1=basisptr->root1;
	small_basisptr->root2=basisptr->root2;
      }
    }
  }

  void switch_roots(const vector<int> & bainv2,vector<basis_t> & basis,vector<small_basis_t> & small_basis,unsigned lp_basis_pos,unsigned nslices,unsigned slicesize,unsigned bv,int be,int afact,const vector<ushort_t> & pos,gen b,mpz_t & zq,int M){
    unsigned bs=basis.size();
    const int * bvpos=&bainv2[(bv-1)*bs];
#ifdef LP_TAB_TOGETHER
    const int * bvposend=bvpos+lp_basis_pos;
#else
    const int * bvposend=bvpos+bs;
#endif
    basis_t * basisptr=&basis[0];
    if (be>0){
      for (;bvpos<bvposend;++basisptr,++bvpos){
	// PREFETCH(basisptr+4);
	// PREFETCH(bvpos+4);
	register unsigned p=basisptr->p;
	register int r=basisptr->root1-(*bvpos);
	if (r<0)
	  r+=p;
	basisptr->root1=r;
	r=basisptr->root2-(*bvpos);
	if (r<0)
	  r+=p;
	basisptr->root2=r;
      }
    }
    else {
      for (;bvpos<bvposend;++basisptr,++bvpos){
	// PREFETCH(basisptr+4);
	// PREFETCH(bvpos+4);
	register unsigned p=basisptr->p;
	register int r=basisptr->root1+(*bvpos);
	if (r>p)
	  r-=p;
	basisptr->root1=r;
	r=basisptr->root2+(*bvpos);
	if (r>p)
	  r-=p;
	basisptr->root2=r;
      }
    }
    // adjust sieve position for prime factors of a, 
    for (int j=0;j<afact;++j){
      int pj=pos[j];
      ushort_t p=basis[pj].p; 
      int q,bmodp=p-modulo(*b._ZINTptr,p);
      int cmodp=modulo(zq,p);
      q=(M+longlong(cmodp)*invmodnoerr((2*bmodp)%p,p))%p;
      if (q<0)
	q+=p;
      basis[pj].root1=q;
      basis[pj].root2=q;
    }
    // set small primes position for sieving
    copy(basis,small_basis);
  }

#else // LP_SMALL_PRIMES

  void switch_roots(const vector<int> & bainv2,vector<basis_t> & basis,unsigned lp_basis_pos,unsigned nslices,unsigned slicesize,unsigned bv,int be,int afact,const vector<ushort_t> & pos,gen b,mpz_t & zq,int M){
    unsigned bs=basis.size();
#ifdef LP_TAB_SIZE
    const int * bvpos=&bainv2[(bv-1)*bs],* bvposend=bvpos+lp_basis_pos;
#else
    const int * bvpos=&bainv2[(bv-1)*bs],* bvposend=bvpos+bs;
#endif
    basis_t * basisptr=&basis[0];
    unsigned decal0=nslices*slicesize;
    if (decal0>=basis.back().p){
      if (be<0){
	for (;bvpos<bvposend;++basisptr,++bvpos){
	  register unsigned p=basisptr->p;
	  register unsigned decal = (decal0+(*bvpos))% p;
	  register unsigned r=basisptr->root1+decal;
	  if (r>p)
	    r -= p;
	  basisptr->root1 = r;
	  r = basisptr->root2+decal;
	  if (r>p)
	    r -= p;
	  basisptr->root2 = r;
	}
      }
      else {
	for (;bvpos<bvposend;++basisptr,++bvpos){
	  register unsigned p=basisptr->p;
	  register unsigned decal = (decal0-(*bvpos))% p;
	  register unsigned r=basisptr->root1+decal;
	  if (r>p)
	    r -= p;
	  basisptr->root1 = r;
	  r = basisptr->root2+decal;
	  if (r>p)
	    r -= p;
	  basisptr->root2 = r;
	}
      }
    }
    else 
      { // should not be reached since Mtarget is about basis.back()
	for (;bvpos<bvposend;++basisptr,++bvpos){
	  register unsigned p=basisptr->p;
	  register unsigned decal = (decal0+p-be*(*bvpos))% p;
	  register unsigned r=basisptr->root1+decal;
	  if (r>p)
	    r -= p;
	  basisptr->root1 = r;
	  r = basisptr->root2+decal;
	  if (r>p)
	    r -= p;
	  basisptr->root2 = r;
	}
      }
    // adjust sieve position for prime factors of a, 
    for (int j=0;j<afact;++j){
      int pj=pos[j];
      ushort_t p=basis[pj].p; 
      int q,bmodp=p-modulo(*b._ZINTptr,p);
      int cmodp=modulo(zq,p);
      q=(M+longlong(cmodp)*invmodnoerr((2*bmodp)%p,p))%p;
      if (q<0)
	q+=p;
      basis[pj].root1=q;
      basis[pj].root2=q;
    }
#if defined(LP_TAB_SIZE) && !defined(LP_TAB_TOGETHER)
    bvposend += bs-lp_basis_pos;
    if (be>0){
      for (;bvpos<bvposend;++basisptr,++bvpos){
	// PREFETCH(basisptr+4);
	// PREFETCH(bvpos+4);
	register unsigned p=basisptr->p;
	register int r=basisptr->root1-(*bvpos);
	if (r<0)
	  r+=p;
	basisptr->root1=r;
	r=basisptr->root2-(*bvpos);
	if (r<0)
	  r+=p;
	basisptr->root2=r;
      }
    }
    else {
      for (;bvpos<bvposend;++basisptr,++bvpos){
	// PREFETCH(basisptr+4);
	// PREFETCH(bvpos+4);
	register unsigned p=basisptr->p;
	register int r=basisptr->root1+(*bvpos);
	if (r>int(p))
	  r-=p;
	basisptr->root1=r;
	r=basisptr->root2+(*bvpos);
	if (r>int(p))
	  r-=p;
	basisptr->root2=r;
      }
    }
#endif
  }
#endif // LP_SMALL_PRIMES
#endif // PRIMES32

  // Change a, the leading coeff of polynomial: initialize all roots (small and large primes)
  void init_roots(vector<basis_t> & basis,
#ifdef LP_SMALL_PRIMES
		  vector<small_basis_t> & small_basis,
#endif
#ifdef WITH_INVA
		  vector<ushort_t> & Inva,
#endif
#ifdef SQRTMOD_OUTSIDE
		  const vector<ushort_t> & sqrtmod,
#endif
#ifdef PRIMES32
		  vector<int> & bainv2,int afact,int afact0,
#else
		  ulonglong usqrta,
#endif
		  const gen & a,const gen & b,const vecteur & bvalues,mpz_t & zq,unsigned M){
    unsigned bs=unsigned(basis.size());
    basis_t * basisptr=&basis.front(),*basisend=basisptr+bs; 
#ifdef SQRTMOD_OUTSIDE
    vector<ushort_t>::const_iterator sqrtmodit=sqrtmod.begin();
#endif
    for (int i=0;basisptr!=basisend;++i,++basisptr){
      ushort_t p=basisptr->p;
      // find inverse of a mod p
#ifdef PRIMES32
      int j=invmodnoerr(modulo(*a._ZINTptr,p),p);
      // deltar[i]=((2*ulonglong(basis[i].sqrtmod))*j)%p;
#else // PRIMES32
      unsigned modu=usqrta%p;
      modu=(modu*modu)%p;
      int j=invmodnoerr(modu,p);	
#endif // PRIMES32
      if (j<0) 
	j += p;
      unsigned inva=j;
#ifdef WITH_INVA
      Inva[i]=inva;
#else
#ifdef PRIMES32
      // set roots change values for all b coeffs for this a
      if (afact>afact0){
	int * ptr=&bainv2[i];
	for (int j=1;j<afact;ptr+=bs,++j){
	  // PREFETCH(ptr+bs);
	  *ptr=modmult(modulo(*bvalues[j]._ZINTptr,p),2*inva,p);
	}
      }
#endif // PRIMES32
#endif // WITH_INVA
      // compute roots mod p
#ifdef SQRTMOD_OUTSIDE
      ushort_t sqrtm=*sqrtmodit;
      ++sqrtmodit; 
#else
      ushort_t sqrtm=basisptr->sqrtmod;
#endif
      int bmodp=p-modulo(*b._ZINTptr,p);
      if (inva){
	if (p<=37000){
	  // sqrtm<=p/2, bmodp<p, inva<p hence (bmodp+sqrtm)*inva<=(3p/2-1)*(p-1)
	  // this leaves M up to about 203 millions
	  basisptr->root1=(M+(bmodp+sqrtm)*inva) % p;
	  basisptr->root2=(M+(bmodp+p-sqrtm)*inva) % p;
	  continue;
	}
#ifdef _I386_
	register int q=M;
	addmultmod(q,bmodp+sqrtm,inva,p);
	basisptr->root1=q;
	q=M;
	addmultmod(q,bmodp+p-sqrtm,inva,p);
	basisptr->root2=q;
#else
	basisptr->root1=(M+longlong(bmodp+sqrtm)*inva) % p;
	basisptr->root2=(M+longlong(bmodp+p-sqrtm)*inva) % p;
#endif
	continue;
      }
      int cmodp=modulo(zq,p);
      int q=(M+longlong(cmodp)*invmodnoerr((2*bmodp)%p,p))%p;
      if (q<0)
	q+=p;
      basisptr->root2=q;
      basisptr->root1=q;
    }
#ifdef WITH_INVA
#ifdef PRIMES32
    if (afact>afact0){
      int * bainv2ptr=&bainv2.front();
      basis_t * basisptr,*basisend=&basis.front()+bs; 
      for (int j=1;j<afact;++j){
	if (bvalues[j].type==_INT_){
	  int bjj=bvalues[j].val;
	  vector<ushort_t>::const_iterator invait=Inva.begin();
	  for (basisptr=&basis.front();basisptr<basisend;++invait,++bainv2ptr,++basisptr){
	    register int r=(bjj*longlong(2*(*invait))) % basisptr->p;
	    if (r<0)
	      r += basisptr->p;
	    *bainv2ptr=r;
	  }
	}
	else {
	    // longlong up1=up1tmp[2*j]; 
	    // longlong tmp=up1tmp[2*j+1];
	    // tmp is <= P^2 where P is the largest factor of a
	  mpz_t & bz=*bvalues[j]._ZINTptr;
	  vector<ushort_t>::const_iterator invait=Inva.begin();
	  for (basisptr=&basis.front();basisptr<basisend;++invait,++bainv2ptr,++basisptr){
	    register int p=basisptr->p;
	    *bainv2ptr=((modulo(bz,p))*longlong(2*(*invait))) % p;
	  }
	}
      }
    }
#endif // PRIMES32
#endif // WITH_INVA
    
#ifdef LP_SMALL_PRIMES // copy primes<2^16 into small_basis
    copy(basis,small_basis);
#endif
  }

  // find relations using (a*x+b)^2=a*(a*x^2+b*x+c) mod n where
  // we sieve on [-M,M] for as many polynomials as required
  // a is a square, approx sqrt(2*n)/M, and n is a square modulo all primes dividing a
  // b satisifies b^2=n mod a (b in [0,a[)
  // c=(n-b^2)/a
  bool msieve(const gen & n_orig,gen & pn,GIAC_CONTEXT){
    if (n_orig.type!=_ZINT)
      return false;
    // find multiplier
    double delta;
    int multiplier=find_multiplier(n_orig,delta,contextptr);
    gen N(multiplier*n_orig);
    double Nd=evalf_double(N,1,contextptr)._DOUBLE_val;
#if defined RTOS_THREADX || defined NSPIRE
    if (Nd>1e40) return false;
#endif
#ifdef BESTA_OS
    if (Nd>1e40) return false;
#endif
#ifdef PRIMES32
    if (Nd>1e76) return false;
#else
    if (Nd>1e63) return false;
#endif
    int Ndl=int(std::log10(Nd)-std::log10(double(multiplier))+.5); // +2*delta);
#ifdef LP_TAB_SIZE
    int slicesize=(1 << LP_TAB_SIZE);
#else
    int slicesize=(QS_SIZE>=65536 && Ndl<61)?32768:QS_SIZE;
#endif
    double B=std::exp(std::sqrt(2.0)/4*std::sqrt(std::log(Nd)*std::log(std::log(Nd))))*0.45;
    if (B<200) B=200;
    int pos1=70,pos0=23,afact=2,afixed=0; // pos position in the basis, afact number of factors
    // FIXME Will always include the 3 first primes of the basis
    // set a larger Mtarget gives less polynomials but also use less memory
#if defined(RTOS_THREADX) || defined(RTOS_THREADX) || defined NSPIRE
    double Mtarget=0.95e5;
    if (Nd>1e36)
      Mtarget=1.2e5;
#else
    double Mtarget=0.55e5;
#ifndef USE_MORE_PRIMES // FIXME improve! in fact use more primes on Core, less on Opteron
    if (Ndl>=50){
      Ndl-=50;
      short int Btab[]={ 
	// 50
	1900,2100,2300,2500,2700,2900,3100,3400,3700,4000,
	// 60
	4300,4600,4900,5300,5700,6200,6800,7500,8300,9200,
	// 70
	10000,11000,12000,13000,14000,15000,16000
      };
      if (Ndl<sizeof(Btab)/sizeof(short int))
	B=Btab[Ndl];
      Mtarget=0.66e5;
      if (Ndl>7)
	Mtarget=0.95e5;
      if (Ndl>11) 
	Mtarget=1.3e5;
      if (Ndl>15) 
	Mtarget=1.6e5;
      if (Ndl>19) 
	Mtarget=1.92e5;
    }
#else
    if (Ndl>=50)
      Mtarget=0.85e5;
    if (Ndl>65) 
      Mtarget=1.3e5;
#endif
#endif
    if (debug_infolevel)
      *logptr(contextptr) << "" << CLOCK() << gettext(" sieve on ") << N << '\n' << gettext("Number of primes ") << B << '\n';
    // first compute the prime basis and sqrt(N) mod p, p in basis
    vector<basis_t> basis;
    basis.reserve(unsigned(B));
#ifdef SQRTMOD_OUTSIDE
    vector<ushort_t> sqrtmod;
    sqrtmod.reserve(basis.capacity());
    basis.push_back(2);
    sqrtmod.push_back(1);
#else
    basis.push_back(basis_t(2,1)); // I assume that N is odd... hence has sqrt 1 mod 2
#endif
    N.uncoerce();
    // vector<ushort_t> N256;
    int i;
    mpz_t zx,zy,zq,zr;
    mpz_init(zx); mpz_init(zy); mpz_init(zq); mpz_init(zr);
    // fastsmod_prepare(N,zx,zy,zr,N256);
    for (i=1;i<int(sizeof(giac_primes)/sizeof(short));++i){
#ifdef TIMEOUT
      control_c();
#endif
      if (ctrl_c || interrupted)
	break;
      ushort_t j=giac_primes[i];
      if (debug_infolevel>6 && (i%500==99))
	*logptr(contextptr) << CLOCK() << gettext(" sieve current basis size ") << basis.size() << '\n';
#if 1 // def USE_GMP_REPLACEMENTS
      // int n=fastsmod_compute(N256,j);
      int n=modulo(*N._ZINTptr,j),s;
#else
      int n=smod(N,j).val,s;
#endif
      if (n<0)
	n+=j;
      if (n==0){
#ifdef SQRTMOD_OUTSIDE
	basis.push_back(j);
	sqrtmod.push_back(0);
#else
	basis.push_back(basis_t(j,0));
#endif
      }
      else {
	if (powmod(n,(unsigned long)((j-1)/2),(int)j)==1){
	  s=sqrt_mod(n,int(j),true,contextptr).val;
	  if (s<0)
	    s+=j;
#ifdef SQRTMOD_OUTSIDE
	  basis.push_back(j);
	  sqrtmod.push_back(s);
#else
	  basis.push_back(basis_t(j,s));
#endif
	}
      }
      if (basis.size()>=B)
	break;
    }
    vector<unsigned> crible;
    int jp=0;
    if (basis.size()<B){
#ifdef PRIME_SIEVE
      fill_crible(crible,int(2.5*B*std::log(B)));
      jp=nextprime(crible,basis.back().p+1);
#else
      jp=nextprime(int(basis.back().p+1)).val;
#endif
    }
    unsigned lp_basis_pos=0; // position of first prime > 2^16 in the basis
    for (;basis.size()<B;++i){
#ifdef TIMEOUT
      control_c();
#endif
      if (ctrl_c || interrupted)
	break; 
#ifndef PRIMES32
      if (jp>65535){ 
	break;
      }
#endif
      if (debug_infolevel>6 && (i%500==99))
	*logptr(contextptr) << CLOCK() << gettext(" sieve current basis size ") << basis.size() << '\n';
#if 1 // def USE_GMP_REPLACEMENTS
      // int n=fastsmod_compute(N256,jp);
      int n=modulo(*N._ZINTptr,jp),s;
#else
      int n=smod(N,jp).val,s;
#endif
      if (n<0)
	n+=jp;
      if (powmod(n,(unsigned long)((jp-1)/2),jp)==1){
	s=sqrt_mod(n,jp,true,contextptr).val;
	if (s<0)
	  s += jp;
#ifdef LP_TAB_SIZE
	if (!lp_basis_pos && jp> (1<<LP_BIT_LIMIT))
	  lp_basis_pos=basis.size();
#endif
#ifdef SQRTMOD_OUTSIDE
	basis.push_back(jp);
	sqrtmod.push_back(s);
#else
	basis.push_back(basis_t(jp,s));
#endif
      }
#ifdef PRIME_SIEVE
      jp=nextprime(crible,jp+1);
#else
      jp=nextprime(int(jp+1)).val;
#endif
    }
    if (!lp_basis_pos)
      lp_basis_pos=unsigned(basis.size());
#ifdef LP_SMALL_PRIMES
    vector<small_basis_t> small_basis(lp_basis_pos); // will be filled by primes<2^16
#endif
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted){
      mpz_clear(zx); mpz_clear(zy); mpz_clear(zq);  mpz_clear(zr);
      return false;
    }
    double dtarget=1.1;
    if (Mtarget<basis.back().p*dtarget){
      Mtarget=basis.back().p*dtarget; // (int(basis.back().p*1.1)/slicesize)*slicesize;
    }
    unsigned ps=sizeinbase2(basis.back().p);
#if !defined(RTOS_THREADX) && !defined(BESTA_OS) && !defined NSPIRE // def USE_MORE_PRIMES
    unsigned maxadditional=(2+(basis.back().p>>16))*basis.back().p*ps;
#else
    unsigned maxadditional=3*basis.back().p*ps;
#endif
    if (debug_infolevel)
      *logptr(contextptr) << CLOCK() << gettext(" sieve basis OK, size ") << basis.size() << " largest prime in basis " << basis.back().p << " large prime " << maxadditional << " Mtarget " << Mtarget << '\n' ;
    int bs=int(basis.size());
    gen isqrtN=isqrt(N);
    isqrtN.uncoerce(); 
    // now compare isqrtN to a^2 for a in the basis
    double seuil=1.414*evalf_double(isqrtN,1,contextptr)._DOUBLE_val/Mtarget; // should be a
    seuil=std::sqrt(seuil); // should be product of primes of the basis
#ifdef OLD_AFACT
    double dfactors=std::log10(seuil)/3;
    // fixed primes are choosen at basis[pos0], variables are choosen around 2000
    afact=int(dfactors+.5);
    if (afact<=1){
      afact=1;
      int i=20;
      for (;i<3*bs/4;++i){
	if (seuil<basis[i].p){
	  pos1=i;
	  break;
	}
      }
      if (i>=3*bs/4){
	afact=2;
	for (;i<3*bs/4;++i){
	  if (seuil<basis[i].p){
	    pos1=i;
	    break;
	  }
	}
      }
    }
    else {
      if (afact==2){
	seuil=std::sqrt(seuil);
	for (int i=20;i<3*bs/4;++i){
	  if (seuil<basis[i].p){
	    pos1=i;
	    break;
	  }
	}
      }
      else { // afact>=3, 
	if (dfactors>5.4){
	  dfactors -= 3; // 3 large primes
	  afixed = dfactors/.8; // at least 3 fixed
	  afact = 3 + afixed;
	}
	else {
	  dfactors -= 2; // 2 large primes
	  afixed = dfactors/.8;
	  if (afixed==0)
	    afixed=1;
	  afact = 2 +afixed;
	}
	for (int i=0;i<afixed;++i){
	  seuil=seuil/basis[pos0+i].p;
	}
	seuil=std::pow(seuil,1./(afact-afixed));
	for (int i=pos0+afixed+10;i<3*bs/4;++i){
	  if (seuil<basis[i].p){
	    pos1=i;
	    break;
	  }
	}
      }
    }
#else // OLD_AFACT
    double logprod=std::log10(seuil);
    if (logprod<9){  
      afixed=0; 
      afact=int(std::ceil(logprod/3));
      if (logprod-3*afact<-1)
	afixed=1;
    }
    else {
      double logfixed=std::log10(double(basis[pos0].p));
      int maxfixed=int(logprod/logfixed)-2;
      if (maxfixed==0) maxfixed=1;
      double curseuil=1e10;
      afact=0;
      for (int i=1;i<=maxfixed;++i){
	double variable=(logprod-i*logfixed)/3; // we want variable primes to be around 1000
	int ivariable=int(variable);
	double seuiltest=variable/ivariable;
	if (i+ivariable>afact){
	  afixed=i;
	  afact=i+ivariable;
	  curseuil=seuiltest;
	}
      }
    }
    for (int i=0;i<afixed;++i)
      seuil=seuil/basis[pos0+i].p;
    seuil=std::pow(seuil,1./(afact-afixed));
    for (int i=pos0+afixed+10;i<3*bs/4;++i){
      if (seuil<basis[i].p){
	pos1=i-1; // -afact/2;
	break;
      }
    }
#endif // OLD_AFACT
    if (debug_infolevel){
      *logptr(contextptr) << gettext("Using ") << afact << " square factors per a coefficient in polynomials" << '\n';
      *logptr(contextptr) << afixed << gettext(" fixed begin at ") << basis[pos0].p << " and " << afact-afixed << " variables at " << basis[pos1].p << '\n'; 
    }
    vector<ushort_t> isqrtN256;
    // fastsmod_prepare(isqrtN,zx,zy,zr,isqrtN256);
    vector<short_t> isqrtNmodp(bs);
    for (int i=0;i<bs;++i){
#if 1
      // isqrtNmodp[i]=fastsmod_compute(isqrtN256,basis[i].p);
      isqrtNmodp[i]=modulo(*isqrtN._ZINTptr,basis[i].p);
#else
      isqrtNmodp[i]=smod(isqrtN,basis[i].p).val;
#endif
    }
#if defined(RTOS_THREADX) || defined(BESTA_OS) || defined NSPIRE
    unsigned puissancestablength=10000;
#else
#if 1 // def USE_MORE_PRIMES
    unsigned puissancestablength=bs*(80+bs/500);
#else
    unsigned puissancestablength=bs*80;
#endif
#endif
    ushort_t * puissancestab=new ushort_t[puissancestablength];
    ushort_t * puissancesptr=puissancestab;
    ushort_t * puissancesend=puissancestab+puissancestablength;
    slicetype * slice=0;
    if (puissancestab)
      slice=new slicetype[QS_SIZE];
    if (!slice){
      mpz_clear(zx); mpz_clear(zy); mpz_clear(zq);  mpz_clear(zr);
      return false;
    }
    // relations will be written in column
    vector<axbinv> axbmodn; // contains (sqrta,b,x)
    vector<additional_t> additional_primes;
#ifndef ADDITIONAL_PRIMES_HASHMAP
    vector<bool> additional_primes_twice;
#endif
#ifdef LP_TAB_SIZE
    vector<lp_tab_t> lp_map(128); // at most 128 slices in a sieve
#endif
    vecteur sqrtavals,bvals;
#ifdef GIAC_ADDITIONAL_PRIMES
#ifdef ADDITIONAL_PRIMES_HASHMAP
#if defined(EMCC) || defined(EMCC2)
    additional_map_t additional_primes_map;
#else
    additional_map_t additional_primes_map(8*bs);
#endif
    axbmodn.reserve(bs);
#else 
#if defined(RTOS_THREADX) || defined(BESTA_OS) || defined NSPIRE
    additional_primes.reserve(bs);
    additional_primes_twice.reserve(bs);
    axbmodn.reserve(2*bs);
#else
    additional_primes.reserve(4*bs);
    additional_primes_twice.reserve(4*bs);
    axbmodn.reserve(5*bs);
#endif
    sqrtavals.reserve(bs/7);
    bvals.reserve(2*bs/7);
#endif // ADDITIONAL_PRIMES_HASHMAP
#else // GIAC_ADDITIONAL_PRIMES
    axbmodn.reserve(bs+1);
#endif
    // now sieve
    unsigned todo_rel;
    unsigned marge=bs/100;
    if (marge<15)
      marge=15;
    mpz_t alloc1,alloc2,alloc3,alloc4,alloc5;
    mpz_init(alloc1); mpz_init(alloc2); mpz_init(alloc3); mpz_init(alloc4); mpz_init(alloc5);
    // vector<ushort_t> a256,b256,tmpv;
    vector<ushort_t> curpuissances,recheck,pos(afact);
#ifdef WITH_INVA
    vector<ushort_t> Inva(bs);
#endif
    vecteur bvalues; // will contain values of b if afact<=afact0 or components of b if afact>afact0
    // array for efficient polynomial switch (same a change b) when at least afact0 factors/a
#ifdef PRIMES32
    const int afact0=3;
    vector<int> bainv2((afact-1)*bs);
    vector<longlong> up1tmp;
#endif
    for (int i=0;i<afixed;++i)
      pos[i]=pos0+i;
    for (int i=afixed;i<afact;++i)
      pos[i]=pos1+i-afixed; // FIXME should be -afixed
    double Mval=1;
    for (int i=0;i<afact;++i)
      Mval=Mval*basis[pos[i]].p;
    Mval=std::sqrt(2*Nd)/(Mval*Mval);
    if (debug_infolevel)
      *logptr(contextptr) << gettext("First M ") << Mval << '\n';
    Mtarget=Mval;
    int avar=afact-afixed;
    int end_pos1=2*pos1;
    if (avar>1)
      end_pos1=pos1+100;
    if (avar>2)
      end_pos1=pos1+30;
    if (int(lp_basis_pos)<end_pos1)
      end_pos1=lp_basis_pos;
    for (;puissancesptr<puissancesend;++pos.back()){
      double bpos2=1;
      if (int(pos.back())>=end_pos1 || basis[pos.back()].p>=45000){
	int i=afact-2;
	for (;i>afixed;--i){
	  if (int(pos[i])<end_pos1-(afact-i)){
	    ++pos[i];
	    for (int j=i+1;j<afact;++j)
	      pos[j]=pos[i]+(j-i);
	    break;
	  }
	}
	if (i<=afixed){
	  --pos1;
	  if (pos1<=5){
	    mpz_clear(zx); mpz_clear(zy); mpz_clear(zq);  mpz_clear(zr);
	    mpz_clear(alloc1); mpz_clear(alloc2); mpz_clear(alloc3); mpz_clear(alloc4); mpz_clear(alloc5);
	    delete [] puissancestab;
	    return false;
	  }
	  // reset fixed factors
	  for (i=0;i<afixed;++i)
	    pos[i]=pos0+i;
	  for (i=afixed;i<afact;++i)
	    pos[i]=pos1+i-afixed;
	}
      }
      for (int i=0;i<afact;++i)
	bpos2=bpos2*basis[pos[i]].p;
      Mval=std::sqrt(2*Nd)/(bpos2*bpos2);
      if (afixed){
	// move "fixed" factors so that Mval becomes closer to Mtarget
	// NB: for later threads, afixed should be >=3, so that we can move pos[1] by thread
	while (Mval>1.1*Mtarget && int(pos[afixed-1])<int(pos1)-10){
	  // Mval is too large, hence one factor of a is too small, increase it
	  if (int(pos[0])<pos0){
	    ++pos[0];
	    double coeff=basis[pos[0]].p/double(basis[pos[0]-1].p);
	    Mval=Mval/(coeff*coeff);
	  }
	  else {
	    ++pos[afixed-1];
	    double coeff=basis[pos[afixed-1]].p/double(basis[pos[afixed-1]-1].p);
	    Mval=Mval/(coeff*coeff);	    
	  }
	}
	while (Mval<0.9*Mtarget && pos[0]>10){
	  // Mval is too small, decrease one factor of a
	  --pos[0];
	  if (pos[0]<=10){
	    Mval=0;
	    break;
	  }
	  double coeff=basis[pos[0]].p/double(basis[pos[0]+1].p);
	  Mval=Mval/(coeff*coeff);
	}
      }
      if ( Mval <0.7*Mtarget ){
	if (pos1>pos0+afixed+5 || Mval<32768){
	  // CERR << pos ;
	  int i=afact-1;
	  for (;i>afixed+1;--i){
	    if (pos[i]>pos[i-1]+5)
	      break;
	  }
	  if (i<=afixed+1){
	    --pos1;
	    for (i=0;i<afixed;++i)
	      pos[i]=pos0+i;
	    for (i=afixed;i<afact;++i)
	      pos[i]=pos1+i-afixed;
	  }
	  else {
	    ++pos[i-1];
	    for (;i<afact;++i)
	      pos[i]=pos[i-1]+1;
	  }
	  // CERR << pos << '\n';
	}
      }
      // finished?
#ifdef TIMEOUT
      control_c();
#endif
      if (ctrl_c || interrupted)
	break;
#ifdef ADDITIONAL_PRIMES_HASHMAP
      todo_rel=bs+marge;
#else
      todo_rel=bs+marge+unsigned(additional_primes.size());
#endif
      if (axbmodn.size()>=todo_rel)
	break;
      int nrelationsa=0;
      // Not finished yet, construct a new value of a around ad=sqrt(2*n)/M 
      // using a product of afact square of primes that are in the basis
      // and construct a vector of 2^(afact-1) corresponding values of b
      // and compute the values of inverses of a mod p
      ulonglong usqrta(basis[pos[0]].p);
      for (int i=1;i<afact;++i)
	usqrta=basis[pos[i]].p*usqrta; // works up to about N=1e86
      gen sqrta((int) basis[pos[0]].p);
      for (int i=1;i<afact;++i)
	sqrta=gen(int(basis[pos[i]].p))*sqrta;
      sqrtavals.push_back(sqrta);
      gen a=sqrta*sqrta; // a should be about sqrt(Nd/2)/M
      a.uncoerce();
      int M=int(std::floor(std::sqrt(Nd*2)/evalf_double(a,1,contextptr)._DOUBLE_val));
      if (debug_infolevel>6)
	*logptr(contextptr) << CLOCK() << gettext(" initial value for M= ") << M << '\n';
      int nslices=int(std::ceil((2.*M)/slicesize));
      M=(nslices*slicesize)/2;
      bvalues.clear();
      gen curprod=1;
      for (int i=0;;){
#ifdef SQRTMOD_OUTSIDE
	int s=sqrtmod[pos[i]]; 
#else
	int s=basis[pos[i]].sqrtmod; 
#endif
	int p=basis[pos[i]].p;
	longlong p2=p*longlong(p); 
	// Hensel lift s to be a sqrt of n mod p^2: (s+p*r)^2=s^2+2p*r*s=n => r=(n-s^2)/p*inv(2*s mod p)
	int r=p<37000?int((modulo(*N._ZINTptr,p2)-s*s)/p):((smod(N,p2)-s*s)/p).val;
	r=(r*invmod(2*s,p))%p;
	// overflow should not happen because p is a factor of a hence choosen
	// in the 1000 range (perhaps up to 10 000, but not much larger)
	// if ((longlong(r)*p)!=r*p) CERR << "overflow" << '\n';
	s += p*r;
#ifdef PRIMES32
	if (afact>afact0){
	  // store s*(inv( product(basis[pos[j]]^2,j!=i) mod p2)) in bvalues[i]
	  longlong up1=(usqrta/p);
	  longlong up=up1%p2;
	  up=(up*up)%p2;
	  //longlong tmp=(s*invmodnoerr(up,p2))%p2;
	  //up1tmp.push_back(up1);
	  //up1tmp.push_back(tmp);
	  gen tmp=smod(s*invmod(gen(up),gen(p2)),p2);
	  if (is_greater(0,tmp,contextptr)) tmp=-tmp;
	  gen gup1(up1);
	  bvalues.push_back(gup1*gup1*tmp);
	  bvalues.back().uncoerce();
	}
	else 
#endif
	  {
	    if (bvalues.empty())
	      bvalues.push_back(s);
	    else {
	      int js=int(bvalues.size());
	      for (int j=0;j<js;++j){
		bvalues.push_back(ichinrem(bvalues[j],-s,curprod,p2));
		bvalues[j]=ichinrem(bvalues[j],s,curprod,p2);
	      }
	    }
	  }
	++i;
	if (i==afact)
	  break;
	curprod = p*(p*curprod);
      } // end for
      // compute inverse of a modulo p (will set to 0 if not invertible)
      if (debug_infolevel>6)
	*logptr(contextptr) << CLOCK() << gettext(" Computing inverses mod p of the basis ") << '\n';
      // fastsmod_prepare(a,zx,zy,zr,a256);
      gen b;
      for (int i=0;i< (1<<(afact-1));++i){
#ifdef TIMEOUT
	control_c();
#endif
	if (ctrl_c || interrupted)
	  break;
#ifdef ADDITIONAL_PRIMES_HASHMAP
	todo_rel=bs+marge;
#else
	todo_rel=bs+marge+unsigned(additional_primes.size());
#endif
	if (axbmodn.size()>=todo_rel)
	  break;
	if (debug_infolevel>6)
	  *logptr(contextptr) << CLOCK() << gettext(" Computing c ") << '\n';
#ifdef PRIMES32
	int bv=1,be=-1;
	if (afact>afact0){
	  if (i==0){
	    b=0;
	    for (unsigned j=0;j<bvalues.size();j++)
	      b += bvalues[j];
	  }
	  else {
	    find_bv_be(i,bv,be);
	    b += (2*be)*bvalues[bv];
	  }
	}
	else
	  b=bvalues[i];
#else
	b=bvalues[i];
#endif
	b.uncoerce();
	mpz_mul(zx,*b._ZINTptr,*b._ZINTptr);
	mpz_sub(zy,zx,*N._ZINTptr);
#ifdef USE_GMP_REPLACEMENTS
	mp_grow(&alloc1,zy.used+2);
	mpz_set_ui(alloc1,0);
	alloc1.used = zy.used +2 ;
	mpz_set(alloc2,zy);
	mpz_set(alloc3,*a._ZINTptr);
	alloc_mp_div(&zy,a._ZINTptr,&zq,&zr,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
#else
	mpz_divexact(zq,zy,*a._ZINTptr);
#endif
	// gen c=zq; // gen c=(b*b-N)/a;
	// c.uncoerce();
#ifdef PRIMES32
	if (afact<=afact0)
#endif
	  {
	    bool bneg=mpz_cmp_ui(*b._ZINTptr,0)<0;
	    if (bneg)
	      mpz_neg(*b._ZINTptr,*b._ZINTptr);
	  }
	bvals.push_back(b);
	if (debug_infolevel>6)
	  *logptr(contextptr) << CLOCK() << gettext(" Computing roots mod the basis ") << '\n';
	// fastsmod_prepare(b,zx,zy,zr,b256);
#ifdef PRIMES32 
	if (i && afact>afact0)
	  switch_roots(bainv2,basis,
#ifdef LP_SMALL_PRIMES
		       small_basis,
#endif
		       lp_basis_pos,nslices,slicesize,bv,be,afact,pos,b,zq,M);
	else {
	  init_roots(basis,
#ifdef LP_SMALL_PRIMES
		     small_basis,
#endif
#ifdef WITH_INVA
		     Inva,
#endif
#ifdef SQRTMOD_OUTSIDE
		     sqrtmod,
#endif
		     bainv2,afact,afact0,
		     a,b,bvalues,zq,M);
#ifdef LP_TAB_TOGETHER
	  // init all hashtable for large primes at once
	  unsigned cl;
	  if (debug_infolevel>3){
	    cl=CLOCK();
	    *logptr(contextptr) << cl << gettext(" Init large prime hashtables ") << '\n';
	  }
	  int total=(nslices << (afact-1));
	  if (int(lp_map.size()) < total)
	    lp_map.resize(total);
	  for (int k=0;k< total;++k)
	    lp_map[k].clear();
	  if (lp_basis_pos){
	    for (int k=0;;){
	      basis_t * bit=&basis[0]+lp_basis_pos, * bitend=&basis[0]+bs;
	      unsigned endpos=nslices*slicesize;
	      lp_tab_t * ptr=&lp_map[0]+k*nslices;
	      for (;bit!=bitend;++bit){
		register ushort_t p=bit->p;
		register unsigned pos=bit->root1;
		for (;pos<endpos; pos += p){
		  (ptr+(pos >> LP_TAB_SIZE))->push_back(lp_entry_t((pos & LP_MASK),p));
		}
		pos=bit->root2;
		for (;pos<endpos; pos += p){
		  (ptr+(pos >> LP_TAB_SIZE))->push_back(lp_entry_t((pos & LP_MASK),p));
		}
	      }
	      ++k;
	      if (k== (1 << (afact-1))){
		if (debug_infolevel>3){
		  unsigned cl2=CLOCK();
		  *logptr(contextptr) << cl2 << gettext(" End large prime hashtables ") << cl2-cl << '\n';
		}
		break;
	      }
	      find_bv_be(k,bv,be);
	      // switch roots to next polynomial
	      int * bvpos=&bainv2[(bv-1)*bs],* bvposend=bvpos+bs;
	      bvpos += lp_basis_pos;
	      basis_t * basisptr=&basis[0]+lp_basis_pos;
	      if (be>0){
		for (;bvpos<bvposend;++basisptr,++bvpos){
		  register unsigned p=basisptr->p;
		  register int r=basisptr->root1-(*bvpos);
		  if (r<0)
		    r+=p;
		  basisptr->root1=r;
		  r=basisptr->root2-(*bvpos);
		  if (r<0)
		    r+=p;
		  basisptr->root2=r;
		}
	      }
	      else {
		for (;bvpos<bvposend;++basisptr,++bvpos){
		  register unsigned p=basisptr->p;
		  register int r=basisptr->root1+(*bvpos);
		  if (r>int(p))
		    r-=p;
		  basisptr->root1=r;
		  r=basisptr->root2+(*bvpos);
		  if (r>int(p))
		    r-=p;
		  basisptr->root2=r;
		}
	      }
	    }
	  }
#endif // LP_TAB_TOGETHER
	} // end else of if i==0
#if defined(LP_TAB_SIZE) && !defined(LP_TAB_TOGETHER)
	if (int(lp_map.size()) < nslices)
	  lp_map.resize(nslices);
	for (int k=0;k< nslices;++k)
	  lp_map[k].clear();
	if (lp_basis_pos){
	  basis_t * bit=&basis[0]+lp_basis_pos, * bitend=&basis[0]+bs;
	  unsigned endpos=nslices*slicesize;
	  for (;bit!=bitend;++bit){
	    register ushort_t p=bit->p;
	    register unsigned pos=bit->root1;
	    for (;pos<endpos; pos += p){
	      lp_map[pos >> LP_TAB_SIZE].push_back(lp_entry_t((pos & LP_MASK),p));
	    }
	    pos=bit->root2;
	    for (;pos<endpos; pos += p){
	      lp_map[pos >> LP_TAB_SIZE].push_back(lp_entry_t((pos & LP_MASK),p));
	    }
	  }
	}
#endif // LP_TAB_SIZE && !LP_TAB_TOGETHER
#else // PRIMES32
	init_roots(basis,
#ifdef WITH_INVA
		     Inva,
#endif
#ifdef SQRTMOD_OUTSIDE
		   sqrtmod,
#endif
		   usqrta,a,b,bvalues,zq,M);
#endif // PRIMES32
	// we can now sieve in [-M,M[ by slice of size slicesize
#ifndef GIAC_HAS_STO_38  
	if (debug_infolevel>5){
	  *logptr(contextptr) << CLOCK();
	  *logptr(contextptr) << gettext(" Polynomial a,b,M=") << a << "," << b << "," << M << " (" << pos << ")" ;
	  *logptr(contextptr) << CLOCK() << '\n';
	}
#endif  
	int nrelationsb=0;
#ifdef LP_TAB_SIZE
#endif
	for (int l=0;l<nslices;l++){
#ifdef TIMEOUT
	  control_c();
#endif
	  if (ctrl_c || interrupted)
	    break;
#ifdef ADDITIONAL_PRIMES_HASHMAP
	  todo_rel=bs+marge;
#else
	  todo_rel=bs+marge+unsigned(additional_primes.size());
#endif
	  if (axbmodn.size()>=todo_rel)
	    break;
	  int shift=-M+l*slicesize;
	  int slicerelations=msieve(a,sqrtavals,
				    bvals,zq,basis,lp_basis_pos,
#ifdef LP_SMALL_PRIMES
				    small_basis,
#endif
				    maxadditional,
#ifdef ADDITIONAL_PRIMES_HASHMAP
				    additional_primes_map,
#else
				    additional_primes,additional_primes_twice,
#endif
				    N,isqrtN,
				    slice,slicesize,shift,puissancestab,puissancesptr,puissancesend,curpuissances,recheck,
				    axbmodn,
				    zx,zy,zr,alloc1,alloc2,alloc3,alloc4,alloc5,
#ifdef LP_TAB_SIZE
#ifdef LP_TAB_TOGETHER
				    lp_map[l+nslices*i],
#else
				    lp_map[l],
#endif
#endif
				    contextptr);
	  if (slicerelations==-1){
	    *logptr(contextptr) << gettext("Sieve error: Not enough memory ") << '\n';
	    break;
	  }
	  nrelationsb += slicerelations;
#ifdef ADDITIONAL_PRIMES_HASHMAP
	  todo_rel=bs+marge;
#else
	  todo_rel=bs+marge+unsigned(additional_primes.size());
#endif
	}
	if (nrelationsb==0) 
	  bvals.pop_back();
	else
	  nrelationsa += nrelationsb;
      }
#if defined( RTOS_THREADX) || defined(BESTA_OS) || defined NSPIRE
      if (debug_infolevel){
#ifdef NSPIRE
	static int count_print=0;
	++count_print;
	if (count_print%4==0)
#endif
	  *logptr(contextptr) << axbmodn.size() << " of " << todo_rel << " (" << 100-100*(todo_rel-axbmodn.size())/double(bs+marge) << "%)" << '\n';
      }
#endif
      if (nrelationsa==0){
	sqrtavals.pop_back();
      }
#if !defined(RTOS_THREADX) && !defined(BESTA_OS) && !defined NSPIRE
      if (debug_infolevel>1)
	*logptr(contextptr) << CLOCK()<< gettext(" sieved : ") << axbmodn.size() << " of " << todo_rel << " (" << 100-100*(todo_rel-axbmodn.size())/double(bs+marge) << "%), M=" << M << '\n';
#endif
    } // end sieve loop
    if (debug_infolevel)
      *logptr(contextptr) << gettext("Polynomials a,b in use: #a ") << sqrtavals.size() << " and #b " << bvals.size() << '\n';
    delete [] slice;
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted || puissancesptr==puissancesend){
      mpz_clear(zx); mpz_clear(zy); mpz_clear(zq);  mpz_clear(zr);
      mpz_clear(alloc1); mpz_clear(alloc2); mpz_clear(alloc3); mpz_clear(alloc4); mpz_clear(alloc5);
      delete [] puissancestab;
      return false;
    }
    // We have enough relations, make matrix, reduce it then find x^2=y^2 mod n congruences
    if (debug_infolevel)
      *logptr(contextptr) << CLOCK() << gettext(" sieve done: used ") << (puissancesptr-puissancestab)*0.002 << " K for storing relations (of " << puissancestablength*0.002 << ")" << '\n';
    release_memory(isqrtNmodp);
#ifdef GIAC_ADDITIONAL_PRIMES 
#ifdef ADDITIONAL_PRIMES_HASHMAP
    additional_primes.reserve(axbmodn.size());
    vector<axbinv>::const_iterator it=axbmodn.begin(),itend=axbmodn.end();
    for (;it!=itend;++it) {
      unsigned u=largep(*it,puissancestab);
      if (u)
	additional_primes.push_back(u);
    }
    sort(additional_primes.begin(),additional_primes.end()); // for binary search later
#else
    if (debug_infolevel)
      *logptr(contextptr) << CLOCK() << gettext(" removing additional primes") << '\n';
    // remove relations with additional primes which are used only once
    int lastp=int(axbmodn.size())-1,lasta=int(additional_primes.size())-1;
    for (int i=0;i<=lastp;++i){
      ushort_t * curbeg=puissancestab+axbmodn[i].first, * curend=puissancestab+axbmodn[i].second;
      bool done=false;
      for (;curbeg!=curend;++curbeg){
	if (*curbeg==1)
	  break;
      }
      if (curbeg==curend)
	continue;
      ++curbeg;
      additional_t u=*curbeg;
#if GIAC_ADDITIONAL_PRIMES==32 && !defined(PRIMES32)
      u <<=16 ;
      ++curbeg;
      u += *curbeg;
#endif
      int pos=_equalposcomp(additional_primes,u);
      if (!pos)
	continue;
      if (pos>lasta){
	// *logptr(contextptr) << cur << '\n';
	continue;
      }
      --pos;
      if (additional_primes_twice[pos])
	continue;
      axbmodn[i]=axbmodn[lastp];
      --lastp;
      additional_primes[pos]=additional_primes[lasta];
      additional_primes_twice[pos]=additional_primes_twice[lasta];
      --lasta;
      --i; // recheck at current index
    }
    axbmodn.resize(lastp+1);
    additional_primes.resize(lasta+1);
    if (debug_infolevel)
      *logptr(contextptr) << CLOCK() << gettext(" end removing additional primes") << '\n';
#endif // ADDTIONAL_PRIMES_HASHMAP
#endif // GIAC_ADDITIONAL_PRIMES
    // Make relations matrix (currently dense, FIXME improve to sparse and Lanczos algorithm)
    int C32=int(std::ceil(axbmodn.size()/32./GIAC_RREF_UNROLL))*GIAC_RREF_UNROLL;
    unsigned * tab=new unsigned[axbmodn.size()*C32],*tabend=tab+axbmodn.size()*C32;
    if (!tab){
      mpz_clear(zx); mpz_clear(zy); mpz_clear(zq); mpz_clear(zr);
      mpz_clear(alloc1); mpz_clear(alloc2); mpz_clear(alloc3); mpz_clear(alloc4); mpz_clear(alloc5);
      delete [] puissancestab;
      return false;
    }
    // init tab
    for (unsigned * ptr=tab;ptr!=tabend;++ptr)
      *ptr=0;
    int l32=C32*32;
    vector< line_t > relations(axbmodn.size());
    for (unsigned i=0;i<axbmodn.size();++i){
      relations[i].tab=tab+i*C32;
    }
    for (unsigned j=0;j<axbmodn.size();j++){
      ushort_t * curpui=puissancestab+axbmodn[j].first, * curpuiend=puissancestab+axbmodn[j].second;
      add_relation(relations,j,curpui,curpuiend,basis,additional_primes);
#ifdef ADDITIONAL_PRIMES_HASHMAP
      unsigned u=largep(axbmodn[j],puissancestab);
      if (u){
	axbinv & A=additional_primes_map[u];
	curpui=puissancestab+A.first; curpuiend=puissancestab+A.second;
	add_relation(relations,j,curpui,curpuiend,basis,additional_primes);
      }
#endif
    } // end loop on j in puissances
#ifdef RREF_SORT // seems slower
    unsigned count0=0,count1=0;
    for (int i=0;i<relations.size();++i){
      int c=relations[i].count=count_ones(relations[i].tab,C32);
      if (c==0)
	++count0;
      if (c==1)
	++count1;
      if (debug_infolevel>2){
	cout << i << ", p=";
	if (i==0) 
	  cout << "-1";
	else {
	  if (i<=bs)
	    cout << basis[i-1].p << " " << relations[i].count << '\n';
	  else
	    cout << '\n';
	}
      }
    }
    if (debug_infolevel)
      *logptr(contextptr) << CLOCK() << " begin rref size " << relations.size() << "x" << l32 << " K " << 0.004*relations.size()*C32 << ", " << count0 << " null lines, " << count1 << " 1-line" << '\n';
#if 0 // debug only
    for (int i=0;i<relations.size();++i){
      cout << i << ", p=";
      if (i==0) 
	cout << "-1";
      else {
	if (i<=bs)
	  cout << basis[i-1].p << " " << relations[i].count << '\n';
	else
	  cout << '\n';
      }
    }
#endif
    sort(relations.begin(),relations.end()); // put 0 lines at end, otherwise asc. sort
#else // RREF_SORT
    if (debug_infolevel)
      *logptr(contextptr) << CLOCK() << " begin rref size " << relations.size() << "x" << l32 << " K " << 0.004*relations.size()*C32 << '\n';
    reverse(relations.begin(),relations.end());
#endif // RREF_SORT
    // rref(relations,relations.size(),C32,0);
    rref(relations,int(relations.size()),C32,1);
    rref(relations,int(relations.size()),C32,2);
    if (debug_infolevel)
      *logptr(contextptr) << CLOCK() << " end rref" << '\n';
    // printbool(*logptr(contextptr),relations);
    // move pivots on the diagonal by inserting 0 lines
    vector< unsigned * > relations2(l32);
    i=0;
    int j=0,rs=int(relations.size());
    for (;i<rs && j<l32;++j){
      if (relations[i].tab[j/32] & (1 << j%32)){
	swap(relations2[j],relations[i].tab);
	++i;
      }
    }
    // printbool(*logptr(contextptr),relations2);
    // for each element of the kernel compute x and y / x^2=y^2[N] 
    // then gcd(x-y,n_orig)
    for (i=0;i<l32;++i){
      if (relations2[i] && (relations2[i][i/32] & (1<<i%32)))
	continue;
      // using column i of relations2 which is in the kernel, build x and y
      // for x, we can compute the product of the axbmodn mod N
      // for y, compute the product of sqrta mod N and multiply later by ax^2+bx+c factored part
      // since (a*x+b)^2=a*(a*x^2+2*b*x+c) mod N
      gen x=1,y=1,cur;
      mpz_set_ui(zx,1);
      mpz_set_ui(zy,1);
      vector<short_t> p(bs), add_p(additional_primes.size());
      for (int j=0;j<l32;++j){
	if (j<int(axbmodn.size()) && (i==j || (relations2[j] && (relations2[j][i/32] & (1<<(i%32)))))){
	  if (axbmodn[j].aindex>=sqrtavals.size() || axbmodn[j].bindex>=bvals.size())
	    return false; // check added because ifactor(nextprime(alog10(17))*nextprime(alog10(19))); fails on Prime (and unable to do parallel debug in giac)
	  update_xy(axbmodn[j],zx,zy,p,add_p,N,basis,additional_primes,sqrtavals,bvals,puissancestab,zq,zr,alloc1,alloc2,alloc3,alloc4,alloc5);
#ifdef ADDITIONAL_PRIMES_HASHMAP
	  unsigned u=largep(axbmodn[j],puissancestab);
	  if (u)
	    update_xy(additional_primes_map[u],zx,zy,p,add_p,N,basis,additional_primes,sqrtavals,bvals,puissancestab,zq,zr,alloc1,alloc2,alloc3,alloc4,alloc5);
#endif
	} // end if (j<axbmodn.size() ...) 
      } // end for unsigned j=0; j<l32
      for (int i=0;i<bs;++i){
	if (p[i] % 2)
	  *logptr(contextptr) << gettext("error, odd exponent for prime ") << basis[i].p << '\n';
	if (p[i]){
#if 1
	  mpz_set_ui(alloc1,basis[i].p);
	  for (int j=0;j<p[i]/2;++j)
	    mpz_mul(zy,zy,alloc1);
#ifdef USE_GMP_REPLACEMENTS
	  mp_grow(&alloc1,zy.used+2);
	  mpz_set_ui(alloc1,0);
	  alloc1.used = zy.used +2 ;
	  mpz_set(alloc2,zy);
	  mpz_set(alloc3,*N._ZINTptr);
	  // mpz_set_si(alloc4,0);
	  // mpz_set_si(alloc5,0);
	  alloc_mp_div(&zy,N._ZINTptr,&zq,&zr,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
#else
	  mpz_tdiv_r(zr,zy,*N._ZINTptr);
#endif
	  mpz_set(zy,zr);
#else
	  y=y*pow(gen(basis[i].p),int(p[i]/2));
	  y=smod(y,N);
#endif
	}
      }
      for (unsigned i=0;i<additional_primes.size();++i){
	if (add_p[i] % 2)
	  *logptr(contextptr) << gettext("error") << i << '\n';
	if (add_p[i]){
#if 1
	  mpz_set_ui(alloc1,additional_primes[i]);
	  for (int j=0;j<add_p[i]/2;++j)
	    mpz_mul(zy,zy,alloc1);
#ifdef USE_GMP_REPLACEMENTS
	  mp_grow(&alloc1,zy.used+2);
	  mpz_set_ui(alloc1,0);
	  alloc1.used = zy.used +2 ;
	  mpz_set(alloc2,zy);
	  mpz_set(alloc3,*N._ZINTptr);
	  // mpz_set_si(alloc4,0);
	  // mpz_set_si(alloc5,0);
	  alloc_mp_div(&zy,N._ZINTptr,&zq,&zr,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
#else
	  mpz_tdiv_r(zr,zy,*N._ZINTptr);
#endif
	  mpz_set(zy,zr);
#else
	  y=y*pow(gen(additional_primes[i]),int(add_p[i]/2));
	  y=smod(y,N);
#endif
	}
      }
#if 1
      y=zy;
      x=zx;
#endif
      cur=gcd(x-y,n_orig);
      if (debug_infolevel>6)
	*logptr(contextptr) << CLOCK() << gettext("checking gcd") << cur << " " << N << '\n';
      if ( (cur.type==_INT_ && cur.val>7) || 
	   (cur.type==_ZINT && is_strictly_greater(n_orig,cur,contextptr))){
	pn=cur;
	mpz_clear(zx); mpz_clear(zy); mpz_clear(zq); mpz_clear(zr);
	mpz_clear(alloc1); mpz_clear(alloc2); mpz_clear(alloc3); mpz_clear(alloc4); mpz_clear(alloc5);
	delete [] puissancestab;
	delete [] tab;
	return true;
      }
    }
    mpz_clear(zx); mpz_clear(zy); mpz_clear(zq); mpz_clear(zr);
    mpz_clear(alloc1); mpz_clear(alloc2); mpz_clear(alloc3); mpz_clear(alloc4); mpz_clear(alloc5);
    delete [] puissancestab;
    delete [] tab;
    return false;
  }

  // elliptic curve method, 
  // http://math.univ-lyon1.fr/~roblot/resources/factorisation.pdf
  // This is a very naive implementation
  // It does not use Montgomery representation and only phase 1
  // For professional implementations, cf.
  // https://members.loria.fr/PZimmermann/papers/ecm-submitted.pdf
  // https://pdfs.semanticscholar.org/e8eb/13b75292b15dd63c3e7e4b1c8dc334d278ba.pdf
  // ecm will be used if available
#define ECM_MAXITER 1000
  static gen L(double alpha,double beta,double N){
    double lnN=std::log(N);
    return std::exp(beta*std::pow(lnN,alpha)*std::pow(std::log(lnN),1-alpha));
  }


#ifndef USE_GMP_REPLACEMENTS
  // addition in elliptic curve, returns 1 on success or 0 and m=a divisor of n
  int ecm_add(const mpz_t &x1,const mpz_t &y1,const mpz_t & x2,const mpz_t &y2,const mpz_t & a,const mpz_t & n,mpz_t & m,mpz_t & x,mpz_t &y){
    if (mpz_cmp(x1,x2)){
      mpz_sub(x,x2,x1); // x=x2-x1
      int res=mpz_invert(m,x,n); // m=inv(x2-x1) mod n
      if (res==0){ // not invertible
	mpz_gcd(m,x,n);
	return 0; // m has non trivial gcd with n
      }
      mpz_sub(y,y2,y1);
      mpz_mul(m,m,y); // m=(y2-y1)*invmod(x2-x1,n);
    }
    else {
      mpz_mul_ui(y,y1,2);
      int res=mpz_invert(m,y,n); // m=inv(2*y) mod n
      if (res==0){ // not invertible
	mpz_gcd(m,y,n);
	return 0; // m has non trivial gcd with n
      }
      mpz_mul(x,x1,x1);
      mpz_mul_ui(x,x,3);
      mpz_add(x,x,a);
      mpz_mul(m,m,x); // m=(3*x1*x1+a)*invmod(2*y1,n);
    }
    mpz_fdiv_r(m,m,n); // m=mod(m,n);
    mpz_mul(x,m,m);
    mpz_sub(x,x,x1);
    mpz_sub(x,x,x2);
    mpz_fdiv_r(x,x,n); // x=mod(m*m-x1-x2,n);
    mpz_sub(y,x1,x);
    mpz_mul(y,m,y);
    mpz_sub(y,y,y1);
    mpz_fdiv_r(y,y,n); // y=mod(m*(x1-x)-y1,n);
    // smod x and y
    mpz_add(m,x,x);
    if (mpz_cmp(m,n)>0)
      mpz_sub(x,x,n);
    mpz_add(m,y,y);
    if (mpz_cmp(m,n)>0)
      mpz_sub(y,y,n);
    return 1;
  }
#endif

  gen ecm_add(const gen &x1,const gen &y1,const gen & x2,const gen &y2,const gen & a,const gen & n,gen & m,gen & x,gen &y){
    if (is_inf(x1)){
      x=x2; y=y2; return 1;
    }
    if (is_inf(x2)){
      x=x1; y=y1; return 1;
    }
    if (y1+y2==0){
      y=x=unsigned_inf; return 1;
    }
#ifndef USE_GMP_REPLACEMENTS
    if (x1.type==_ZINT && y1.type==_ZINT && x2.type==_ZINT && y2.type==_ZINT && a.type==_ZINT && n.type==_ZINT ){
      m=gen(1LL<<33);
      x=gen(1LL<<33);
      y=gen(1LL<<33);
      if (!ecm_add(*x1._ZINTptr,*y1._ZINTptr,*x2._ZINTptr,*y2._ZINTptr,*a._ZINTptr,*n._ZINTptr,*m._ZINTptr,*x._ZINTptr,*y._ZINTptr)){
	return gen(*m._ZINTptr);
      }
      return 1;
    }
#endif
    if (x1!=x2){
      m=gcd(x2-x1,n);
      if (m!=1)
	return m;
      m=(y2-y1)*invmod(x2-x1,n);
    }
    else {
      m=gcd(y1,n);
      if (m!=1)
	return m;
      m=(3*x1*x1+a)*invmod(2*y1,n);
    }
    m=smod(m,n);
    x=smod(m*m-x1-x2,n);
    y=smod(m*(x1-x)-y1,n);
    return 1;
  }
  // multiplication in elliptic curve,
  gen ecm_mult(const gen &x1,const gen &y1,ulonglong m,const gen & a,const gen & n,gen & x,gen &y){
    gen x2(x1),y2(y1),xtmp,ytmp,g,M;
    y=x=plus_inf;
    while (m){
      if (m%2){
	g=ecm_add(x,y,x2,y2,a,n,M,xtmp,ytmp);
	if (g!=1) 
	  return g;
	swapgen(x,xtmp); swapgen(y,ytmp);// x=xtmp; y=ytmp;
      }
      m/=2;
      g=ecm_add(x2,y2,x2,y2,a,n,M,xtmp,ytmp); // improve: ecmdup
      if (g!=1) 
	return g;
      swapgen(x2,xtmp);swapgen(y2,ytmp);// x2=xtmp; y2=ytmp;
    }
    return 1;
  }
  gen _ecm_factor(const gen &n_,GIAC_CONTEXT){
    gen B,n(n_);
    int maxiter(ECM_MAXITER);
    if (n.type==_VECT && n._VECTptr->size()>=2){
      const vecteur & v=*n._VECTptr;
      B=v[1];
      if (v.size()>=3 && v[2].type==_INT_)
	maxiter=giacmax(1,v[2].val);
      n=v.front();
    }
    if (!is_integer(n) || is_positive(-n,contextptr))
      return gensizeerr(contextptr);
    if (_isprime(n,contextptr)!=0)
      return n;
    double logp=.5*std::log(evalf_double(n,1,contextptr)._DOUBLE_val);
#ifdef HAVE_LIBECM
    double epsilon=.02; // to be adjusted
#else
    double epsilon=.45; // to be adjusted
#endif
    if (logp>80) // research factors of size not exceeding 35 digits
      logp=80;
    if (B==0)
      B=L(.5,0.707+epsilon,std::exp(logp));
    // B=1000;
    B=_ceil(B,contextptr);
#ifdef HAVE_LIBECM
    *logptr(contextptr) << "ECM-GMP factor n="<< n << " , B=" << B << ", #curves <=" << maxiter << '\n';
    n.uncoerce();
    double B1=evalf_double(B,1,contextptr)._DOUBLE_val;
    /* From ECM README, table of optimal values of B1
       digits D  optimal B1   default B2           expected curves
                                                       N(B1,B2,D)
                                              -power 1         default poly
          20       11e3         1.9e6             74               74 [x^1]
          25        5e4         1.3e7            221              214 [x^2]
          30       25e4         1.3e8            453              430 [D(3)]
          35        1e6         1.0e9            984              904 [D(6)]
          40        3e6         5.7e9           2541             2350 [D(6)]
          45       11e6        3.5e10           4949             4480 [D(12)]
          50       43e6        2.4e11           8266             7553 [D(12)]
          55       11e7        7.8e11          20158            17769 [D(30)]
          60       26e7        3.2e12          47173            42017 [D(30)]
          65       85e7        1.6e13          77666            69408 [D(30)]

     */
    int res;
    gen F(1LL<<33);
    for (int i=0;i<maxiter;++i){
      res=ecm_factor(*F._ZINTptr, *n._ZINTptr, B1, 0);
      if (res!=0) break;
    }
    if (res==0) return undef;
    return F;
#else
    *logptr(contextptr) << "ECM naive factor n="<< n << " , B=" << B << ", #curves <=" << maxiter << '\n';
    for (int i=0;i<maxiter;++i){
      gen a,x,y,b,d,g;
      a= rand_interval(makevecteur(0,n-1),true,contextptr);// a=1078104638; 
      x= smod(rand_interval(makevecteur(0,n-1),true,contextptr),n);// 317359960;
      y= smod(rand_interval(makevecteur(0,n-1),true,contextptr),n);// 983830906;
      b=smod(y*y-x*x*x-a*x,n);
      if (debug_infolevel)
	COUT << CLOCK()*1e-6 << " Factor "<< n << " ECM curve " << i << ", B="<<B << ", a=" << a << ", x=" << x << ", y=" << y << '\n';
      d=4*a*a*a-27*b*b;
      g=gcd(d,n);
      if (g==n)
	continue;
      if (g!=1)
	return g;
      gen p(2),pe,tmp,xm,ym;
      for (;is_greater(B,pe=p*p,contextptr);p=nextprime(p+1)){
	int e=2;
	while (is_greater(B,tmp=pe*p,contextptr)){
	  ++e;
	  pe=tmp;
	}
	tmp=evalf_double(pe,1,contextptr);
	if (pe==tmp){
	  g=ecm_mult(x,y,ulonglong(tmp._DOUBLE_val),a,n,xm,ym);
	  if (g!=1){
	    if (debug_infolevel)
	      COUT << CLOCK()*1e-6 << " ECM success p=" << p << ", p^e=" << pe << '\n';
	    return g;
	  }
	  swapgen(x,xm); swapgen(y,ym); // x=xm;y=ym;
	}
      }
      B=_floor(1.001*B,contextptr)+1; // to be adjusted
    } // end maxiter loop
    return undef;
#endif
  }
  static const char _ecm_factor_s []="ecm_factor";
  static define_unary_function_eval (__ecm_factor,&_ecm_factor,_ecm_factor_s);
  define_unary_function_ptr5( at_ecm_factor ,alias_at_ecm_factor,&__ecm_factor,0,true);

  // Pollard-rho algorithm
  const int POLLARD_GCD=64;
#ifdef GIAC_MPQS 
#if defined(RTOS_THREADX) // !defined(BESTA_OS)
  const int POLLARD_MAXITER=3000;
#else
  const int POLLARD_MAXITER=15000;
#endif
#else
  const int POLLARD_MAXITER=15000;
#endif  

  static gen pollard(gen n, gen k,GIAC_CONTEXT){
    k.uncoerce();
    n.uncoerce();
    int maxiter=POLLARD_MAXITER;
    double nd=evalf_double(n,1,contextptr)._DOUBLE_val;
#if defined(RTOS_THREADX) || defined(BESTA_OS) || defined NSPIRE
    int nd1=int(2000*(std::log10(nd)-34));
#else
    int nd1=int(1500*std::pow(16.,(std::log10(nd)-40)/10));
#endif
    if (nd1>maxiter)
      maxiter=nd1;
    int m,m1,a,a1,j;
    m1=m=2;
    a1=a=1;
    int c=0;
    mpz_t g,x,x1,x2,x2k,y,y1,p,q,tmpq,alloc1,alloc2,alloc3,alloc4,alloc5;
    mpz_init_set_si(g,1); // ? mp_init_size to specify size
    mpz_init_set_si(x,2);
    mpz_init_set_si(x1,2);
    mpz_init_set_si(y,2);
    mpz_init(y1);
    mpz_init(x2);
    mpz_init(x2k);
    mpz_init_set_si(p,1);
    mpz_init(q);
    mpz_init(tmpq);
    mpz_init(alloc1);
    mpz_init(alloc2);
    mpz_init(alloc3);
    mpz_init(alloc4);
    mpz_init(alloc5);
    while (!ctrl_c && !interrupted && mpz_cmp_si(g,1)==0) {
#ifdef TIMEOUT
      control_c();
#endif
      a=2*a+1;//a=2^(e+1)-1=2*l(m)-1 
      while (!ctrl_c && !interrupted && mpz_cmp_si(g,1)==0 && a>m) { // ok
#ifdef TIMEOUT
	control_c();
#endif
	// x=f(x,k,n,q);
#ifdef USE_GMP_REPLACEMENTS
	mp_sqr(&x,&x2);
	mpz_add(x2k,x2,*k._ZINTptr);
	if (mpz_cmp(x2k,*n._ZINTptr)>0){
	  mp_grow(&alloc1,x2k.used+2);
	  mpz_set_ui(alloc1,0);
	  alloc1.used = x2k.used +2 ;
	  mpz_set(alloc2,x2k);
	  mpz_set(alloc3,*n._ZINTptr);
	  // mpz_set_si(alloc4,0);
	  // mpz_set_si(alloc5,0);
	  alloc_mp_div(&x2k,n._ZINTptr,&tmpq,&x,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
	}
	else
	  mpz_set(x,x2k);
#else 
	mpz_mul(x2,x,x);
	mpz_add(x2k,x2,*k._ZINTptr);
	mpz_tdiv_r(x,x2k,*n._ZINTptr);
#endif
	m += 1;
	if (debug_infolevel && ((m % 
#if defined(RTOS_THREADX) || defined(BESTA_OS) || defined NSPIRE
				 (1<<10)
#else
				 (1<<18)
#endif
				 )==0))
	  *logptr(contextptr) << CLOCK() << gettext(" Pollard-rho try ") << m << '\n';
	if (m > maxiter ){
	  if (debug_infolevel)	  
	    *logptr(contextptr) << CLOCK() << gettext(" Pollard-rho failure, ntries ") << m << '\n';
	  mpz_clear(alloc5);
	  mpz_clear(alloc4);
	  mpz_clear(alloc3);
	  mpz_clear(alloc2);
	  mpz_clear(alloc1);
	  mpz_clear(tmpq);
	  mpz_clear(x);
	  mpz_clear(x1);
	  mpz_clear(x2);
	  mpz_clear(x2k);
	  mpz_clear(y);
	  mpz_clear(y1);
	  mpz_clear(p);
	  mpz_clear(q);
	  return -1;
	}
	// p=irem(p*(x1-x),n,q);
	mpz_sub(q,x1,x);
	mpz_mul(x2,p,q);
#if 0 // def USE_GMP_REPLACEMENTS
	if (mpz_cmp(x2,*n._ZINTptr)>0){
	  mp_grow(&alloc1,x2.used+2);
	  mpz_set_ui(alloc1,0);
	  alloc1.used = x2.used +2 ;
	  mpz_set(alloc2,x2);
	  mpz_set(alloc3,*n._ZINTptr);
	  // mpz_set_si(alloc4,0);
	  // mpz_set_si(alloc5,0);
	  alloc_mp_div(&x2,n._ZINTptr,&tmpq,&p,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
	}
	else 
	  mpz_set(p,x2);
#else
	mpz_tdiv_r(p,x2,*n._ZINTptr);
#endif
	c += 1;
	if (c==POLLARD_GCD) {
	  // g=gcd(abs(p,context0),n); 
	  mpz_abs(q,p);
	  my_mpz_gcd(g,q,*n._ZINTptr);
	  if (mpz_cmp_si(g,1)==0) {
	    mpz_set(y,x); // y=x;
	    mpz_set(y1,x1); // y1=x1;
	    mpz_set_si(p,1); // p=1;
	    a1=a;
	    m1=m;
	    c=0;
	  }
	}
      }//m=a=2^e-1=l(m)
      if (mpz_cmp_si(g,1)==0) {
	mpz_set(x1,x); // x1=x;//x1=x_m=x_l(m)-1
	j=3*(a+1)/2; // j=3*iquo(a+1,2);
	for (long i=m+1;i<=j;i++){
	  // x=f(x,k,n,q);
	  mpz_mul(x2,x,x);
	  mpz_add(x2k,x2,*k._ZINTptr);
#if 0 // def USE_GMP_REPLACEMENTS
	  if (mpz_cmp(x2k,*n._ZINTptr)>0){
	    mp_grow(&alloc1,x2k.used+2);
	    mpz_set_ui(alloc1,0);
	    alloc1.used = x2k.used +2 ;
	    mpz_set(alloc2,x2k);
	    mpz_set(alloc3,*n._ZINTptr);
	    // mpz_set_si(alloc4,0);
	    // mpz_set_si(alloc5,0);
	    alloc_mp_div(&x2k,n._ZINTptr,&tmpq,&x,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
	  }
	  else 
	    mpz_set(x,x2);
#else
	  mpz_tdiv_r(x,x2k,*n._ZINTptr);
#endif
	}
	m=j;
      }
    }
    //g<>1 ds le paquet de POLLARD_GCD
    if (debug_infolevel>5)
      CERR << CLOCK() << " Pollard-rho nloops " << m << '\n';
    mpz_set(x,y); // x=y;
    mpz_set(x1,y1); // x1=y1;
    mpz_set_si(g,1); // g=1;
    a=(a1-1)/2; // a=iquo(a1-1,2);
    m=m1;
    while (!ctrl_c && !interrupted && mpz_cmp_si(g,1)==0) {
#ifdef TIMEOUT
      control_c();
#endif
      a=2*a+1;
      while (!ctrl_c && !interrupted && mpz_cmp_si(g,1)==0 && a>m) { // ok
#ifdef TIMEOUT
	control_c();
#endif
	// x=f(x,k,n,q);
	mpz_mul(x2,x,x);
	mpz_add(x2k,x2,*k._ZINTptr);
#if 0 // def USE_GMP_REPLACEMENTS
	if (mpz_cmp(x2k,*n._ZINTptr)>0){
	  mp_grow(&alloc1,x2k.used+2);
	  mpz_set_ui(alloc1,0);
	  alloc1.used = x2k.used +2 ;
	  mpz_set(alloc2,x2k);
	  mpz_set(alloc3,*n._ZINTptr);
	  alloc_mp_div(&x2k,n._ZINTptr,&tmpq,&x,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
	}
	else
	  mpz_set(x,x2k);	  
#else
	mpz_tdiv_r(x,x2k,*n._ZINTptr);
#endif
	m += 1;
	if (m > maxiter ){
	  mpz_clear(alloc5);
	  mpz_clear(alloc4);
	  mpz_clear(alloc3);
	  mpz_clear(alloc2);
	  mpz_clear(alloc1);
	  mpz_clear(tmpq);
	  mpz_clear(x);
	  mpz_clear(x1);
	  mpz_clear(x2);
	  mpz_clear(x2k);
	  mpz_clear(y);
	  mpz_clear(y1);
	  mpz_clear(p);
	  mpz_clear(q);
	  return -1;
	}
	// p=irem(x1-x,n,q);
	mpz_sub(q,x1,x);
#if 0 // def USE_GMP_REPLACEMENTS
	if (mpz_cmp(q,*n._ZINTptr)>0){
	  mp_grow(&alloc1,q.used+2);
	  mpz_set_ui(alloc1,0);
	  alloc1.used = q.used +2 ;
	  mpz_set(alloc2,q);
	  mpz_set(alloc3,*n._ZINTptr);
	  // mpz_set_si(alloc4,0);
	  // mpz_set_si(alloc5,0);
	  alloc_mp_div(&q,n._ZINTptr,&tmpq,&p,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
	}
	else 
	  mpz_set(p,q);
#else
	mpz_tdiv_r(p,q,*n._ZINTptr);
#endif
	// g=gcd(abs(p,context0),n);  // ok
	mpz_abs(q,p);
	my_mpz_gcd(g,q,*n._ZINTptr);
      }
      if (mpz_cmp_si(g,1)==0) {
	mpz_set(x1,x); // x1=x;
	j=3*(a+1)/2; // j=3*iquo(a+1,2);
	for (long i=m+1;j>=i;i++){
	  // x=f(x,k,n,q);
	  mpz_mul(x2,x,x);
	  mpz_add(x2k,x2,*k._ZINTptr);
	  mpz_tdiv_qr(tmpq,x,x2k,*n._ZINTptr);
	}
	m=j;
      }
    }
    mpz_clear(alloc5);
    mpz_clear(alloc4);
    mpz_clear(alloc3);
    mpz_clear(alloc2);
    mpz_clear(alloc1);
    mpz_clear(tmpq);
    mpz_clear(x);
    mpz_clear(x1);
    mpz_clear(x2);
    mpz_clear(x2k);
    mpz_clear(y);
    mpz_clear(y1);
    mpz_clear(p);
    mpz_clear(q);
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted){
      mpz_clear(g);
      return 0;
    }
    if (mpz_cmp(g,*n._ZINTptr)==0) {
      if (k==1) {
	mpz_clear(g);
	return(pollard(n,-1,contextptr)); 
      }
      else {
	if (k*k==1){
	  mpz_clear(g);
	  return(pollard(n,3,contextptr));
	}
	else {
	  if (is_greater(k,50,contextptr)){
#if 1
	    return -1;
#else
	    ref_mpz_t * ptr=new ref_mpz_t;
	    mpz_init_set(ptr->z,g);
	    mpz_clear(g);
	    return ptr;
#endif
	  }
	  else {
	    mpz_clear(g);
	    return(pollard(n,k+2,contextptr));
	  }
	} 
      }
    } 
    ref_mpz_t * ptr=new ref_mpz_t;
    mpz_init_set(ptr->z,g);
    mpz_clear(g);
    return ptr;
  }

  // const short int giac_primes[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997};

  bool eratosthene(double n,vector<bool> * & v){
    static vector<bool> *ptr=0;
    if (!ptr)
      ptr=new vector<bool>;
    vector<bool> &erato=*ptr;
    v=ptr;
    if (n+1>erato.size()){
      unsigned N=int(n);
      ++N;
#if defined BESTA_OS 
      if (N>2e6)
	return false;
#else
      if (N>2e9)
	return false;
#endif
      N = (N*11)/10;
      erato=vector<bool>(N+1,true); 
      // insure that we won't recompute all again from start for ithprime(i+1)
      for (unsigned p=2;;++p){
	while (!erato[p]) // find next prime
	  ++p;
	if (p*p>N) // finished
	  return true;
	for (unsigned i=2*p;i<=N;i+=p) 
	  erato[i]=false; // remove p multiples
      }
    }
    return true;
  }

  bool eratosthene2(double n,vector<bool> * & v){
    static vector<bool> *ptr=0;
    if (!ptr)
      ptr=new vector<bool>;
    vector<bool> &erato=*ptr;
    v=ptr;
    if (n/2>=erato.size()){
      unsigned N=int(n);
      ++N;
#if defined BESTA_OS 
      if (N>4e6)
	return false;
#else
      if (N>2e9)
	return false;
#endif
      // 11/20 insures that we won't recompute all again from start for ithprime(i+1)
      N = (N*11)/20; // keep only odd numbers in sieve
      erato=vector<bool>(N+1,true); //erato[i] stands for 2*i+1 <-> n corresponds to erato[n/2]
      for (unsigned p=3;;p+=2){
	while (!erato[p/2]) // find next prime (first one is p==3)
	  p+=2;
	if (p*p>2*N+1) // finished
	  return true;
	// p is prime, set p*p, (p+2)*p, etc. to be non prime
	for (unsigned i=(p*p)/2;i<=N;i+=p) 
	  erato[i]=false; // remove p multiples
      }
    }
    return true;
  }

  // ithprime(n) is approx invli(n)+invli(sqrt(n))/4 where invli is reciproc.
  // of Li(x)=Ei(ln(x))
  // For fast code, cf. https://github.com/kimwalisch/primecount
  static const char _ithprime_s []="ithprime";
  static symbolic symb_ithprime(const gen & args){
    return symbolic(at_ithprime,args);
  }
  static gen ithprime(const gen & g_,GIAC_CONTEXT){
    gen g(g_);
    if (!is_integral(g))
      return gentypeerr(contextptr);
    if (g.type!=_INT_)
      return gensizeerr(contextptr); // symb_ithprime(g);
    int i=g.val;
    if (i<0)
      return gensizeerr(contextptr);
    if (i==0)
      return 1;
    if (i<=int(sizeof(giac_primes)/sizeof(short int)))
      return giac_primes[i-1];
    vector<bool> * vptr=0;
#if 1
    if (!eratosthene2(i*std::log(double(i))*1.1,vptr))
      return gensizeerr(contextptr);
    unsigned count=2;
    unsigned s=unsigned(vptr->size());
    for (unsigned k=2;k<s;++k){
      if ((*vptr)[k]){
	++count;
	if (i==count)
	  return int(2*k+1);
      }
    }
    return undef;
#else
    if (!eratosthene(i*std::log(double(i))*1.1,vptr))
      return gensizeerr(contextptr);
    unsigned count=2;
    unsigned s=vptr->size();
    for (unsigned k=4;k<s;++k){
      if ((*vptr)[k]){
	++count;
	if (i==count)
	  return int(k);
      }
    }
    return undef;
#endif
  }
  gen _ithprime(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT)
      return apply(args,_ithprime,contextptr);
    return ithprime(args,contextptr);
  }
  static define_unary_function_eval (__ithprime,&_ithprime,_ithprime_s);
  define_unary_function_ptr5( at_ithprime ,alias_at_ithprime,&__ithprime,0,true);

  static const char _nprimes_s []="nprimes";
  static gen nprimes(const gen & g_,GIAC_CONTEXT){
    gen g(g_);
    if (!is_integral(g))
      return gentypeerr(contextptr);
    if (g.type!=_INT_)
      return gensizeerr(contextptr); // symb_ithprime(g);
    int i=g.val;
    if (i<0)
      return gensizeerr(contextptr);
    if (i<2)
      return 0;
    vector<bool> * vptr=0;
    if (!eratosthene2(i+2,vptr))
      return gensizeerr(contextptr);
    unsigned count=1; // 2 is prime, then count odd primes
    i=(i-1)/2;
    for (int k=1;k<=i;++k){
      if ((*vptr)[k])
	++count;
    }
    return int(count);
  }
  gen _nprimes(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT)
      return apply(args,_nprimes,contextptr);
    return nprimes(args,contextptr);
  }
  static define_unary_function_eval (__nprimes,&_nprimes,_nprimes_s);
  define_unary_function_ptr5( at_nprimes ,alias_at_nprimes,&__nprimes,0,true);

  bool is_divisible_by(const gen & n,unsigned long a){
    if (n.type==_ZINT){
#if defined USE_GMP_REPLACEMENTS 
      mp_digit c;
      mp_mod_d(n._ZINTptr, a, &c);
      return c==0;
#else
      return mpz_divisible_ui_p(*n._ZINTptr,a);
#endif
    }
    return n.val%a==0;
  }

  // find trivial factors of n, 
  // if add_last is true the remainder is put in the vecteur,
  // otherwise n contains the remainder
  vecteur pfacprem(gen & n,bool add_last,GIAC_CONTEXT){
    gen a;
    gen q;
    int p,i,prime;
    vecteur v(2);
    vecteur u;
    if (is_zero(n))
      return u;
    if (n.type==_ZINT){
      ref_mpz_t * cur = new ref_mpz_t;
      mpz_t div,q,r,alloc1,alloc2,alloc3,alloc4,alloc5;
      mpz_set(cur->z,*n._ZINTptr);
      mpz_init_set(q,*n._ZINTptr);
      mpz_init(r);
      mpz_init(div);
      mpz_init(alloc1);
      mpz_init(alloc2);
      mpz_init(alloc3);
      mpz_init(alloc4);
      mpz_init(alloc5);
      for (i=0;i<int(sizeof(giac_primes)/sizeof(short int));++i){
	if (mpz_cmp_si(cur->z,1)==0) 
	  break;
	prime=giac_primes[i];
	mpz_set_ui(div,prime);
#ifdef USE_GMP_REPLACEMENTS
	for (p=0;;p++){
	  mp_grow(&alloc1,cur->z.used+2);
	  mpz_set_ui(alloc1,0);
	  alloc1.used = cur->z.used +2 ;
	  mpz_set(alloc2,cur->z);
	  mpz_set(alloc3,div);
	  alloc_mp_div(&cur->z,&div,&q,&r,&alloc1,&alloc2,&alloc3,&alloc4,&alloc5);
	  // mpz_tdiv_qr(q,r,cur->z,div);
	  if (mpz_cmp_si(r,0))
	    break;
	  mp_exch(&cur->z,&q);
	}
	// *logptr(contextptr) << "Factor " << prime << " " << p << '\n';
	if (p){
	  u.push_back(prime);
	  u.push_back(p);
	}
#else
	if (mpz_divisible_ui_p(cur->z,prime)){
	  mpz_set_ui(div,prime);
	  for (p=0;;p++){
	    mpz_tdiv_qr(q,r,cur->z,div);
	    if (mpz_cmp_si(r,0))
	      break;
	    mpz_swap(cur->z,q);
	  }
	  // *logptr(contextptr) << "Factor " << prime << " " << p << '\n';
	  u.push_back(prime);
	  u.push_back(p);
	}
#endif
      } // end for on smal primes
      mpz_clear(alloc5);
      mpz_clear(alloc4);
      mpz_clear(alloc3);
      mpz_clear(alloc2);
      mpz_clear(alloc1);
      mpz_clear(div); mpz_clear(r); mpz_clear(q);
      n=cur;
    }
    else {
      for (i=0;i<int(sizeof(giac_primes)/sizeof(short int));++i){
	if (n==1) 
	  break;
	a.val=giac_primes[i];
	p=0;
	while (is_divisible_by(n,a.val)){ // while (irem(n,a,q)==0){
	  n=iquo(n,a); 
	  p=p+1;
	}
	if (p!=0){
	  // *logptr(contextptr) << "Factor " << a << " " << p << '\n';
	  u.push_back(a);
	  u.push_back(p);
	}
      }
    }
    if (add_last && i==1229 && !is_one(n)){
      // hack: check if n is a perfect square
      double nf=evalf_double(n,1,contextptr)._DOUBLE_val;
      nf=std::sqrt(nf);
      gen n2=_round(nf,contextptr);
      if (n2*n2==n){
	u.push_back(n2);
	u.push_back(2);	
      }
      else {
	u.push_back(n);
	u.push_back(1);
      }
      n=1;
    }
    //v[0]=n;
    //v[1]=u;
    
    return(u);
  }

#ifdef USE_GMP_REPLACEMENTS
  static gen inpollardsieve(const gen &a,gen k,bool & do_pollard,GIAC_CONTEXT){
    gen b=do_pollard?pollard(a,k,contextptr):-1;
#ifdef TIMEOUT
    control_c();
#endif
#ifdef GIAC_MPQS
    if (b==-1 && !ctrl_c && !interrupted){ 
      do_pollard=false;
      if (msieve(a,b,contextptr)) return b; else return -1; }
#endif
    return b;
  }
  static gen pollardsieve(const gen &a,gen k,bool & do_pollard,GIAC_CONTEXT){
#if defined( GIAC_HAS_STO_38) || defined(EMCC) || defined(EMCC2) || defined NSPIRE
    int debug_infolevel_=debug_infolevel;
#if defined RTOS_THREADX || defined NSPIRE
    debug_infolevel=2;
    if (do_pollard)
      *logptr(contextptr) << gettext("Pollard-rho on ") << a << '\n'; 
#else
    debug_infolevel=0;
#endif
#endif
    gen res=inpollardsieve(a,k,do_pollard,contextptr);
#if defined( GIAC_HAS_STO_38) || defined(EMCC) || defined (EMCC2) || defined NSPIRE
    debug_infolevel=debug_infolevel_;
#ifdef GIAC_HAS_STO_38
    Calc->Terminal.MakeUnvisible();
#endif
#endif
    return res;
  }
#else // USE_GMP_REPLACEMENTS
  static gen pollardsieve(const gen &a,gen k,bool & do_pollard,GIAC_CONTEXT){
    gen b=do_pollard?pollard(a,k,contextptr):-1;
#ifdef TIMEOUT
    control_c();
#endif
#ifdef HAVE_LIBECM
    if (is_greater(a,1e60,context0) && b==-1  && !ctrl_c && !interrupted && _isprime(a,contextptr)==0){ 
      int res;
      gen F(1LL<<33);
      for (int i=0;i<200;++i){ // searching factors of size about 20 digits
	res=ecm_factor(*F._ZINTptr, *a._ZINTptr, 11e3, 0);
	if (res!=0) break;
      }
      if (res!=0) 
	b=F;
    }
#endif
#ifdef GIAC_MPQS
    if (b==-1 && !ctrl_c && !interrupted){ 
      do_pollard=false;
      if (msieve(a,b,contextptr)) return b; else return -1; }
#else
    if (b==-1)
      *logptr(contextptr) << "Integer too large for factorization algorithm\n";
#endif
    if (b==-1)
      b=a;
    return b;
  }
#endif // USE_GMP_REPLACEMENTS

  static gen ifactor2(const gen & n,vecteur & v,bool & do_pollard,GIAC_CONTEXT){
    if (is_greater(giac_last_prime*giac_last_prime,n,contextptr) || is_probab_prime_p(n) ){
      v.push_back(n);
      return 1;
    }
    // Check for power of integer: arg must be > 1e4, n*ln(arg)=d => n<d/ln(1e4)
    double d=evalf_double(n,1,contextptr)._DOUBLE_val;
    int maxpow=int(std::ceil(std::log(d)/std::log(1e4)));
    for (int i=2;i<=maxpow;++i){
      if ( (i>2 && i%2==0) ||
	   (i>3 && i%3==0) ||
	   (i>5 && i%5==0) ||
	   (i>7 && i%7==0) )
	continue;
      gen u;
      if (i==2)
	u=isqrt(n);
      else {
	double x=std::pow(d,1./i);
	u=longlong(x);
      }
      if (pow(u,i,contextptr)==n){
	vecteur w;
	do_pollard=true;
	ifactor2(u,w,do_pollard,contextptr);
	for (int j=0;j<i;j++)
	  v=mergevecteur(v,w);
	return v;
      }
    }
    gen a=pollardsieve(n,1,do_pollard,contextptr);
    if (a==-1)
      return a;
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted)
      return gensizeerr("Interrupted");
    gen ba=n/a;
    if (a!=n)
      a=ifactor2(a,v,do_pollard,contextptr);
    else {
      a=1;
      v.push_back(n);
    }
    if (is_strictly_greater(ba,1,contextptr))
      a=ifactor2(ba,v,do_pollard,contextptr);
    return a;
  }

  static vecteur facprem(gen & n,GIAC_CONTEXT){
    vecteur v;    
    if (n==1) { return v; }
    if ( (n.type==_INT_ && n.val<giac_last_prime*giac_last_prime) || is_probab_prime_p(n)) {
      v.push_back(n);
      n=1;
      return v;
    }
    if (debug_infolevel>5)
      CERR << "Pollard begin " << CLOCK() << '\n';
    bool do_pollard=true;
    gen a=ifactor2(n,v,do_pollard,contextptr);
    if (a==-1)
      return makevecteur(gensizeerr(gettext("Quadratic sieve failure, perhaps number too large")));
    if (is_zero(a))
      return makevecteur(gensizeerr(gettext("Stopped by user interruption")));
    n=1;
    return v;
  }

  void mergeifactors(const vecteur & f,const vecteur &g,vecteur & h){
    h=f;
    for (unsigned i=0;i<g.size();i+=2){
      unsigned j=0;
      for (;j<f.size();j+=2){
	if (f[j]==g[i])
	  break;
      }
      if (j<f.size())
	h[j+1] += g[i+1];
      else {
	h.push_back(g[i]);
	h.push_back(g[i+1]);
      }
    }
  }

  static vecteur giac_ifactors(const gen & n0,GIAC_CONTEXT){
    if (!is_integer(n0) || is_zero(n0))
      return vecteur(1,gensizeerr(gettext("ifactors")));
    if (is_one(n0))
      return vecteur(0);
    gen n(n0);
    vecteur f;
    vecteur g;
    vecteur u;
    // First find if |n-k^d|<=1 for d = 2, 3, 5 or 7
    double nd=evalf_double(n,1,contextptr)._DOUBLE_val;
    double nd2=std::floor(std::sqrt(nd)+.5);
    if (absdouble(1-nd2*nd2/nd)<1e-10){
      gen n2=isqrt(n+1);
      if (n==n2*n2){
	f=ifactors(n2,contextptr);
	iterateur it=f.begin(),itend=f.end();
	for (;it!=itend;++it){
	  ++it;
	  *it = 2 * *it;
	}
	return f;
      }
      if (n==n2*n2-1){
	f=ifactors(n2-1,contextptr);
	g=ifactors(n2+1,contextptr);
	mergeifactors(f,g,u);
	return u;
      }
    }
    for (int k=3;;){
      nd2=std::floor(std::pow(nd,1./k)+.5);
      if (absdouble(1-std::pow(nd2,k)/nd)<1e-10){
	gen n2=_floor(nd2,contextptr),nf=n2*n2;
	for (int j=2;j<k;j++)
	  nf=nf*n2;
	if (n==nf){
	  f=ifactors(n2,contextptr);
	  iterateur it=f.begin(),itend=f.end();
	  for (;it!=itend;++it){
	    ++it;
	    *it = k * *it;
	  }
	  return f;
	}
	if (n==nf-1){ // n2^k-1
	  f=ifactors(n2-1,contextptr);
	  g=ifactors(n/(n2-1),contextptr);
	  mergeifactors(f,g,u);
	  return u;
	}
	if (n==nf+1){ // n2^k+1
	  f=ifactors(n2+1,contextptr);
	  g=ifactors(n/(n2+1),contextptr);
	  mergeifactors(f,g,u);
	  return u;
	}
      }
      if (k==11) break;
      if (k==7) break; // k=11;
      if (k==5) k=7;
      if (k==3) k=5;
    }
    //f=pfacprem(n,false,contextptr);
    //cout<<n<<" "<<f<<'\n';
    while (n!=1) {
      g=facprem(n,contextptr);
      if (is_undef(g))
	return g;
      islesscomplexthanf_sort(g.begin(),g.end());
      gen last=0; int p=0;
      for (unsigned i=0;i<g.size();++i){
	if (g[i]==last)
	  ++p;
	else {
	  if (last!=0){
	    u.push_back(last);
	    u.push_back(p);
	  }
	  last=g[i];
	  p=1;
	}
      }
      u.push_back(last);
      u.push_back(p);
    }   
    g=mergevecteur(f,u);
    return g;
  }

  static vecteur ifactors1(const gen & n0,GIAC_CONTEXT){
    if (is_greater(1e71,n0,contextptr))
      return giac_ifactors(n0,contextptr);
    if (n0.type==_VECT && !n0._VECTptr->empty())
      return giac_ifactors(n0._VECTptr->front(),contextptr);
    if (!is_integer(n0) || is_zero(n0))
      return vecteur(1,gensizeerr(gettext("ifactors")));
    if (is_one(n0))
      return vecteur(0);
    if (_isprime(n0,contextptr)!=0)
      return makevecteur(n0,1);
#if 1 // set to 0 to disable ecm and pari, using giac sieve only
    bool ifactor_pari=true;
#ifdef HAVE_LIBECM
    int res;
    gen F(1LL<<33);
    double B1=1e6;
    int nbits=sizeinbase2(n0);
    int maxiter=ECM_MAXITER;
    if (nbits<242)
      maxiter = 32;
    else if (nbits<244)
      maxiter = 64;
    else if (nbits<246)
      maxiter = 128;
    else if (nbits<248)
      maxiter = 256;
    else if (nbits<250)
      maxiter = 512;
    for (int i=0;i<maxiter;++i){
      res=ecm_factor(*F._ZINTptr, *n0._ZINTptr, B1, 0);
      if (res!=0) break;
    }
    if (res!=0){
      vecteur tmp=ifactors1(n0/F,contextptr);
      tmp=mergevecteur(tmp,ifactors1(F,contextptr));
      return tmp;
    }
    ifactor_pari=nbits>=256;
#endif
#ifdef HAVE_LIBPARI
    if (ifactor_pari){
#ifdef __APPLE__
      return vecteur(1,gensizeerr(gettext("(Mac OS) Large number, you can try pari(); pari_factor(")+n0.print(contextptr)+")"));
#endif
      gen g(pari_ifactor(n0),contextptr); 
      if (g.type==_VECT){
	matrice m(mtran(*g._VECTptr));
	vecteur res;
	const_iterateur it=m.begin(),itend=m.end();
	for (;it!=itend;++it){
	  if (it->type!=_VECT) return vecteur(1,gensizeerr(gettext("ifactor.cc/ifactors")));
	  res.push_back(it->_VECTptr->front());
	  res.push_back(it->_VECTptr->back());
	}
	return res;
      }
    }
#endif // LIBPARI
#endif
    return giac_ifactors(n0,contextptr);
  }

  vecteur ifactors(const gen & n0,GIAC_CONTEXT){
    gen n(n0);
    vecteur f=pfacprem(n,false,contextptr);
    if (is_undef(f))
      return f;
    vecteur g=ifactors1(n,contextptr);
    if (is_undef(g))
      return g;
    return mergevecteur(f,g);
  }

  vecteur ifactors(const gen & r,const gen & i,const gen & ri,GIAC_CONTEXT){
    gen norm=r*r+i*i;
    gen reste(ri);
    const vecteur & facto = ifactors(norm,contextptr);
    if (is_undef(facto))
      return facto;
    int l=int(facto.size())/2;
    vecteur res;
    for (int i=0;i<l;++i){
      gen prime=facto[2*i];
      int mult=facto[2*i+1].val,multp=0;
      int n=smod(prime,4).val;
      if (n==2){
	res.push_back(1+cst_i);
	res.push_back(mult);
	reste=reste/pow(1+cst_i,mult,contextptr);
	continue;
      }
      if (n==-1){
	res.push_back(prime);
	res.push_back(mult/2);
	reste=reste/pow(prime,mult/2,contextptr);
	continue;
      }
      prime=pa2b2(prime,contextptr);
      prime=gen(prime[0],prime[1]);
      for (;mult>0;--mult,++multp){
	if (!is_zero(reste % prime))
	  break;
	reste=reste/prime;
      }
      if (multp){
	res.push_back(prime);
	res.push_back(multp);
      }
      if (mult){
	prime=conj(prime,contextptr);
	res.push_back(prime);
	res.push_back(mult);
	reste=reste/pow(prime,mult,contextptr);
      }
    }
    if (!is_one(reste)){
      res.insert(res.begin(),1);
      res.insert(res.begin(),reste);
    }
    return res;
  }

  gen ifactors(const gen & args,int maplemode,GIAC_CONTEXT){
    if ( (args.type==_INT_) || (args.type==_ZINT)){
      if (is_zero(args)){
	if (maplemode==1)
	  return makevecteur(args,vecteur(0));
	else
	  return makevecteur(args);
      }
      vecteur v(ifactors(abs(args,contextptr),contextptr)); // ok
      if (!v.empty() && is_undef(v.front()))
	return v.front();
      if (maplemode!=1){
	if (is_positive(args,context0))
	  return v;
	return mergevecteur(makevecteur(minus_one,plus_one),v);
      }
      vecteur res;
      const_iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;it+=2){
	res.push_back(makevecteur(*it,*(it+1)));
      }
      if (is_positive(args,context0))
	return makevecteur(plus_one,res);
      else
	return makevecteur(minus_one,res);	
    }
    if (args.type==_CPLX && is_integer(*args._CPLXptr) && is_integer(*(args._CPLXptr+1)))
      return ifactors(*args._CPLXptr,*(args._CPLXptr+1),args,contextptr);
    return gentypeerr(gettext("ifactors"));
  }

  gen _ifactors(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT && args.subtype==_SEQ__VECT && args._VECTptr->size()==2 ){
      gen g=args._VECTptr->front();
      gen b=args._VECTptr->back();
      if (b==at_matrix){
	g=_ifactors(g,contextptr);
	if (g.type!=_VECT || g._VECTptr->size()%2)
	  return g;
	return _matrix(makesequence(g._VECTptr->size()/2,2,g),contextptr);
      }
#if !defined EMCC && defined HAVE_LIBPARI
      if (b.type==_SYMB){
	gen res;
	// b is assumed to be a minimal polynomial check if g is a norm 
	if (!pari_intnorm(g,b,lvar(b),res,contextptr))
	  return gensizeerr(gettext("Not implemented. Try to compile with PARI"));
	return res;
      }
#endif
    }
    if (args.type==_VECT)
      return apply(args,_ifactors,contextptr);
    gen g(args);
    if (!is_integral(g))
      return gensizeerr(contextptr);
    if (calc_mode(contextptr)==1){ // ggb returns factors repeted instead of multiplicites
      vecteur res;
      gen in=ifactors(g,0,contextptr);
      if (in.type==_VECT){
	for (unsigned i=0;i<in._VECTptr->size();i+=2){
	  gen f=in[i],m=in[i+1];
	  if (m.type==_INT_){
	    for (int j=0;j<m.val;++j)
	      res.push_back(f);
	  }
	}
	return res;
      }
    }
    return ifactors(g,0,contextptr);
  }
  static const char _ifactors_s []="ifactors";
  static define_unary_function_eval (__ifactors,&_ifactors,_ifactors_s);
  define_unary_function_ptr5( at_ifactors ,alias_at_ifactors,&__ifactors,0,true);

  static const char _facteurs_premiers_s []="facteurs_premiers";
  static define_unary_function_eval (__facteurs_premiers,&_ifactors,_facteurs_premiers_s);
  define_unary_function_ptr5( at_facteurs_premiers ,alias_at_facteurs_premiers,&__facteurs_premiers,0,true);

  static const char _maple_ifactors_s []="maple_ifactors";
  gen _maple_ifactors(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT)
      return apply(args,_maple_ifactors,contextptr);
    return ifactors(args,1,contextptr);
  }
  static define_unary_function_eval (__maple_ifactors,&_maple_ifactors,_maple_ifactors_s);
  define_unary_function_ptr5( at_maple_ifactors ,alias_at_maple_ifactors,&__maple_ifactors,0,true);

  static vecteur in_factors(const gen & gf,GIAC_CONTEXT){
    if (gf.type!=_SYMB)
      return makevecteur(gf,plus_one);
    unary_function_ptr & u=gf._SYMBptr->sommet;
    if (u==at_inv){
      vecteur v=in_factors(gf._SYMBptr->feuille,contextptr);
      iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;it+=2)
	*(it+1)=-*(it+1);
      return v;
    }
    if (u==at_neg){
      vecteur v=in_factors(gf._SYMBptr->feuille,contextptr);
      v.push_back(minus_one);
      v.push_back(plus_one);
      return v;
    }
    if ( (u==at_pow) && (gf._SYMBptr->feuille._VECTptr->back().type==_INT_) ){
      vecteur v=in_factors(gf._SYMBptr->feuille._VECTptr->front(),contextptr);
      gen k=gf._SYMBptr->feuille._VECTptr->back();
      iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;it+=2)
	*(it+1)=k* *(it+1);
      return v;
    }
    if (u!=at_prod)
      return makevecteur(gf,plus_one);
    vecteur res;
    const_iterateur it=gf._SYMBptr->feuille._VECTptr->begin(),itend=gf._SYMBptr->feuille._VECTptr->end();
    for (;it!=itend;++it){
      res=mergevecteur(res,in_factors(*it,contextptr));
    }
    return res;
  }
  static vecteur in_factors1(const vecteur & res,GIAC_CONTEXT){
    gen coeff(1);
    vecteur v;
    const_iterateur it=res.begin(),itend=res.end();
    for (;it!=itend;it+=2){
      if (lidnt(*it).empty())
	coeff=coeff*(pow(*it,*(it+1),contextptr));
      else
	v.push_back(makevecteur(*it,*(it+1)));
    }
    return makevecteur(coeff,v);
  }
  vecteur factors(const gen & g,const gen & x,GIAC_CONTEXT){
    gen gf=factor(g,x,false,contextptr);
    vecteur res=in_factors(gf,contextptr);
    if (xcas_mode(contextptr)!=1)
      return res;
    return in_factors1(res,contextptr);
  }
  vecteur sqff_factors(const gen & g,GIAC_CONTEXT){
    gen gf=_sqrfree(g,contextptr);
    return in_factors(gf,contextptr);
  }
  static const char _factors_s []="factors";
  gen _factors(const gen & args,GIAC_CONTEXT){ 
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT && args.subtype==_SEQ__VECT && args._VECTptr->size()>=2 && args._VECTptr->back()==at_matrix){
      gen g;
      if (args._VECTptr->size()==2)
	g=args._VECTptr->front();
      else 
	g=gen(vecteur(args._VECTptr->begin(),args._VECTptr->end()-1),_SEQ__VECT);
      g=_factors(g,contextptr);
      if (g.type!=_VECT || g._VECTptr->size()%2)
	return g;
      return _matrix(makesequence(g._VECTptr->size()/2,2,g),contextptr);
    }
    if (args.type==_VECT && args.subtype==_POLY1__VECT){
      gen x(identificateur("xfactors"));
      gen res=_poly2symb(makesequence(args,x),contextptr);
      res=_factors(res,contextptr);
      if (res.type==_VECT && res._VECTptr->size()==2){
	vecteur v(*res._VECTptr);
	for (size_t i=0;i<v.size();i+=2){
	  v[i]=_symb2poly(makesequence(v[i],x),contextptr);
	}
	return v;
      }
      return res;
    }
    if (args.type==_VECT && args.subtype==_SEQ__VECT && args._VECTptr->size()==2){
      gen j=args._VECTptr->back();
      gen res=_factors(args._VECTptr->front()*j,contextptr);
      if (res.type==_VECT && xcas_mode(contextptr)!=1)
	res=in_factors1(*res._VECTptr,contextptr);
      if (res.type==_VECT && res._VECTptr->size()==2){
	res._VECTptr->front()=recursive_normal(res._VECTptr->front()/j,contextptr);
	if (xcas_mode(contextptr)!=1){
	  if (is_one(res._VECTptr->front()))
	    res=res._VECTptr->back();
	  else {
	    j=res._VECTptr->front();
	    res=res._VECTptr->back();
	    if (res.type==_VECT)
	      res=mergevecteur(makevecteur(j,1),*res._VECTptr);
	  }
	  vecteur v;
	  aplatir(*res._VECTptr,v,contextptr);
	  res=v;
	}
      }
      return res;
    }
    if (args.type==_VECT)
      return apply(args,_factors,contextptr);
    return factors(args,vx_var,contextptr);
  }
  static define_unary_function_eval (__factors,&_factors,_factors_s);
  define_unary_function_ptr5( at_factors ,alias_at_factors,&__factors,0,true);

  static gen ifactors2ifactor(const vecteur & l,bool quote){
    int s;
    s=int(l.size());
    gen r;
    vecteur v(s/2);
    for (int j=0;j<s;j=j+2){
      if (!is_one(l[j+1]))
	v[j/2]=symbolic(at_pow,gen(makevecteur(l[j],l[j+1]),_SEQ__VECT));
      else
	v[j/2]=l[j];
    }
    if (v.size()==1){
#if defined(GIAC_HAS_STO_38) && defined(CAS38_DISABLED)
      return symb_quote(v.front());
#else
      if (quote)
	return symb_quote(v.front());
      return v.front();
#endif
    }
    r=symbolic(at_prod,gen(v,_SEQ__VECT));
#if defined(GIAC_HAS_STO_38) && defined(CAS38_DISABLED)
    r=symb_quote(r);
#endif
    if (quote)
      return symb_quote(r);
    return r;
  }
  gen ifactor(const gen & n,GIAC_CONTEXT){
    vecteur l;
    l=ifactors(n,contextptr);
    if (!l.empty() && is_undef(l.front())) return l.front();
    return ifactors2ifactor(l,calc_mode(contextptr)==1);
  }
  gen _ifactor(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_CPLX && is_integer(*args._CPLXptr) && is_integer(*(args._CPLXptr+1))){
      const vecteur & v=ifactors(*args._CPLXptr,*(args._CPLXptr+1),args,contextptr);
      return ifactors2ifactor(v,calc_mode(contextptr)==1);
    }
    gen n=args;
    if (n.type==_VECT && n._VECTptr->size()==1 && is_integer(n._VECTptr->front()))
      return ifactor(n,contextptr);
    if (n.type==_VECT)
      return apply(n,_ifactor,contextptr);
    if (!is_integral(n))
      return gensizeerr(contextptr);
    if (is_strictly_positive(-n,0))
      return -_ifactor(-n,contextptr);
    if (n.type==_INT_ && n.val<=3)
      return n;
    return ifactor(n,contextptr);
  }
  static const char _ifactor_s []="ifactor";
  static define_unary_function_eval (__ifactor,&_ifactor,_ifactor_s);
  define_unary_function_ptr5( at_ifactor ,alias_at_ifactor,&__ifactor,0,true);

  static const char _factoriser_entier_s []="factoriser_entier";
  static define_unary_function_eval (__factoriser_entier,&_ifactor,_factoriser_entier_s);
  define_unary_function_ptr5( at_factoriser_entier ,alias_at_factoriser_entier,&__factoriser_entier,0,true);

  static vecteur divis(const vecteur & l3,GIAC_CONTEXT){
    vecteur l1(1);
    gen d,e;
    int s=int(l3.size());
    gen taille=1;
    for (int k=0;k<s;k+=2){
      taille=taille*(l3[k+1]+1);
    }
    if (taille.type!=_INT_ || taille.val>LIST_SIZE_LIMIT)
      return vecteur(1,gendimerr(contextptr));
    l1.reserve(taille.val);
    l1[0]=1;//l3.push_back(..);
    for (int k=0;k<s;k=k+2) {
      vecteur l2;
      l2.reserve(taille.val);
      int s1;
      s1=int(l1.size());
      vecteur l4(s1);
      d=l3[k];
      e=l3[k+1];
      int ei;
      if (e.type==_INT_){
	ei=e.val;
      }
      else
	return vecteur(1,gensizeerr(gettext("Integer too large")));
      for (int j=1;j<=ei;j++){
	gen dj=pow(d,j);
	for (int l=0;l<s1;l++){ 
	  l4[l]=l1[l]*dj;
	}
	// l2=mergevecteur(l2,l4);
	iterateur it=l4.begin(),itend=l4.end();
	for (;it!=itend;++it)
	  l2.push_back(*it);
      }
      // l1=mergevecteur(l1,l2);
      iterateur it=l2.begin(),itend=l2.end();
      for (;it!=itend;++it)
	l1.push_back(*it);
    }
    return(l1); 
  }
  gen idivis(const gen & n,GIAC_CONTEXT){
    vecteur l3(ifactors(n,contextptr));
    if (!l3.empty() && is_undef(l3.front())) return l3.front();
    return divis(l3,contextptr);
  }
  gen _idivis(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT)
      return apply(args,_idivis,contextptr);
    gen n=args;
    if (is_zero(n) || (!is_integral(n) && !is_integer(n)) || n.type==_CPLX) 
      return gentypeerr(contextptr);
    return _sort(idivis(abs(n,contextptr),contextptr),contextptr);
  }
  static const char _idivis_s []="idivis";
  static define_unary_function_eval (__idivis,&_idivis,_idivis_s);
  define_unary_function_ptr5( at_idivis ,alias_at_idivis,&__idivis,0,true);

  gen _divis(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT)
      return apply(args,_divis,contextptr);
    return divis(factors(args,vx_var,contextptr),contextptr);
  }
  static const char _divis_s []="divis";
  static define_unary_function_eval (__divis,&_divis,_divis_s);
  define_unary_function_ptr5( at_divis ,alias_at_divis,&__divis,0,true);

  /*
  gen ichinreme(const vecteur & a,const vecteur & b){
    vecteur r(2);
    gen p=a[1],q=b[1],u,v,d;
    egcd(p,q,u,v,d);
    if (d!=1)  return gensizeerr(contextptr);
    r[0]=(u*p*b[0]+v*q*a[0]%p*q);
    r[1]=p*q;
    return(r);
  }
  gen _ichinreme(const gen & args){
  if ( args){
    if ( (args.type!=_VECT) || (args._VECTptr->size()!=4) )
      return gensizeerr(contextptr);
    vecteur a(2).type==_STRNG && args.subtype==-1{
    if ( (args.type!=_VECT) || (args._VECTptr->size()!=4) )
      return gensizeerr(contextptr);
    vecteur a(2))) return  args){
    if ( (args.type!=_VECT) || (args._VECTptr->size()!=4) )
      return gensizeerr(contextptr);
    vecteur a(2);
    if ( (args.type!=_VECT) || (args._VECTptr->size()!=4) )
      return gensizeerr(contextptr);
    vecteur a(2),b(2);
    a[0]=args[0];
    a[1]=args[1];
    b[0]=args[2];
    b[1]=args[3];
    //gen a=args[0],p=args[1], b=args[2],q=args[3];
    return ichinreme(a,b);
  }
  static const char _ichinreme_s []="ichinreme";
  static define_unary_function_eval (__ichinreme,&_ichinreme,_ichinreme_s);
  define_unary_function_ptr5( at_ichinreme ,alias_at_ichinreme,&__ichinreme,0,true); 
  */

  gen euler(const gen & e,GIAC_CONTEXT){
    if (e==0)
      return e;
    vecteur v(ifactors(e,contextptr));
    if (!v.empty() && is_undef(v.front())) return v.front();
    const_iterateur it=v.begin(),itend=v.end();
    for (gen res(plus_one);;){
      if (it==itend)
	return res;
      gen p=*it;
      ++it;
      int n=it->val;
      res = res * (p-plus_one)*pow(p,n-1);
      ++it;
    }
  }
  static const char _euler_s []="euler";
  gen _euler(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT)
      return apply(args,_euler,contextptr);
    if ( is_integer(args) && is_positive(args,contextptr))
      return euler(args,contextptr);
    return gentypeerr(contextptr);
  }
  static define_unary_function_eval (__euler,&_euler,_euler_s);
  define_unary_function_ptr5( at_euler ,alias_at_euler,&__euler,0,true);

  gen pa2b2(const gen & p,GIAC_CONTEXT){
    if (p==2)
      return makevecteur(1,1);
    if (!is_integer(p) || (p%4)!=1 || is_greater(1,p,contextptr)) return gensizeerr(contextptr);// car p!=1 mod 4
    gen q=(p-1)/4;
    gen a=2;
    gen ra;
    ra=powmod(a,q,p);
    //on cherche ra^2=-1 mod p avec ra!=1 et ra !=p-1
    while ((a!=p-1) && ((ra==1)|| (ra==p-1))){
      a=a+1;
      ra=powmod(a,q,p);
    }
    if ((ra==1)||(ra==p-1))  return gensizeerr(contextptr);//car p n'est pas premier
    gen ux=1,uy=ra,vx=0,vy=p,wx,wy; 
    gen m=1;
    while(m!=0){
      if (is_positive(vx*vx+vy*vy-ux*ux+uy*uy,0)){
	//on echange u et v
	wx=vx;
	wy=vy;
	vx=ux;
	vy=uy;
	ux=wx;
	uy=wy;
      }
      gen alpha=inv(2,contextptr)-(ux*vx+uy*vy)*inv(vx*vx+vy*vy,contextptr);
      //m=partie entiere de alpha (-v.v/2<(u+mv).v<=v.v/2)
      m=_floor(alpha,contextptr);
      ux=ux+m*vx;
      uy=uy+m*vy;
    }
    vecteur v(2);
    //v repond a la question
    v[0]=abs(vx,contextptr); // ok
    v[1]=abs(vy,contextptr); // ok
    if (vx*vx+vy*vy!=p)
      return gensizeerr(contextptr);
    return v;
  }
  gen _pa2b2(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (!is_integer(args)) 
      return gensizeerr(contextptr);
    gen n=args;
    return pa2b2(n,contextptr);
  }
  static const char _pa2b2_s []="pa2b2";
  static define_unary_function_eval (__pa2b2,&_pa2b2,_pa2b2_s);
  define_unary_function_ptr5( at_pa2b2 ,alias_at_pa2b2,&__pa2b2,0,true);

  static gen ipropfrac(const gen & a,const gen & b,GIAC_CONTEXT){
    if (!is_integer(a) || !is_integer(b))
      return gensizeerr(contextptr);
    gen r=a%b;
    gen q=(a-r)/b;
    gen d=gcd(r,b);
    r=r/d;
    gen b1=b/d;
    if (r==0)
      return q;
    gen v;
    v=symbolic(at_division,gen(makevecteur(r,b1),_SEQ__VECT));
    gen w;
    w=symbolic(at_plus,gen(makevecteur(q,v),_SEQ__VECT));
    if (calc_mode(contextptr)==1)
      return symbolic(at_quote,w);
    return w;
  }
  gen _propfrac(const gen & arg,GIAC_CONTEXT){
    if ( arg.type==_STRNG && arg.subtype==-1) return  arg;
    gen args(arg);
    vecteur v;
    if (arg.type==_VECT && arg._VECTptr->size()==2){
      v=vecteur(1,arg._VECTptr->back());
      args=arg._VECTptr->front();
      lvar(args,v);
    }
    else
      v=lvar(arg);
    gen g=e2r(args,v,contextptr);
    gen a,b;
    fxnd(g,a,b);
    if (v.empty())
      return ipropfrac(a,b,contextptr);
    else {
      gen d=r2e(b,v,contextptr);
      g=_quorem(makesequence(r2e(a,v,contextptr),d,v.front()),contextptr);
      if (is_undef(g)) return g;
      vecteur &v=*g._VECTptr;
      return v[0]+rdiv(v[1],d,contextptr);
    }
  }
  static const char _propfrac_s []="propfrac";
  static define_unary_function_eval (__propfrac,&_propfrac,_propfrac_s);
  define_unary_function_ptr5( at_propfrac ,alias_at_propfrac,&__propfrac,0,true);
  
  void step_egcd(int a,int b,GIAC_CONTEXT){
    gprintf("===============",vecteur(0),1,contextptr);
    gprintf("Extended Euclide algorithm for a=%gen and b=%gen",makevecteur(a,b),1,contextptr);
    gprintf("L%gen: 1*a+0*b=%gen",makevecteur(1,a),1,contextptr);
    gprintf("L%gen: 0*a+1*b=%gen",makevecteur(2,b),1,contextptr);
    int i=3;
    int u0=1,v0=0,u1=0,v1=1,u2,v2;
    for (;b;++i){
      int q=a/b;
      u2=u0-q*u1;
      v2=v0-q*v1;
      int r=a-q*b;
      gprintf("iquo(%gen,%gen)=%gen",makevecteur(a,b,q),1,contextptr);
      gprintf("L%gen=L%gen-%gen*L%gen: %gen*a+%gen*b=%gen",makevecteur(i,i-2,q,i-1,u2,v2,r),1,contextptr);
      u0=u1;
      u1=u2;
      v0=v1;
      v1=v2;
      a=b;
      b=r;
    }
    gprintf("Bezout identity %gen*a+%gen*b=%gen",makevecteur(u0,v0,a),1,contextptr);
  }

  gen iabcuv(const gen & a,const gen & b,const gen & c,GIAC_CONTEXT){
    gen d=gcd(a,b);
    if (c%d!=0)  return gensizeerr(gettext("No solution in ring"));
    gen a1=a/d,b1=b/d,c1=c/d;
    gen u,v,w;
    if (a1.type==_INT_ && b1.type==_INT_ && step_infolevel(contextptr))
      step_egcd(a1.val,b1.val,contextptr);
    egcd(a1,b1,u,v,w);
    vecteur r(2);
    r[0]=smod(u*c1,b);
    r[1]=iquo(c-r[0]*a,b);
    return r;
  }
  gen _iabcuv(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT) || (args._VECTptr->size()!=3) )
      return gensizeerr(contextptr);
    gen a=args[0],b=args[1],c=args[2];
    return iabcuv(a,b,c,contextptr);
  }
  static const char _iabcuv_s []="iabcuv";
  static define_unary_function_eval (__iabcuv,&_iabcuv,_iabcuv_s);
  define_unary_function_ptr5( at_iabcuv ,alias_at_iabcuv,&__iabcuv,0,true);

  gen abcuv(const gen & a,const gen & b,const gen & c,const gen & x,GIAC_CONTEXT){
    gen g=_egcd(makesequence(a,b,x),contextptr);
    if (is_undef(g)) return g;
    vecteur & v=*g._VECTptr;
    gen h=_quorem(makesequence(c,v[2],x),contextptr);
    if (is_undef(h)) return h;
    vecteur & w=*h._VECTptr;
    if (!is_zero(w[1]))
      return gensizeerr(gettext("No solution in ring"));
    gen U=v[0]*w[0],V=v[1]*w[0];
    if (_degree(makesequence(c,x),contextptr).val<_degree(makesequence(a,x),contextptr).val+_degree(makesequence(b,x),contextptr).val ){
      U=_rem(makesequence(U,b,x),contextptr);
      V=_rem(makesequence(V,a,x),contextptr);
    }
    return makevecteur(U,V);
  }
  gen _abcuv(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT) || (args._VECTptr->size()<3) )
      return gensizeerr(contextptr);
    vecteur & v =*args._VECTptr;
    if (v.size()>3)
      return abcuv(v[0],v[1],v[2],v[3],contextptr);
    return abcuv(v[0],v[1],v[2],vx_var,contextptr);
  }
  static const char _abcuv_s []="abcuv";
  static define_unary_function_eval (__abcuv,&_abcuv,_abcuv_s);
  define_unary_function_ptr5( at_abcuv ,alias_at_abcuv,&__abcuv,0,true);

  gen simp2(const gen & a,const gen & b,GIAC_CONTEXT){
    vecteur r(2);
    gen d=gcd(a,b);
    r[0]=normal(a/d,contextptr);
    r[1]=normal(b/d,contextptr);
    return r;
  }
  gen _simp2(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT) || (args._VECTptr->size()!=2) )
      return gensizeerr(contextptr);
    gen a=args[0],b=args[1];
    if ( (a.type==_VECT) || (b.type==_VECT) )
      return gensizeerr(contextptr);
    return simp2(a,b,contextptr);
  }
  static const char _simp2_s []="simp2";
  static define_unary_function_eval (__simp2,&_simp2,_simp2_s);
  define_unary_function_ptr5( at_simp2 ,alias_at_simp2,&__simp2,0,true);
 
  gen fxnd(const gen & a){
    vecteur v(lvar(a));
    gen g=e2r(a,v,context0); // ok
    gen n,d;
    fxnd(g,n,d);
    return makevecteur(r2e(n,v,context0),r2e(d,v,context0)); // ok
  }
  gen _fxnd(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT)
      return apply(args,fxnd);
    return fxnd(args);
  }
  static const char _fxnd_s []="fxnd";
  static define_unary_function_eval (__fxnd,&_fxnd,_fxnd_s);
  define_unary_function_ptr5( at_fxnd ,alias_at_fxnd,&__fxnd,0,true); 

  int generator(int p,const vecteur & v){
    vector<int> w;
    for (int i=0;i<v.size();i+=2){
      if (v[i].type!=_INT_)
	return 0;
      w.push_back((p-1)/v[i].val);
    }
    for (int a=2;a<p;++a){
      int r=0;
      for (int i=0;i<w.size();i++){
	r=powmod(a,w[i],p);
	if (r==1)
	  break;
      }
      if (r!=1)
	return a;
    }
    return 0; // p is not prime!
  }

  // p assumed to be prime, find a generator of (Z/pZ^*,*)
  int generator(int p){
    vecteur v=ifactors(p-1,context0);
    return generator(p,v);
  }

  gen _znprimroot(const gen & p,GIAC_CONTEXT){
    if (p==2) return 1;
    if (p==4) return 3;
    gen o=p-1,q=p; // order
    if (modulo(p,2)==0){
      q=p/2;
      if (modulo(q,2)==0)
	return undef;
    }
    int cyclic=0;
    if (is_probab_prime_p(q))
      cyclic=1;
    else {
      // is q a power of a prime?
      double d=evalf_double(q,1,contextptr)._DOUBLE_val;
      int maxpow=int(std::ceil(std::log(d)/std::log(3.0)));
      for (int i=2;i<=maxpow;++i){
	if ( (i>2 && i%2==0) ||
	     (i>3 && i%3==0) ||
	     (i>5 && i%5==0) ||
	     (i>7 && i%7==0) )
	  continue;
	gen u;
	if (i==2)
	  u=isqrt(q);
	else if (i==4)
	  u=isqrt(isqrt(q));
	else {
	  double x=std::pow(d,1./i);
	  u=longlong(x);
	}
	if (pow(u,i,contextptr)==q){
	  cyclic=2;
	  o=q*(1-inv(u,contextptr));
	  break;
	}
      }
    }
    if (cyclic){
      if (cyclic==1 && p.type==_INT_) 
	return makemod(generator(p.val),p);
      gen g=prime_factors(o,true,contextptr);
      if (g.type!=_VECT) return undef;
      vecteur & v=*g._VECTptr;
      vecteur w;
      for (int i=0;i<v.size();++i){
	w.push_back(o/v[i]);
      }
      for (int a=2;a<65536;++a){
	if (gcd(a,p)!=1)
	  continue;
	int i;
	for (i=0;i<w.size();++i){
	  if (powmod(a,w[i],p)==1)
	    break;
	}
	if (i==w.size())
	  return makemod(a,p);
      }
      return undef;
    }
#ifdef HAVE_LIBPARI
    if (!is_integer(p))
      return gentypeerr(contextptr);
    return _pari(makesequence(string2gen("znprimroot",false),p),contextptr);
#endif
    if (p.type!=_INT_ || !is_probab_prime_p(p))
      return gentypeerr("PARI not compiled in => currently, znprimroot(k) expects a prime<2^31");
    return makemod(generator(p.val),p);
  }
  static const char _znprimroot_s []="znprimroot";
  static define_unary_function_eval (__znprimroot,&_znprimroot,_znprimroot_s);
  define_unary_function_ptr5( at_znprimroot ,alias_at_znprimroot,&__znprimroot,0,true); 

  int znorder(int k,int p,int phi,const vecteur & v){
    int o=1;
    for (int i=0;i<v.size();i+=2){
      int pi=v[i].val;
      int mi=v[i+1].val;
      int pimi=pow((unsigned) pi,(unsigned) mi).val;
      int a=powmod(k,phi/pimi,p);
      while (a!=1){
	o *= pi;
	a=powmod(a,pi,p);
      }
    }
    return o;
  }

  int znorder(int k,int p){
    k %= p;
    if (gcd(k,p)!=1)
      return 0;
    if (k==1)
      return 1;
    int phi=euler(p,context0).val;
    vecteur v=ifactors(phi,context0);
    return znorder(k,p,phi,v);
  }

  gen znorder(const gen & k,const gen & p,const gen & phi,const vecteur & v){
    gen o=1;
    for (int i=0;i<v.size();i+=2){
      gen pi=v[i];
      if (is_greater(1,pi,context0))
	continue;
      int mi=v[i+1].val;
      gen pimi=pow(pi,mi);
      gen a=powmod(k,phi/pimi,p);
      while (a!=1){
	o = o*pi;
	a=powmod(a,pi,p);
      }
    }
    return o;
  }

  gen znorder(gen & k,const gen & p){
    k=k % p;
    if (gcd(k,p)!=1)
      return 0;
    if (k==1)
      return 1;
    gen phi=_euler(p,context0);
    gen v=_ifactors(phi,context0);
    if (v.type!=_VECT)
      return undef;
    return znorder(k,p,phi,*v._VECTptr);
  }

  gen _znorder(const gen & args,GIAC_CONTEXT){
    if (args.type==_MOD)
      return _znorder(makevecteur(*args._MODptr,*(args._MODptr+1)),contextptr);
    if (args.type!=_VECT || args._VECTptr->size()!=2)
      return gensizeerr(contextptr);
    gen k=args._VECTptr->front(),p=args._VECTptr->back();
#ifdef HAVE_LIBPARI
    if (gcd(p,k)!=1)
      return 0;
    return _pari(makesequence(string2gen("znorder",false),makemod(k,p)),contextptr);
#endif
    if (is_greater(1,p,contextptr))
      return undef;
    if (k.type==_INT_ && p.val==_INT_ )
      return znorder(k.val,p.val);
    return znorder(k,p);
  }
  static const char _znorder_s []="znorder";
  static define_unary_function_eval (__znorder,&_znorder,_znorder_s);
  define_unary_function_ptr5( at_znorder ,alias_at_znorder,&__znorder,0,true); 

  // b1 *= m mod p
  //  m += (m>>31) &p;
  //  int msurp=((1LL<<31)*m)/p+1;
  inline int precond_mulmod31(int b1,int m,int p,int msurp){
    // b1 += (b1>>31) &p;
    int t=longlong(b1)*m-((longlong(b1)*msurp)>>31)*p;
    // t += (t>>31)&p; // t positive (or at least t-p is valid)
    return t;
  }

  // Harvey algorithm for Bernoulli numbers
  // https://arxiv.org/pdf/0807.1347.pdf
  // k must be even and p prime
  // https://web.maths.unsw.edu.au/~davidharvey/code/bernmm/index.html
  // bernmm lib
  int bernoulli_mod(int k,int p){
#ifdef HAVE_LIBBERNMM
    return bernmm::bern_modp(p,k);
#endif
    if (k>p-3){
      int m=k % (p-1); // now m<p-1 is even therefore <=p-3
      int bm=bernoulli_mod(m,p);
      // bk/k=bm/m mod p
      return ( ( (longlong(bm)*k) %p)*invmod(m,p) )%p;
    }
    vecteur v=ifactors(p-1,context0);
    int g=generator(p,v),r=powmod(g,k-1,p),u;
    int N=p>11?znorder(2,p,p-1,v):0;
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << " end generator/znorder \n";
    if (N>4 && k%N){
      // faster summation is possible
      int n=(N%2)?N:N/2;
      int m=(p-1)/2/n;
      // twokm1=2^(k-1), gi=g^i, gkm1i=(g^(k-1))^i
      longlong S=0,twokm1=powmod(2,(k-1)%N,p),gi=1,gkm1i=1;
      int msurp=((1LL<<31)*twokm1)/p+1;
      for (int i=0;i<m;++i){
	longlong s=0,pow2=1;
	int gi2j=gi; // g^i*2^j
	for (int j=0;j<n;++j){
#if 1
	  gi2j = (gi2j<<1) -p;
	  s -= (1+ ((gi2j>>31)<<1))*pow2;// (2*(1+(gi2j>>31))-1)*pow2;
	  gi2j -= (gi2j>>31)*p;
	  pow2=precond_mulmod31(pow2,twokm1,p,msurp);// (pow2*twokm1)%p;
#else	  
	  gi2j <<= 1;
	  if (gi2j>=p){
	    gi2j -= p;
	    // f=-1
	    s -= pow2;
	    if (s<0)
	      s += p;
	  }
	  else {
	    // f=1
	    s += pow2;
	    if (s>=p)
	      s -= p;
	  }
	  pow2=(pow2*twokm1)%p;
#endif
	}
	// update g^i for next i iteration
	gi=(gi*g)%p;
	// s*(g^(k-1))^i
	s=((s%p)*gkm1i)%p;
	S += s;
	if (S>=p)
	  S -=p;
	// update (g^(k-1))^i for next i iteration
	gkm1i=(gkm1i*r)%p;
      }
      // final answer k/(2^(-(k-1))-2)*S
      S=(S*k)%p;
      S=(S*invmod(invmod(twokm1,p)-2,p))%p;
      return S;
    }
    if (g%2)
      u=(g-1)/2;
    else
      u=(longlong(g-1)*invmod(2,p))%p;
    int S=0,X=1,Y=r;
    for (int i=1;i<=p/2;i++){
      int q=(longlong(g)*X)/p;
      S=(S+(longlong(u)-q)*Y) % p;
      X=(longlong(g)*X) % p;
      Y=(longlong(r)*Y) % p;
    }
    int res=(2*longlong(k)*S)%p;
    res=(longlong(res)*invmod(1-powmod(g,k,p),p))%p;
    return res;
  }

#ifndef USE_GMP_REPLACEMENTS
  void ichinrem_inplace(int r,int m,gen & res,const gen & pim,int & proba,mpz_t & tmpz){
    if (pim.type==_ZINT && res.type==_ZINT){
      longlong amodm=mpz_fdiv_ui(*res._ZINTptr,m);
      if (amodm!=r){
	gen u,v,d; longlong U;
	egcd(pim,m,u,v,d);
	if (u.type==_ZINT)
	  U=mpz_fdiv_ui(*u._ZINTptr,m);
	else
	  U=u.val;
	if (d==-1){ U=-U; v=-v; d=1; }
	mpz_mul_si(tmpz,*pim._ZINTptr,(U*(r-amodm))%m);
	mpz_add(*res._ZINTptr,*res._ZINTptr,tmpz);
	proba=0;
      }
      else ++proba;
    }
  }
#endif

  const double m_ln2=0.69314718055994531;
  // Inspired by David Harvey code (bernmm)
  gen bernoulli_rat(int k){
    long bound1 = (long) std::ceil((k + 0.5) * std::log(double(k)) /m_ln2);
    if (bound1<37)
      bound1=37;
    // Computes the denominator of B_k using Clausen/von Staudt.
    // loop through factors of k
    gen D=1;
    for (int f=1; f*f<=k; f++){
      // if f divides k....
      if (k % f == 0){
	// ... then both f + 1 and k/f + 1 are candidates for primes
	// dividing the denominator of B_k
	if (is_probab_prime_p(f+1))
	  D = (f+1)*D; 
	if (f*f != k){
	  int tmp=k/f+1;
	  if (is_probab_prime_p(tmp))
	    D = tmp*D;
	}
      }
    }
    double bits= (k+0.5)*std::log(double(k))/m_ln2 - 4.094*k + 2.470 +
      std::log(evalf_double(D,1,context0)._DOUBLE_val)/m_ln2 ;
    gen res(0.0),pip=1;
    mpz_t tmpz; mpz_init(tmpz);
    for (int p = 5; ; p = nextprime(p+1).val){
      if (k % (p-1) == 0)
	continue;
      if (debug_infolevel)
	COUT << CLOCK()*1e-6 << " start bernoulli_mod " << p << '\n';
      int cur=bernoulli_mod(k,p);
      if (debug_infolevel)
	COUT << CLOCK()*1e-6 << " end bernoulli_mod " << p << '\n';
      if (res.type==_DOUBLE_)
	res=cur;
      else {
#ifndef USE_GMP_REPLACEMENTS
	if (res.type==_ZINT && pip.type==_ZINT){
	  int proba=0; // not used
	  ichinrem_inplace(cur,p,res,pip,proba,tmpz);
	} else
#endif
	  res=ichinrem(gen(cur),res,gen(p),pip);
      }
      pip = p*pip;
      bits -= std::log(double(p))/m_ln2;
      if (bits<-1)
	break;
    }
    mpz_clear(tmpz);
    res=smod(res*D,pip);
    int s=fastsign(res,context0);
    if (k%4==2){
      if (s==-1)
	res += pip;
    }
    else {
      if (s==1)
	res -= pip;
    }
    //COUT << _evalf(makesequence(res/pip,30),context0) << '\n';
    return res/D;
  }
  
  gen _bernoulli_mod(const gen & args,GIAC_CONTEXT){
    if (args.type!=_VECT || args._VECTptr->size()!=2)
      return gensizeerr(contextptr);
    gen k=args._VECTptr->front(),p=args._VECTptr->back();
    if (k.type!=_INT_ || k.val<2 || k.val%2 || p.type!=_INT_  || !is_probab_prime_p(p) )
      return gentypeerr(contextptr);
    return bernoulli_mod(k.val,p.val);
  }
  static const char _bernoulli_mod_s []="bernoulli_mod";
  static define_unary_function_eval (__bernoulli_mod,&_bernoulli_mod,_bernoulli_mod_s);
  define_unary_function_ptr5( at_bernoulli_mod ,alias_at_bernoulli_mod,&__bernoulli_mod,0,true); 

  inline void new_xab(int & x, int & a, int& b,const int N,const int n,const int alpha,const int beta) {
    switch (x % 3) {
    case 0: 
      x = longlong(x)*x % N;  
      a = longlong(a)*2 % n;  
      b = longlong(b)*2 % n;  
      break;
    case 1: 
      x = longlong(x)*alpha % N;  
      a = (a+1) % n;                  
      break;
    case 2: 
      x = longlong(x)*beta % N;                  
      b = (b+1) % n;  
      break;
    }
  }

  // N prime, solve alpha^e=beta mod N using Pollard-Rho
  int baby64(int alpha,int beta,int N,int n){
    int x = 1, a = 0, b = 0; // x=alpha^a*beta^b
    int X = x, A = a, B = b; // X=alpha^A*beta^B
    for (int i = 1; i < n; ++i) {
      new_xab(x,a,b,N,n,alpha,beta);
      new_xab(X,A,B,N,n,alpha,beta);
      new_xab(X,A,B,N,n,alpha,beta);
      if (x==X){ 
	// alpha^a*beta^b=alpha^A*beta^B hence
	// alpha^(A-a)=beta^(B-b) mod n, (B-b)*e=(A-a) mod n
	int b1=B-b,a1=a-A;
	int g=gcd(b1,n);
	if (a1%g) 
	  return -1;
	// b1*e=a1+k*n1 <-> b2*e=a2+k*n2
	int b2=b1/g,a2=a1/g,n2=n/g;
	longlong b3=invmod(b2,n2); // e=b3*a2 mod n2 
	int e0=(b3*a2) % n2;
	for (longlong k=0;k<g;++k){
	  int e=e0+k*n2;
	  if (e<0) e+=n; 
	  longlong chk=powmod(alpha,e,N);
	  if ((chk-beta)%N==0)
	    return e;
	  if ((chk+beta)%N==0)
	    return e+n/2;
	}
      }
    }
    return -1; // means not found
  }

#ifdef INT128
  inline void new_xab(longlong & x, longlong & a, longlong& b,const longlong N,const longlong n,const longlong alpha,const longlong beta) {
    switch (x % 3) {
    case 0: 
      x = int128_t(x)*x % N;  
      a = int128_t(a)*2 % n;  
      b = int128_t(b)*2 % n;  
      break;
    case 1: 
      x = int128_t(x)*alpha % N;  
      a = (a+1) % n;                  
      break;
    case 2: 
      x = int128_t(x)*beta % N;                  
      b = (b+1) % n;  
      break;
    }
  }

  // N prime, solve alpha^e=beta mod N using Pollard-Rho
  longlong baby128(longlong alpha,longlong beta,longlong N,longlong n){
    longlong x = 1, a = 0, b = 0; // x=alpha^a*beta^b
    longlong X = x, A = a, B = b; // X=alpha^A*beta^B
    for (longlong i = 1; i < n; ++i) {
      new_xab(x,a,b,N,n,alpha,beta);
      new_xab(X,A,B,N,n,alpha,beta);
      new_xab(X,A,B,N,n,alpha,beta);
      if (x==X){ 
	// alpha^a*beta^b=alpha^A*beta^B hence
	// alpha^(A-a)=beta^(B-b) mod n, (B-b)*e=(A-a) mod n
	longlong b1=B-b,a1=a-A;
	longlong g=gcd(b1,n);
	if (a1%g) 
	  return -1;
	// b1*e=a1+k*n1 <-> b2*e=a2+k*n2
	longlong b2=b1/g,a2=a1/g,n2=n/g;
	int128_t b3=invmod(b2,n2); // e=b3*a2 mod n2 
	if (b3<0) b3+=n2;
	longlong e0=(b3*a2) % n2;
	for (longlong k=0;k<g;++k){
	  longlong e=e0+k*n2;
	  int128_t chk=powmod(alpha,e,N);
	  if ((chk-beta)%N==0)
	    return e;
	  if ((chk+beta)%N==0)
	    return e+n/2;
	}
      }
    }
    return -1; // means not found
  }
#endif

  void new_xab(gen & x, gen & a, gen& b,const gen & N,const gen & n,const gen & alpha,const gen & beta) {
    mpz_t & xz=*x._ZINTptr;
    mpz_t & az=*a._ZINTptr;
    mpz_t & bz=*b._ZINTptr;
    const mpz_t & Nz=*N._ZINTptr;
    const mpz_t & nz=*n._ZINTptr;
    int t=modulo(xz,3);
    switch (t) {
    case 0: 
      mpz_mul(xz,xz,xz); mpz_tdiv_r(xz,xz,Nz); // x = longlong(x*x) % N;  
      mpz_mul_ui(az,az,2); mpz_tdiv_r(az,az,nz);  // a = longlong(a*2) % n;  
      mpz_mul_ui(bz,bz,2); mpz_tdiv_r(bz,bz,nz); // b = longlong(b*2) % n;  
      break;
    case 1: 
      mpz_mul(xz,*alpha._ZINTptr,xz); mpz_tdiv_r(xz,xz,Nz); // x = longlong(x*alpha) % N;  
      mpz_add_ui(az,az,1);  mpz_tdiv_r(az,az,nz); // a = (a+1) % n;                  
      break;
    case 2: 
      mpz_mul(xz,*beta._ZINTptr,xz); mpz_tdiv_r(xz,xz,Nz); // x = longlong(x*beta) % N;                  
      mpz_add_ui(bz,bz,1);  mpz_tdiv_r(bz,bz,nz); // b = (b+1) % n;  
      break;
    }
  }

  // N prime, solve alpha^e=beta mod N using Pollard-Rho
  gen baby(const gen & alpha,const gen & beta,const gen & N,const gen & n_){
    if (alpha.type==_INT_ || beta.type==_INT_ || N.type==_INT_){
      gen al=alpha,be=beta,NN=N;
      al.uncoerce(); be.uncoerce(); NN.uncoerce();
      return baby(al,be,NN,n_);
    }
    gen n=n_; n.uncoerce();
    gen x = 1, a = 0, b = 0; // x=alpha^a*beta^b
    gen X = x, A = a, B = b; // X=alpha^A*beta^B
    x.uncoerce(); a.uncoerce(); b.uncoerce(); X.uncoerce(); A.uncoerce(); B.uncoerce();
    for (longlong i=0; i<(1LL<<46); ++i) {
      new_xab(x,a,b,N,n,alpha,beta);
      new_xab(X,A,B,N,n,alpha,beta);
      new_xab(X,A,B,N,n,alpha,beta);
      if (x==X){ 
	// alpha^a*beta^b=alpha^A*beta^B hence
	// alpha^(A-a)=beta^(B-b) mod n, (B-b)*e=(A-a) mod n
	gen b1=B-b,a1=a-A;
	gen g=gcd(b1,n);
	if (a1%g!=0) 
	  return -1;
	// b1*e=a1+k*n1 <-> b2*e=a2+k*n2
	gen b2=b1/g,a2=a1/g,n2=n/g;
	gen b3=invmod(b2,n2); // e=b3*a2 mod n2 
	if (is_positive(-b3,context0))
	  b3 += n2;
	gen e0=(b3*a2) % n2;
	for (gen k=0;is_greater(g,k,context0);k+=1){
	  gen e=e0+k*n2;
	  gen chk=powmod(alpha,e,N);
	  if ((chk-beta)%N==0)
	    return e;
	  if ((chk+beta)%N==0) 
	    return e+n/2; // works only if alpha^(n/2)=-1
	}
      }
    }
    return -1; // means not found
  }

  // solve g^x=h mod N, where g^(p^e)=1 mod N and g^(p^(e-1))!=1 mod N
  // start from the fact that h^-1*g^0 is of order p^(e)  mod N, x0=0
  // then for k>=0 find dk such that
  // h^-1*g^(xk+dk*p^(k)) is of order p^(e-(k+1)) mod N 
  // where h^-1*g^xk of order p^(e-k) mod N
  // (g^(-xk)*h)^-1*g^(dk*p^(k)) must be of order p^(e-(k+1)) mod N
  // let hk:=(g^(-xk)*h)^(p^(e-1-k)) and gamma=g^(p^(e-1)
  // Take power p^(e-1-k) -> hk^-1*gamma^dk of order 1 mod N
  // gamma^dk=hk mod N
  gen padic_logb(const gen & g,const gen & h,const gen & p,int e,const gen & N){
    if (h==1) return 0;
    if (h==g) return 1;
    gen xk=0;
    gen pe1=pow(p,e-1),pe=pe1*p,invg=invmod(g,N);
    if (is_positive(-invg,context0))
      invg+=N;
    gen gamma=powmod(g,pe1,N);
    gen pk=1;
    for (int k=0;k<e;++k){
      gen g1=powmod(invg,xk,N)*h;
      gen hk=powmod(g1,pow(p,e-1-k),N);
      gen dk=logb(hk,gamma,N,context0,p);
      xk = xk+dk*pk;
      pk=p*pk;
    }
    return xk;
  }

  gen logb(const gen & a,const gen &b,const gen & N,GIAC_CONTEXT,gen order){
    if (a==0)
      return undef;
    if (N==2)
      return 0;
    if (a==b)
      return 1;
    if (a==1)
      return 0;
    gen n(order);
    if (order==0){
      n=N-1;
      if (_isprime(N,context0)==0)
	return gensizeerr(gettext("Expecting an odd prime as 3rd argument"));
      // solve b^x=a mod N (that's x=log_b(a))
      gen chk=powmod(b,(N-1)/2,N);
      if ((chk+1) % N !=0)
	*logptr(contextptr) << "2nd arg is not a generator, answer might be wrong\n";
      gen n=N-1;
      vecteur v(ifactors(n,contextptr));
      if (v.size()==2){
	gen res=padic_logb(b,a,v[0],v[1].val,N);
	return res;
      }
      // chinese remaindering step
      gen x,curprod(1);
      for (int i=0;i<v.size();i+=2){
	gen pi=v[i],ei=v[i+1];
	if (ei.type!=_INT_)
	  return gensizeerr(contextptr);
	gen piei=pow(pi,ei.val),coorder=n/piei;
	gen gi=powmod(b,coorder,N);
	gen hi=powmod(a,coorder,N);
	gen xi=padic_logb(gi,hi,pi,ei.val,N);
	if (i==0) 
	  x=xi;
	else
	  x=ichinrem(x,xi,curprod,piei);
	curprod=curprod*piei;
      }
      if (is_positive(x,contextptr))
	return x;
      return x+n;
    }
    if (is_integer(a) && is_integer(b) && N.type==_INT_){
      int A=a.type==_INT_?a.val:modulo(*a._ZINTptr,N.val),B=b.type==_INT_?b.val:modulo(*b._ZINTptr,N.val);
      int res=baby64(B,A,N.val,order.val);
      if (order!=0)
	return res % order.val;
      return res;
    }
    // ifactor n==N-1
    gen res=baby(a,b,N,order);
    if (order!=0)
      return res % order;
    return res;
  }

  // Find a list of prime factors 
  // such that g/factors^multiplicity<sqrt(g) if full==false
  // if not found return undef
  gen prime_factors(const gen &g0,bool full,GIAC_CONTEXT){
    gen g(g0);
    vecteur res;
    if (full){
      g=_ifactors(g0,contextptr);
      if (g.type!=_VECT)
	return undef;
      vecteur & v=*g._VECTptr;
      for (int i=0;i<v.size();i+=2){
	if (is_greater(v[i],1,contextptr))
	  res.push_back(v[i]);
      }
      return res;
    }
    gen gstop=isqrt(g);
    // trivial factor
    for (int i=0;i<sizeof(giac_primes)/sizeof(short int);++i){
      int p=giac_primes[i];
      if (modulo(g,p))
	continue;
      res.push_back(p);
      for (;;){
	g=g/p;
	if (modulo(g,p))
	  break;
      }
      if (is_greater(gstop,g,contextptr))
	return res;
      if (is_greater(g0,g*g,contextptr)){
	res.push_back(g);
	return res;
      }
    }
    // pollard(a,k,contextptr) -> factor or -1/0
    for (;;){
      if (is_greater(gstop,g,contextptr))
	return res;
      if (is_probab_prime_p(g)){
	// leave the user compute a certificate for this prime factor...
	res.push_back(g);
	return res;
      }
      gen b=pollard(g,1,contextptr);
      if (!is_greater(b,2,contextptr)){
	// _ecm_factor(n,contextptr) -> factor or undef
	b=_ecm_factor(g,contextptr);
	if (is_undef(b))
	  return undef; // could not partial factor
      }
      gen c=_ifactors(b,contextptr);
      if (c.type!=_VECT)
	return undef;
      vecteur & v=*c._VECTptr;
      for (int i=0;i<v.size();i+=2){
	if (is_greater(v[i],2,contextptr))
	  res.push_back(v[i]);
      }
      g=g/b;
    }
    return undef; // could not partial factor (never reached)
  }

  gen prime_cert(const gen & g,GIAC_CONTEXT){
    if (!is_probab_prime_p(g))
      return 0;
    gen o=g-1; // order
    gen lf=prime_factors(o,false,contextptr); // partial list of prime factors
    if (lf.type!=_VECT)
      return undef;
    vecteur & v=*lf._VECTptr;
    // for each element p of v, we must find a such that 
    // * a^o==1 mod g
    // * gcd(a^(o/p)-1,g)==1
    vecteur res;
    for (int i=0;i<v.size();++i){
      gen p=v[i];
      for (int a=2;;++a){
	if (a==RAND_MAX) return undef;
	if (powmod(a,o,g)!=1)
	  continue;
	if (gcd(powmod(a,o/p,g)-1,g)==1){
	  res.push_back(makevecteur(p,a,p.type==_INT_?1:0));
	  break;
	}
      }
    }
    return res;
  }
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC

