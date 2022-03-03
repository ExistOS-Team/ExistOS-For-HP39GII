/*
    SymbolicC++ : An object oriented computer algebra system written in C++

    Copyright (C) 2008 Yorick Hardy and Willi-Hans Steeb

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "verylong.h"


// Class Data

const Verylong Verylong::zero = Verylong("0");

const Verylong Verylong::one = Verylong("1");

const Verylong Verylong::two = Verylong("2");

// Constructors, Destructors and Conversion operators.

Verylong::Verylong(const string &value)
{
   string s = (value == "") ? "0" : value;

   vlsign = (s[0] == '-') ? 1:0;        // check for negative sign
   if(ispunct(s[0]))                    // if the first character
     vlstr = s.substr(1,s.length()-1);  // is a punctuation mark.
   else vlstr = s;
}

Verylong::Verylong(int n)
{
   if(n < 0) { vlsign = 1; n = (-n); } // check for sign and convert the
   else vlsign = 0;                    // number to positive when negative

   if(n > 0) 
     while(n >= 1)           // extract the number digit by digit and store 
     {                       // internally
       vlstr = char(n%10 + '0') + vlstr;
       n /= 10;
     }
   else vlstr = string("0"); // else number is zero
}

Verylong::Verylong(const Verylong &x) : vlstr(x.vlstr),vlsign(x.vlsign) { }

Verylong::~Verylong() { }

Verylong::operator int() const
{
   int number, factor = 1;
   static Verylong max0(numeric_limits<int>::max());
   static Verylong min0(numeric_limits<int>::min()+1);
   string::const_reverse_iterator j=vlstr.rbegin();

   if(*this > max0)
   {
     cerr << "Error: Conversion Verylong->integer is not possible" << endl;
     return numeric_limits<int>::max();
   }
   else if(*this < min0)
   {
     cerr << "Error: Conversion Verylong->integer is not possible" << endl;
     return numeric_limits<int>::min();
   }

   number = *j - '0';

   for(j++;j!=vlstr.rend();j++)
   {
     factor *= 10;
     number += (*j-'0') * factor;
   }

   if(vlsign) return -number;
   return number;
}
   
Verylong::operator double() const
{
   double sum, factor = 1.0;
   string::const_reverse_iterator i = vlstr.rbegin();

   sum = double(*i) - '0';
   for(i++;i!=vlstr.rend();i++)
   {
     factor *= 10.0;
     sum += double(*i-'0') * factor;
   }
   if(vlsign) return -sum;
   return sum;
}

Verylong::operator string () const
{
   if(vlstr.length() == 0) return string("0");
   return vlstr;
}

// Various member operators

const Verylong & Verylong::operator = (const Verylong &rhs)
{
   if(this == &rhs) return *this;

   vlstr = rhs.vlstr;
   vlsign = rhs.vlsign;

   return *this;
}

// Unary - operator
Verylong Verylong::operator -() const
{
    Verylong temp(*this);
    if(temp != zero)  temp.vlsign = !vlsign;
    return temp;
}

// Prefix increment operator
Verylong Verylong::operator ++ ()
{ return *this = *this + one; }

// Postfix increment operator
Verylong Verylong::operator ++ (int)
{
   Verylong result(*this);
   *this = *this + one;
   return result;
}

// Prefix decrement operator
Verylong Verylong::operator -- ()
{ return *this = *this - one; }

// Postfix decrement operator
Verylong Verylong::operator -- (int)
{
   Verylong result(*this);
   *this = *this - one;
   return result;
}

Verylong Verylong::operator += (const Verylong &v)
{  return *this = *this + v; }

Verylong Verylong::operator -= (const Verylong &v)
{  return *this = *this - v; }

Verylong Verylong::operator *= (const Verylong &v)
{  return *this = *this * v; }

Verylong Verylong::operator /= (const Verylong &v)
{  return *this = *this / v; }

Verylong Verylong::operator %= (const Verylong &v)
{  return *this = *this % v; }

// Various friendship operators and functions.

Verylong operator + (const Verylong &u,const Verylong &v)
{
    char digitsum, d1, d2, carry = 0;
    string temp;
    string::const_reverse_iterator j, k;

    if(u.vlsign ^ v.vlsign)
    {
      if(u.vlsign == 0) return u-abs(v);
      else               return v-abs(u);
    }

    for(j=u.vlstr.rbegin(),  k=v.vlstr.rbegin();
        j!=u.vlstr.rend() || k!=v.vlstr.rend();)
    {
      d1 = (j == u.vlstr.rend()) ? 0 : *(j++)-'0'; // get digit
      d2 = (k == v.vlstr.rend()) ? 0 : *(k++)-'0'; // get digit
      digitsum = d1 + d2 + carry;                  // add digits
      carry = (digitsum >= 10) ? 1 : 0;
      digitsum -= 10*carry;
      temp = char(digitsum+'0') + temp;
    }
    if(carry) temp = '1' + temp;  // if carry at end, last digit is 1
    if(u.vlsign) temp = '-' + temp;
    return Verylong(temp);
}

Verylong operator - (const Verylong &u,const Verylong &v)  
{
    char d, d1, d2, borrow = 0;
    int negative;
    string temp, temp2;
    string::reverse_iterator i, j;
    if(u.vlsign ^ v.vlsign)
    {
      if(u.vlsign == 0) return u+abs(v);
      else              return -(v+abs(u));
    }
    Verylong w, y;
    if(u.vlsign == 0)  // both u,v are positive
     if(u<v) { w=v; y=u; negative=1;}
     else    { w=u; y=v; negative=0;}
    else               // both u,v are negative
     if(u<v) { w=u; y=v; negative=1;}
     else    { w=v; y=u; negative=0;}

    for(i=w.vlstr.rbegin(),  j=y.vlstr.rbegin();
        i!=w.vlstr.rend() || j!=y.vlstr.rend();)
    {
      d1 = (i == w.vlstr.rend()) ? 0:*(i++)-'0';
      d2 = (j == y.vlstr.rend()) ? 0:*(j++)-'0';
      d = d1 - d2 - borrow;
      borrow = (d < 0) ? 1 : 0;
      d += 10 * borrow;
      temp = char(d+'0') + temp;
    }

    while(temp[0] == '0') temp = temp.substr(1);
    if(negative) temp = '-' + temp;
    return Verylong(temp);
}

Verylong operator * (const Verylong &u,const Verylong &v)
{
    Verylong pprod("1"), tempsum("0");
    string::const_reverse_iterator r = v.vlstr.rbegin();

    for(int j=0;r!=v.vlstr.rend();j++,r++)
    {
      int digit = *r - '0';           // extract a digit
      pprod = u.multdigit(digit);     // multiplied by the digit
      pprod = pprod.mult10(j);        // "adds" suitable zeros behind
      tempsum = tempsum + pprod;      // result added to tempsum
    }
    tempsum.vlsign = u.vlsign^v.vlsign; // to determine sign
    return tempsum;
}

//  This algorithm is the long division algorithm.
Verylong operator / (const Verylong &u,const Verylong &v)
{
    int len = u.vlstr.length() - v.vlstr.length();
    string temp;
    Verylong w,y,b,c,d,quotient=Verylong::zero;

    if(v == Verylong::zero) 
    {
      cerr << "Error : division by zero" << endl;
      return Verylong::zero;
    }

    w = abs(u); y = abs(v);
    if(w < y) return Verylong::zero;
    c = Verylong(w.vlstr.substr(0,w.vlstr.length()-len));

    for(int i=0;i<=len;i++)
    {
      quotient = quotient.mult10(1);
      b = d = Verylong::zero;         // initialize b and d to 0
      while(b < c) { b = b + y; d = d + Verylong::one; }
      if(c < b)                       // if b>c, then
      {                               // we have added one count too many 
        b = b - y; 
        d = d - Verylong::one;
      }
      quotient = quotient + d;        // add to the quotient
      if(i < len)
      {
        // partial remainder * 10 and add to next digit
        c = (c-b).mult10(1);
        c += Verylong(w.vlstr[w.vlstr.length()-len+i]-'0');
      }
    }
    quotient.vlsign = u.vlsign^v.vlsign;  // to determine sign
    return quotient;
}

Verylong operator % (const Verylong &u,const Verylong &v)
{ return (u - v*(u/v)); }

int operator == (const Verylong &u,const Verylong &v)
{ return (u.vlsign==v.vlsign && u.vlstr==v.vlstr); }

int operator != (const Verylong &u,const Verylong &v)  
{ return !(u==v); }

int operator < (const Verylong &u,const Verylong &v)
{
    if      (u.vlsign < v.vlsign) return 0;
    else if(u.vlsign > v.vlsign) return 1;
    // exclusive or (^) to determine sign
    if      (u.vlstr.length() < v.vlstr.length()) return (1^u.vlsign);
    else if(u.vlstr.length() > v.vlstr.length()) return (0^u.vlsign);
    return (u.vlstr < v.vlstr && !u.vlsign) || (u.vlstr > v.vlstr && u.vlsign);
}

int operator <= (const Verylong &u,const Verylong &v)
{ return (u<v || u==v); }

int operator > (const Verylong &u,const Verylong &v)
{ return (!(u<v) && u!=v); }

int operator >= (const Verylong &u,const Verylong &v)
{ return (u>v || u==v); }

// Calculate the absolute value of a number
Verylong abs(const Verylong &v)
{
   Verylong u(v);
   if(u.vlsign) u.vlsign = 0;
   return u;
}

// Calculate the integer square root of a number
// based on the formula (a+b)^2 = a^2 + 2ab + b^2
Verylong sqrt(const Verylong &v)
{
   // if v is negative, error is reported
   if(v.vlsign) {cerr << "NaN" << endl; return Verylong::zero; }

   int j, k = v.vlstr.length()+1, num = k >> 1;
   Verylong y, z, sum, tempsum, digitsum;
   string temp, w(v.vlstr);

   k = 0;
   j = 1;

   // segment the number 2 digits by 2 digits
   if(v.vlstr.length() % 2) digitsum = Verylong(w[k++] - '0');
   else            
   { 
     digitsum = Verylong((w[k] - '0')*10 + w[k+1] - '0'); 
     k += 2;
   }

   // find the first digit of the integer square root
   sum = z = Verylong(int(sqrt(double(digitsum))));
   // store partial result
   temp = char(int(z) + '0');
   digitsum = digitsum - z*z;

   for(;j<num;j++)
   {
     // get next digit from the number
     digitsum = digitsum.mult10(1) + Verylong(w[k++] - '0');
     y = z + z;        // 2*a
     z = digitsum/y;
     tempsum = digitsum.mult10(1) + Verylong(w[k++] - '0');
     digitsum = -y*z.mult10(1) + tempsum - z*z;
     // decrease z by 1 and re-calculate when it is over-estimated.
     while(digitsum < Verylong::zero)
     {
       --z;
       digitsum = -y*z.mult10(1) + tempsum - z*z;
     }
     temp = temp + char(int(z) + '0');// store partial result
     z = sum = sum.mult10(1) + z;     // update value of the partial result
   }
   Verylong result(temp);
   return result;
}

// Raise a number X to a power of degree
Verylong pow(const Verylong &X,const Verylong &degree)
{
   Verylong N(degree), Y("1"), x(X);

   if(N == Verylong::zero) return Verylong::one;
   if(N < Verylong::zero) return Verylong::zero;

   while(1)
   {
     if(N%Verylong::two != Verylong::zero)
     {
       Y = Y * x;
       N = N / Verylong::two;
       if(N == Verylong::zero) return Y;
     }
     else  N = N / Verylong::two;
     x = x * x;
   }
}

// Double division function
double div(const Verylong &u,const Verylong &v)
{
   double qq = 0.0, qqscale = 1.0;
   Verylong w,y,b,c;
   int d, count;
   // number of significant digits
   int decno = numeric_limits<double>::digits;

   if(v == Verylong::zero) 
   {
     cerr << "ERROR : Division by zero" << endl;
     return 0.0;
   }
   if(u == Verylong::zero) return 0.0;

   w=abs(u); y=abs(v);
   while(w<y) { w = w.mult10(1); qqscale *= 0.1; }

   int len = w.vlstr.length() - y.vlstr.length();
   string temp = w.vlstr.substr(0,w.vlstr.length()-len);
   c = Verylong(temp);

   for(int i=0;i<=len;i++)
   {
     qq *= 10.0;
     b = Verylong::zero; d = 0;   // initialize b and d to 0
     while(b < c) { b += y; d += 1;}

     if(c < b) { b -= y; d -= 1;} // if b>c, then we have added one too many
     qq += double(d);             // add to the quotient
     c = (c-b).mult10(1);         // the partial remainder * 10
     if(i < len)                  // and add to next digit
        c += Verylong(w.vlstr[w.vlstr.length()-len+i]-'0');
   }
   qq *= qqscale; count = 0;

   while(c != Verylong::zero && count < decno)
   {
     qqscale *= 0.1;
     b = Verylong::zero; d = 0;   // initialize b and d to 0
     while(b < c) { b += y; d += 1;}
     if(c < b) { b -= y; d -= 1;} // if b>c, then we have added one too many
     qq += double(d)*qqscale;
     c = (c-b).mult10(1);
     count++;
   }
   if(u.vlsign^v.vlsign) qq *= (-1.0); // check for the sign
   return qq;
}

ostream & operator << (ostream &s,const Verylong &v)
{
    if(v.vlstr.length() > 0) { if(v.vlsign) s << "-"; s << v.vlstr; }
    else s << "0";
    return s;
}

istream & operator >> (istream &s,Verylong &v)
{
   string temp(10000, ' ');
   s >> temp;
   v = Verylong(temp);
   return s;
}


// Private member functions: multdigit(), mult10().

// Multiply this Verylong number by num
Verylong Verylong::multdigit(int num) const
{
    int carry = 0;
    string::const_reverse_iterator r;

    if(num)
    {
      string temp;
      for(r=vlstr.rbegin();r!=vlstr.rend();r++)
      {
        int d1 = *r - '0',               // get digit and multiplied by
            digitprod = d1*num + carry;  // that digit plus carry
        if(digitprod >= 10)              // if there's a new carry,
        {
          carry = digitprod/10;          // carry is high digit
          digitprod -= carry*10;         // result is low digit
        }
        else carry = 0;                  // otherwise carry is 0
        temp = char(digitprod + '0') + temp;   // insert char in string
       }
       if(carry) temp = char(carry + '0') + temp; //if carry at end,
       Verylong result(temp);
       return result;
   }
   else return zero;
}

// Multiply this Verylong number by 10*num
Verylong Verylong::mult10(int num) const
{
    if(*this != zero)
    {
      string temp;
      for(int j=0;j<num;j++) temp = temp + '0';
      Verylong result(vlstr+temp);
      if(vlsign) result = -result;
      return result;
    }
    else return zero;
}

template <> Verylong zero(Verylong) { return Verylong::zero; }
template <> Verylong one(Verylong) { return Verylong::one; }

