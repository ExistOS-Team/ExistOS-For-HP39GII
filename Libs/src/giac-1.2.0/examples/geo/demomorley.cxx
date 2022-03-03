// Demonstration of Morley theorem
// The parameters are the angle values at A 3*a1 and B 3*a2
assume(a1=0.3);
assume(a2=0.4);
A:=point(0);
B:=point(1);
C:=point(texpand(tan(a2*3)/(tan(a1*3)+tan(a2*3))*(1+i*tan(a1*3))));
P:=normal(point(texpand(tan(a2)/(tan(a1)+tan(a2))*(1+i*tan(a1)))));
R:=normal(inter_unique(droite(0,1+i*texpand(tan(2*a1))),droite(C,C+1+i*texpand(tan(pi/3+2*a1-a2)))));
Q:=normal(inter_unique(droite(1,i*texpand(tan(2*a2))),droite(C,C+1+i*texpand(tan(2*pi/3+a1-2*a2)))));
triangle(A,R,C);
triangle(B,Q,C);
triangle(A,P,B);
triangle(P,Q,R);
pq2:=longueur2(P,Q);
pr2:=longueur2(P,R);
qr2:=longueur2(Q,R);
[normal(pq2-pr2),normal(pq2-qr2)];
