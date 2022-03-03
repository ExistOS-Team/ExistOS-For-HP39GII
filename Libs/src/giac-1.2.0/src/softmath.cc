// -*- compile-command: "/usr/local/wince/bin/arm-wince-pe-g++ softmath.cc -march=armv4 -mapcs-32 -malignment-traps -msoft-float -DNEWLIB -DSARM -DWIN32 -DGNUWINCE -DLINK -lm /usr/local/wince/arm-wince-pe/lib/libc.a -lgcc -lwinsock -lcoredll" -*-

// Transcendental functions implementation (not optimized at all)
// Use -DLINK to test or -c for object creation
#include "softmath.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>

namespace std {
  double m_ln2=0.69314718055994530942;
  double m_twopi=2.0*M_PI;
  double m_sqrt3=1.73205080756887729;

  double m_undef=0.0/0.0;
  double m_plusinf=1.0/0.0;
  double m_minusinf=-1.0/0.0;

  // 14!(14-k)!
  double fact14[]={1.0,14.0,182.0,2184.0,24024.0,240240.0,2162160.0,17297280.0,121080960.0,726485760.0,3632428800.0,14529715200.0,43589145600.0,87178291200.0};

  // k is an integer >=0 and < 1024
  double giac_gnuwince_exp2(double k){
    if (k<31.0){
      unsigned i=0x1 << int(k);
      return i;
    }
    double k1=int(k/32.0+0.001); // k1<32
    double res=1.0;
    k -= k1*32.0; // k<32
    for (;k>0.0;k -=1.0)
      res *= 2.0;
    for (;k1>0.0; k1 -= 1.0)
      res *= 4294967296.0;
    return res;
  }

  double giac_gnuwince_exp(double d){
    if (d<0)
      return 1.0/giac_gnuwince_exp(-d);
#ifdef DEBUG
    ofstream of("int.txt");
#endif
    double k=int(d/m_ln2+.5);
#ifdef DEBUG
    of << "k:" << k << char(10) << char(13) ;
#endif
    if (k>1023.0)
      return m_plusinf;
    d=d-k*m_ln2; // |d|<m_ln2/2
#ifdef DEBUG
    of << "d:" << d << char(10) << char(13) ;
#endif
    if (k<0)
      k=1.0/giac_gnuwince_exp2(-k);
    else
      k=giac_gnuwince_exp2(k);
#ifdef DEBUG
    of << "2^k:" << k << char(10) << char(13) ;
#endif
    // Insure d is < 0, if it was > 0 compute inv(exp(-d))
    bool inv=d>0;
    if (inv)
      d=-d;
    // use Taylor expansion at order 14 since
    // (1+(ln(2.0)/2)^14/14!)-1 -> 0.0
    double res=d;
    for (int i=1;i<14;++i){
      res += fact14[i];
      res *= d;
#ifdef DEBUG
      of << "i:" << i << " ,res:" << res << char(10) << char(13) ;
#endif
    }
    res /= 87178291200.0;
    res += 1.0;
#ifdef DEBUG
    of << "res:" << res << char(10) << char(13) ;
#endif
    if (inv)
      return k/res;
    else
      return res*k;
  }

  double giac_gnuwince_sinh(double d){
    double k=giac_gnuwince_exp(d);
    return k-1.0/k;
  }

  double giac_gnuwince_cosh(double d){
    double k=giac_gnuwince_exp(d);
    return k+1.0/k;
  }

  double giac_gnuwince_tanh(double d){
    if (d>700.0)
      return 1.0;
    if (d<-700.0)
      return -1.0;
    double k=giac_gnuwince_exp(d);
    k=k*k;
    return (k-1)/(k+1);
  }

