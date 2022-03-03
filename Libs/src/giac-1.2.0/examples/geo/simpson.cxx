// Voir aussi Exemples->geometrie->simpson.xws
// ABC un triangle quelconque, G le cercle circonscrit
// M un point du plan, de projection P sur BC, Q sur CA, R sur AB
// P,Q,R sont alignes ssi M est sur C
xa:=0;
ya:=0;
xb:=1;
yb:=0;
assume(xc=(-1));
assume(yc=1);
A:=point(xa,ya);
B:=point(xb,yb);
C:=point(xc,yc);
triangle(A,B,C);
G:=circonscrit(A,B,C);
assume(x=1);
assume(y=1);
M:=point(x,y);
P:=projection(droite(B,C),M);
Q:=projection(droite(C,A),M);
R:=projection(droite(A,B),M);
D1:=factor(numer(im((Q-P)*conj(R-P))));
D12:=D1[2];
D2:=equation(G);
D22:=factor(numer(equal2diff(D2)));
normal(D12/D22)
