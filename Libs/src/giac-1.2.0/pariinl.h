/* $Id: level0.h,v 1.3 2000/11/03 21:00:25 karim Exp $

Copyright (C) 2000  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, see <http://www.gnu.org/licenses/>. */

/* This file defines some "level 0" kernel functions for Intel x386  */
/* It is intended for use with an external "asm" definition          */

#ifndef ASMINLINE
#define LOCAL_OVERFLOW
#define SAVE_OVERFLOW
#define LOCAL_HIREMAINDER
#define SAVE_HIREMAINDER

BEGINEXTERN
extern  ulong overflow;
extern  ulong hiremainder;
extern long addll(ulong x, ulong y);
extern long addllx(ulong x, ulong y);
extern long subll(ulong x, ulong y);
extern long subllx(ulong x, ulong y);
extern long shiftl(ulong x, ulong y);
extern long shiftlr(ulong x, ulong y);
extern long mulll(ulong x, ulong y);
extern long addmul(ulong x, ulong y);
extern long divll(ulong x, ulong y);
extern int  bfffo(ulong x);
ENDEXTERN

#else /* ASMINLINE */

/* $Id: level0.h,v 1.3 2000/11/03 21:00:25 karim Exp $ */
/* Written by Bruno Haible, 1996-1998. */

/* This file can assume the GNU C extensions.
   (It is included only if __GNUC__ is defined.) */


/* Use local variables whenever possible. */
#define LOCAL_HIREMAINDER  register ulong hiremainder
#define SAVE_OVERFLOW \
     { ulong _temp_overf = overflow; \
       extern ulong overflow; \
       overflow = _temp_overf; }
#define LOCAL_OVERFLOW  ulong overflow
#define SAVE_HIREMAINDER \
     { ulong _temp_hirem = hiremainder; \
       extern ulong hiremainder; \
       hiremainder = _temp_hirem; }
/* The global variable `hiremainder' is still necessary for the 2nd value of
   divss, divis, divsi. The global variable `overflow' is not necessary. */
extern ulong overflow;
extern ulong hiremainder;


/* Different assemblers have different syntax for the "shldl" and "shrdl"
   instructions. */
#if defined(__EMX__) || defined(__DJGCC__) || defined(__GO32__) || (defined(linux) && !defined(__ELF__)) || defined(__386BSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(NeXT) || defined(__CYGWIN32__) || defined(__MINGW32__) || defined(COHERENT)
#  define SHCL "%%cl,"
#else
#  define SHCL
#endif


#define addll(a,b) \
({ ulong __value, __arg1 = (a), __arg2 = (b); \
   __asm__ ("addl %3,%0 ; adcl %1,%1" \
        : "=r" (__value), "=r" (overflow) \
        : "0" (__arg1), "g" (__arg2), "1" ((ulong)0) \
        : "cc"); \
  __value; \
})

#define addllx(a,b) \
({ ulong __value, __arg1 = (a), __arg2 = (b), __temp; \
   __asm__ ("subl %5,%2 ; adcl %4,%0 ; adcl %1,%1" \
        : "=r" (__value), "=r" (overflow), "=r" (__temp) \
        : "0" (__arg1), "g" (__arg2), "g" (overflow), "1" ((ulong)0), "2" ((ulong)0) \
        : "cc"); \
  __value; \
})


#define subll(a,b) \
({ ulong __value, __arg1 = (a), __arg2 = (b); \
   __asm__ ("subl %3,%0 ; adcl %1,%1" \
        : "=r" (__value), "=r" (overflow) \
        : "0" (__arg1), "g" (__arg2), "1" ((ulong)0) \
        : "cc"); \
  __value; \
})

#define subllx(a,b) \
({ ulong __value, __arg1 = (a), __arg2 = (b), __temp; \
   __asm__ ("subl %5,%2 ; sbbl %4,%0 ; adcl %1,%1" \
        : "=r" (__value), "=r" (overflow), "=r" (__temp) \
        : "0" (__arg1), "g" (__arg2), "g" (overflow), "1" ((ulong)0), "2" ((ulong)0) \
        : "cc"); \
  __value; \
})


