hilbg(l):={
si (l<10) alors
avance(l);
sinon
tourne_gauche(90);hilbd(l/2);
tourne_droite(90);hilbg(l/2);
tourne_droite(90);bertg(l/2);
tourne_gauche(90);bertd(l/2);
fsi;
};
hilbd(l):={
si (l<10) alors
avance(l);
sinon
tourne_droite(90);;hilbg(l/2);
tourne_gauche(90);;hilbd(l/2);
tourne_gauche(90);bertd(l/2);
tourne_droite(90);bertg(l/2);
fsi;
};
bertg(l):={
si (l<10) alors
avance(l);
sinon
tourne_droite(180);hilbd(l/2);
tourne_droite(90);hilbg(l/2);
tourne_droite(90);bertg(l/2);
hilbd(l/2);
fsi;
};
bertd(l):={
si (l<10) alors
avance(l);
sinon
tourne_droite(180);hilbg(l/2);
tourne_gauche(90);hilbd(l/2);
tourne_gauche(90);bertd(l/2);
hilbg(l/2);
fsi;
};
