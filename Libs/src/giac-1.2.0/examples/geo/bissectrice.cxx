// Les bissectrices d'un triangle sont concourrantes
// les calculs sont faits en minimisant le nombre de parametres a 2
A:=point(0);
B:=point(1,0);
assume(x=1.2);
assume(y=-1);
//y:=-1;
C:=point(x,y);
triangle(A,B,C);
a:=bissectrice(A,B,C);
b:=bissectrice(B,C,A);
c:=bissectrice(C,A,B);
M:=head(inter(a,b));
N:=head(inter(a,c));
normal(M-N);
