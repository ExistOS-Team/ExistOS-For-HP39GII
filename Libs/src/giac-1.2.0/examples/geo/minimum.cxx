C:=cercle(0,1);
t:=element(0..2*pi);
M:=point(exp(i*t)); // ou M:=element(C,t);
L(A,B,t):=evalf(longueur(A,exp(i*t))+longueur(B,exp(i*t)));
G:=plotfunc(L(A,B,x)-2,x);
N:=element(G,t);
bissectrice(M,A,B);
exbissectrice(M,A,B)