#if 1
#define shiftl(a,c) \
({ ulong __valuelo = (a), __count = (c), __valuehi; \
   __asm__ ("shldl "SHCL"%2,%0" /* shift %0 left by %cl bits, feeding in %2 from the right */ \
        : "=q" (__valuehi) \
        : "0" ((ulong)0), "q" (__valuelo), "c" /* %ecx */ (__count)); \
   hiremainder = __valuehi; \
   __valuelo << __count; \
})
#define shiftlr(a,c) \
({ ulong __valuehi = (a), __count = (c), __valuelo; \
   __asm__ ("shrdl "SHCL"%2,%0" /* shift %0 right by %cl bits, feeding in %2 from the left */ \
        : "=q" (__valuelo) \
        : "0" ((ulong)0), "q" (__valuehi), "c" /* %ecx */ (__count)); \
   hiremainder = __valuelo; \
   __valuehi >> __count; \
})
#else
#define shiftl(a,c) \
({ ulong __valuelo = (a), __count = (c), __valuehi; \
   __asm__ ("shldl "SHCL"%2,%0" /* shift %0 left by %cl bits, feeding in %2 from the right */ \
        : "=d" (hiremainder) \
        : "0" ((ulong)0), "q" (__valuelo), "c" /* %ecx */ (__count)); \
   __valuelo << __count; \
})
#define shiftlr(a,c) \
({ ulong __valuehi = (a), __count = (c), __valuelo; \
   __asm__ ("shrdl "SHCL"%2,%0" /* shift %0 right by %cl bits, feeding in %2 from the left */ \
        : "=d" (hiremainder) \
        : "0" ((ulong)0), "q" (__valuehi), "c" /* %ecx */ (__count)); \
   __valuehi >> __count; \
})
#endif


#define mulll(a,b) \
({ ulong __valuelo, __arg1 = (a), __arg2 = (b); \
   __asm__ ("mull %3" \
        : "=a" /* %eax */ (__valuelo), "=d" /* %edx */ (hiremainder) \
        : "0" (__arg1), "rm" (__arg2)); \
   __valuelo; \
})

#define addmul(a,b) \
({ ulong __valuelo, __arg1 = (a), __arg2 = (b), __temp; \
   __asm__ ("mull %4 ; addl %5,%0 ; adcl %6,%1" \
        : "=a" /* %eax */ (__valuelo), "=&d" /* %edx */ (hiremainder), "=r" (__temp) \
        : "0" (__arg1), "rm" (__arg2), "g" (hiremainder), "2" ((ulong)0)); \
   __valuelo; \
})

#define addmullow(a,b) \
({ ulong __valuelo, __arg1 = (a), __arg2 = (b), __temp; \
   __asm__ ("mull %3 ; addl %4,%0" \
        : "=a" /* %eax */ (__valuelo), "=&d" /* %edx */ (__temp) \
        : "0" (__arg1), "rm" (__arg2), "g" (hiremainder)); \
   __valuelo; \
})

#define divll(a,b) \
({ ulong __value, __arg1 = (a), __arg2 = (b); \
   __asm__ ("divl %4" \
        : "=a" /* %eax */ (__value), "=d" /* %edx */ (hiremainder) \
        : "0" /* %eax */ (__arg1), "1" /* %edx */ (hiremainder), "g" (__arg2)); \
   __value; \
})

#ifndef _ASMI386INLINE_H_
#  define _ASMI386INLINE_H_
#  ifdef INLINE
static inline int
bfffo(ulong x)
{
  int leading_one_position;
  __asm__ ("bsrl %1,%0" : "=r" (leading_one_position) : "rm" (x));
  return 31-leading_one_position;
}
#  endif
#endif

