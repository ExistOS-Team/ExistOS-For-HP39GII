flocon(A,B):={
DispG();
if (longueur2(A,B)<0.01) {segment(A,B);return 0;}
flocon(A,A+(B-A)/3);
flocon(A+(B-A)/3,A+(B-A)/3*(1+exp(i*pi/3)));
flocon(A+(B-A)/3*(1+exp(i*pi/3)),A+2*(B-A)/3);
flocon(A+2*(B-A)/3,B);
};
floconp(A,B,n):={
local h;
DispG();
if (n==0) {segment(A,B);return 0;}
h:=(B-A)/3;
floconp(A,A+h,n-1);
floconp(A+h,A+h*(1+exp(i*pi/3)),n-1);
floconp(A+h*(1+exp(i*pi/3)),A+2*h,n-1);
floconp(A+2*h,B,n-1);
};
flocong(A,B,a,n):={
local h,t;
DispG();
if (n==0) {segment(A,B);return 0;}
t:=acos((0.5-a)/a);
h:=(B-A)*a;
flocong(A,A+h,a,n-1);
flocong(A+h,A+h*(1+exp(i*t)),a,n-1);
flocong(A+h*(1+exp(i*t)),B-h,a,n-1);
flocong(B-h,B,a,n-1);
};

