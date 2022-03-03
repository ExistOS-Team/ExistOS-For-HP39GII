[],
"Warning: some commands like subs might change arguments order",
0,0,0,1,0,1e-10,10,[1,50,0,25],0,0,0,
3,
x,
2,
2,
6,
1,
1,
[[1,1]],
[[3,0,0,0],[0,6,0,0],[0,0,18,0],[0,0,0,36]],
"Done",
0,
"Done",
matrix[[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]],
proc(A) 
  local m,u,v,i,j; 
  m:=0;  
  u:=0;  
  v:=0;  
  for i from 1 to (dim(A))[1] do for j from 1 to (dim(A))[2] do if ((abs(A[i,j]))>0) and ((m=0) or ((abs(A[i,j]))<m)) then  
    m:=abs(A[i,j]); 
    u:=i; 
    v:=j 
  fi ; 
   od;; 
   od;;  
  u,v;  
 
end;,
matrix[[6,0,7,9],[6,8,6,9],[3,2,4,6],[3,2,3,3]],
3,2,
3,2,
matrix[[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]],
"Done",
"Done",
[[1,0,0,0],[-1,1,-2,-3],[0,0,1,0],[0,0,0,1]],
matrix[[6,0,7,9],[-2,8,-10,-15],[1,2,0,0],[1,2,-1,-3]],
proc(A,i,j) 
  local n,U,V,l; 
  n:=(dim(A))[1];  
  U:=identity(n);  
  V:=identity(n);  
  for l from 1 to n do V[l,i]:=-iquo(A[l,j],A[i,j]); 
   od;;  
  V[i,i]:=1;  
  for l from 1 to n do U[j,l]:=-iquo(A[i,l],A[i,j]); 
   od;;  
  U[j,j]:=1;  
  V*A*U;  
 
end;,
matrix[[6,1,7,9],[6,8,6,9],[3,1,4,6],[3,2,1,3]],
matrix[[0,-1,-1,-3],[0,6,-2,-3],[3,1,1,0],[0,1,-3,-3]],
proc(A) 
  k:=0;  
  n:=(dim(A))[1];  
  l:=[seq(0,k=(1 .. n))];  
  B:=A;  
  i,j:=minval(B);  
  while (([i,j])<>[0,0]) and ((nops(B))>1) do  
    B:=trans(B,i,j); 
    if [i,j]=[minval(B)] then  
      k:=k+1; 
      l[k]:=B[i,j]; 
      B:=delcols(B,j .. j); 
      B:=delrows(B,i .. i) 
    fi ; 
    i,j:=minval(B); 
   od;;  
  diag([seq(l[k],k=(1 .. (n-1))),B[1,1]]);  
 
end;,
matrix[[2,2,2],[6,12,6],[6,4,6]],
[[2,0,0],[0,-2,0],[0,0,0]],
"Done",
[[2,2,2],[6,12,6],[6,4,12]],
[[2,0,0],[0,-2,0],[0,0,18]],
[[2,0,0],[0,2,0],[0,0,18]],[[2,0,0],[0,2,0],[0,0,18]],
matrix[[4,0,0],[0,6,0],[0,0,8]],
[[2,0,0],[0,4,0],[0,0,24]],
[[4,0,0],[0,6,0],[0,0,8]],
 (i,j)->if (i-j)*(i-1)=0 then 1; 
fi ,
matrix[[1,1,1],[0,1,0],[0,0,1]],
matrix[[4,6,8],[0,6,0],[0,0,8]],
[[2,0,0],[0,8,0],[0,0,-12]],
proc(A,i,j) 
  local n,U,l; 
  n:=(dim(A))[1];  
  U:=identity(n);  
  for l from 1 to n do U[j,l]:=-iquo(A[i,l],A[i,j]); 
   od;;  
  U[j,j]:=1;  
  A*U;  
 
end;,
proc(A) 
  k:=0;  
  n:=(dim(A))[1];  
  l:=[seq(0,k=(1 .. n))];  
  B:=A;  
  i,j:=minval(B);  
  while (([i,j])<>[0,0]) and ((nops(B))>1) do  
    B:=transC(B,i,j); 
    if [i,j]=[minval(B)] then  
      k:=k+1; 
      l[k]:=B[i,j]; 
      B:=delcols(B,j .. j); 
      B:=delrows(B,i .. i) 
    fi ; 
    i,j:=minval(B); 
   od;;  
  diag([seq(l[k],k=(1 .. (n-1))),B[1,1]]);  
 
end;,
proc(A) 
  n:=(dim(A))[1];  
  d:=Zequiv(A);  
  L:=[];  
  for i from 1 to n-1 do  
  T:=matrix(n+1-i,n+1-i,f); 
  d:=ZequivC(T*d); 
  L:=[op(L),d[1,1]]; 
  d:=delrows(delcols(d,1 .. 1),1 .. 1); 
   od;;  
  [op(L),d[1,1]];  
 
end;,
matrix[[2,2,2],[6,12,6],[6,4,12]],
[2,-2,18],
[[2,0,0],[0,2,0],[0,0,18]],
[2,4,48],
[[2,0,0],[0,4,0],[0,0,48]]
