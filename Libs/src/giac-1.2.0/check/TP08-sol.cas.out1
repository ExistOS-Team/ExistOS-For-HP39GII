"Warning: some commands like subs might change arguments order",
 (t)-> 
begin 
  'nop';  
  cos(t)/sin(t)^3,sin(t)/sin(t)^3;  
 
end,
 (x,y,z)->x^3-3*x*y*z-z^3,
"Done",
"Done",
4,
(t^2+1)/t,
t,
proc(P,Q) 
  a:=expand(P[1]*Q[2]-P[2]*Q[1]);  
  b:=expand(P[3]*Q[2]-P[2]*Q[3]);  
  c:=expand(P[1]*Q[3]-P[3]*Q[1]);  
  if [a,b,c]=[0,0,0] then true; 
  fi ;  
 
end;,
proc(a,b,c,P,Q) 
  local d,C,QQ,M,sol,R; 
  C:=(-y^2)*z+x^3+a*x^2*z+b*x*z^2+c*z^3;  
  if egal(P,Q) then  
    QQ:=[subs(x=(P[1]),y=(P[2]),z=(P[3]),diff(C,y)),subs(x=(P[1]),y=(P[2]),z=(P[3]),-(diff(C,x))),0]; 
    M:=normal(P+t*QQ); 
    d:=subs(x=(M[1]),y=(M[2]),z=(M[3]),C); 
    sol:=quo(d,t^2,t); 
    if egal(QQ,[0,1,0]) then R:=[0,1,0]; else R:=normal((-coeff(sol,t,1))*P+coeff(sol,t,0)*QQ); 
    fi  else  
    M:=expand(P+t*Q); 
    d:=subs(x=(M[1]),y=(M[2]),z=(M[3]),C); 
    sol:=quo(d,t,t); 
    R:=expand((-coeff(sol,t,1))*P+coeff(sol,t,0)*Q) 
  fi ;  
 
end;,
[0,0,1],
[1,1,1],
[0,1,0],
0,
0,
[-1,-1,1],
[0,0,1],
[0,0,1],
[0,1,0],
[0,1,0],
proc(a,b,c,P,Q) 
  local R; 
  R:=pointresiduel(a,b,c,P,Q);  
  R[2]:=-(R[2]);  
  R;  
 
end;,
[0,-1,0],
[0,0,1],
[0,-1,0],
[-1,1,1],
proc(a,b,c,P,n) 
  local Y,m,X; 
  Y:=[0,1,0];  
  X:=P;  
  m:=n;  
  while m>0 do if odd(m) then  
      Y:=plus(a,b,c,X,Y); 
      X:=plus(a,b,c,X,X); 
      m:=(m-1)/2 else  
      X:=plus(a,b,c,X,X); 
      m:=m/2 
    fi ; 
   od;;  
  Y;  
 
end;,
[0,-1,0],[64,-64,64],[0,-512,0],[0,0,-512],[64,-64,64],[0,9223372036854775808,0],
"Done",
[37247908142/10128208321,7601802384416381/1019292757217119],
[-427565988743127340745490673457692672,-867064742032008311629520873313009664,-116260955875848891425809428000014336],
[0,0],
proc(P,N) 
  local Q,a,b,c,g; 
  Q:=[0,1,0];  
  a:=expand(P[1]*Q[2]-P[2]*Q[1]);  
  b:=expand(P[3]*Q[2]-P[2]*Q[3]);  
  c:=expand(P[1]*Q[3]-P[3]*Q[1]);  
  g:=igcd(a,b,c,N);  
  if (g<>1) and (g<>N) then g; else 1; 
  fi ;  
 
end;,
proc(a,b,c,P,n,N) 
  local Y,m,X; 
  Y:=[0,1,0];  
  X:=P;  
  m:=n;  
  while (m>0) and ((egalomega(Y,N)) = 1) do  
    if odd(m) then  
      Y:=plus(a,b,c,X,Y); 
      X:=plus(a,b,c,X,X); 
      m:=(m-1)/2 else  
      X:=plus(a,b,c,X,X); 
      m:=m/2 
    fi ; 
    Y:=mods(Y,N); 
    X:=mods(X,N); 
   od;;  
  egalomega(Y,N);  
 
end;,
232792560,
[2,1,1],
800620099,
1,
"Done",
9449868410449,
3099044504245996706400,
"Done"