#endif /* ASMINLINE */
/* $Id: level1.h,v 1.6 2000/11/03 21:00:26 karim Exp $

Copyright (C) 2000  The PARI group.

This file is part of the PARI/GP package.

PARI/GP is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation. It is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY WHATSOEVER.

Check the License for details. You should have received a copy of it, along
with the package; see the file 'COPYING'. If not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

/* This file defines some "level 1" kernel functions                 */
/* These functions can be inline, with gcc                           */
/* If not gcc, they are defined externally with "level1.c"           */

/* level1.c includes this file and never needs to be changed         */
/* The following seven lines are necessary for level0.c and level1.c */
#ifdef LEVEL1
#  undef  INLINE
#  define INLINE
#endif
#ifdef LEVEL0
#  undef  INLINE
#endif

#ifndef INLINE
void   addsii(long x, GEN y, GEN z);
long   addssmod(long a, long b, long p);
void   addssz(long x, long y, GEN z);
void   affii(GEN x, GEN y);
void   affsi(long s, GEN x);
void   affsr(long s, GEN x);
GEN    cgetg(long x, long y);
GEN    cgeti(long x);
GEN    cgetr(long x);
int    cmpir(GEN x, GEN y);
int    cmpsr(long x, GEN y);
int    divise(GEN x, GEN y);
long   divisii(GEN x, long y, GEN z);
void   divisz(GEN x, long y, GEN z);
void   divrrz(GEN x, GEN y, GEN z);
void   divsiz(long x, GEN y, GEN z);
GEN    divss(long x, long y);
long   divssmod(long a, long b, long p);
void   divssz(long x, long y, GEN z);
void   dvmdiiz(GEN x, GEN y, GEN z, GEN t);
GEN    dvmdis(GEN x, long y, GEN *z);
void   dvmdisz(GEN x, long y, GEN z, GEN t);
GEN    dvmdsi(long x, GEN y, GEN *z);
void   dvmdsiz(long x, GEN y, GEN z, GEN t);
GEN    dvmdss(long x, long y, GEN *z);
void   dvmdssz(long x, long y, GEN z, GEN t);
ulong  evallg(ulong x);
ulong  evallgef(ulong x);
ulong  evalvalp(ulong x);
ulong  evalexpo(ulong x);
#ifndef __M68K__
long   expi(GEN x);
#endif
double gtodouble(GEN x);
GEN    icopy(GEN x);
GEN    icopy_av(GEN x, GEN y);
long   itos(GEN x);
GEN    modis(GEN x, long y);
GEN    mpabs(GEN x);
GEN    mpadd(GEN x, GEN y);
void   mpaff(GEN x, GEN y);
int    mpcmp(GEN x, GEN y);
GEN    mpcopy(GEN x);
GEN    mpdiv(GEN x, GEN y);
int    mpdivis(GEN x, GEN y, GEN z);
GEN    mpmul(GEN x, GEN y);
GEN    mpneg(GEN x);
GEN    mpsub(GEN x, GEN y);
void   mulsii(long x, GEN y, GEN z);
long   mulssmod(ulong a, ulong b, ulong c);
void   mulssz(long x, long y, GEN z);
GEN    new_chunk(long x);
void   resiiz(GEN x, GEN y, GEN z);
GEN    resis(GEN x, long y);
GEN    ressi(long x, GEN y);
GEN    shiftr(GEN x, long n);
long   smodis(GEN x, long y);
GEN    stoi(long x);
GEN    subii(GEN x, GEN y);
GEN    subir(GEN x, GEN y);
GEN    subri(GEN x, GEN y);
GEN    subrr(GEN x, GEN y);
GEN    subsi(long x, GEN y);
GEN    subsr(long x, GEN y);
long   subssmod(long a, long b, long p);
GEN    utoi(ulong x);
long   vali(GEN x);

#else /* defined(INLINE) */
INLINE ulong
evallg(ulong x)
{
  if (x & ~LGBITS) err(errlg);
  return m_evallg(x);
}

