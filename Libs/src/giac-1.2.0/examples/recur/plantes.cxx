//Voici des sapins....sapin(0,2*i)
sapin(x,y):={
 DispG();
 if (abs(x-y)<0.5) {segment(x,y); return 0;}
 sapin(x,x+(y-x)*0.5*exp(i));
 sapin(x,x+(y-x)*0.5*exp(-i));
 segment(x,(3*x+y)/4);
 sapin((3*x+y)/4,y);
};
//Voici des fleurs....fleur(0,2*i)
fleur(x,y):={
 DispG();
 if (abs(x-y)<0.5) {segment(x,y);cercle(y,(y-x)*0.3); 
                    return 0;}
 segment(x,y);cercle(y,(y-x)*0.3);cercle(y,(y-x)*0.2);
 fleur(x,x+(y-x)*0.5*exp(i*0.5));
 fleur(x,x+(y-x)*0.5*exp(-i*0.5));
};
//Voici des arbres....arbre(0,2*i)
arbre(x,y):={
 DispG();
 if (abs(x-y)<0.2) {segment(x,y); return 0;}
 segment(x,(x+y)/2);
 arbre((x+y)/2,(x+y)/2+(y-x)*0.5*exp(i*0.5));
 arbre((x+y)/2,(x+y)/2+(y-x)*0.5*exp(-i*0.5));
};
//et des arbres moins deplumes arbre2(0,2*i)
arbre2(x,y):={
 DispG();
 if (abs(x-y)<0.2) {segment(x,y); return 0;}
 segment(x,(x+y)/2);
 arbre2((x+y)/2,(x+y)/2+(y-x)*0.5*exp(i*0.5));
 arbre2((x+y)/2,(x+y)/2+(y-x)*0.5*exp(-i*0.5));
 arbre2((x+y)/2,(x+y)/2+(y-x)*0.5*exp(i));
 arbre2((x+y)/2,(x+y)/2+(y-x)*0.5*exp(-i));
};
//un epineux  arbre3(0,2*i)
arbre3(x,y):={
 DispG();
 if (abs(x-y)<0.2) {segment(x,y); return 0;}
 segment(x,(x+y)*0.5);
 arbre3((3*x+y)/4,(3*x+y)/4+(y-x)*0.25*exp(i*0.5));
 arbre3((3*x+y)/4,(3*x+y)/4+(y-x)*0.25*exp(-i*0.5));
 arbre3((x+y)/2,(x+y)/2+(y-x)*0.5*exp(i));
 arbre3((x+y)/2,(x+y)/2+(y-x)*0.5*exp(-i));
};
//une fougere par ex fougeres(-2*i,2*i)
fougere(x,y):={
local a;
DispG();
 if (abs(x-y)<0.2) {segment(x,y); return 0;}
a:=x+(y-x)*0.15*exp(-i*0.2);
segment(x,a);
fougere(a,a+(y-x)*0.33*exp(i*1.2));
fougere(a,a+(y-x)*0.33*exp(-i*1.2));
fougere(a,a+(y-x)*0.85*exp(-i*0.2));
};
//par ex fougeres(-2*i,2*i,0.05,6)
fougeres(x,y,t,n):={
local a;
DispG();
if (n==0) {segment(x,(x+y)/2); return 0;}
//a:=x+(y-x)*0.15*exp(-i*t);
a:=x+(y-x)*0.15;
segment(x,a);
fougeres(a,a+(y-x)*0.33*exp(i*1.2),t,n-1);
fougeres(a,a+(y-x)*0.33*exp(-i*1.2),t,n-1);
fougeres(a,a+(y-x)*0.85*exp(-i*t),t,n-1);
};
//et le bouquet final bouquet(0,2*i)
bouquet(x,y):={
 DispG();
 if (abs(x-y)<0.2) {segment(x,y); return 0;}
 segment(x,(x+y)*0.5);
 bouquet((3*x+y)/4,(3*x+y)/4+(y-x)*0.25*exp(i*0.5));
 bouquet((3*x+y)/4,(3*x+y)/4+(y-x)*0.25*exp(-i*0.5));
 bouquet((x+y)/2,(x+y)/2+(y-x)*0.5*exp(i));
 bouquet((x+y)/2,(x+y)/2+(y-x)*0.5*exp(-i));
 bouquet((x+y)/2,(x+y)/2+(y-x)*0.5);
};
