// Author P. Aublin
// Let f a function. To compute its integral using approximated methods (middle point, left or right rectangles, trapezoid) we use the function defined as below. mode allow us to say which method we want to use : 1 is for left rectangles, 2 is for right rectangles, 3 is for middle point and 4 is for trapezoid. f(x) is the function with which we want to calculate, b_inf and b_sup are the bounds of the interval where we want to integrate and n is the number of small intervals, i.e. the step. 

integraleApprochee(f,b_inf,b_sup,n,mode):= {
  local h,alpha,Bta,P,integ,j,t; 
  integ:=0; 
  h:=(b_sup-b_inf)/n;
  for (j:=b_inf;j<=(b_sup-h);j:=j+h) { 
    alpha:=j;  
    Bta:=j+h;  
    switch (mode){ 
    case 1: 
      { 
        P:=f(alpha);  break;
      } 
    case 2: 
      { 
        P:=f(Bta);  break;
      } 
    case 3: 
      { 
        P:=f((alpha+Bta)/2); break;
      } 
    case 4: 
      { 
        P:=(f(alpha)+f(Bta))/2; break;
      } 
    } 
    integ:=integ+P;  
  } 
  return evalf(h*integ);  
};

// Approx. computation of integral of f using Simpson method
// N = number of subdivisions of [a, b] 

simpsonex3(f, N, a, b):={ 
  local alpha,Bta,h,j,some,integ;  
  h:=(b-a)/N;   
  some:=0;  
  for (j:=a;j<b;j:=j+h) { 
      alpha:=j;  
      Bta:=j+h;  
      integ:=h/6*(f(alpha)+4*f((alpha+Bta)/2)+f(Bta));  
      some:=some+integ;  
    } 
  return some;  
};