INLINE ulong
evallgef(ulong x)
{
  if (x & ~LGEFBITS) err(errlgef);
  return m_evallgef(x);
}

INLINE ulong
evalvalp(ulong x)
{
  const long v = m_evalvalp(x);
  if (v & ~VALPBITS) err(errvalp);
  return v;
}

INLINE ulong
evalexpo(ulong x)
{
  const long v = m_evalexpo(x);
  if (v & ~EXPOBITS) err(errexpo);
  return v;
}

INLINE GEN
new_chunk(long x)
{
  const GEN z = ((GEN) avma) - x;
  if ((ulong)x > (ulong)((GEN)avma-(GEN)bot)) err(errpile);
#ifdef MEMSTEP
  checkmemory(z);
#endif
#ifdef _WIN32
  if (win32ctrlc) dowin32ctrlc();
#endif
  avma = (long)z; return z;
}

/* THE FOLLOWING ONES ARE IN mp.s */
#  ifndef __M68K__

INLINE GEN
cgetg(long x, long y)
{
  const GEN z = new_chunk(x);
  z[0] = evaltyp(y) | evallg(x);
  return z;
}

INLINE GEN
cgeti(long x)
{
  const GEN z = new_chunk(x);
  z[0] = evaltyp(t_INT) | evallg(x);
  return z;
}

INLINE GEN
cgetr(long x)
{
  const GEN z = new_chunk(x);
  z[0] = evaltyp(t_REAL) | evallg(x);
  return z;
}
#  endif /* __M68K__ */

/* cannot do memcpy because sometimes x and y overlap */
INLINE GEN
mpcopy(GEN x)
{
  register long lx = lg(x);
  const GEN y = new_chunk(lx);

  while (--lx >= 0) y[lx]=x[lx];
  return y;
}

INLINE GEN
icopy(GEN x)
{
  register long lx = lgefint(x);
  const GEN y = cgeti(lx);

  while (--lx > 0) y[lx]=x[lx];
  return y;
}

/* copy integer x as if we had avma = av */
INLINE GEN
icopy_av(GEN x, GEN y)
{
  register long lx = lgefint(x);

  y -= lx; while (--lx >= 0) y[lx]=x[lx];
  return y;
}

INLINE GEN
mpneg(GEN x)
{
  const GEN y=mpcopy(x);
  setsigne(y,-signe(x)); return y;
}

INLINE GEN
mpabs(GEN x)
{
  const GEN y=mpcopy(x);
  if (signe(x)<0) setsigne(y,1);
  return y;
}

INLINE long
smodis(GEN x, long y)
{
  const long av=avma; divis(x,y); avma=av;
  if (!hiremainder) return 0;
  return (signe(x)>0) ? hiremainder: (y>0?y:-y)+hiremainder;
}

INLINE GEN
utoi(ulong x)
{
  GEN y;

  if (!x) return gzero;
  y=cgeti(3); y[1] = evalsigne(1) | evallgefint(3); y[2] = x;
  return y;
}

#  ifndef __M68K__
INLINE GEN
stoi(long x)
{
  GEN y;

  if (!x) return gzero;
  y=cgeti(3);
  if (x>0) { y[1] = evalsigne(1) | evallgefint(3); y[2] = x; }
  else { y[1] = evalsigne(-1) | evallgefint(3); y[2] = -x; }
  return y;
}

INLINE long
itos(GEN x)
{
  const long s=signe(x);
  long p1;

  if (!s) return 0;
  if (lgefint(x)>3) err(affer2);
  p1=x[2]; if (p1 < 0) err(affer2);
  return (s>0) ? p1 : -(long)p1;
}
#endif

INLINE GEN
stosmall(long x)
{
  if ( (x>0?x:-x) & SMALL_MASK) return stoi(x);
  return (GEN) (1 | (x<<1));
}

#  ifndef __M68K__

