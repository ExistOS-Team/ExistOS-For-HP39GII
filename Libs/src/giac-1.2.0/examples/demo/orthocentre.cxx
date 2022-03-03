// Le symetrique de l'orthocentre / a un cote d'un triangle appartient
// au cercle circonscrit
assume(b=1);
assume(c=-1.66);
O:=point(0);
A:=point(1,0);
B:=point(cos(b),sin(b));
C:=point(cos(c),sin(c));
couleur(triangle(A,B,C),vert);
couleur(cercle(0,1),vert);
E:=hauteur(A,B,C);
F:=hauteur(B,C,A);
G:=hauteur(C,A,B);
H:=head(inter(E,F));
simplify(H-head(inter(F,G)));
droite(A,B);
S:=symetrie(droite(A,B),H);
simplify(longueur2(O,S));
