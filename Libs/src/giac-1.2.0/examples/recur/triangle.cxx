triangles(A,B,C):={
DispG();
triangle(A,B,C);
if (longueur2(A,B)>0.01) {
   triangles(A+(B-A)/2,B+(C-B)/2,C+(A-C)/2);
}
};
trianglesp(A,B,C):={
DispG();
if (longueur2(A,B)>0.01) {
  trianglesp(A+(B-A)/2,B+(C-B)/2,C+(A-C)/2);
}
triangle(A,B,C);
};
trianglep(A,B,C,a):={
DispG();
triangle(A,B,C);
if (longueur2(A,B)>0.01) {
trianglep(A+(B-A)*a,B+(C-B)*a,C+(A-C)*a,a);
}
};
triangle3(A,B,C):={
DispG();
triangle(A,B,C);
 if (longueur2(A,B)<0.01) return 0;
 triangle3(A,A+(B-A)/2,C+(A-C)/2);
 triangle3(A+(B-A)/2,B,B+(C-B)/2);
 triangle3(C+(A-C)/2,B+(C-B)/2,C);
};
trianglep(A,B,C):={
DispG();
if (longueur2(A,B)<0.01) {triangle(A,B,C);return 0;}
trianglep(A,A+(B-A)/2,C+(A-C)/2);
trianglep(A+(B-A)/2,B,B+(C-B)/2);
trianglep(C+(A-C)/2,B+(C-B)/2,C);
};
triangle3p(A,B,C,a):={
DispG();
if (longueur2(A,B)<0.02) {triangle(A,B,C);return 0;}
triangle3p(A,A+(B-A)*a,C+(A-C)*a,a);
triangle3p(A+(B-A)*a,B,B+(C-B)*a,a);
triangle3p(C+(A-C)*a,B+(C-B)*a,C,a);
};
triangle3an(A,B,C,a,n):={
DispG();
if (n==0) {triangle(A,B,C);return 0;}
triangle3an(A,A+(B-A)*a,C+(A-C)*a,a,n-1);
triangle3an(A+(B-A)*a,B,B+(C-B)*a,a,n-1);
triangle3an(C+(A-C)*a,B+(C-B)*a,C,a,n-1);
};