INLINE void
affii(GEN x, GEN y)
{
  long lx;

  if (x==y) return;
  lx=lgefint(x); if (lg(y)<lx) err(affer3);
  while (--lx) y[lx]=x[lx];
}

INLINE void
affsi(long s, GEN x)
{
  if (!s) { x[1]=2; return; }
  if (lg(x)<3) err(affer1);
  if (s>0) { x[1] = evalsigne(1) | evallgefint(3); x[2] = s; }
  else { x[1] = evalsigne(-1) | evallgefint(3); x[2] = -s; }
}

INLINE void
affsr(long s, GEN x)
{
  long l;

  if (!s)
  {
    l = -bit_accuracy(lg(x));
    x[1]=evalexpo(l); x[2]=0; return;
  }
  if (s<0) { x[1] = evalsigne(-1); s = -s; }
  else x[1] = evalsigne(1);
  l=bfffo(s); x[1] |= evalexpo((BITS_IN_LONG-1)-l);
  x[2] = s<<l; for (l=3; l<lg(x); l++) x[l]=0;
}

INLINE void
mpaff(GEN x, GEN y)
{
  if (typ(x)==t_INT)
   { if (typ(y)==t_INT) affii(x,y); else affir(x,y); }
  else
   { if (typ(y)==t_INT) affri(x,y); else affrr(x,y); }
}

INLINE GEN
shiftr(GEN x, long n)
{
  const long e = evalexpo(expo(x)+n);
  const GEN y = rcopy(x);

  if (e & ~EXPOBITS) err(shier2);
  y[1] = (y[1]&~EXPOBITS) | e; return y;
}

INLINE int
cmpir(GEN x, GEN y)
{
  long av;
  GEN z;

  if (!signe(x)) return -signe(y);
  av=avma; z=cgetr(lg(y)); affir(x,z); avma=av;
  return cmprr(z,y); /* cmprr does no memory adjustment */
}

INLINE int
cmpsr(long x, GEN y)
{
  long av;
  GEN z;

  if (!x) return -signe(y);
  av=avma; z=cgetr(3); affsr(x,z); avma=av;
  return cmprr(z,y);
}	

INLINE void
addssz(long x, long y, GEN z)
{
  if (typ(z)==t_INT) gops2ssz(addss,x,y,z);
  else
  {
    const long av=avma;
    const GEN p1=cgetr(lg(z));

    affsr(x,p1); affrr(addrs(p1,y),z); avma=av;
  }
}

INLINE GEN
subii(GEN x, GEN y)
{
  const long s=signe(y);
  GEN z;

  if (x==y) return gzero;
  setsigne(y,-s); z=addii(x,y);
  setsigne(y, s); return z;
}

INLINE GEN
subrr(GEN x, GEN y)
{
  const long s=signe(y);
  GEN z;

  if (x==y) return realzero(lg(x)+2);
  setsigne(y,-s); z=addrr(x,y);
  setsigne(y, s); return z;
}

INLINE GEN
subir(GEN x, GEN y)
{
  const long s=signe(y);
  GEN z;

  setsigne(y,-s); z=addir(x,y);
  setsigne(y, s); return z;
}

INLINE GEN
subri(GEN x, GEN y)
{
  const long s=signe(y);
  GEN z;

  setsigne(y,-s); z=addir(y,x);
  setsigne(y, s); return z;
}

INLINE GEN
subsi(long x, GEN y)
{
  const long s=signe(y);
  GEN z;

  setsigne(y,-s); z=addsi(x,y);
  setsigne(y, s); return z;
}

INLINE GEN
subsr(long x, GEN y)
{
  const long s=signe(y);
  GEN z;

  setsigne(y,-s); z=addsr(x,y);
  setsigne(y, s); return z;
}

INLINE void
mulssz(long x, long y, GEN z)
{
  if (typ(z)==t_INT) gops2ssz(mulss,x,y,z);
  else
  {
    const long av=avma;
    const GEN p1=cgetr(lg(z));

    affsr(x,p1); mpaff(mulsr(y,p1),z); avma=av;
  }
}

