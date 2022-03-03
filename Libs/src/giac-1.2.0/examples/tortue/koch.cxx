koch1(l):={
si (l<10) alors
avance(l);
sinon
koch1(l/3);tourne_gauche(60);
koch1(l/3);tourne_droite(120);
koch1(l/3);tourne_gauche(60);
koch1(l/3);
fsi;
};
koch2(l,n):={
si (n==0) alors
avance(l);
sinon
koch2(l/3,n-1);tourne_gauche(60);
koch2(l/3,n-1);tourne_droite(120);
koch2(l/3,n-1);tourne_gauche(60);
koch2(l/3,n-1);
fsi;
};