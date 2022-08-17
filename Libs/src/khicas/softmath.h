// -*- mode:C++; compile-command: "g++ -DLINK softmath.cc" -*-
#ifndef _SOFTMATH_H
#define _SOFTMATH_H

#include <complex>
namespace std {
  double giac_gnuwince_exp2(double k);
  double giac_gnuwince_exp(double d);
  double giac_gnuwince_sinh(double d);
  double giac_gnuwince_cosh(double d);
  double giac_gnuwince_tanh(double d);
  double giac_gnuwince_tan(double d);
  double giac_gnuwince_sin(double d);
  double giac_gnuwince_cos(double d);
  double giac_gnuwince_floor(double d);
  double giac_gnuwince_ceil(double d);
  double giac_gnuwince_atan(double d);
  double giac_gnuwince_sqrt(double d);
  double giac_gnuwince_asin(double d);
  double giac_gnuwince_acos(double d);
  double giac_gnuwince_log(double d);
  double giac_gnuwince_log10(double d);
  double giac_gnuwince_acosh(double d);
  double giac_gnuwince_asinh(double d);
  double giac_gnuwince_atanh(double d);
  double giac_gnuwince_pow(double x,double y);
  double giac_gnuwince_hypot(double x,double y);
  double giac_gnuwince_atan2(double x,double y);
  complex<double> giac_gnuwince_exp(const complex<double> & c);
  complex<double> giac_gnuwince_log(const complex<double> & c);
  complex<double> giac_gnuwince_sqrt(const complex<double> & c);
  complex<double> giac_gnuwince_cos(const complex<double> & c);
  complex<double> giac_gnuwince_sin(const complex<double> & c);
  complex<double> giac_gnuwince_tan(const complex<double> & c);
  complex<double> giac_gnuwince_sinh(const complex<double> & c);
  complex<double> giac_gnuwince_cosh(const complex<double> & c);
  complex<double> giac_gnuwince_tanh(const complex<double> & c);

} // end std namespace

#endif // SOFTMATH