  // 0<d<=pi/4
  double in_giac_gnuwince_tan(double d){
    if (d>0.07){
      // tan(2*x)=2*tan(x)/(1-tan(x)^2)
      double tandsur2=in_giac_gnuwince_tan(d/2);
      return 2.0*tandsur2/(1.0-tandsur2*tandsur2);
    }
    // d*(1382*d^2^5+3410*d^2^4+8415*d^2^3+20790*d^2^2+51975*d^2+155925)/155925
    double d2=d*d;
    double res=((((1382.0*d2+3410.0)*d2+8415.0)*d2+20790.0)*d2+51975.0)*d2+155925.0;
    return res*d/155925.0;
  }

  double giac_gnuwince_tan(double d){
    if (d<0)
      return -giac_gnuwince_tan(-d);
    double k=int(d/M_PI+.5); 
    d=d-k*M_PI; // |d|<pi/2
    bool neg=d<0,inv=false;
    if (neg)
      d=-d;
    if (d>M_PI_4){
      d=M_PI_2-d;
      inv=true;
    }
    // 0<d<=pi/4
    k= in_giac_gnuwince_tan(d);
    if (neg)
      k=-k;
    if (inv)
      k=1.0/k;
    return k;
  }

  double giac_gnuwince_sin(double d){
    if (d<0)
      return -giac_gnuwince_sin(-d);
    double k=int(d/m_twopi+.5); 
    d=d-k*m_twopi; 
    bool neg=d<0;
    if (neg)
      d=-d;
    if (d>M_PI_2)
      d=M_PI-d;
    // now |d|<pi/2
    k=in_giac_gnuwince_tan(d/2.0);
    k=(2.0*k)/(k*k+1.0);
    if (neg)
      k=-k;
    return k;
  }

  double giac_gnuwince_cos(double d){
    if (d<0)
      return giac_gnuwince_cos(-d);
    double k=int(d/m_twopi+.5); 
    d=d-k*m_twopi; 
    if (d<0)
      d=-d;
    bool neg=false;
    if (d>M_PI_2){
      neg=true;
      d=M_PI-d;
    }
    // now |d|<pi/2, compute sin(pi/2-d)
    k=in_giac_gnuwince_tan((M_PI_2-d)/2.0);
    k=(2.0*k)/(k*k+1.0);
    if (neg)
      k=-k;
    return k;
  }

  double giac_gnuwince_floor(double d){
    if (d>0)
      return int(d);
    double k=int(d);
    if (k==d)
      return k;
    else
      return k-1;
  }

  double giac_gnuwince_ceil(double d){
    if (d<0)
      return int(d);
    double k=int(d);
    if (k==d)
      return k;
    else
      return k+1;
  }

  // Taylor expansion order 23, |d|<0.27
  double in_giac_gnuwince_atan(double d){
    double d2=d*d;
    double res=((((((((((-14549535.0*d2+15935205.0)*d2-17612595.0)*d2+19684665.0)*d2-22309287.0)*d2+25741485.0)*d2-30421755.0)*d2+37182145.0)*d2-47805615.0)*d2+66927861.0)*d2-111546435.0)*d2+334639305.0;
    return res*d/334639305.0;
  }

  double giac_gnuwince_atan(double d){
    if (d<0)
      return -giac_gnuwince_atan(-d);
    bool inv=false;
    if (d>1){
      inv=true;
      d=1.0/d;
    }
    double k;
    if (d<0.265)
      k=in_giac_gnuwince_atan(d);
    else {
      if (d>0.7)
	k=M_PI_4-in_giac_gnuwince_atan((1.0-d)/(1.0+d));
      else
	k=M_PI/6.0+in_giac_gnuwince_atan((m_sqrt3*d-1)/(m_sqrt3+d));
    }
    if (inv)
      k=M_PI_2-k;
    return k;
  }

