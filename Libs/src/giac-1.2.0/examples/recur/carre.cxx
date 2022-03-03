carres(A,B):={
DispG();
carre(A,B);
if (longueur2(A,B)>0.01) {
  carres(A+(B-A)/2,B+(B-A)*i/2);
}
};
carres2(A,B):={
DispG();
if (longueur2(A,B)>0.01) {
  carres2(A+(B-A)/2,B+(B-A)*i/2);
}
carre(A,B);
};
carrep(A,B,n):={
DispG();
carre(A,B);
 if (n==0) return 0;
 carrep(A+(B-A)/2,B+(B-A)*i/2,n-1);
};
carresp(A,B,a):={
DispG();
carre(A,B);
if (longueur2(A,B)>0.01) {
carresp(A+(B-A)*a,B+(B-A)*i*a,a);
}
};
carre8(A,B):={
local h;
DispG();
carre(A,B);
if (longueur2(A,B)<0.01) return 0;
h:=(B-A)/3;
carre8(A,A+h);
carre8(A+h,A+2*h);
carre8(A+2*h,B);
carre8(A+i*h,A+i*h+h);
carre8(A+i*h+2*h,B+i*h);
carre8(A+2*i*h,A+2*i*h+h);
carre8(A+2*i*h+h,A+2*i*h+2*h);
carre8(A+2*i*h+2*h,B+2*i*h);
};
carre8p(A,B,n):={
local h;
h:=(B-A)/3;
DispG();
carre(A,B);
if (n==0)  return 0;
h:=(B-A)/3;
carre8p(A,A+h,n-1);
carre8p(A+h,A+2*h,n-1);
carre8p(A+2*h,B,n-1);
carre8p(A+i*h,A+i*h+h,n-1);
carre8p(A+i*h+2*h,B+i*h,n-1);
carre8p(A+2*i*h,A+2*i*h+h,n-1);
carre8p(A+2*i*h+h,A+2*i*h+2*h,n-1);
carre8p(A+2*i*h+2*h,B+2*i*h,n-1);
};

