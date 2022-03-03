chaine2n(m):={
//chaine2n(m) transforme la chaine m en l'entier n 
//m est l'ecriture de n dans la base 256 
local l,n,s;
s:=size(m);
l:=asc(m);
n:=0;
for (k:=0;k<s;k++){
n:=n*256+l[k];
}
return(n);
};

ecritu256(n):={
//transforme l'entier n en son ecriture en base 256
local s,r;
//n est un entier et b=256, ecritu256 est une fonction iterative 
//ecritu256(n)=le mot de caracteres l'ecriture de n en base 256 
s:="";
while (n>=256){
r:=irem(n,256);
r:=char(r);
s:=r+s;
n:=iquo(n,256);
}
n:=char(n);
s:=n+s;
return(s);
};

//mess est une chaine u:=115769854373006801 n:=152415790094497781
codrsa(mess,u,n):={
local s,j,j3,l,mot;
s:=size(mess);
j:=0;
j3:=3;
l:=[];
//j est le nombre de paquets de 3 lettres
while (j3<s) {
mot:="";
for (k:=j;k<j3;k++){
mot:=mot+mess[k];
}
//on code le mot
a:=chaine2n(mot);
l:=append(l,powmod(a,u,n));
j:=j3;
j3:=j+3;
}
mot:="";
for (k:=j;k<s;k++){
mot:=mot+mess[k];
}
a:=chaine2n(mot);
l:=append(l,powmod(a,u,n));
return(l);
};

//l=codrsa(mess,u,n) m:=12345701 n:=152415790094497781
decodrsa(l,m,n):={
local mess,s,a,j,b;
s:=size(l);
mess:="";
for (j:=0;j<s;j++){
b:=l[j];
a:=powmod(b,m,n);
mess:=mess+ecritu256(a);
}
return(mess);
};