  double pow2tab[]={0.13407807929942597100e155,0.11579208923731619542e78,0.34028236692093846346e39,0.18446744073709551616e20,0.42949672960000000000e10,65536.0,256.0,16.0,4.0,2.0,1.4142135623730950488,1.1892071150027210667,1.0905077326652576592};
  double giac_gnuwince_sqrt(double d){
    if (d<0)
      return m_undef;
    if (d==0 || d==1)
      return d;
    if (d<1)
      return 1.0/giac_gnuwince_sqrt(1.0/d);
    double k=0.0,k2=512.0;
    for (int i=0;i<8;++i,k2 /= 2.0){
      if (d>=pow2tab[i]){
	k += k2;
	d /= pow2tab[i];
      }
    }
    double d0=(1.0+d)/2.0,d1;
    for (;;d0=d1){
      d1=(d0+d/d0)/2.0;
      if ((d0-d1)<1e-15)
	return d1*giac_gnuwince_exp2(k/2.0);
    }
  }

  double giac_gnuwince_asin(double d){
    if (d==1 || d==-1)
      return d*M_PI_2;
    double d2=d*d;
    if (d2>1)
      return m_undef;
    return giac_gnuwince_atan(d/giac_gnuwince_sqrt(1-d2));
  }

  double giac_gnuwince_acos(double d){
    return M_PI_2-giac_gnuwince_asin(d);
  }

  double giac_gnuwince_log(double d){
    if (d<0)
      return m_undef;
    if (d==0)
      return m_minusinf;
    if (d<1)
      return -giac_gnuwince_log(1.0/d);
    double k=0.0,k2=512.0; 
    // find number of powers of 2 in d
    for (int i=0;i<12;++i,k2 /= 2.0){
      if (d>=pow2tab[i]){
	k += k2;
	d /= pow2tab[i];
      }
    }
    // now d>1 and d<2^(1/8) -> d-1>0 and ||<0.091
    // Taylor expansion at order 14
    d -= 1;
    double res=(((((((((((((-25740.0*d+27720.0)*d-30030.0)*d+32760.0)*d-36036.0)*d+40040.0)*d-45045.0)*d+51480.0)*d-60060.0)*d+72072.0)*d-90090.0)*d+120120.0)*d-180180.0)*d+360360.0)*d;
    return res/360360.0+k*m_ln2;
  }

  double giac_gnuwince_log10(double d){
    return giac_gnuwince_log(d)/M_LN10;
  }

  double giac_gnuwince_acosh(double d){
    double d2=d*d;
    if (d2<1)
      return m_undef;
    return giac_gnuwince_log(d+giac_gnuwince_sqrt(d2-1));
  }

  double giac_gnuwince_asinh(double d){
    double d2=d*d;
    return giac_gnuwince_log(d+giac_gnuwince_sqrt(d2+1));
  }

  double giac_gnuwince_atanh(double d){
    if (d==1)
      return m_plusinf;
    if (d==-1)
      return m_minusinf;
    double d2=d*d;
    if (d2>1)
      return m_undef;
    return giac_gnuwince_log(giac_gnuwince_sqrt(1-d2)/(1-d));
  }

  double giac_gnuwince_pow(double x,double y){
    return giac_gnuwince_exp(y*giac_gnuwince_log(x));
  }

  double giac_gnuwince_hypot(double x,double y){
    return giac_gnuwince_sqrt(x*x+y*y);
  }

  double giac_gnuwince_atan2(double x,double y){
    if (x=0){
      if (y>0)
	return M_PI_2;
      if (y<0)
	return -M_PI_2;
      return m_undef;
    }
    double res=giac_gnuwince_atan(y/x);
    if (x>0)
      return res;
    if (y>0)
      return M_PI+res;
    else
      return res-M_PI;
  }

  complex<double> giac_gnuwince_exp(const complex<double> & c){
    double t=giac_gnuwince_tan(c.imag()/2),t2=t*t;
    return giac_gnuwince_exp(c.real())/(1.0+t2)*complex<double>(1.0-t2,(2.0*t));
  }

