// 
sphinxd(x,y):={
local z,u,t;
z:=x+2*(y-x)/3*exp(evalf(pi)*i/3);
t:= y+(x-y)/3*exp(-evalf(pi)*i/3);
u:=t+(x-y)/3;
segment(x,z);
segment(z,u);
segment(u,t);
segment(t,y);
segment(y,x);
};

// Voici un sphinx gauche :
sphinxg(x,y):={
local z,u,t;
z:=y+2*(x-y)/3*exp(-evalf(pi)*i/3);
t:= x+(y-x)/3*exp(evalf(pi)*i/3);
u:=t+(y-x)/3;
segment(y,z);
segment(z,u);
segment(u,t);
segment(t,x);
segment(x,y);
};

// Voici un sphinx droit et ses 4 petits
sphinxd4(x,y):={
local z,u,t;
z:=x+2*(y-x)/3*exp(3.14*i/3);
t:= y+(x-y)/3*exp(-3.14*i/3);
u:=t+(x-y)/3;
segment(x,z);
segment(z,u);
segment(u,t);
segment(t,y);
segment(y,x);
sphinxg(x,(x+y)/2);
sphinxg((x+y)/2,y);
sphinxg(t,t+(x-y)/2);
sphinxd(z,(3*x+z)/4);
};

// Voici un sphinx gauche et ses 4 petits
sphinxg4(x,y):={
local z,u,t;
z:=y+2*(x-y)/3*exp(-evalf(pi)*i/3);
t:= x+(y-x)/3*exp(evalf(pi)*i/3);
u:=t+(y-x)/3;
segment(y,z);
segment(z,u);
segment(u,t);
segment(t,x);
segment(x,y);
sphinxd(x,(x+y)/2);
sphinxd((x+y)/2,y);
sphinxd(t+(y-x)/2,t);
sphinxg((3*y+z)/4,z);
};

// et toute la famille des sphinx droits
sphinxds(x,y,n):={
local z,u,t;
if (n<=0) return 0;
z:=x+2*(y-x)/3*exp(evalf(pi)*i/3);
t:= y+(x-y)/3*exp(-evalf(pi)*i/3);
u:=t+(x-y)/3;
segment(x,z);
segment(z,u);
segment(u,t);
segment(t,y);
segment(y,x);
sphinxgs(x,(x+y)/2,n-1);
sphinxgs((x+y)/2,y,n-1);
sphinxgs(t,t+(x-y)/2,n-1);
sphinxds(z,(3*x+z)/4,n-1);
};

// et toute la famille des sphinx gauches
sphinxgs(x,y,n):={
local z,u,t,p;
if (n<=0) return 0;
z:=y+2*(x-y)/3*exp(-evalf(pi)*i/3);
t:= x+(y-x)/3*exp(evalf(pi)*i/3);
u:=t+(y-x)/3;
segment(y,z);
segment(z,u);
segment(u,t);
segment(t,x);
segment(x,y);
sphinxds(x,(x+y)/2,n-1);
sphinxds((x+y)/2,y,n-1);
sphinxds(t+(y-x)/2,t,n-1);
sphinxgs((3*y+z)/4,z,n-1);
};

// et toute la famille des sphynx droits
sphynxds(x,y,n):={
  local t,z;
if (n<=1) return sphinxd(x,y);
z:=x+2*(y-x)/3*exp(evalf(pi)*i/3);
t:= y+(x-y)/3*exp(-evalf(pi)*i/3);
sphynxgs(x,(x+y)/2,n-1);
sphynxgs((x+y)/2,y,n-1);
sphynxgs(t,t+(x-y)/2,n-1);
sphynxds(z,(3*x+z)/4,n-1);
};

// et toute la famille des sphynx gauches
sphynxgs(x,y,n):={
  local z,t;
if (n<=1) return sphinxg(x,y);
z:=y+2*(x-y)/3*exp(-evalf(pi)*i/3);
t:= x+(y-x)/3*exp(evalf(pi)*i/3);
sphynxds(x,(x+y)/2,n-1);
sphynxds((x+y)/2,y,n-1);
sphynxds(t+(y-x)/2,t,n-1);
sphynxgs((3*y+z)/4,z,n-1);
};
