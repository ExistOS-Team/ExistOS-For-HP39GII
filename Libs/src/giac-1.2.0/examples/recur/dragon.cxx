// dessine un dragon dragong(-i,2+i,10)
//x=a,y=b et d=u
 dragong(x,y,n):={
local u;
DispG();
if (n==0){segment(x,y);return 0;}
u:=(y+i*x)*(1-i)/2;
dragong(x,u,n-1);
dragond(u,y,n-1);
};
// dessine un dragon dragond(-i,2+i,10)
//x=a,y=b et c=u
dragond(x,y,n):={
local u;
DispG();
if (n==0){segment(x,y);return 0;}
u:=(y-i*x)*(1+i)/2;
dragong(x,u,n-1);
dragond(u,y,n-1);
};
dragonpapierg(x,y,n):={
local u,v,a,b;
DispG();
if (n==0){segment(x,y); return y;}
u:=x+(y-x)/2;
a:=dragonpapierg(x,u,n-1);
v:=a+(y-x)*i/2;
b:=dragonpapierd(a,v,n-1);
return b
};
dragonpapierd(x,y,n):={
local u,v,a,b;
DispG();
if (n==0){segment(x,y); return y;}
v:=x+(y-x)*i/2;
b:=dragonpapierg(x,v,n-1);
u:=a+(y-x)/2;
a:=dragonpapierd(b,u,n-1);
return a
};
dragonpaperg(x,y,n):={
local u,v,a,b;
DispG();
if (n==0){segment(x,y); return (x,y);}
u:=x+(y-x)/2;
a:=dragonpaperg(x,u,n-1);
v:=a[1]+abs((y-x)/(a[1]-a[0]))*(a[1]-a[0])*i/2;
b:=dragonpaperd(a[1],v,n-1);
return b
};
dragonpaperd(x,y,n):={
local u,v,a,b;
DispG();
if (n==0){segment(x,y); return (x,y);}
u:=x+(y-x)/2;
a:=dragonpaperg(x,u,n-1);
v:=a[1]-abs((y-x)/(a[1]-a[0]))*(a[1]-a[0])*i/2;
b:=dragonpaperd(a[1],v,n-1);
return b
};
