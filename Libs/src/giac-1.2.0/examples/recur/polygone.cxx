Hexagone(x,y):={
local a,b,c;
a:=x;
b:=y;
DispG();
for (j:=1;j<=6;j++) {
c:=a+(b-a)*exp(evalf(i*pi*2/3));
segment(a,c);
b:=a;
a:=c;
}
};
hexagones(x,y,n):={
local a,b,c;
a:=x;
b:=y;
DispG();
if (n==0) {Hexagone(a,b);return 0;}
c:=a+(b-a)*2/3*exp(evalf(i*pi/3))
// dessin de l'hexagone central
hexagones(c,c+(b-a)/3, n-1);
//dessin des 6 hexagones dans les angles
for (j:=1;j<=6;j++) {
c:=a+(b-a)*exp(evalf(i*pi*2/3));
hexagones(c,c+(a-c)/3,n-1);
b:=a;
a:=c;
} 
};
//napperon de Cantor ou de Sierpinski k=3,4...
//utilise isopolygone(a,b,k) k>0 
//ex polyserp(-1-2*i,1-2*i,5,3); polyserp(-2*i,1-2*i,9,2)
polyserp(x,y,k,n):={
local a,b,c,h,j,q,p,s;
DispG();
if (n==0) {isopolygone(x,y,k);return 0;}
a:=x;
b:=y;
//pour k=3 ou 4  h:=(b-a)/3;
//pour k=5,6,7,8 h:=(b-a)/2/(cos(evalf(2*pi/k))+1);
//pour autre k il faut calculer s avec la trigo ou avec
//s:=1;for (l:=1;l<=iquo(k-1,4);l++){s:=s+cos(2*l*evalf(pi)/k);} 
p:=iquo(k-1,4);
s:=(sin(evalf(pi)/k)+sin((2*p+1)*evalf(pi)/k))/
    2/sin(evalf(pi)/k);
for (j:=1;j<=k;j++) {
h:=(b-a)/2/s;
polyserp(a,a+h,k,n-1);
c:=a+(b-a)*exp(evalf(i*pi*(k-2)/k));
b:=a;
a:=c;
}
};
// dessin d'un polygone regulier de k cotes
polyreg(x,y,k):={
local a,b,c;
a:=x;
b:=y;
DispG();
for (j:=1;j<=k;j++) {
c:=a+(b-a)*exp(evalf(i*pi*(k-2)/k));
segment(a,c);
b:=a;
a:=c;
}
};
//utilise polyreg(a,b,k) k>0 k=nb de cotes
//ex polyserr(-2*i,1-2*i,8,2); polyserr(-2*i,1-2*i,9,2)
polyserr(x,y,k,n):={
local a,b,c,h,j,q,p,s;
//if (n==0) return isopolygone(x,y,k);
DispG();
if (n==0) {polyreg(x,y,k);return 0;}
a:=x;
b:=y;
p:=iquo(k-1,4);
s:=(sin(evalf(pi)/k)-sin((2*p+1)*evalf(pi)/k))/
    2/sin(evalf(pi)/k);
for (j:=1;j<=k;j++) {
if ( s!=0) h:=(b-a)/2/s; else h:=(b-a)/3;
polyserr(a,a+h,k,n-1);
c:=a+(b-a)*exp(evalf(i*pi*(k-2)/k));
b:=a;
a:=c;
}
};
polyserp8(x,y,n):={
local a,b,c,h,j,q,p,s,k;
//if (n==0) return isopolygone(x,y,8);
k:=8;
DispG();
if (n==0) {polyreg(x,y,k);return 0;}
a:=x;
b:=y;
p:=iquo(k-1,4);
s:=(sin(evalf(pi)/k)+sin((2*p+1)*evalf(pi)/k))/
    2/sin(evalf(pi)/k);
h:=(b-a)/2/s;
polyserp8(a+h+i*h*(sqrt(2)+1),a+h*(sqrt(2)+1)+i*h*(sqrt(2)+1),n-1);
for (j:=1;j<=k;j++) {
polyserp8(a,a+h,n-1);
c:=a+(b-a)*exp(evalf(i*pi*(k-2)/k));
b:=a;
a:=c;
 h:=(b-a)/2/s;}
};

