//l:=[1,2,3]; circulaire(l) 
//renvoie la liste l ou la tete est mise a la fin. 
circulaire(l):={
return concat(tail(l),l[0]);
};
//l:=[1,2,3];permcc(l,l) 
//renvoie les permutations circulares de l
//sans variable locale, ld liste reference de depart
permcc(l,ld):={
if (ld==[]) return [];
return [l,op(permcc(concat(tail(l),l[0]),tail(ld)))];
};
//l:=[1,2,3];permutation_circ(l) 
//renvoie les permutations circulares de l
//utilise permcc
permutation_circ(l):={
return permcc(l,l);
};
//echange ds l les elements d'indices j et k
echange(l,j,k):={
local a;
a:=l[j];
l[j]:=l[k];
l[k]:=a;
return l;
};
//utilise echange permuts([1,2,3,4],0)permuts(l,k):={
local lr;
if (k==size(l)-1) return [l];
lr:=[];
for (j:=k;j<size(l);j++){
l:=echange(l,k,j);
lr:=[op(lr),op(permuts(l,k+1))];
}
return lr;
};
//l:=[1,2,3];permutation(l) 
//renvoie toutes les permutations de l
//utilise permuts
permutation(l):={
return permuts(l,0);
};
//au debut ld=[] et lf=l,
//groupe_s([],l) renvoie toutes les permutations de l
groupe_s(ld,lf):={
 local j,n,res;
 n:=size(lf);
 res:=NULL;
 if (n==1)
   return concat(ld,lf);
 for (j:=0;j<n;j++){
   res:=res,groupe_s(append(ld,lf[0]),tail(lf));
   // permutation circulaire
   lf:=append(tail(lf),lf[0]);
 }
 return res;
};
//utilise groupe_s
//groupesym(l) renvoie toutes les permutations de l
groupesym(l):=return(groupe_s([],l));

//l:=[1,2];permss2([],l,l); 
//ldl=debut de l, l=liste a permuter,
//ld=liste de reference (=l au debut)
// lr liste a renvoyer en variable locale
permss2(ldl,l,ld):={
local lr;
if (l==[]) return [ldl];
if (ld==[]) return [];
lr:=permss2(concat(ldl,l[0]),tail(l),tail(l));
lr:=append(lr,op(permss2(ldl,concat(tail(l),l[0]),tail(ld))));
return lr
};
//utilise permss2, 
//permute(l) renvoie toutes les permutations de l
permute(l):={
return permss2([],l,l);
};
