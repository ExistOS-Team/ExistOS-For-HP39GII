pgcd(a,b):={
  local r; 
  while (b!=0){
   r:=irem(a,b);
   a:=b;
   b:=r;
  } 
  return(a);
};
pgcdr(a,b):={
  if (b==0) 
    return(a);
  else 
    return(pgcdr(b,irem(a,b)));
};
bezout(a,b):={
//renvoie la liste [u,v,d] telle que a*u+b*v=d=pgcd(a,b) (fct iterative)
local la,lb,lr,q,lb2;
la:=[1,0,eval(a)];
lb:=[0,1,eval(b)];
lb2:=eval(b);
while (lb2 !=0){
q:=iquo(la[2],lb2);
lr:=la+(-q)*lb;
la:=lb;
lb:=lr;
lb2:=lb[2];
}
return(la);
};
bezoutr(a,b):={
//renvoie la liste [u,v,d] telle que a*u+b*v=d=pgcd(a,b) (fct recursive)
local lb,q,r;
if (b!=0) {
q:=iquo(a,b);
r:=irem(a,b);
lb:=bezoutr(b,r);
return([lb[1],lb[0]+(-q)*lb[1],lb[2]]);
} else 
return([1,0,a]);
};