carres(x,n):={
  local a,b,j,ab;
  DispG ;
  a:=0;
  b:=x;
  for (j:=0;j<n;j++){
    carre(a,b);
    ab:=(b-a)/4;
    a:=a+ab*i;
    b:=b-ab;  
  }
}
