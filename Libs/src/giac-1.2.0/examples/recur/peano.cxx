//arc qui remplit le carre de cote x,y 
peano(x,y,n):={
local u,v;
DispG();
if (n==0) {segment(x,y+3*v);return 0;}
u:=(y-x)/3;
v:=i*u;
peano(x,x+u,n-1);
peano(x+u+v,x+u,n-1);
peano(x+2*u,y,n-1);
peano(y+v,y+2*v,n-1);
peano(x+2*(u+v),x+u+2*v,n-1);
peano(x+(u+v),x+u+2*v,n-1);
peano(x+2*v,x+2*v+u,n-1);
peano(x+3*v+u,x+u+2*v,n-1);
peano(x+2*(u+v),y+2*v,n-1);
};
//arc x y  de mesure +pi/3
arcg(x,y):={
return arc(x,y,pi/3);
};
//arc x y de de mesure -pi/3
arcd(x,y):={
return arc(x,y,-pi/3);
};
// courbe de peano avec 13 arcs
//par ex peanod(-2-2*i,2-2*i,3)
peanod(x,y,n):={
local c1,b,c,d,e1,f,g,h,i1,j,k,l,m;
DispG();
if (n==0) {arc(x,y,-pi/3);return 0;}
c1:=x+(y-x)*exp(evalf(pi)*2*i/3)/3;
b:=x+(y-x)/3*exp(evalf(pi)*i/3);
c:=x+(y-x)/3*2*exp(evalf(pi)*i/3);
d:=c+(y-x)/3;
e1:=b+2*(y-x)/3;
f:=c1+(y-x)*(15+i*sqrt(3))/18;
g:=c1+(y-x)*(6+i*sqrt(3))/9;
h:=f-(y-x)/3;
i1:=h-i*(y-x)/9*sqrt(3);
j:=i1+(y-x)/3;
k:=g-i*2*(y-x)/9*sqrt(3);
l:=x+(y-x)/3;
m:=x+2*(y-x)/3;
peanog(x,b,n-1);
peanod(b,c,n-1);
peanod(c,d,n-1);
peanod(d,e1,n-1);
peanod(e1,f,n-1);
peanog(f,g,n-1);
peanog(g,h,n-1);
peanog(h,i1,n-1);
peanod(i1,j,n-1);
peanod(j,k,n-1);
peanog(k,l,n-1);
peanog(l,m,n-1);
peanod(m,y,n-1);
};
peanog(x,y,n):={
local c1,b,c,d,e1,f,g,h,i1,j,k,l,m;
DispG();
if (n==0) {
arc(x,y,pi/3);return 0;}
c1:=x+(y-x)*exp(evalf(-2*pi)*i/3)/3;
b:=x+(y-x)/3*exp(evalf(-pi)*i/3);
c:=x+(y-x)/3*2*exp(evalf(-pi)*i/3);
d:=c+(y-x)/3;
e1:=b+2*(y-x)/3;
f:=c1+(y-x)*(15-i*sqrt(3))/18;
g:=c1+(y-x)*(6-i*sqrt(3))/9;
h:=f-(y-x)/3;
i1:=h+i*(y-x)/9*sqrt(3);
j:=i1+(y-x)/3;
k:=g+i*2*(y-x)/9*sqrt(3);
l:=x+(y-x)/3;
m:=x+2*(y-x)/3;
peanod(x,b,n-1);
peanog(b,c,n-1);
peanog(c,d,n-1);
peanog(d,e1,n-1);
peanog(e1,f,n-1);
peanod(f,g,n-1);
peanod(g,h,n-1);
peanod(h,i1,n-1);
peanog(i1,j,n-1);
peanog(j,k,n-1);
peanod(k,l,n-1);
peanod(l,m,n-1);
peanog(m,y,n-1);
};
bases(x,y):={
local a,b,c,d,e1,f,g,h,i1,k;
DispG();
h:=(y-x)/3;
a:=x+h/2+i*h/2;
b:=a+i*h;
c:=b+i*h;
d:=c+h
e1:=b+h;
f:=a+h;
g:=f+h;
k:=e1+h;
i1:=d+h;
arc(a,b,pi/2);
arc(c,b,pi/2);
arc(d,c,pi/2);
arc(e1,d,pi/2);
arc(e1,f,pi/2);
arc(f,g,pi/2);
arc(g,k,pi/2);
arc(i1,k,pi/2);
};
//un arc et le motif de base
un(x,y):={
local h,a;
DispG();
h:=(y-x)/3;
a:=x+h/2+i*h/2;
arc(a-h,a,pi/2);
bases(x,y);
};
//un  autre arc et le motif de base
deux(x,y):={
local h,a;
DispG();
h:=(y-x)/3;
a:=x+h/2+i*h/2;
arc(a,a-h*i,pi/2);
bases(x,y);
};

//courbe qui remplit un carre debute par le motif bases
// ex peano0(-1,2,1) ou  peano0(-1,2,3)  
// utilise bases un deux peano1 peano2
peano0(x,y,n):={
local a,h;
DispG();
if (n==0) {bases(x,y);return 0;}
h:=(y-x)/3;
a:=x+h;
peano0(x,a,n-1);
peano2(a+i*h,a,n-1);
peano1(a+h,y,n-1);
peano1(y+i*h,y+2*i*h,n-1);
peano1(a+h+2*i*h,a+2*i*h,n-1);
peano2(a+i*h,a+2*i*h,n-1);
peano2(x+2*i*h,a+2*i*h,n-1);
peano2(a+3*i*h,a+2*i*h,n-1);
peano1(a+h+2*i*h,y+2*i*h,n-1);
};
//courbe qui remplit un carre debute par le motif un
// ex peano1(-1,2,1) 
// utilise bases un deux peano2
peano1(x,y,n):={
local a,h;
DispG();
if (n==0) {un(x,y);return 0;}
h:=(y-x)/3;
a:=x+h;
peano1(x,a,n-1);
peano2(a+i*h,a,n-1);
peano1(a+h,y,n-1);
peano1(y+i*h,y+2*i*h,n-1);
peano1(a+h+2*i*h,a+2*i*h,n-1);
peano2(a+i*h,a+2*i*h,n-1);
peano2(x+2*i*h,a+2*i*h,n-1);
peano2(a+3*i*h,a+2*i*h,n-1);
peano1(a+h+2*i*h,y+2*i*h,n-1);
};
//courbe qui remplit un carre debute par le motif deux
// ex peano2(-1,2,1) 
// utilise bases un deux peano1
peano2(x,y,n):={
local a,h;
DispG();
if (n==0) {deux(x,y);return 0;}
h:=(y-x)/3;
a:=x+h;
peano2(x,a,n-1);
peano2(a+i*h,a,n-1);
peano1(a+h,y,n-1);
peano1(y+i*h,y+2*i*h,n-1);
peano1(a+h+2*i*h,a+2*i*h,n-1);
peano2(a+i*h,a+2*i*h,n-1);
peano2(x+2*i*h,a+2*i*h,n-1);
peano2(a+3*i*h,a+2*i*h,n-1);
peano1(a+h+2*i*h,y+2*i*h,n-1);
};
