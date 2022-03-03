[],
"Warning: some commands like subs might change arguments order",
0,0,0,1,0,1e-10,10,[1,50,0,25],0,0,0,
2*x+1,
poly1[2,1],
0,
2*X+1,
X^6-1,
2*X^3+5,
(1*X+1)*(1*X-1)*(1*X+2)*(1*X-2)*(1*X+3)*(1*X-3),
-3*X^3+0*X^2+0*X-3,
1*X^3-1,
1,
"Done",
(1 % 7)*X^6-1 % 7,
((1 % 7)*X+1 % 7)*((1 % 7)*X-1 % 7)*((1 % 7)*X+2 % 7)*((1 % 7)*X-2 % 7)*((1 % 7)*X+3 % 7)*((1 % 7)*X-3 % 7),
(-3 % 7)*X^3-3 % 7,
X^6-1,
2*X^3-2 % 7,
(2*X^3-2 % 7)*(1 % 7),
1/2*X^3-3 % 7,
(-3 % 7)*X^3+(0 % 7)*X^2+(0 % 7)*X-3 % 7,
(1 % 7)*X^3-1 % 7,
"No such variable a","No such variable b","No such variable c","No such variable d","No such variable e",
a*b^2*c^3*d^4*e^6,
a*b*d,
a*b^2*c^3*d^4*e^6,
a*b*d,
a*b*d,
b*c^3*d^3*e^6,
b*d,
c^3*d^2*e^6,
a,
d,
c^3*d*e^6,
b,
d,
c^3*e^6,
1,
c*e^2,
c*e,
c*e,
e,
e,
1,
c,
1,
e,
proc(P,p) 
  local VV,T,V,k,L; 
  T:=irem(Gcd(P,diff(P,x)),p);  
  V:=irem(Quo(P,T),p);  
  V:=irem(Gcd(P,V),p);  
  T:=irem(Quo(P,V),p);  
  L:=[];  
  k:=1;  
  while (degree(V))>0 do  
    VV:=V; 
    V:=irem(Gcd(T,V),p); 
    T:=irem(Quo(T,V),p); 
    if (degree(V))<(degree(VV)) then L:=[op(L),[irem(Quo(VV,V),p),k]]; 
    fi ; 
    k:=k+1; 
   od;;  
  L;  
 
end;,
1*x*(1*x+1)*(1*x-1)*(1*x+2)*(1*x-2)*(1*x+3)*(1*x-3)*(1*x^2+1)*(1*x^2+2)*(1*x^2-3)*(1*x^2+1*x-1)*(1*x^2+1*x+3)*(1*x^2+1*x-3)*(1*x^2-1*x-1)*(1*x^2-1*x+3)*(1*x^2-1*x-3)*(1*x^2+2*x+2)*(1*x^2+2*x-2)*(1*x^2+2*x+3)*(1*x^2-2*x+2)*(1*x^2-2*x-2)*(1*x^2-2*x+3)*(1*x^2+3*x+1)*(1*x^2+3*x-1)*(1*x^2+3*x-2)*(1*x^2-3*x+1)*(1*x^2-3*x-1)*(1*x^2-3*x-2),
x^81+3*x^80+5*x^79+4*x^78+4*x^77+4*x^75+3*x^74+4*x^73+3*x^71+5*x^70+6*x^69+6*x^68+x^67+5*x^66+2*x^64+6*x^63+6*x^62+x^61+3*x^59+5*x^58+4*x^57+2*x^56+2*x^55+x^54+6*x^52+5*x^51+x^50+3*x^49+4*x^48+4*x^47+6*x^45+6*x^44+x^43+6*x^42+4*x^41+5*x^40+5*x^39+4*x^38+x^37+3*x^36+5*x^35+2*x^34+6*x^33+x^32+x^31+6*x^29+x^27+4*x^26+3*x^24+6*x^23+4*x^22+5*x^21+2*x^20+2*x^19+x^18+6*x^17+x^16+x^15+2*x^13+3*x^12+5*x^11+x^10+5*x^9+6*x^8+2*x^7,
(1*x)^7*(1*x+1)*(1*x-1)^4*(1*x+2)^3*(1*x^2-1*x-1)^9*(1*x^2-1*x-3)^7*(1*x^2-2*x-2)^14*(1*x^2+3*x-2)^3,
[[1*x+1,1],[1*x^3-2*x^2-3*x+3,3],[1*x-1,4],[1*x^2-1*x-1,9]],
proc(P,p) 
  local VV,T,V,k,L,j; 
  T:=irem(Gcd(P,diff(P,x)),p);  
  V:=irem(Quo(P,T),p);  
  T:=irem(Quo(P,V),p);  
  L:=[];  
  k:=1;  
  j:=1;  
  while (degree(T))>0 do  
    while (degree(V))>0 do  
      VV:=V; 
      V:=irem(Gcd(T,V),p); 
      T:=irem(Quo(T,V),p); 
      if (degree(V))<(degree(VV)) then L:=[op(L),[irem(Quo(VV,V),p),k]]; 
      fi ; 
      k:=k+j; 
     od;; 
    j:=j*p; 
    k:=j; 
    T:=poly2symb([seq(coeff(T,i*p),i=((degree(T)/p) .. 1))],x); 
    V:=irem(Quo(T,irem(Gcd(T,diff(T,x)),p)),p); 
    T:=irem(Quo(T,V),p); 
   od;;  
  L;  
 
end;,
[[1*x+1,1],[1*x^3-2*x^2-3*x+3,3],[1*x-1,4],[1*x^2-1*x-1,9],[1*x^2-1*x-3,7],[1*x^2-2*x-2,14]]
