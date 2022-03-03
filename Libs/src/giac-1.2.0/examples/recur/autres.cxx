//courbe de hilbert par exemple hilg(-2,0,4) est 
//compos\'e par 4 morceaux hilg hild berg et berd
//ou hild(0,2,4) ou berg(2,2-2*i,4) ou berd(2,2-2*i,4)
//le morceau 1
hilg(x,y,n):={
local u,v,a,b;
DispG();
if (n==0) {segment(x,y);return 0;}
u:=(y-x)/2;
v:=u*i;
hild(x,x+v,n-1);
hilg(x+v,x+v+u,n-1);
a:=berg(x+v+u,x+u,n-1);
b:=berd(a,a+u,n-1);
};
//le morceau 2
hild(x,y,n):={
local u,v,a,b;
DispG();
if (n==0) {segment(x,y);return 0;}
u:=(y-x)/2;
v:=u*i;
hilg(x,x-v,n-1);
hild(x-v,x-v+u,n-1);
b:=berd(x-v+u,x+u,n-1);
a:=berg(b,b+u,n-1);
};
//le morceau 3
berg(x,y,n):={
local u,v,b;
DispG();
if (n==0) {segment(x,y);return y;}
v:=(x-y)/2;
u:=-v*i;hild(x,x+v,n-1);
hilg(x+v,x+v+u,n-1);
b:=berg(x+v+u,x+u,n-1);
hild(b,b-v,n-1);
return(b-v);
};
//le morceau 4
berd(x,y,n):={
local u,v,a;
DispG();
if (n==0) {segment(x,y);return y;}
v:=(x-y)/2;
u:=-v*i;
hilg(x,x+v,n-1);
hild(x+v,x+v-u,n-1);
a:=berd(x+v-u,x-u,n-1);
hilg(a,a-v,n-1);
return a-v;
};
//gosper(-2-2*i,2-2*i,2)ou gosper(-2-2*i,2-2*i,3)
gosper(x,y,n):={
local a,b,c,d,f,g;
DispG();
if (n==0) return segment(x,y);
a:=x+(y-x)/sqrt(7)*exp(evalf(-i*acos(5*sqrt(7)/14)));
c:=x+(a-x)*exp(evalf(i*pi/3));
b:=c+a-x;
d:=c+(a-x)*exp(evalf(2*i*pi/3));
f:=d+2*(a-x);
g:=(d+f)/2;
gosper(x,a,n-1);
gosper(b,a,n-1);
gosper(c,b,n-1);
gosper(c,d,n-1);
gosper(d,g,n-1);
gosper(g,f,n-1);
gosper(y,f,n-1);
};

