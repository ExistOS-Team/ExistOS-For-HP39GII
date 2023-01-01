/* -*- mode:C++ ; compile-command: "g++-3.4 -I.. -g -c permu.cc -DHAVE_CONFIG_H -DIN_GIAC" -*- */
#include "giacPCH.h"
/*
 *  Copyright (C) 2005, 2007 R. De Graeve & B. Parisse, 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

using namespace std;
#include "permu.h"
#include "usual.h"
#include "sym2poly.h"
#include "rpn.h"
#include "prog.h"
#include "derive.h"
#include "subst.h"
#include "misc.h"
#include "plot.h"
#include "intg.h"
#include "ifactor.h"
#include "lin.h"
#include "modpoly.h"
#include "giacintl.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  vecteur vector_int_2_vecteur(const vector<int> & v,GIAC_CONTEXT){
    //transforme un vector<int> en vecteur 
    vector<int>::const_iterator it=v.begin(),itend=v.end();
    vecteur res;
    res.reserve(itend-it);
    if (array_start(contextptr)){ //(xcas_mode(contextptr) || abs_calc_mode(contextptr)==38){
      for (;it!=itend;++it)
	res.push_back(*it+1);
    }
    else {
      for (;it!=itend;++it)
	res.push_back(*it);
    }
    return res;
  } 

  vecteur vector_int_2_vecteur(const vector<int> & v){
    //transforme un vector<int> en vecteur 
    vector<int>::const_iterator it=v.begin(),itend=v.end();
    vecteur res;
    res.reserve(itend-it);
    for (;it!=itend;++it)
      res.push_back(*it);
    return res;
  } 

  static vector<int> vecteur_2_vector_int(const vecteur & v,GIAC_CONTEXT){
    //transforme un vecteur en vector<int>  -> empty vector on error
    vecteur::const_iterator it=v.begin(),itend=v.end();
    vector<int> res;
    res.reserve(itend-it);
    if (array_start(contextptr)){ //(xcas_mode(contextptr) || abs_calc_mode(contextptr)==38){
      for (;it!=itend;++it)
	if ((*it).type==_INT_) 
	  res.push_back((*it).val-1);
	else 
	  return vector<int>(0);
    }
    else {
      for (;it!=itend;++it)
	if ((*it).type==_INT_) 
	  res.push_back((*it).val); 
	else 
	  return vector<int>(0);
    }
    return res;
  } 

  vector<int> vecteur_2_vector_int(const vecteur & v){
    //transforme un vecteur en vector<int>  -> empty vector on error
    vecteur::const_iterator it=v.begin(),itend=v.end();
    vector<int> res;
    res.reserve(itend-it);
    for (;it!=itend;++it){
      if ((*it).type==_INT_) 
	res.push_back((*it).val); 
      else 
	return vector<int>(0);
    }
    return res;
  } 

  static vector< vector<int> > vecteur_2_vectvector_int(const vecteur & v,GIAC_CONTEXT){
    //transforme un vecteur en vector< vector<int> >  -> empty vector on error
    vecteur::const_iterator it=v.begin(),itend=v.end();
    vector< vector<int> > res;
    res.reserve(itend-it);
    for (;it!=itend;++it){
      if (it->type!=_VECT)
	return  vector< vector<int> >(0);
      res.push_back(vecteur_2_vector_int(*it->_VECTptr,contextptr));
    }
    return res;
  }

  vector< vector<int> > vecteur_2_vectvector_int(const vecteur & v){
    //transforme un vecteur en vector< vector<int> >  -> empty vector on error
    vecteur::const_iterator it=v.begin(),itend=v.end();
    vector< vector<int> > res;
    res.reserve(itend-it);
    for (;it!=itend;++it){
      if (it->type!=_VECT)
	return  vector< vector<int> >(0);
      res.push_back(vecteur_2_vector_int(*it->_VECTptr));
    }
    return res;
  }

  static vecteur vectvector_int_2_vecteur(const vector< vector<int> > & v,GIAC_CONTEXT){
    //transforme un vector< vector<int> > en vecteur  
    int s=int(v.size());
    vecteur res;
    res.reserve(s);
    for (int i=0;i<s;++i)
      res.push_back(vector_int_2_vecteur(v[i],contextptr));
    return res;
  }

  vecteur vectvector_int_2_vecteur(const vector< vector<int> > & v){
    //transforme un vector< vector<int> > en vecteur  
    int s=int(v.size());
    vecteur res;
    res.reserve(s);
    for (int i=0;i<s;++i)
      res.push_back(vector_int_2_vecteur(v[i]));
    return res;
  }

  vector<int> sizes(const vector< vector<int> > & v){
    //donne la liste des tailles des vecteurs qui forment v
    int s=int(v.size());
    vector<int> res(s);
    for (int i=0;i<s;i++){
      vector<int> vi;
      vi=v[i];
      res[i]=int(vi.size());
      //res.push_back(vi.size());pourqoi?
    }
    return res;
  }

  gen _sizes(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT) 
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr); 
    vecteur res;
    vecteur::const_iterator it=v.begin(),itend=v.end();
    res.reserve(itend-it);
    for (;it!=itend;++it){
      if (it->type!=_VECT)
	return gensizeerr(contextptr);
      res.push_back(int(it->_VECTptr->size()));
    }
    return res;
  } 
  static const char _sizes_s[]="sizes";
  static define_unary_function_eval (__sizes,&_sizes,_sizes_s);
  define_unary_function_ptr5( at_sizes ,alias_at_sizes,&__sizes,0,true);

  longlong lcm(int a,int b){
    int d=gcd(a,b);
    return (a/d)*longlong(b);
  }

  int lcm(const vector<int> & v){
    vector<int>::const_iterator it=v.begin(),itend=v.end();
    if (itend==it) return 0;
    if (itend==it+1) return *it;
    int r=lcm(*it,*(it+1));
    for (it+=2;it<itend;++it)
      r=lcm(r,*it);
    return r;
  }

  static int cyclesorder(const vector< vector<int> > & v,GIAC_CONTEXT){
    vector<int> ss;
    ss=sizes(v);   
    return lcm(ss);
  } 
  static int permuorder(const vector<int> & p,GIAC_CONTEXT){
    vector< vector<int> > c;  
    c=permu2cycles(p);
    return lcm(sizes(c));
  }

  gen _permuorder(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if  (args.type!=_VECT) 
      return gensizeerr(contextptr);
    vecteur v(*args._VECTptr); 
    vector<int> p;
    if (!is_permu(v,p,contextptr))
      return gensizeerr(contextptr);
    return (permuorder(p,contextptr));
  }
  static const char _permuorder_s[]="permuorder";
  static define_unary_function_eval (__permuorder,&_permuorder,_permuorder_s);
  define_unary_function_ptr5( at_permuorder ,alias_at_permuorder,&__permuorder,0,true);

  void shuffle(vector<int> & temp){
    int n=int(temp.size());
    // source wikipedia Fisher-Yates shuffle article
    for (int i=0;i<n-1;++i){
      // j ← random integer such that i ≤ j < n
      // exchange a[i] and a[j]
      int j=i+int((std_rand()/(rand_max2+1.0))*(n-i));
      std::swap(temp[i],temp[j]);
    }
  }

  vector<int> rand_k_n(int k,int n,bool sorted){
    if (k<=0 || n<=0)
      return vector<int>(0);
    if (//n>=65536 && 
	k*double(k)<=n/4){
      vector<int> t(k),ts(k); 
      for (int essai=20;essai>=0;--essai){
	int i;
	for (i=0;i<k;++i)
	  ts[i]=t[i]=int(std_rand()/(rand_max2+1.0)*n);
	sort(ts.begin(),ts.end());
	for (i=1;i<k;++i){
	  if (ts[i]==ts[i-1])
	    break;
	}
	if (i==k)
	  return sorted?ts:t;
      }
    }
    if (k>=n/3 || (sorted && k*std::log(double(k))>n) ){
      vector<int> t; t.reserve(k);
      // (algorithm suggested by O. Garet)
      while (n>0){
	int r=int(std_rand()/(rand_max2+1.0)*n);
	if (r<n-k) // (n-k)/n=proba that the current n is not in the list
	  --n;
	else {
	  --n;
	  t.push_back(n);
	  --k;
	}
      }
      if (sorted)
	reverse(t.begin(),t.end());
      else
	shuffle(t);
      return t;
    }
    vector<bool> tab(n,true);
    vector<int> v(k);
    for (int j=0;j<k;++j){
      int r=-1;
      for (;;){
	r=int(std_rand()/(rand_max2+1.0)*n);
	if (tab[r]) break;
      }
      v[j]=r;
    }
    if (sorted)
      sort(v.begin(),v.end());
    return v;
  }
  
  vector<int> randperm(const int & n){
    //renvoie une permutation au hasard de long n
    vector<int> temp(n);
    for (int k=0;k<n;k++) temp[k]=k;
#if 1
    shuffle(temp);
    return temp;
#else
    vector<int> p(n);
    //on chosit au hasard h et alors p[k]=temp[h]
    int m;
    m=n;
    for (int k=0;k<n;k++) {
      int h=int(std::floor(m*(std_rand()/(rand_max2+1.0))));
      p[k]=temp[h]; m=m-1;
      //mise a jour de temp :il faut supprimer temp[h]
      for (int j=h;j<m;j++) {
	temp[j]=temp[j+1];
      }
    }    
    return(p); 
#endif
  }
  gen _randperm(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT){
      vecteur & v = *args._VECTptr;
      vector<int> p=randperm(int(v.size()));
      vecteur w(v);
      for (unsigned i=0;i<v.size();++i){
	w[i]=v[p[i]];
      }
      return w;
    }
    gen n=args;
    if (!is_integral(n) || n.type!=_INT_ || n.val<=0) 
      return gensizeerr(contextptr);
    return vector_int_2_vecteur(randperm(n.val),contextptr);
  }
  static const char _randperm_s[]="randperm";
  static define_unary_function_eval (__randperm,&_randperm,_randperm_s);
  define_unary_function_ptr5( at_randperm ,alias_at_randperm,&__randperm,0,true);
  
  bool is_permu(const vecteur &p,vector<int> & p1,GIAC_CONTEXT) {
    //renvoie true si p est une perm et transforme p en le vector<int> p1  
    int n;
    n=int(p.size());
    vector<int> p2(n);
    p1=p2;
    vector<int> temp(n);
   
    for (int j=0;j<n;j++){ if (p[j].type!=_INT_){return(false);}}
     
    for (int j=0;j<n;j++){
      if (array_start(contextptr)) //xcas_mode(contextptr)>0 || abs_calc_mode(contextptr)==38) 
	p1[j]=p[j].val-1; 
      else 
	p1[j]=p[j].val;
      if ((n<=p1[j])|| (p1[j])<0) {
	return(false);
      }
    }
    int k;
    k=0;
    while (k<n) {
      int p1k=p1[k];
      if (p1k<0 || p1k>=n) {return(false);}
      if (temp[p1k]) {
	return(false);} 
      else {temp[p1k]=1;}
      k=k+1;
    }
    return(true);
  } 
  gen _is_permu(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT) 
      return gensizeerr(contextptr);
    vecteur v(*args._VECTptr);
    vector<int> p1;
    return is_permu(v,p1,contextptr);
  }
  static const char _is_permu_s[]="is_permu";
  static define_unary_function_eval (__is_permu,&_is_permu,_is_permu_s);
  define_unary_function_ptr5( at_is_permu ,alias_at_is_permu,&__is_permu,0,true);


  bool is_cycle(const vecteur & c,vector<int> & c1,GIAC_CONTEXT) {
    //renvoie true si c est un cycle et transf c vecteur en le cycle c1 vector<int>
    int n1;
    n1=int(c.size());
    //vector<int> p;
    c1.resize(n1);
    int b=array_start(contextptr);
    for (int j=0;j<n1;j++){
      int tmp=c[j].val-b;
      if (tmp<0) 
	return false;
      c1[j]=tmp;
    }   
    vector<int> c2(c1);
    sort(c2.begin(),c2.end());
    for (int k=1;k<n1;k++) {
      if (c2[k]==c2[k-1])
	return false;
    }
    return true;
  } 
  gen _is_cycle(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT) 
      return gensizeerr(contextptr);
    vecteur v(*args._VECTptr);
    vector<int> c1;
    return is_cycle(v,c1,contextptr);
  }
  static const char _is_cycle_s[]="is_cycle";
  static define_unary_function_eval (__is_cycle,&_is_cycle,_is_cycle_s);
  define_unary_function_ptr5( at_is_cycle ,alias_at_is_cycle,&__is_cycle,0,true);
 
  vector<int> cycle2perm(const vector<int> & c) {
    //transforme c en la permu p et renvoie p
    int n1;
    n1=int(c.size());
    //vector<int> c1(n1);
    int n;
    n=c[0];
    for (int k=1;k<n1;k++) {
      if (n<c[k]) {
	n=c[k];
      }
    }
    n=n+1;   
    vector<int> p(n);
    for (int k=0;k<n;k++) {
      p[k]=k;}
    for (int k=0;k<n1-1;k++) {p[c[k]]=c[k+1];}
    p[c[n1-1]]=c[0];
    return(p);
  } 
  
  gen _cycle2perm(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT) 
      return gensizeerr(contextptr);
    vecteur v(*args._VECTptr); 
    vector<int> c;
    if (!is_cycle(v,c,contextptr))
      return gensizeerr(contextptr);
    return vector_int_2_vecteur(cycle2perm(c),contextptr);
  }
  static const char _cycle2perm_s[]="cycle2perm";
  static define_unary_function_eval (__cycle2perm,&_cycle2perm,_cycle2perm_s);
  define_unary_function_ptr5( at_cycle2perm ,alias_at_cycle2perm,&__cycle2perm,0,true);
 
 
  vector<int> p1op2(const vector<int> & p1,const vector<int> & p2) {
    //composition de 2 perm p1 et p2 de long n1 et n2 : a pour long max(n1,n2)
    int n1;
    n1=int(p1.size());
    int n2;
    n2=int(p2.size());
    vector<int> p3;
    vector<int> p4;
    p3=p1;
    p4=p2;   
    if (n1>n2) {
      for (int k=n2;k<n1;k++) {p4.push_back(k);n2=n1;} 
    } else {
      for (int k=n1;k<n2;k++) {p3.push_back(k);n1=n2;}
    }
    ;
    vector<int> p(n1);     
    for (int k=0;k<n1;k++) {
      p[k]=p3[p4[k]];
    }      
    return(p);
  }
 
  gen _p1op2(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    gen v1=v.front(),v2=v.back();
    if ( (v1.type!=_VECT) || (v2.type!=_VECT))
      return gentypeerr(contextptr);
    vector<int> p1,p2;   
    if (!is_permu(*v1._VECTptr,p1,contextptr) || !is_permu(*v2._VECTptr,p2,contextptr))
      return gensizeerr(contextptr);
    return vector_int_2_vecteur(p1op2(p1,p2),contextptr);
  }
  static const char _p1op2_s[]="p1op2";
  static define_unary_function_eval (__p1op2,&_p1op2,_p1op2_s);
  define_unary_function_ptr5( at_p1op2 ,alias_at_p1op2,&__p1op2,0,true);

  vector<int> c1oc2(const vector<int> & c1,const vector<int> & c2) {
    //composition de 2 cycles en une perm de long min
    vector<int> p1;
    p1=cycle2perm(c1);
    vector<int> p2;
    p2=cycle2perm(c2);
    int n1;
    n1=int(p1.size());
    int n2;
    n2=int(p2.size());
    int n;
    if (n1>n2) {
      n=n1;
      for (int k=n2;k<n;k++) {p2.push_back(k);} 
    } 
    else {
      n=n2;
      for (int k=n1;k<n;k++) {p1.push_back(k);} 
    }   
    vector<int> p(n);
    for (int k=0;k<n;k++) {
      p[k]=p1[p2[k]];
    }
    return(p);
  }
  
  gen _c1oc2(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    gen v1=v.front(),v2=v.back();
    if ( (v1.type!=_VECT) || (v2.type!=_VECT))
      return gentypeerr(contextptr);
    vector<int> c1,c2;
    //c1=vecteur_2_vector_int(*v1._VECTptr);
    //c2=vecteur_2_vector_int(*v2._VECTptr);
    if (!is_cycle(*v1._VECTptr,c1,contextptr) || !is_cycle(*v2._VECTptr,c2,contextptr))
      return gensizeerr(contextptr);
    return vector_int_2_vecteur(c1oc2(c1,c2),contextptr);
  }
  static const char _c1oc2_s[]="c1oc2";
  static define_unary_function_eval (__c1oc2,&_c1oc2,_c1oc2_s);
  define_unary_function_ptr5( at_c1oc2 ,alias_at_c1oc2,&__c1oc2,0,true);
 
  vector<int> c1op2(const vector<int> & c1, const vector<int> & p2) {
    //composition d'un cycle et d'une perm en une perm de long min
    vector<int> p1,p3;    
    p1=cycle2perm(c1);
    int n1;
    n1=int(p1.size());
    int n2;
    n2=int(p2.size());
    p3=p2;
    int n;
    if (n1>n2) {
      n=n1;
      for (int k=n2;k<n;k++) {p3.push_back(k);} 
    } 
    else {
      n=n2;
      for (int k=n1;k<n;k++) {p1.push_back(k);} 
    }
    vector<int> p(n);
    for (int k=0;k<n;k++) {
      p[k]=p1[p3[k]];
    }
    return(p);
  }
  
  gen _c1op2(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    gen v1=v.front(),v2=v.back();
    if ( (v1.type!=_VECT) || (v2.type!=_VECT))
      return gentypeerr(contextptr);
    vector<int> c1,p2;
    if (!is_cycle(*v1._VECTptr,c1,contextptr) || !is_permu(*v2._VECTptr,p2,contextptr))
      return gensizeerr(contextptr);
    return vector_int_2_vecteur(c1op2(c1,p2),contextptr);
  }
  static const char _c1op2_s[]="c1op2";
  static define_unary_function_eval (__c1op2,&_c1op2,_c1op2_s);
  define_unary_function_ptr5( at_c1op2 ,alias_at_c1op2,&__c1op2,0,true);

  vector<int> p1oc2(const vector<int> & p1, const vector<int> & c2) {
    //composition d'une perm et d'un cycle en une perm de long min
    vector<int> p2,p3;
    p2=cycle2perm(c2);
    int n2;
    n2=int(p2.size());
    int n1;
    n1=int(p1.size());
    p3=p1;
    int n;
    if (n1>n2) {n=n1;
      for (int k=n2;k<n;k++) {p2.push_back(k);} 
    } else {n=n2;
      for (int k=n1;k<n;k++) {p3.push_back(k);} 
    }
    vector<int> p(n);
    for (int k=0;k<n;k++) {
      p[k]=p3[p2[k]];
    }
    return(p);
  }
  
  gen _p1oc2(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    gen v1=v.front(),v2=v.back();
    if ( (v1.type!=_VECT) || (v2.type!=_VECT))
      return gentypeerr(contextptr);
    vector<int> p1,c2;
    if (!is_cycle(*v2._VECTptr,c2,contextptr) || !is_permu(*v1._VECTptr,p1,contextptr))
      return gensizeerr(contextptr);
    return vector_int_2_vecteur(p1oc2(p1,c2),contextptr);
  }
  static const char _p1oc2_s[]="p1oc2";
  static define_unary_function_eval (__p1oc2,&_p1oc2,_p1oc2_s);
  define_unary_function_ptr5( at_p1oc2 ,alias_at_p1oc2,&__p1oc2,0,true);

  vector<int> cycles2permu(const vector< vector<int> > & c) {
    //transforme une liste de cycles en la permutation produit
    int n;
    n=int(c.size());
    vector<int> pk;
    vector<int> p;
    vector<int> ck;
    vector<int> c1;
    ck=c[n-1];    
    vector<int> c0(1);
    c0[0]=0;
    p=c1oc2(ck,c0);
    for (int k=n-2;k>=0;k--){
      ck=c[k];
      p=c1op2(ck,p);
    } 
    return(p);
  }

  gen _cycles2permu(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT) 
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr); 
    vecteur::const_iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      vector<int> c;
      if (it->type!=_VECT || !is_cycle(*it->_VECTptr,c,contextptr))
	return gensizeerr(contextptr);
    }
    return vector_int_2_vecteur(cycles2permu(vecteur_2_vectvector_int(v,contextptr)),contextptr);
  }
  static const char _cycles2permu_s[]="cycles2permu";
  static define_unary_function_eval (__cycles2permu,&_cycles2permu,_cycles2permu_s);
  define_unary_function_ptr5( at_cycles2permu ,alias_at_cycles2permu,&__cycles2permu,0,true);

  vector< vector<int> > permu2cycles(const vector<int> & p) {
    //transforme la permutation p en une liste de cycles (p= produit de ces cycles)
    int l=int(p.size());
    int n=0;
    int deb;
    vector<int> p1(l);
    p1=p;
    vector<int>  temp(l+1);
    vector< vector<int> > c;
    if (p1[l-1]==l-1) {
      vector<int> c1;  
      c1.push_back(l-1);
      c.push_back(c1); l=l-1;
    }
    temp[l]=0;
    for (int k=0;k<l;k++) temp[k]=p1[k];
    while (n<l){ 
      vector<int> v;
      v.push_back(n);deb=n;
      while (p1[n]!=deb){
	v.push_back(p1[n]);
	temp[n]=0;
	n=p1[n];
      }
      if (n!=deb) {c.push_back(v);}
      temp[n]=0;
      n=deb+1;
      while ((n<l)&&(temp[n]==0)) n++;
    }
    return(c);
  }

  gen _permu2cycles(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT) 
      return gensizeerr(contextptr);
    vecteur v(*args._VECTptr);
    vector<int> p;
    if (!is_permu(v,p,contextptr))
      return gensizeerr(contextptr);
    return gen(vectvector_int_2_vecteur(permu2cycles(p),contextptr),_LIST__VECT);
  }

  static const char _permu2cycles_s[]="permu2cycles";
  static define_unary_function_eval (__permu2cycles,&_permu2cycles,_permu2cycles_s);
  define_unary_function_ptr5( at_permu2cycles ,alias_at_permu2cycles,&__permu2cycles,0,true);

  vector<int> perminv(const vector<int> & p){
    int n;
    n=int(p.size());
    vector<int> p1(n);
    for (int j=0;j<n;j++){
      p1[p[j]]=j;
    }
    return p1;
  }
  gen _perminv(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if  (args.type!=_VECT) 
      return gensizeerr(contextptr);
    vecteur v(*args._VECTptr); 
    vector<int> p;
    if (!is_permu(v,p,contextptr))
      return gensizeerr(contextptr);
    return vector_int_2_vecteur(perminv(p),contextptr);
  }
  static const char _perminv_s[]="perminv";
  static define_unary_function_eval (__perminv,&_perminv,_perminv_s);
  define_unary_function_ptr5( at_perminv ,alias_at_perminv,&__perminv,0,true);
  
  vector<int> cycleinv(const vector<int> & c){
    int n;
    n=int(c.size());
    vector<int> c1(n);
    for (int j=0;j<n;j++){
      c1[j]=c[n-j-1];
    }
    return c1;
  }
  gen _cycleinv(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if  (args.type!=_VECT) 
      return gensizeerr(contextptr);
    vecteur v(*args._VECTptr);
    vector<int> c;
    if (!is_cycle(v,c,contextptr))
      return gensizeerr(contextptr);
    return vector_int_2_vecteur(cycleinv(c),contextptr);
  }
  static const char _cycleinv_s[]="cycleinv";
  static define_unary_function_eval (__cycleinv,&_cycleinv,_cycleinv_s);
  define_unary_function_ptr5( at_cycleinv ,alias_at_cycleinv,&__cycleinv,0,true);
  

  int signature(const vector<int> & p) {
    //renvoie la signature de la permutation p
    int s; 
    s=1;     
    vector< vector<int> > c;
    c=permu2cycles(p);
    int l=int(c.size());
    for (int k=0;k<l;k++){
      int lk=int(c[k].size())-1;
      if (lk%2) s=s*-1;
    }
    return(s);
  }

  gen _signature(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT) 
      return gensizeerr(contextptr);
    vecteur v(*args._VECTptr);
    vector<int> p;
    if (!is_permu(v,p,contextptr))
      return gensizeerr(contextptr);
    return signature(p);
  }

  static const char _signature_s[]="signature";
  static define_unary_function_eval (__signature,&_signature,_signature_s);
  define_unary_function_ptr5( at_signature ,alias_at_signature,&__signature,0,true);

  vecteur vector_giac_double_2_vecteur(const vector<giac_double> & v){
    //transforme un vector<double> en vecteur 
    vector<giac_double>::const_iterator it=v.begin(),itend=v.end();
    vecteur res;
    res.reserve(itend-it);
    for (;it!=itend;++it)
      res.push_back(double(*it));
    return res;
  } 

  vecteur vectvector_giac_double_2_vecteur(const vector< vector<giac_double> > & v){
    //transforme un vector< vector<double> > en vecteur  
    int s=int(v.size());
    vecteur res;
    res.reserve(s);
    for (int i=0;i<s;++i)
      res.push_back(vector_giac_double_2_vecteur(v[i]));
    return res;
  }  
  
  gen _hilbert(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    int n,p;
    if (args.type==_INT_) {
      n=args.val;
      p=args.val;
    }
    else {
      if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
	return gentypeerr(contextptr);
      vecteur v(*args._VECTptr);
      gen v1=v.front(),v2=v.back();
      n=v1.val;
      p=v2.val; 
    }   
    vecteur c;
    for (int k=0;k<n;k++){
      vecteur l(p);
      for (int j=0;j<p;j++){
	l[j]=rdiv(1,k+j+1,contextptr);
      }
      c.push_back(l);
    } 
    return gen(c,_MATRIX__VECT);
  }

  static const char _hilbert_s[]="hilbert";
  static define_unary_function_eval (__hilbert,&_hilbert,_hilbert_s);
  define_unary_function_ptr5( at_hilbert ,alias_at_hilbert,&__hilbert,0,true);

  gen l2norm2(const gen & g){
    if (g.type!=_VECT)
      return g*g;
    const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();    
    gen res(0);
    mpz_t tmpz;
    mpz_init(tmpz);
    for (;it!=itend;++it){
      if (res.type==_ZINT && is_integer(*it)){
#if defined(INT128) && !defined(USE_GMP_REPLACEMENTS)
	if (it->type==_INT_)
	  mpz_add_ui(*res._ZINTptr,*res._ZINTptr,longlong(it->val)*(it->val));
	else {
	  mpz_mul(tmpz,*it->_ZINTptr,*it->_ZINTptr);
	  mpz_add(*res._ZINTptr,*res._ZINTptr,tmpz);
	}
#else
	if (it->type==_INT_){
	  mpz_set_si(tmpz,it->val);
	  mpz_mul(tmpz,tmpz,tmpz);
	}
	else 
	  mpz_mul(tmpz,*it->_ZINTptr,*it->_ZINTptr);
	mpz_add(*res._ZINTptr,*res._ZINTptr,tmpz);
#endif
      }
      else
	res +=  (*it)*(*it);
    }
    mpz_clear(tmpz);
    return res;
  }

  gen square_hadamard_bound(const matrice & m){
    const_iterateur it=m.begin(),itend=m.end();
    gen prod(1);
    for (;it!=itend;++it){
      type_operator_times(prod,l2norm2(*it),prod);
      // prod=prod*l2norm2(*it);
    }
    return prod;
  }

  gen _hadamard(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (ckmatrix(args) || args[0][0].type!=_VECT){
      // Hadamard bound on det(args)
      matrice & m=*args._VECTptr;
      if (has_num_coeff(m)){
	gen r=1.0;
	for (unsigned i=0;i<m.size();++i)
	  r = r*l2norm(*m[i]._VECTptr,contextptr);
	return r;
      }
      return sqrt(min(square_hadamard_bound(m),square_hadamard_bound(mtran(m)),contextptr),contextptr);
    }
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    gen g1=v.front(),g2=v.back();
     
    if ((g1.type!=_VECT) ||(g2.type!=_VECT))
      return gentypeerr(contextptr);
    vecteur v1(*g1._VECTptr); 
    vecteur v2(*g2._VECTptr);
    if (v1.size()!=v2.size()) return gensizeerr(contextptr);
    int n=int(v1.size());
    vecteur c;
    for (int k=0;k<n;k++){
      if ((v1[k].type!=_VECT) ||(v2[k].type!=_VECT)) return gentypeerr(contextptr); 
      vecteur l1(*(v1[k])._VECTptr);
      vecteur l2(*(v2[k])._VECTptr);
      if (l1.size()!=l2.size()) return gensizeerr(contextptr);
      int p=int(l1.size());
      vecteur l(p);
      for (int j=0;j<p;j++){
	l[j]=l1[j]*l2[j];
      }

      c.push_back(l);
    } 
    return gen(c,_MATRIX__VECT);
  }
  static const char _hadamard_s[]="hadamard";
  static define_unary_function_eval (__hadamard,&_hadamard,_hadamard_s);
  define_unary_function_ptr5( at_hadamard ,alias_at_hadamard,&__hadamard,0,true);

  gen _trn(const gen & args,GIAC_CONTEXT){
    return conj(_tran(args,contextptr),contextptr);
  }

  static const char _trn_s[]="trn";
  static define_unary_function_eval (__trn,&_trn,_trn_s);
  define_unary_function_ptr5( at_trn ,alias_at_trn,&__trn,0,true);

  gen _syst2mat(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    gen g1=_equal2diff(v.front(),contextptr),g2=v.back();
    
    if ((g1.type!=_VECT) ||(g2.type!=_VECT))
      return gentypeerr(contextptr);
    vecteur v1(*g1._VECTptr);
    vecteur v2(*g2._VECTptr);
    
    int n=int(v2.size()),m=int(v1.size());
    vecteur c;
    for (int k=0;k<m;k++){
      vecteur l(n+1);
      gen ln=v1[k];
      for (int j=0;j<n;j++){
	l[j]=derive(v1[k],v2[j],contextptr);
	if (is_undef(l[j])) return l[j];
	ln=subst(ln,v2[j],0,false,contextptr);
      }
      l[n]=ln;
      c.push_back(l);
    } 
    return gen(c,_MATRIX__VECT);
  }
  static const char _syst2mat_s[]="syst2mat";
  static define_unary_function_eval (__syst2mat,&_syst2mat,_syst2mat_s);
  define_unary_function_ptr5( at_syst2mat ,alias_at_syst2mat,&__syst2mat,0,true);

  gen _vandermonde(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)  
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    int n=int(v.size()),m=n;
    if (n==2 && v[1].type==_INT_ && v[0].type==_VECT){
      m=v[1].val;
      v=*v[0]._VECTptr;
      n=int(v.size());
    }
    vecteur c; 
    vecteur l(n); 
    for (int j=0;j<n;j++){
      l[j]=1;
    }
    c.push_back(l);
    for (int k=1;k<m;k++){
      for (int j=0;j<n;j++){
	l[j]=l[j]*v[j];
      }
      c.push_back(l);
    }
    return gen(mtran(c),_MATRIX__VECT);
  }
  static const char _vandermonde_s[]="vandermonde";
  static define_unary_function_eval (__vandermonde,&_vandermonde,_vandermonde_s);
  define_unary_function_ptr5( at_vandermonde ,alias_at_vandermonde,&__vandermonde,0,true);


  gen _laplacian(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_DOUBLE_ && args._DOUBLE_val==int(args._DOUBLE_val)){
      return evalf(_laplacian(int(args._DOUBLE_val),contextptr),1,context0);
    }
    if (args.type==_IDNT){
      gen g=eval(args,1,contextptr);
      if (g.type==_INT_)
	return _laplacian(g,contextptr);
    }
    if (args.type==_INT_ && args.val>0 && args.val<int(std::sqrt(double(LIST_SIZE_LIMIT)))){
      // discrete 1-d laplacian matrix
      int n=args.val;
      vecteur res(n);
      for (int i=0;i<n;++i){
	vecteur resi(n);
	if (i)
	  resi[i-1]=-1;
	resi[i]=2;
	if (i<n-1)
	  resi[i+1]=-1;
	res[i]=resi;
      }
      return res;
    }
    if (args.type==_VECT && args._VECTptr->size()==3){
      gen opt=args._VECTptr->back();
      if (opt.is_symb_of_sommet(at_equal) && (opt._SYMBptr->feuille[0]==at_coordonnees || (opt._SYMBptr->feuille[0].type==_INT_ && opt._SYMBptr->feuille[0].val==_COORDS))){
	gen f=eval(args._VECTptr->front(),1,contextptr);
	gen coord=(*args._VECTptr)[1];
	if (coord.type==_VECT &&  coord._VECTptr->size()==3){
	  if (opt._SYMBptr->feuille[1]==at_sphere){
	    gen r=coord[0],r2=pow(r,2,contextptr),t=coord[1],s=sin(t,contextptr),p=coord[2];
	    gen res=derive(r2*derive(f,r,contextptr),r,contextptr);
	    res += derive(derive(s*f,t,contextptr),t,contextptr)/s;
	    res += derive(derive(f,p,contextptr),p,contextptr)/(s*s);
	    return res/r2;
	  }
	  if (opt._SYMBptr->feuille[1]==at_cylindre){
	    gen r=coord[0],t=coord[1],z=coord[2];
	    gen res=derive(r*derive(f,r,contextptr),r,contextptr);
	    res += derive(derive(f,t,contextptr),t,contextptr)/(r*r);
	    res += derive(derive(f,z,contextptr),z,contextptr);
	    return res;
	  }
	}
      }
    }
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);    
    vecteur v(plotpreprocess(args,contextptr));
    if (is_undef(v))
      return v;
    gen g1=v.front(),g2=v.back();
    if (g2.type!=_VECT) 
      return gentypeerr(contextptr);
    vecteur v2(*g2._VECTptr);
    int n=int(v2.size());
    gen la;
    la=0;
    for (int k=0;k<n;k++){
      la=la+derive(derive(g1,v2[k],contextptr),v2[k],contextptr);
    } 
    return normal(la,contextptr);
  }
  static const char _laplacian_s[]="laplacian";
  static define_unary_function_eval_quoted (__laplacian,&_laplacian,_laplacian_s);
  define_unary_function_ptr5( at_laplacian ,alias_at_laplacian,&__laplacian,_QUOTE_ARGUMENTS,true);

  gen _hessian(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);        
    vecteur v(plotpreprocess(args,contextptr));
    if (is_undef(v))
      return v;
    gen g1=v.front(),g2=v.back();
    if (g2.type!=_VECT) 
      return gentypeerr(contextptr);
    vecteur v2(*g2._VECTptr);
    int n=int(v2.size());
    vecteur he;    
    for (int k=0;k<n;k++){
      vecteur l(n);
      for (int j=0;j<n;j++){
	l[j]=derive(derive(g1,v2[k],contextptr),v2[j],contextptr);
      }
      he.push_back(l);
    }
    return (he);
  }    
  static const char _hessian_s[]="hessian";
  static define_unary_function_eval_quoted (__hessian,&_hessian,_hessian_s);
  define_unary_function_ptr5( at_hessian ,alias_at_hessian,&__hessian,_QUOTE_ARGUMENTS,true);

  gen _divergence(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT && args._VECTptr->size()==3){
      gen opt=args._VECTptr->back();
      if (opt.is_symb_of_sommet(at_equal) && (opt._SYMBptr->feuille[0]==at_coordonnees || (opt._SYMBptr->feuille[0].type==_INT_ && opt._SYMBptr->feuille[0].val==_COORDS))){
	gen g=eval(args._VECTptr->front(),1,contextptr);
	gen coord=(*args._VECTptr)[1];
	if (g.type==_VECT && coord.type==_VECT && g._VECTptr->size()==3 && coord._VECTptr->size()==3){
	  if (opt._SYMBptr->feuille[1]==at_sphere){
	    // spheric: 1/r^2*d_r(r^2*A_r)+1/r/sin(theta)*d_theta(sin(theta)*A_theta)+1/r*sin(theta)*d_phi(A_phi)
	    gen Ar=g[0],At=g[1],Ap=g[2],r=coord[0],r2=pow(r,2,contextptr),t=coord[1],s=sin(t,contextptr),p=coord[2];
	    return derive(r2*Ar,r,contextptr)/r2+(derive(s*At,t,contextptr)+derive(Ap,p,contextptr))/(r*s);
	  }
	  if (opt._SYMBptr->feuille[1]==at_cylindre){
	    // cylindric: 1/r*d_r(r*A_r)+1/r*d_theta(A_theta)+d_z(A_z)
	    gen Ar=g[0],At=g[1],Az=g[2],r=coord[0],t=coord[1],z=coord[2];
	    return (derive(r*Ar,r,contextptr)+derive(At,t,contextptr))/r+derive(Az,z,contextptr);
	  }
	}
      }
    }
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);    
    vecteur v(plotpreprocess(args,contextptr));
    if (is_undef(v))
      return v;
    gen g1=v.front(),g2=v.back();
    if ((g1.type!=_VECT) ||(g2.type!=_VECT))
      return gentypeerr(contextptr);
    vecteur v1(*g1._VECTptr);
    vecteur v2(*g2._VECTptr);
    int n=int(v2.size());
    gen di;
    di=0;
    for (int k=0;k<n;k++){
      di=di+derive(v1[k],v2[k],contextptr);
    } 
    return normal(di,contextptr);
  }    
  static const char _divergence_s[]="divergence";
  static define_unary_function_eval_quoted (__divergence,&_divergence,_divergence_s);
  define_unary_function_ptr5( at_divergence ,alias_at_divergence,&__divergence,_QUOTE_ARGUMENTS,true);

  gen _curl(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT && args._VECTptr->size()==3){
      gen opt=args._VECTptr->back();
      if (opt.is_symb_of_sommet(at_equal) && (opt._SYMBptr->feuille[0]==at_coordonnees || (opt._SYMBptr->feuille[0].type==_INT_ && opt._SYMBptr->feuille[0].val==_COORDS))){
	gen g=eval(args._VECTptr->front(),1,contextptr);
	gen coord=(*args._VECTptr)[1];
	if (g.type==_VECT && coord.type==_VECT && g._VECTptr->size()==3 && coord._VECTptr->size()==3){
	  if (opt._SYMBptr->feuille[1]==at_sphere){
	    // spheric: 1/r/sin(theta)*(d_theta(sin(theta)*A_phi)-d_phi(A_theta)),
	    // 1/r/sin(theta)*d_phi(A_r)-1/r*d_r(r*A_phi), 1/r*(d_r(r*A_theta)-d_theta(A_r))
	    gen Ar=g[0],At=g[1],Ap=g[2],r=coord[0],r2=pow(r,2,contextptr),t=coord[1],s=sin(t,contextptr),p=coord[2];
	    return makevecteur((derive(s*Ap,t,contextptr)-derive(At,p,contextptr))/(r*s),derive(Ar,p,contextptr)/(r*s)-derive(r*Ap,r,contextptr)/r,(derive(r*At,r,contextptr)-derive(Ar,t,contextptr))/r);
	  }
	  if (opt._SYMBptr->feuille[1]==at_cylindre){
	    // cylindric (1/r*d_theta(A_z)-d_z(A_theta)),d_z(A_r)-d_r(A_z),1/r*d_r(r*A_theta)-d_theta(A_r))
	    gen Ar=g[0],At=g[1],Az=g[2],r=coord[0],t=coord[1],z=coord[2];
	    return makevecteur(derive(Az,t,contextptr)/r-derive(At,z,contextptr),derive(Ar,z,contextptr)-derive(Az,r,contextptr),(derive(r*At,r,contextptr)-derive(Ar,t,contextptr))/r);
	  }
	}
      }
    }
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(plotpreprocess(args,contextptr));
    if (is_undef(v))
      return v;
    gen g1=v.front(),g2=v.back();
    if ((g1.type!=_VECT) ||(g2.type!=_VECT))
      return gentypeerr(contextptr);
    vecteur v1(*g1._VECTptr);
    vecteur v2(*g2._VECTptr);
    int n=int(v2.size());
    if (n!=3) return gensizeerr(contextptr);
    vecteur rot(3);
    rot[0]=derive(v1[2],v2[1],contextptr)-derive(v1[1],v2[2],contextptr);
    rot[1]=derive(v1[0],v2[2],contextptr)-derive(v1[2],v2[0],contextptr);
    rot[2]=derive(v1[1],v2[0],contextptr)-derive(v1[0],v2[1],contextptr);
    return rot;
  }
  static const char _curl_s[]="curl";
  static define_unary_function_eval_quoted (__curl,&_curl,_curl_s);
  define_unary_function_ptr5( at_curl ,alias_at_curl,&__curl,_QUOTE_ARGUMENTS,true);

  bool find_n_x(const gen & args,int & n,gen & x,gen & a){
    if (args.type==_INT_){
      n=args.val;
      x=vx_var;
      a=a__IDNT_e;
      return n>=0;
    }
    if (args.type==_DOUBLE_){
      n=int(args._DOUBLE_val);
      if (n!=args._DOUBLE_val)
	return false;
      x=vx_var;
      a=a__IDNT_e;
      return n>=0;
    }
    if (args.type!=_VECT || args._VECTptr->size()<2)
      return false;
    vecteur v=*args._VECTptr;
    if (v[0].type==_DOUBLE_ && v[0]._DOUBLE_val==int(v[0]._DOUBLE_val))
      v[0]=int(v[0]._DOUBLE_val);
    if (v[0].type==_INT_){
      n=v[0].val;
      x=v[1];
    }
    else
      return false;
    if (v.size()>2)
      a=v[2];
    else
      a=a__IDNT_e;
    return n>=0;
  }

  vecteur hermite(int n){
    vecteur v(n+1);
    v[0]=pow(plus_two,n);
    for (int k=2;k<=n;k+=2){
      v[k]=-((n+2-k)*(n+1-k)*v[k-2])/(2*k);
      if (is_undef(v[k]))
	return v;
    }
    return v;
  }

  bool poly2symbmat(matrice & A,const gen & var,GIAC_CONTEXT){
    iterateur it=A.begin(),itend=A.end();
    for (;it!=itend;++it){
      if (it->type!=_VECT) return false;
      iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
      for (;jt!=jtend;++jt){
	*jt=_poly2symb(makesequence(*jt,var),contextptr);
      }
    }
    return true;
  }

  bool unmod(std_matrix<gen> & a,const gen & p){
    for (size_t i=0;i<a.size();++i){
      dbgprint_vector<gen> & v=a[i];
      for (size_t j=0;j<v.size();++j){
	gen & g=v[j];
	if (g.type==_MOD){
	  if (*(g._MODptr+1)!=p)
	    return false;
	  g=unmod(g);
	}
	if (g.type==_VECT){
	  iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
	  for (;it!=itend;++it){
	    gen & h =*it;
	    if (h.type==_MOD){
	      if (*(h._MODptr+1)!=p)
		return false;
	      h=unmod(h);
	    }
	  }
	}
      }
    }
    return true;
  }

  bool hnf(const matrice & Aorig,const gen & var,matrice & U,matrice & A,matrice & V,bool do_smith,GIAC_CONTEXT){
    std_matrix<gen> aorig,u,a,v;
    if (var.type==_VECT){ 
      if (var._VECTptr->empty())
	matrice2std_matrix_gen(Aorig,aorig);
      else
	return false; // multivariate polynomial are not yet allowed
    }
    else {
      gen tmp=_symb2poly(makesequence(Aorig,var),contextptr);
      if (!ckmatrix(tmp))
	return false;
      matrice2std_matrix_gen(*tmp._VECTptr,aorig);
    }
    // fixme, detect modular computation
    environment env;
    gen g=aorig[0][0];
    if (g.type==_VECT && g._VECTptr->back().type==_MOD){
      env.modulo=*(g._VECTptr->back()._MODptr+1);
      env.moduloon=true;
      if (!unmod(aorig,env.modulo))
	return false;
    }
    else 
      env.moduloon=false;
    if (do_smith){
      if (!smith(aorig,u,a,v,&env,contextptr))
	return false;
    }
    else {
      if (!hermite(aorig,u,a,&env,contextptr))
	return false;
    }
    std_matrix_gen2matrice_destroy(u,U);
    std_matrix_gen2matrice_destroy(a,A);
    if (do_smith)
      std_matrix_gen2matrice_destroy(v,V);
    if (var.type!=_VECT || !var._VECTptr->empty()) {
      poly2symbmat(U,var,contextptr);
      poly2symbmat(A,var,contextptr);
      if (do_smith)
	poly2symbmat(V,var,contextptr);
    }
    if (env.moduloon){
      U=*makemod(U,env.modulo)._VECTptr;
      A=*makemod(A,env.modulo)._VECTptr;
      if (do_smith)
	V=*makemod(V,env.modulo)._VECTptr;
    }
    return true;
  }

  gen _hermite(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    int n;
    gen a,x;
    if (!find_n_x(args,n,x,a)){
      matrice U,A,V; // U invertible and A such that U*args==A
      if (ckmatrix(args) && hnf(*args._VECTptr,ggb_var(args),U,A,V,false,contextptr))
	return makesequence(U,A);	
      if (args.type==_VECT && args._VECTptr->size()==2 && ckmatrix(args._VECTptr->front()) && hnf(*args._VECTptr->front()._VECTptr,args._VECTptr->back(),U,A,V,false,contextptr))
	return makesequence(U,A);
      return gensizeerr(contextptr);
    }
    return r2e(hermite(n),x,contextptr);
  }
  static const char _hermite_s[]="hermite";
  static define_unary_function_eval (__hermite,&_hermite,_hermite_s);
  define_unary_function_ptr5( at_hermite ,alias_at_hermite,&__hermite,0,true);

  gen _smith(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    matrice U,A,V; // U invertible and A such that U*args==A
    if (ckmatrix(args) && hnf(*args._VECTptr,ggb_var(args),U,A,V,true,contextptr))
      return makesequence(U,A,V);	
    if (args.type==_VECT && args._VECTptr->size()==2 && ckmatrix(args._VECTptr->front()) && hnf(*args._VECTptr->front()._VECTptr,args._VECTptr->back(),U,A,V,true,contextptr))
      return makesequence(U,A,V);
    return gensizeerr(contextptr);
  }
  static const char _smith_s[]="smith";
  static define_unary_function_eval (__smith,&_smith,_smith_s);
  define_unary_function_ptr5( at_smith ,alias_at_smith,&__smith,0,true);

  vecteur laguerre(int n){
    // L_k(x)=v_k(x)/k!
    // L_{n+1}=1/(n+1)*((2n+1-x)*L_n(x)-nL_{n-1}(x))
    // hence v_{n+1}=(2n+1-x)v_n-n*(n-1)*v_{n-1}
    vecteur v0,v1,tmp1,tmp2,tmpx;
    v0.reserve(n+1);
    v1.reserve(n+1);
    tmp1.reserve(n+1);
    tmp2.reserve(n+1);
    tmpx=makevecteur(-1,0);
    v0.push_back(1); // v0=1
    v1.push_back(-1);
    v1.push_back(1); // v1=1-x
    for (int k=1;k<n;k++){
      tmpx[1]=2*k+1;
      mulmodpoly(tmpx,v1,0,tmp1);
      mulmodpoly(v0,gen(k*k),0,tmp2);
      submodpoly(tmp1,tmp2,0,tmp1);
      // v0, v1, tmp1 -> v1, v0, tmp1 -> v1, tmp1, v0
      v0.swap(v1);
      v1.swap(tmp1);
      if (is_undef(v1))
	return v1;
      // cerr << v0 << " " << v1 << '\n';
    }
    return v1;
  }

  gen _laguerre(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    int n;
    gen a,x;
    if (!find_n_x(args,n,x,a))
      return gensizeerr(contextptr);
    if (is_zero(a))
      return inv(factorial(n),contextptr)*symb_horner(laguerre(n),x);
    gen p0,p1,p2;
    p0=1;
    p1=1+a-x;
    if (n==0) return p0;
    if (n==1) return p1;
    for (int k=2;k<=n;k++){
      //p2=rdiv(2*k+a-1-x,k)*p1-rdiv(k+a-1,k)*p0;
      p2=(2*k+a-1-x)*p1-(k-1)*(k+a-1)*p0;
      p0=p1;
      p1=p2;  
    } 
    //return normal(p2,contextptr);
    return normal(rdiv(p2,factorial(n),contextptr),contextptr);
  }
    
  static const char _laguerre_s[]="laguerre";
  static define_unary_function_eval (__laguerre,&_laguerre,_laguerre_s);
  define_unary_function_ptr5( at_laguerre ,alias_at_laguerre,&__laguerre,0,true);

  // Improved one
  vecteur tchebyshev1(int n){
    if (n==0) return vecteur(1,1);
    vecteur v(n+1);
    v[0]=pow(gen(2),n-1);
    if (n==1) return v;
    for (int k=2;k<=n;k+=2){
      v[k]=-((n-k+2)*(n-k+1)*v[k-2])/(2*k*(n-k/2));
      if (is_undef(v[k]))
	return v;
    }
    return v;
  }
  gen tchebyshev_eval(const gen & n,const gen &x,const vecteur & v,GIAC_CONTEXT){
    if (is_zero(n))
      return 1;
    if (!is_positive(n,contextptr))
      return gensizeerr(contextptr);
#if 1
    vecteur w(v);
    modpoly X(makevecteur(1,0));
    modpoly B(makevecteur(1,-2*x,1));
    environment env;
    if (x.type==_MOD){
      env.moduloon=true;
      env.modulo=*(x._MODptr+1);
      B[1]=*B[1]._MODptr;
      if (v[1].type==_MOD)
	w[1]=*v[1]._MODptr;
    }
    else
      env.moduloon=false;
    modpoly res=powmod(X,n,B,&env);
    if (res.empty())
      return gensizeerr(contextptr);
    if (res.size()==1)
      res[0]=res[0]*w[0];
    else {
      matrice A(makevecteur(makevecteur(res[1],res[0]),makevecteur(-res[0],2*res[0]*x+res[1])));
      multmatvecteur(A,w,res);
    }
    if (env.moduloon && res[0].type!=_MOD)
      res[0]=makemod(res[0],env.modulo);
    return res[0];
#else
    matrice A(makevecteur(makevecteur(0,1),makevecteur(-1,2*x)));
    gen An=pow(A,n,contextptr);
    if (An.type!=_VECT)
      return undef;
    An=ckmultmatvecteur(*An._VECTptr,v,context0);
    return An[0];
#endif
  }
  gen _tchebyshev1(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    int n;
    gen a,x;
    if (args.type==_VECT && args._VECTptr->size()==2 && is_integer(args._VECTptr->front()) && args._VECTptr->back().type!=_IDNT)
      return tchebyshev_eval(args._VECTptr->front(),args._VECTptr->back(),makevecteur(1,args._VECTptr->back()),contextptr);
    if (!find_n_x(args,n,x,a))
      return gensizeerr(contextptr);
    if (n==0)
      return 1;
    return r2e(tchebyshev1(n),x,contextptr);
  }
  static const char _tchebyshev1_s[]="tchebyshev1";
  static define_unary_function_eval (__tchebyshev1,&_tchebyshev1,_tchebyshev1_s);
  define_unary_function_ptr5( at_tchebyshev1 ,alias_at_tchebyshev1,&__tchebyshev1,0,true);

  // Improved one
  vecteur tchebyshev2(int n){
    vecteur v(n+1);
    v[0]=pow(gen(2),n);
    for (int k=1;k<=n/2;++k){
      v[2*k]=-(n+2-2*k)*(n+1-2*k)*v[2*k-2]/(4*k*(n+1-k));
      if (is_undef(v[2*k]))
	return v;
    }
    return v;
  }
  gen _tchebyshev2(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT && args._VECTptr->size()==2 && is_integer(args._VECTptr->front()) && args._VECTptr->back().type!=_IDNT)
      return tchebyshev_eval(args._VECTptr->front()+1,args._VECTptr->back(),makevecteur(0,1),contextptr);
    int n;
    gen a,x;
    if (!find_n_x(args,n,x,a))
      return gensizeerr(contextptr);
    return r2e(tchebyshev2(n),x,contextptr);
  }
  static const char _tchebyshev2_s[]="tchebyshev2";
  static define_unary_function_eval (__tchebyshev2,&_tchebyshev2,_tchebyshev2_s);
  define_unary_function_ptr5( at_tchebyshev2 ,alias_at_tchebyshev2,&__tchebyshev2,0,true);

  // Legendre is the vecteur returned divided by n!
  vecteur legendre(int n){
    vecteur v0,v1,vtmp1,vtmp2;
    v0.push_back(1);
    v1.push_back(1);
    v1.push_back(0);
    if (!n) return v0;
    if (n==1) return v1;
    for (int k=2;k<=n;k++){
      multvecteur(2*k-1,v1,vtmp1);
      vtmp1.push_back(0); // (2k-1)*x*p1
      multvecteur((k-1)*(k-1),v0,vtmp2); // (k-1)^2*p0
      vtmp1=vtmp1-vtmp2; // p2=(2*k-1)*x*p1-(k-1)*(k-1)*p0;
      v0=v1;
      v1=vtmp1;
    } 
    return v1; 
  }

  gen _legendre(const gen & args,GIAC_CONTEXT){ 
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    int n;
    gen a,x;
    if (!find_n_x(args,n,x,a))
      return gensizeerr(contextptr);
    vecteur v=multvecteur(inv(factorial(n),contextptr),legendre(n));
    return r2e(v,x,contextptr);
  }
  static const char _legendre_s[]="legendre";
  static define_unary_function_eval (__legendre,&_legendre,_legendre_s);
  define_unary_function_ptr5( at_legendre ,alias_at_legendre,&__legendre,0,true);

  // arithmetic mean column by column
  vecteur mean(const matrice & m,bool column){
    matrice mt;
    if (column)
      mt=mtran(m);
    else
      mt=m;
    vecteur res;
    const_iterateur it=mt.begin(),itend=mt.end();
    for (;it!=itend;++it){
      const gen & g =*it;
      if (g.type!=_VECT){
	res.push_back(g);
	continue;
      }
      vecteur & v=*g._VECTptr;
      if (v.empty()){
	res.push_back(undef);
	continue;
      }
      const_iterateur jt=v.begin(),jtend=v.end();
      int s=int(jtend-jt);
      gen somme(0);
      for (;jt!=jtend;++jt){
	//somme = somme + evalf(*jt);
	somme = somme + *jt;
      }
      res.push_back(rdiv(somme,s,context0));
    }
    return res;
  }

  vecteur stddev(const matrice & m,bool column,int variance){
    matrice mt;
    if (column)
      mt=mtran(m);
    else
      mt=m; 
    vecteur moyenne(mean(mt,false));
    vecteur res;
    const_iterateur it=mt.begin(),itend=mt.end();
    for (int i=0;it!=itend;++it,++i){
      const gen & g =*it;
      if (g.type!=_VECT){
	res.push_back(0);
	continue;
      }
      vecteur & v=*g._VECTptr;
      if (v.empty()){
	res.push_back(undef);
	continue;
      }
      const_iterateur jt=v.begin(),jtend=v.end();
      int s=int(jtend-jt);
      gen somme(0);
      for (;jt!=jtend;++jt){
	// somme = somme + evalf((*jt)*(*jt));
	somme = somme + (*jt)*(*jt);
      }
      if (variance!=3)
	res.push_back(sqrt(rdiv(somme-s*moyenne[i]*moyenne[i],s-(variance==2),context0),context0));
      else
	res.push_back(rdiv(somme,s,context0)-moyenne[i]*moyenne[i]);
    }
    return res;
  }

  matrice ascsort(const matrice & m,bool column){
    matrice mt;
    if (column)
      mt=mtran(m);
    else
      mt=m;
    iterateur it=mt.begin(),itend=mt.end();
    gen tmp;
    for (;it!=itend;++it){
      gen & g =*it;
      if (g.type!=_VECT)
	continue;
      vecteur v=*g._VECTptr;
      if (v.empty())
	continue;
      const_iterateur jt=v.begin(),jtend=v.end();
      int n=int(jtend-jt);
      vector<double> vv(n);
      for (int j=0;jt!=jtend;++jt,++j){
	if ( (jt->type==_VECT) && (jt->_VECTptr->size()==3) )
	  tmp=(*jt->_VECTptr)[1];
	else
	  tmp=*jt;
	tmp=evalf(tmp,1,0);
	if (tmp.type!=_DOUBLE_)
	  vv[j]=0;
	else
	  vv[j]=tmp._DOUBLE_val;
      }
      sort(vv.begin(),vv.end());
      for (int j=0;j<n;++j)
	v[j]=vv[j];
      *it=v;
    }
    return mt;
  }

  gen _permu2mat(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    //transforme une permutation en une matrice obtenue en permutant les lignes de la matrice identite  
    if (args.type!=_VECT)  
      return gentypeerr(contextptr); 
    vector<int> p1;
    vecteur p(*args._VECTptr);
    if (!(is_permu(p,p1,contextptr)))  
      return gentypeerr(contextptr);
    int n=int(p.size());
    vecteur c; 
    vecteur l(n); 
    for (int k=0;k<n;k++){
      for (int j=0;j<n;j++){
	if (p[k]==j+array_start(contextptr)) {
	  l[j]=1;
	} else {
	  l[j]=0;
	}
      }
      c.push_back(l);
    }
    return c;
  }

  static const char _permu2mat_s[]="permu2mat";
  static define_unary_function_eval (__permu2mat,&_permu2mat,_permu2mat_s);
  define_unary_function_ptr5( at_permu2mat ,alias_at_permu2mat,&__permu2mat,0,true);

  static bool est_dans(const vector<int> & a , const int n, vector< vector<int> > s) {
    //teste si a est egal a l'un des n premiers elements de s 
    bool cont=true;
    int j=0;
    while (j<=n && cont) {
      if (a==s[j]) {
	cont=false;
      }
      j=j+1;
    }
    return (! cont);
  } 

  vector< vector<int> > groupermu(const vector<int> & p1,const vector<int> & p2) {
    //groupe engendre par de 2 perm p1 et p2 de long n1 et n2 : a pour long max(n1,n2)
    int n1;
    n1=int(p1.size());
    int n2;
    n2=int(p2.size());
    vector<int> a;
    vector<int> b;
    a=p1;
    b=p2;
    if (n1>n2) {
      for (int k=n2;k<n1;k++) {b.push_back(k);n2=n1;} 
    } else {
      for (int k=n1;k<n2;k++) {a.push_back(k);n1=n2;}
    }
    ;
    vector< vector<int> > s(2);
    s[0]=a;
    s[1]=b;
    int p=0;
    int q=1;
    bool pfini=true;
    while (pfini) {
      int k=0;
      for (int j=p;j<=q;j++){
	vector<int> na;
	vector<int> nb;
	na=p1op2(a,s[j]);
	if (!(est_dans(na,q+k,s))){
	  k=k+1;
	  s.push_back(na);
	}
	nb=p1op2(b,s[j]);
	if (!(est_dans(nb,q+k,s))){
	  k=k+1;
	  s.push_back(nb);
	}
      }
      if (k!=0) {
	p=q+1;
	q=q+k;
      } else {
	pfini=false;
      }
    } 
    //q est l'ordre du groupe = size(s)     
    return(s);
  }
 
  gen _groupermu(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    gen v1=v.front(),v2=v.back();
    if ( (v1.type!=_VECT) || (v2.type!=_VECT))
      return gentypeerr(contextptr);
    vector<int> p1,p2;   
    if (!is_permu(*v1._VECTptr,p1,contextptr) || !is_permu(*v2._VECTptr,p2,contextptr))
      return gensizeerr(contextptr);
    return vectvector_int_2_vecteur(groupermu(p1,p2),contextptr);
  }
  static const char _groupermu_s[]="groupermu";
  static define_unary_function_eval (__groupermu,&_groupermu,_groupermu_s);
  define_unary_function_ptr5( at_groupermu ,alias_at_groupermu,&__groupermu,0,true);

  gen _nextperm(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)
      return gentypeerr(contextptr);
    const vecteur & v1=*args._VECTptr;
    vector<int> p1;
    if (!is_permu(v1,p1,contextptr))
      return gensizeerr(contextptr);
    if (next_permutation(p1.begin(),p1.end()))
      return vector_int_2_vecteur(p1,contextptr);
    else
      return undef;
  }
  static const char _nextperm_s[]="nextperm";
  static define_unary_function_eval (__nextperm,&_nextperm,_nextperm_s);
  define_unary_function_ptr5( at_nextperm ,alias_at_nextperm,&__nextperm,0,true);

  gen _prevperm(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)
      return gentypeerr(contextptr);
    const vecteur & v1=*args._VECTptr;
    vector<int> p1;
    if (!is_permu(v1,p1,contextptr))
      return gensizeerr(contextptr);
    if (prev_permutation(p1.begin(),p1.end()))
      return vector_int_2_vecteur(p1,contextptr);
    else
      return undef;
  }
  static const char _prevperm_s[]="prevperm";
  static define_unary_function_eval (__prevperm,&_prevperm,_prevperm_s);
  define_unary_function_ptr5( at_prevperm ,alias_at_prevperm,&__prevperm,0,true);

  gen _split(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    //renvoie [ax,ay] si les arg sont g1=ax*an (sans denominateur) et g2=[x,y]
    //sinon renvoie [0]
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    vecteur v(*args._VECTptr);
    gen g1=v.front(),g2=v.back();
    if (g1.type==_STRNG && g2.type==_STRNG){ // Python "Ceci est une phrase".split(" ")
      if (g2._STRNGptr->empty())
	return gendimerr(contextptr);
      vecteur res;
      int pos=0,ss=g1._STRNGptr->size();
      for (;pos<ss;){
	int npos=g1._STRNGptr->find(*g2._STRNGptr,pos);
	if (npos<0 || npos>=ss)
	  break;
	res.push_back(string2gen(g1._STRNGptr->substr(pos,npos-pos),false));
	pos=npos+g2._STRNGptr->size();
      }
      if (pos<ss)
	res.push_back(string2gen(g1._STRNGptr->substr(pos,ss-pos),false));
      return res;
    }
    if (g2.type!=_VECT)
      return gentypeerr(contextptr);
    vecteur v2(*g2._VECTptr);
    int n=int(v2.size());
    if (n!=2) return gensizeerr(contextptr);
    vecteur fa;
    fa=factors(g1,vx_var,contextptr);
    int l=int(fa.size());
    gen ax=1;
    gen ay=1;
    for (int k=0;k<l;k=k+2){
      gen f=fa[k];
      if (derive(f,v2[0],contextptr)==0) {
        ay=ay*pow(f,fa[k+1],contextptr);
      }
      else {
	if (derive(f,v2[1],contextptr)==0){
	  ax=ax*pow(f,fa[k+1],contextptr);
	}
	else {vecteur res(1);return (res);}
      }
    }
    vecteur res(2);
    res[0]=ax;
    res[1]=ay;
    return res;
  }
 
  static const char _split_s[]="split";
  static define_unary_function_eval (__split,&_split,_split_s);
  define_unary_function_ptr5( at_split ,alias_at_split,&__split,0,true);
 
  gen _join(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type==_VECT && args._VECTptr->size()==2){
      gen g1=args._VECTptr->front(),g2=args._VECTptr->back();
      if (g1.type==_STRNG && g2.type==_VECT){ // Python 
	const_iterateur it=g2._VECTptr->begin(),itend=g2._VECTptr->end();
	string res;
	for (;it!=itend;){
	  if (it->type==_STRNG)
	    res += *it->_STRNGptr;
	  else
	    res += it->print(contextptr);
	  ++it;
	  if (it==itend)
	    break;
	  res += *g1._STRNGptr;
	}
	return string2gen(res,false);
      }
    }
    return gensizeerr(contextptr);
  } 
  static const char _join_s[]="join";
  static define_unary_function_eval (__join,&_join,_join_s);
  define_unary_function_ptr5( at_join ,alias_at_join,&__join,0,true);
 
  gen _sum_riemann(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    //renvoie l'equivalent pour n=infini de sum de k=1 a n de g1 avec g2=[n,k])  
    if ( (args.type!=_VECT)  || (args._VECTptr->size()!=2) )
      return gentypeerr(contextptr);
    gen g1=args[0],g2=args[1];
    if (g2.type!=_VECT)
      return gentypeerr(contextptr);
    vecteur v2(*g2._VECTptr);
    int sv2=int(v2.size());
    if (sv2!=2) return gensizeerr(contextptr);
    identificateur x("rieman_sum_x");
    //on pose k=n*x
    //mettre que v2[0]=n et v2[1]=k sont ds N
    //_assume(makevecteur(is_plus(v2[0]))); 
    //_assume(makevecteur(is_plus(v2[1])));
    gen a=recursive_normal(v2[0]*subst(g1,v2[1],x*v2[0],false,contextptr),contextptr);
    gen nda=_fxnd(a,contextptr);
    vecteur nada(*nda._VECTptr);
    //na numerateur de a et da denominateur de a ou k=n*x
    gen na=nada[0];
    gen da=nada[1];
    vecteur var(2);
    var[0]=na;
    v2[1]=x;
    var[1]=v2;
    //var=[na,[n,x]]
    gen b0=_split(var,contextptr);
    if (is_undef(b0)) return b0;
    //on separe les variables du numerateur
    vecteur vb0(*b0._VECTptr);
    if ((vb0.size()==1)&& (vb0[0]==0))
      return string2gen(gettext("Probably not a Riemann sum"),false);
    var[0]=da;
    //var=[da,[n,x]]
    gen b1=_split(var,contextptr);
    if (is_undef(b1)) return b1;
    //on separe les variables du denominateur
    vecteur vb1(*b1._VECTptr);
    if ((vb1.size()==1)&& (vb1[0]==0)) 
      return string2gen(gettext("Probably not a Riemann sum"),false);
    gen an=vb0[0]/vb1[0];
    gen ax=vb0[1]/vb1[1];
    gen tmp=_integrate(makesequence(ax,x,0,1),contextptr);
    if (is_undef(tmp)) return tmp;
    gen tmp2=_limit(makesequence(an,v2[0],plus_inf),contextptr);
    //tmp n'a pas ete calcule sum_riemann(pi/(2*n)*log(sin(pi*k/(2*n))),[n,k])
    //if ((tmp.type==_SYMB)&&(tmp._SYMBptr->sommet==at_integrate))
    //return _limit(makevecteur(tmp*an,v2[0],plus_inf));
    //tmp ne doit pas etre infini qd tmp2 est nul et reciproquement
    if (is_inf(tmp)&& is_zero(tmp2))
      return string2gen(gettext("Probably not a Riemann sum"),false);
    if (is_zero(tmp)&& is_inf(tmp2))
      return string2gen(gettext("Probably not a Riemann sum"),false);
    gen res=recursive_normal(tmp*_series(makesequence(an,v2[0],plus_inf),contextptr),contextptr);
    res=_limit(makesequence(res,v2[0],plus_inf),contextptr);
    return res;
  }
  static const char _sum_riemann_s[]="sum_riemann";
  static define_unary_function_eval (__sum_riemann,&_sum_riemann,_sum_riemann_s);
  define_unary_function_ptr5( at_sum_riemann,alias_at_sum_rieman,&__sum_riemann,0,true);

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
