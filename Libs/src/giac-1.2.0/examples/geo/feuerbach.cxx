// Thm de Feuerbach: 
// le cercle d'Euler est tangent aux cercles inscrit et exinscrits
// On definit le triangle ABC en se ramenant a 2 parametres
// 
assume(xa=1/2);
assume(ya=1);
A:=point(xa,ya);
B:=point(-1,0);
AB:=segment(A,B);
C:=point(1,0);
AC:=segment(A,C);
CB:=segment(C,B);
E:=milieu(AB);
F:=milieu(AC);
G:=milieu(CB);
c:=circonscrit(E,F,G);
H:=hauteur(A,B,C);
I:=(inter(H,CB))[0];
print("teste que le pied de la hauteur est sur c de 2 facons, doit renvoyer 1 puis 0"):;
est_element(I,c);
O:=centre(c):;
l2:=longueur2(I,O):;
rc2:=normal(rayon(c)^2):;
normal(l2-rc2);
print("cercle inscrit de A,B,C, centre"):;
d:=inscrit(A,B,C):;
P:=centre(d);
OP:=longueur2(O,P);
rd:=rayon(d);
if (type(numer(rd))==DOM_SYMBOLIC && sommet(numer(rd))==abs) { rd:=feuille(numer(rd))/denom(rd); };
rd2:=rd^2;
normal( (OP-rc2-rd2)^2- 4*rc2*rd2);