INLINE void
mulsii(long x, GEN y, GEN z)
{
  const long av=avma;
  affii(mulsi(x,y),z); avma=av;
}

INLINE void
addsii(long x, GEN y, GEN z)
{
  const long av=avma;
  affii(addsi(x,y),z); avma=av;
}

INLINE long
divisii(GEN x, long y, GEN z)
{
  const long av=avma;
  affii(divis(x,y),z); avma=av; return hiremainder;
}

INLINE long
vali(GEN x)
{
  long lx,i;

  if (!signe(x)) return -1;
  i = lx = lgefint(x)-1; while (!x[i]) i--;
  return ((lx-i)<<TWOPOTBITS_IN_LONG) + vals(x[i]);
}

INLINE GEN
divss(long x, long y)
{
  long p1;
  LOCAL_HIREMAINDER;

  if (!y) err(diver1);
  hiremainder=0; p1 = divll((ulong)(x>0?x:-x),(ulong)(y>0?y:-y));
  if (x<0) { hiremainder = -((long)hiremainder); p1 = -p1; }
  if (y<0) p1 = -p1;
  SAVE_HIREMAINDER; return stoi(p1);
}

INLINE GEN
dvmdss(long x, long y, GEN *z)
{
  const GEN p1=divss(x,y);
  *z = stoi(hiremainder); return p1;
}

INLINE GEN
dvmdsi(long x, GEN y, GEN *z)
{
  const GEN p1=divsi(x,y);
  *z = stoi(hiremainder); return p1;
}

INLINE GEN
dvmdis(GEN x, long y, GEN *z)
{
  const GEN p1=divis(x,y);
  *z=stoi(hiremainder); return p1;
}

INLINE void
dvmdssz(long x, long y, GEN z, GEN t)
{
  const long av=avma;
  const GEN p1=divss(x,y);

  affsi(hiremainder,t); mpaff(p1,z); avma=av;
}

INLINE void
dvmdsiz(long x, GEN y, GEN z, GEN t)
{
  const long av=avma;
  const GEN p1=divsi(x,y);

  affsi(hiremainder,t); mpaff(p1,z); avma=av;
}

INLINE void
dvmdisz(GEN x, long y, GEN z, GEN t)
{
  const long av=avma;
  const GEN p1=divis(x,y);

  affsi(hiremainder,t); mpaff(p1,z); avma=av;
}

INLINE void
dvmdiiz(GEN x, GEN y, GEN z, GEN t)
{
  const long av=avma;
  GEN p;

  mpaff(dvmdii(x,y,&p),z); mpaff(p,t); avma=av;
}

INLINE GEN
modis(GEN x, long y)
{
  return stoi(smodis(x,y));
}

INLINE GEN
ressi(long x, GEN y)
{
  const long av=avma;
  divsi(x,y); avma=av; return stoi(hiremainder);
}

INLINE GEN
resis(GEN x, long y)
{
  const long av=avma;
  divis(x,y); avma=av; return stoi(hiremainder);
}

INLINE void
divisz(GEN x, long y, GEN z)
{
  if (typ(z)==t_INT) gops2gsz(divis,x,y,z);
  else
  {
    const long av=avma;
    const GEN p1=cgetr(lg(z));

    affir(x,p1); affrr(divrs(p1,y),z); avma=av;
  }
}

INLINE void
divsiz(long x, GEN y, GEN z)
{
  const long av=avma;

  if (typ(z)==t_INT) gaffect(divsi(x,y),z);
  else
  {
    const long lz=lg(z);
    const GEN p1=cgetr(lz), p2=cgetr(lz);

    affsr(x,p1); affir(y,p2);
    affrr(divrr(p1,p2),z);
  }
  avma=av;
}

