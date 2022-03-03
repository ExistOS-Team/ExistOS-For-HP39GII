dragong(l):={
si (l<10) alors
avance(l);
sinon
dragong(l/2);
tourne_gauche(90);
dragond(l/2);
fsi;
};
dragond(l):={
si (l<10) alors
avance(l);
sinon
dragong(l/2);
tourne_droite(90);
dragond(l/2);
fsi;
};
dragon2(l,s):={
si (l<10) alors
avance(l);
sinon
dragon2(l/2,1);
tourne_gauche(90*s);
dragon2(l/2,-1);
fsi;
};
dragon3(l,s,n):={
si (n==0) alors
avance(l);
sinon
dragon3(l/2,1,n-1);
tourne_gauche(90*s);
dragon3(l/2,-1,n-1);
fsi;
};
