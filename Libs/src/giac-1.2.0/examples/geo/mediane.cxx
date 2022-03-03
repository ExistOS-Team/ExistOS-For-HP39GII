// Les medianes d'un triangle sont concourrantes, test avec 6 parametres
assume(Ax=-1);
assume(Ay=-1);
A:=point(Ax,Ay);
assume(Bx=0.5);
assume(By=1);
B:=point(Bx,By);
assume(Cx=1);
assume(Cy=-1);
C:=point(Cx,Cy);
triangle(A,B,C);
a:=mediane(A,B,C);
b:=mediane(B,C,A);
c:=mediane(C,A,B);
M:=head(inter(a,b));
N:=head(inter(b,c));
normal(longueur2(M,N));
