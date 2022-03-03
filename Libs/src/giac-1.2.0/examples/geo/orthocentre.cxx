// On prend A,B,C quelconques sur le cercle de centre 0 et rayon 1
// Le symetrique de l'orthocentre / a un cote d'un triangle appartient
// au cercle circonscrit
assume(b=1);
assume(c=4);
O:=point(0);
A:=point(1,0);
B:=point(cos(b),sin(b));
AB:=segment(A,B);
C:=point(cos(c),sin(c));
HA:=hauteur(A,B,C);
HB:=hauteur(B,C,A);
H:=head(inter(HA,HB)); // H est l'intersection de 2 hauteurs
HC:=hauteur(C,A,B);
H2:=head(inter(HA,HC)); // on verifie que H est sur la 3eme hauteur
simplify(H-H2);
S:=symetrie(AB,H); // symetrique de l'orthocentre par rapport a AB
simplify(longueur2(O,S)); // OS=1 si S est sur le cercle