INLINE void
divssz(long x, long y, GEN z)
{
  const long av=avma;

  if (typ(z)==t_INT) gaffect(divss(x,y),z);
  else
  {
    const GEN p1=cgetr(lg(z));

    affsr(x,p1); affrr(divrs(p1,y),z);
  }
  avma=av;
}

INLINE void
divrrz(GEN x, GEN y, GEN z)
{
  const long av=avma;
  mpaff(divrr(x,y),z); avma=av;
}

INLINE void
resiiz(GEN x, GEN y, GEN z)
{
  const long av=avma;
  affii(resii(x,y),z); avma=av;
}

INLINE int
divise(GEN x, GEN y)
{
  const long av=avma;
  const GEN p1=resii(x,y);
  avma=av; return p1 == gzero;
}

INLINE int
mpcmp(GEN x, GEN y)
{
  if (typ(x)==t_INT)
    return (typ(y)==t_INT) ? cmpii(x,y) : cmpir(x,y);
  return (typ(y)==t_INT) ? -cmpir(y,x) : cmprr(x,y);
}

INLINE GEN
mpadd(GEN x, GEN y)
{
  if (typ(x)==t_INT)
    return (typ(y)==t_INT) ? addii(x,y) : addir(x,y);
  return (typ(y)==t_INT) ? addir(y,x) : addrr(x,y);
}

INLINE GEN
mpsub(GEN x, GEN y)
{
  if (typ(x)==t_INT)
    return (typ(y)==t_INT) ? subii(x,y) : subir(x,y);
  return (typ(y)==t_INT) ? subri(x,y) : subrr(x,y);
}

INLINE GEN
mpmul(GEN x, GEN y)
{
  if (typ(x)==t_INT)
    return (typ(y)==t_INT) ? mulii(x,y) : mulir(x,y);
  return (typ(y)==t_INT) ? mulir(y,x) : mulrr(x,y);
}

INLINE GEN
mpdiv(GEN x, GEN y)
{
  if (typ(x)==t_INT)
    return (typ(y)==t_INT) ? divii(x,y) : divir(x,y);
  return (typ(y)==t_INT) ? divri(x,y) : divrr(x,y);
}

INLINE int
mpdivis(GEN x, GEN y, GEN z)
{
  const long av=avma;
  GEN p2;
  const GEN p1=dvmdii(x,y,&p2);

  if (signe(p2)) { avma=av; return 0; }
  affii(p1,z); avma=av; return 1;
}

/* THE FOLLOWING ONES ARE NOT IN mp.s */
#  endif /* !defined(__M68K__) */

INLINE double
gtodouble(GEN x)
{
  static long reel4[4]={ evaltyp(t_REAL) | m_evallg(4),0,0,0 };

  if (typ(x)==t_REAL) return rtodbl(x);
  gaffect(x,(GEN)reel4); return rtodbl((GEN)reel4);
}

INLINE long
addssmod(long a, long b, long p)
{
  ulong res = a + b;
  return (res >= (ulong)p) ? res - p : res;
}

INLINE long
subssmod(long a, long b, long p)
{
  long res = a - b;
  return (res >= 0) ? res : res + p;
}

INLINE long
mulssmod(ulong a, ulong b, ulong c)
{
  LOCAL_HIREMAINDER;
  {
    register ulong x = mulll(a,b);

    /* alter the doubleword by a multiple of c: */
    if (hiremainder>=c) hiremainder %= c;
    (void)divll(x,c);
  }
  return hiremainder;
}

INLINE long
divssmod(long a, long b, long p)
{
  long v1 = 0, v2 = 1, v3, r, oldp = p;

  while (b > 1)
  {
    v3 = v1 - (p / b) * v2; v1 = v2; v2 = v3;
    r = p % b; p = b; b = r;
  }

  if (v2 < 0) v2 += oldp;
  return mulssmod(a, v2, oldp);
}

INLINE long
expi(GEN x)
{
  const long lx=lgefint(x);
  return lx==2? -HIGHEXPOBIT: bit_accuracy(lx)-bfffo(x[2])-1;
}

#endif