  complex<double> giac_gnuwince_log(const complex<double> & c){
    double r=c.real(),i=c.imag();
    return complex<double>(giac_gnuwince_log(r*r+i*i)/2.0,giac_gnuwince_atan2(r,i));
  }

  complex<double> giac_gnuwince_sqrt(const complex<double> & c){
    double x=c.real(),y=c.imag();
    if (y==0)
      return complex<double>(giac_gnuwince_sqrt(x),0);
    double delta=giac_gnuwince_hypot(x,y);
    double r=giac_gnuwince_sqrt((delta+x)/2.0);
    double i=giac_gnuwince_sqrt((delta-x)/2.0);
    return complex<double>(r,i);
  }

  complex<double> giac_gnuwince_sin(const complex<double> & c){
    complex<double> z=giac_gnuwince_exp(complex<double>(-c.imag(),c.real()));
    return (z-1.0/z)/2.0;
  }

  complex<double> giac_gnuwince_cos(const complex<double> & c){
    complex<double> z=giac_gnuwince_exp(complex<double>(-c.imag(),c.real()));
    return (z+1.0/z)/2.0;
  }

  complex<double> giac_gnuwince_tan(const complex<double> & c){
    complex<double> z=giac_gnuwince_exp(complex<double>(-c.imag(),c.real()));
    z=z*z;
    return (z-1.0)/(z+1.0);
  }

  complex<double> giac_gnuwince_sinh(const complex<double> & c){
    complex<double> z=giac_gnuwince_exp(c);
    return (z-1.0/z)/2.0;
  }

  complex<double> giac_gnuwince_cosh(const complex<double> & c){
    complex<double> z=giac_gnuwince_exp(c);
    return (z+1.0/z)/2.0;
  }

  complex<double> giac_gnuwince_tanh(const complex<double> & c){
    complex<double> z=giac_gnuwince_exp(c);
    z=z*z;
    return (z-1.0)/(z+1.0);
  }


} // end namespace std

#ifdef LINK
using namespace std;
int main(){
  ofstream of("res.txt");
  of << setprecision(15) ;
  for (double x=-8.3;x<10.0;x=x+1){
    of << "Exp" << x << char(10) << char(13) ;
    of << std::exp(x) << " " << giac_gnuwince_exp(x) << char(10) << char(13) ;
  }
  for (double x=-8.3;x<10.0;x=x+1){
    of << "Tan" << x << char(10) << char(13) ;
    of << std::tan(x) << " " << giac_gnuwince_tan(x) << char(10) << char(13) ;
  }
  for (double x=-8.3;x<10.0;x=x+1){
    of << "Sin" << x << char(10) << char(13) ;
    of << std::sin(x) << " " << giac_gnuwince_sin(x) << char(10) << char(13) ;
  }
  for (double x=-8.3;x<10.0;x=x+1){
    of << "Cos" << x << char(10) << char(13) ;
    of << std::cos(x) << " " << giac_gnuwince_cos(x) << char(10) << char(13) ;
  }
  for (double x=-8.3;x<10.0;x=x+1){
    of << "ATan" << x << char(10) << char(13) ;
    of << std::atan(x) << " " << giac_gnuwince_atan(x) << char(10) << char(13) ;
  }
  for (double x=1e-10;x<1e10;x*=10){
    of << "Natural log" << x << char(10) << char(13) ;
    of << std::log(x) << " " << giac_gnuwince_log(x) << char(10) << char(13) ;
  }
  for (double x=1e-10;x<1e10;x*=10){
    of << "Sqrt " << x << char(10) << char(13) ;
    of << std::sqrt(x) << " " << giac_gnuwince_sqrt(x) << char(10) << char(13) ;
  }
  for (double x=-1;x<1;x+=0.12345){
    of << "Asin " << x << char(10) << char(13) ;
    of << std::asin(x) << " " << giac_gnuwince_asin(x) << char(10) << char(13) ;
  }
}
#endif // LINK
