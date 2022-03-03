[],
"Warning: some commands like subs might change arguments order",
0,0,0,1,0,1e-10,10,[1,50,0,25],0,0,0,
GF(3,k^6+2*k^5+2,[k,K,a],undef),
(-a^4-a^3-a^2),
1*x^3-1*x+1,
(x+(-a^4-a^3+a^2+a))*(x+(-a^4-a^3+a^2+a+1))*(x+(-a^4-a^3+a^2+a-1)),
x^3-1 % 3*x+a,
[1*x*(1*x+1)*(1*x-1)*(1*x^5-1*x+1)*(1*x^5-1*x-1)*(1*x^5+1*x^4-1)*(1*x^5-1*x^4+1)*(1*x^5+1*x^2+1*x-1)*(1*x^5-1*x^2+1*x+1)*(1*x^5+1*x^3+1*x+1)*(1*x^5+1*x^3+1*x-1)*(1*x^5+1*x^3+1*x^2-1)*(1*x^5+1*x^3-1*x^2+1)*(1*x^5-1*x^3+1*x^2+1)*(1*x^5-1*x^3-1*x^2-1)*(1*x^5+1*x^4+1*x-1)*(1*x^5+1*x^4-1*x+1)*(1*x^5+1*x^4+1*x^2+1)*(1*x^5+1*x^4-1*x^3+1)*(1*x^5-1*x^4+1*x+1)*(1*x^5-1*x^4-1*x-1)*(1*x^5-1*x^4-1*x^2-1)*(1*x^5-1*x^4-1*x^3-1)*(1*x^5+1*x^3+1*x^2-1*x-1)*(1*x^5+1*x^3-1*x^2-1*x+1)*(1*x^5-1*x^3+1*x^2+1*x-1)*(1*x^5-1*x^3+1*x^2-1*x-1)*(1*x^5-1*x^3-1*x^2+1*x+1)*(1*x^5-1*x^3-1*x^2-1*x+1)*(1*x^5+1*x^4+1*x^2+1*x+1)*(1*x^5+1*x^4+1*x^2-1*x-1)*(1*x^5+1*x^4+1*x^3+1*x+1)*(1*x^5+1*x^4-1*x^3-1*x-1)*(1*x^5+1*x^4-1*x^3+1*x^2-1)*(1*x^5+1*x^4-1*x^3-1*x^2+1)*(1*x^5+1*x^4-1*x^3-1*x^2-1)*(1*x^5-1*x^4-1*x^2+1*x-1)*(1*x^5-1*x^4-1*x^2-1*x+1)*(1*x^5-1*x^4+1*x^3+1*x-1)*(1*x^5-1*x^4-1*x^3-1*x+1)*(1*x^5-1*x^4-1*x^3+1*x^2+1)*(1*x^5-1*x^4-1*x^3+1*x^2-1)*(1*x^5-1*x^4-1*x^3-1*x^2+1)*(1*x^5+1*x^4+1*x^3+1*x^2-1*x+1)*(1*x^5+1*x^4+1*x^3-1*x^2+1*x+1)*(1*x^5+1*x^4+1*x^3-1*x^2+1*x-1)*(1*x^5+1*x^4-1*x^3+1*x^2+1*x+1)*(1*x^5-1*x^4+1*x^3+1*x^2+1*x+1)*(1*x^5-1*x^4+1*x^3+1*x^2+1*x-1)*(1*x^5-1*x^4+1*x^3-1*x^2-1*x-1)*(1*x^5-1*x^4-1*x^3-1*x^2+1*x-1)],
GF(3,k^20-k^19-k^18-k^17-k^16+k^12-k^11-k^10+k^9+k^8+k^7-k^6+k-1,[k,L,b],undef),
(1*X^4+1*X^2-1)*(1*X^4-1*X^2-1),
{1,2,3,4},
{2,4},
{ },
proc(n) 
  local a,i,j,k,l,o,liste; 
  liste:=[];  
  if irem(n,3)=0 then print("Erreur: 3 divise",n); else  
    l:={seq(i,i=(0 .. (n-1)))}; 
    j:=1; 
    while l<>{ } do  
      i:=l[1]; 
      o:={i}; 
      a:=irem(3*i,n); 
      while a<>i do  
        o:=o union {a}; 
        a:=irem(3*a,n); 
       od;; 
      l:=l minus o; 
      liste:=[op(liste),o]; 
     od; 
  fi ;  
  liste;  
 
end;,
(1*X+1)*(1*X-1)*(1*X^2+1)*(1*X^2+1*X-1)*(1*X^2-1*X-1)*(1*X^4+1*X^2-1)*(1*X^4-1*X^2-1)*(1*X^8+1*X^4-1)*(1*X^8-1*X^4-1),
[{0},{1,3,9,27,17,19,25,11},{2,6,18,22},{4,12},{5,15,13,7,21,31,29,23},{8,24},{10,30,26,14},{16},{20,28}],
(1*X+1)*(1*X-1)*(1*X^6+1*X^5+1*X^4+1*X^3+1*X^2+1*X+1)*(1*X^6-1*X^5+1*X^4-1*X^3+1*X^2-1*X+1),
[{0},{1,3,9,13,11,5},{2,6,4,12,8,10},{7}],
0,
0,
 (n)->X^(2^(n-1))+1,
0,
0,
(1*X^64+1*X^32-1)*(1*X^64-1*X^32-1),
X^64+X^32-1,
1*X^64+1*X^32-1,
x^64+x^32-1,
proc(g,n) 
  local u,v; 
  u:=1;  
  v:=g;  
  while n>1 do if irem(n,2)=0 then  
      v:=irem(Rem(v*v,P),3); 
      n:=n/2 else  
      u:=irem(Rem(u*v,P),3); 
      v:=irem(Rem(v*v,P),3); 
      n:=(n-1)/2 
    fi ; 
   od;;  
  irem(Rem(u*v,P),3);  
 
end;,
1*x^61+1*x^60-1*x^59+1*x^58+0*x^57-1*x^56+1*x^55+0*x^54-1*x^53+0*x^52+0*x^51-1*x^49+0*x^48+1*x^47-1*x^46+1*x^45-1*x^44+1*x^43+1*x^42+1*x^41+1*x^40-1*x^39+0*x^38+0*x^37+0*x^36+0*x^35+0*x^34-1*x^33+1*x^32-1*x^31+1*x^30-1*x^29+1*x^28-1*x^27-1*x^25-1*x^24+1*x^23-1*x^22-1*x^21+1*x^20-1*x^19+1*x^17+0*x^16+0*x^15+1*x^13-1*x^12+1*x^11-1*x^10-1*x^9-1*x^8+1*x^7+1*x^6-1*x^5+1*x^4-1*x^3+1*x^2-1*x+1,
 (g,n)->powmod(g,n,3,P,x),
x^61+x^60-x^59+x^58-x^56+x^55-x^53-x^49+x^47-x^46+x^45-x^44+x^43+x^42+x^41+x^40-x^39-x^33+x^32-x^31+x^30-x^29+x^28-x^27-x^25-x^24+x^23-x^22-x^21+x^20-x^19+x^17+x^13-x^12+x^11-x^10-x^9-x^8+x^7+x^6-x^5+x^4-x^3+x^2-x+1,
3433683820292512484657849089281,
13412827423017626893194723005,
proc(g) 
  evalb(puiss(g,(q-1)/2)=1);  
 
end;,
1,
0,
x^63-x^31,
[],
[x^63-x^31,-x^62,-x^60-x^28,x^56+x^24,x^48+x^16,x^32+1,x^32-1,-1,1],
proc(v) 
  puiss(v,q-2);  
 
end;,
1+x,
1,
[-107,5606142711964398740514981881],
-x^50-x^18,
-x^15-x^14,
1*x+1,
1+x,
-x^50-x^18,
1*x+1,
1+x,
2,
x^63+x^60+x^59+x^57+x^56+x^55-x^54-x^53+x^52+x^51-x^50-x^49-x^48+x^47-x^46-x^45+x^44+x^39-x^37+x^36-x^35+x^33+x^32-x^31+x^30+x^28+x^27-x^26-x^25-x^24+x^22-x^21-x^20-x^18-x^16+x^15+x^13+x^12+x^11+x^9-x^8+x^6+x^5+x^4+x^3+x^2+x,
-x^15-x^14,
-x^15-x^14,
[0,0,0,0,0,0,0,0],
-x^50-x^18,
1,
1,
"Done",
-x^50-x^18,
-x^50-x^18,
1,
"Done",
-x^50-x^18,
-x^50-x^18,
-x^63+x^62+x^60-x^59+x^57+x^55-x^53-x^52-x^51+x^48+x^46+x^44+x^43-x^42+x^41-x^40-x^39-x^37+x^36+x^34-x^33+x^32+x^30-x^29+x^28-x^26-x^25-x^24-x^23+x^19-x^18-x^17+x^16+x^15-x^13+x^12-x^10-x^7+x^6-x^4-x^2+x+1,
x+1,
1+x,
[[2,2],[3,2],[7,1]],
[[2,2],[3,2],[7,1]],
[2,3,7],
proc(x,n) 
  local m,l,p,y; 
  m:=Phi(n);  
  l:=(maple_ifactors(m))[2];  
  for i from 1 to rowdim(l) do  
  m:=iquo(m,(l[i,1])^(l[i,2])); 
  y:=powmod(x,m,n); 
  while y<>1 do  
    y:=powmod(y,l[i,1],n); 
    m:=m*l[i,1]; 
   od;; 
   od;;  
  m;  
 
end;,
2,
"All PARI functions are now defined with the pari_ prefix.
PARI functions are also defined without prefix except:
% abs acos acosh apply arg asin asinh atan atanh binomial bitand bitor bitxor break ceil charpoly concat conj content cos cosh default divisors erfc eval exp factor factorial floor frac gcd global hilbert imag isprime kill lcm length local matrix max min next nextprime norm print real round select shift sign simplify sin sinh solve sqrt subst sum tan tanh taylor trace truncate type until valuation vector version write 
When working with p-adic numbers use them in a pari() call
Type ?pari for short help
Inside xcas, try Help->Manuals->PARI for HTML help",
0
