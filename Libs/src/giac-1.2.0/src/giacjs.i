// Giac CAS and Javascript: first install libwebkit-dev
// 1/ Making giacjs_wrap.cxx requires a javascript enabled version of swig
// (gsoc-javascript)
// swig -c++ -javascript -jsc -c++ giacjs.i
// then modify in giacjs_wrap.cxx #define SWIG_fail                 return NULL;
// (search for goto fail;)
// 2/ Compiling giacjs_wrap.cxx
// With libgiac dynamically linked (check path webkitgtk-1.0 might be webkit-1.0)
// g++ -g -c -fpic  giacjs_wrap.cxx  -I/usr/include/webkit-1.0/ && g++ -shared  -fpic giacjs_wrap.o -L/usr/lib/ -lgiac  -o libgiacjs.so # -ljavascriptcoregtk-1.0 
// 3/ Install the javascript giac library and compile an interpreter with giac enabled
// sudo cp libgiacjs.so /usr/lib && sudo ldconfig
// g++  -I/usr/include/webkit-1.0 javascript.cxx -o javascript -lwebkit-1.0
// 4/ Running an example:
// ./javascript -l giacjs giacjs.js

/* File : giacjs.i 
   Interface for giac using swig for javascript
   Assumes gen are stored on 64 bits
 */
%module giacjs

%{
#include "giac.h"
%}

namespace giac {
  enum gen_unary_types {
    // immediate type (without mem allocation) should be < _ZINT
    _INT_= 0, // int val
    _DOUBLE_= 1, // double _DOUBLE_val
    // all type below or equal to _DOUBLE_ must be non pointers
    _ZINT= 2, // mpz_t * _ZINTptr
    _REAL= 3, // mpf_t * _REALptr
    // all type strictly below _CPLX must be real types
    _CPLX= 4, // gen * _CPLXptr
    _POLY= 5, // polynome * _POLYptr
    _IDNT= 6, // identificateur * _IDNTptr
    _VECT= 7, // vecteur * _VECTptr
    _SYMB= 8, // symbolic * _SYMBptr
    _SPOL1= 9, // sparse_poly1 * _SPOL1ptr
    _FRAC= 10, // fraction * _FRACptr
    _EXT= 11, // gen * _EXTptr
    _STRNG= 12, // string * _STRNGptr
    _FUNC= 13, // unary_fonction_ptr * _FUNCptr
    _ROOT= 14, // real_complex_rootof *_ROOTptr
    _MOD= 15, // gen * _MODptr
    _USER= 16, // gen_user * _USERptr
    _MAP=17, // map<gen.gen> * _MAPptr
    _EQW=18, // eqwdata * _EQWptr
    _GROB=19, // grob * _GROBptr
    _POINTER_=20, // void * _POINTER_val
    _FLOAT_=21 // immediate, _FLOAT_val
  } ;

  enum gen_comp_subtypes {
    _SEQ__VECT=1,
    _SET__VECT=2,
    _RPN_FUNC__VECT=3,
    _RPN_STACK__VECT=4,
    _GROUP__VECT=5,
    _LINE__VECT=6,
    _VECTOR__VECT=7,
    _PNT__VECT=8,
    _CURVE__VECT=8,
    _HALFLINE__VECT=9,
    _POLY1__VECT=10,
    _MATRIX__VECT=11,
    _RUNFILE__VECT=12,
    _ASSUME__VECT=13,
    _SPREAD__VECT=14,
    _CELL__VECT=15,
    _EQW__VECT=16,
    _HIST__VECT=17,
    _TILIST__VECT=0,
    _FOLDER__VECT=18,
    _SORTED__VECT=19,
    _POINT__VECT=20,
    _POLYEDRE__VECT=21,
    _RGBA__VECT=22,
    _LIST__VECT=23,
    _LOGO__VECT=24
  } ;

}
%include "std_vector.i"
%include "std_string.i"

namespace std {
  %template(vectori) vector<int>;
  %template(vectord) vector<double>;
  %template(vectorg) vector<giac::gen>;
  // %template(vectors) vector<std::string>;
};

namespace giac {

  class context {
  public:
    giac::sym_tab * tabptr ;
    context * globalcontextptr ;
    context * previous ;
    giac::global * globalptr; 
    const context * parent;
    giac::vecteur * quoted_global_vars;
    giac::vecteur * history_in_ptr, * history_out_ptr;
    context();
    context(const context & c);
#ifndef RTOS_THREADX
#ifndef BESTA_OS
    context(const std::string & name);
#endif
#endif
    ~context();
    context * clone() const;
  };

  class gen {
  public:
#ifdef GIAC_TYPE_ON_8BITS
    unsigned char type;  // see dispatch.h
#else
    unsigned char type:5;  // 32 types is enough, keep 3 bits more for double
    unsigned char type_unused:3; 
#endif
    signed char subtype;
    unsigned short reserved; // used if SMARTPTR is defined on 64 bit CPU (16 bits for pointer val)
    int val;
  
    inline volatile int & ref_count() const { 
#ifdef SMARTPTR64
      return ((ref_mpz_t *) ((* (longlong *) (this))>>16))->ref_count;
#else
      return __ZINTptr->ref_count;
#endif
    }
    gen(): type(_INT_),subtype(0),val(0) {
#ifdef COMPILE_FOR_STABILITY
      control_c();
#endif
    };
#ifdef SMARTPTR64
    gen(void *ptr,short int subt)  {
#ifdef COMPILE_FOR_STABILITY
      control_c();
#endif
      longlong __POINTERptr = (longlong ) new ref_void_pointer(ptr); 
#ifndef NO_STDEXCEPT
      if (__POINTERptr & 0xffff000000000000)
	setsizeerr(gettext("Pointer out of range"));
#endif
      * ((longlong *) this) = __POINTERptr << 16;
      subtype=subt;
      type=_POINTER_;
    };
#else
    gen(void *ptr,short int subt): type(_POINTER_),subtype(char(subt)) {
#ifdef COMPILE_FOR_STABILITY
      control_c();
#endif
      __POINTERptr=new ref_void_pointer(ptr); 
    };
#endif
    gen(int i): type(_INT_),subtype(0),val(i) {
#ifdef COMPILE_FOR_STABILITY
      control_c();
#endif
    };
    gen(size_t i): type(_INT_),subtype(0),val((int)i)  {
#ifdef COMPILE_FOR_STABILITY
      control_c();
#endif
    };
    gen(longlong i);
#ifdef INT128
    gen(int128_t i);
#endif
#ifdef DOUBLEVAL
    gen(double d): type(_DOUBLE_),_DOUBLE_val(d) {};
#else
    // Warning this does not work on ia64 with -O2
    gen(double d) { *((double *) this) = d; type=_DOUBLE_; };
#endif
    gen(const giac_float & f);
#ifdef BCD
    gen(accurate_bcd_float * b);
#endif
    // inline
    double DOUBLE_val() const ;
    giac_float FLOAT_val() const ;
    gen(int a,int b);
    gen(double a,double b);
    gen(const gen & a,const gen & b);
    gen(const std::complex<double> & c);
    gen(const gen & e);
    gen (const giac::identificateur & s);
    gen (giac::ref_identificateur * sptr);
    gen (const giac::vecteur & v,short int s=0);
    gen (giac::ref_vecteur * vptr,short int s=0); 
    // vptr must be a pointer allocated by new, do not delete it explicitly
    gen (const giac::symbolic & s);
    gen (giac::ref_symbolic * sptr);
    gen (const giac::gen_user & g);
    gen (const giac::real_object & g);
    // Pls do not use this constructor unless you know exactly what you do
    gen (giac::Tref_tensor<gen> * pptr);
    gen (const giac::polynome & p);
    gen (const giac::fraction & p);
    gen (const std::string & s,giac::context * contextptr);
    // gen (const wchar_t * s,giac::context * contextptr);
    // gen (const char * s,giac::context * contextptr);
    // gen (const giac::sparse_poly1 & p);
    gen (const giac::unary_function_ptr & f,int nargs=1);
    // gen (const giac::unary_function_ptr * f,int nargs=1);
    gen (const giac::gen_map & m);
    gen (const giac::eqwdata & );
    gen (const giac::grob & );
#ifdef HAVE_GMPXX_H
    gen (const giac::mpz_class &);
#endif
    gen (const giac::my_mpz &);
    ~gen();
    inline mpz_t * ref_ZINTptr() const ;
    inline giac::real_object * ref_REALptr() const ;
    inline giac::gen * ref_CPLXptr() const ;
    inline giac::identificateur * ref_IDNTptr() const ;
    inline giac::symbolic * ref_SYMBptr() const ;
    inline giac::gen * ref_MODptr () const ;
    inline giac::Tfraction<gen> * ref_FRACptr() const ;
    inline giac::gen * ref_EXTptr () const ;
    inline giac::polynome * ref_POLYptr() const ;
    inline giac::vecteur * ref_VECTptr() const ;
    inline giac::sparse_poly1 * ref_SPOL1ptr() const ;
    inline std::string * ref_STRNGptr() const ;
    inline giac::unary_function_ptr * ref_FUNCptr() const ;
    inline giac::gen_user * ref_USERptr() const ;
    inline giac::gen_map * ref_MAPptr() const ;
    inline giac::eqwdata * ref_EQWptr() const ;
    inline giac::grob * ref_GROBptr() const ;
    inline void * ref_POINTER_val() const ;

    bool in_eval(int level,gen & evaled,const giac::context * contextptr) const;
    gen eval(int level,const giac::context * contextptr) const;
    // inline gen eval() const { return eval(DEFAULT_EVAL_LEVEL,context0); }
    bool in_evalf(int level,gen & evaled,const giac::context * contextptr) const;
    gen evalf(int level,const giac::context * contextptr) const;
    // inline gen evalf() const { return evalf(DEFAULT_EVAL_LEVEL,context0); }
    gen evalf_double(int level,const giac::context * contextptr) const ;
    gen evalf2double(int level,const giac::context * contextptr) const;
    %rename(operator_equal) gen::operator =;
    // gen & operator = (const gen & a);
    int to_int() const ;
    bool is_real(giac::context * contextptr) const ;
    bool is_cinteger() const ;
    bool is_integer() const ;
    bool is_constant() const;
    std::string print(giac::context * contextptr) const;
    const char * printcharptr() const ;
    // if sptr==0, return length required, otherwise print at end of *sptr
    int sprint(std::string * sptr,giac::context * contextptr) const; 
    std::string print_universal(giac::context * contextptr) const;
    std::string print() const;
    wchar_t * wprint(giac::context * contextptr) const ; 
    // print then convert to a malloc-ated wchar_t *
    void modify(int i) { *this =gen(i); };
    void dbgprint() const; 
    void uncoerce() ;
    gen conj(giac::context * contextptr) const;
    gen re(giac::context * contextptr) const ;
    gen im(giac::context * contextptr) const ;
    gen inverse(giac::context * contextptr) const;
    gen squarenorm(giac::context * contextptr) const;
    int bindigits() const ;
    // gen operator [] (int i) const ;
    // gen operator [] (const gen & i) const;
    gen operator_at(int i,giac::context * contextptr) const;
    gen operator_at(const gen & i,giac::context * contextptr) const;
    // gen & operator [] (int i) ;
    // gen & operator [] (const gen & i) ;
    %rename(operator_of) operator () (const gen &,giac::context *) const;
    gen operator () (const gen & i,giac::context * contextptr) const;
    // gen operator () (const gen & i,const gen & progname,giac::context * contextptr) const;
    bool islesscomplexthan(const gen & other) const;
    bool is_approx() const ; // true if double/real or cmplx with re/im
    int symb_size() const;
    gen change_subtype(int newsubtype);
    bool is_symb_of_sommet(const giac::unary_function_ptr & u) const ;
    // bool is_symb_of_sommet(const giac::unary_function_ptr * u) const ;
    gen makegen(int i) const; // make a gen of same type as this with integer i
  };
  gen genfromstring(const std::string & s);
  giac::vecteur makevecteur(const gen & a);
  giac::vecteur makevecteur(const gen & a,const gen & b);
  giac::vecteur makevecteur(const gen & a,const gen & b,const gen & c);
  giac::vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d);
  giac::vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e);
  giac::vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f);
  giac::vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g);
  giac::vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h);
  giac::vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i);

  gen _factor(const gen & g,giac::context * contextptr);
  gen _simplify(const gen & g,giac::context * contextptr);
  gen _size(const gen & g,giac::context * contextptr);
  %rename(add) operator + (const gen &,const gen &);
  gen operator + (const gen & a,const gen & b);
  %rename(sub) operator - (const gen &,const gen &);
  gen operator - (const gen & a,const gen & b);
  %rename(neg) operator - (const gen &);
  gen operator - (const gen & a);
  %rename(mul) operator * (const gen &,const gen &);
  gen operator * (const gen & a,const gen & b);
  gen rdiv(const gen & a,const gen & b); // rational division
  %rename(mod) operator % (const gen &,const gen &);
  gen operator %(const gen & a,const gen & b); // for int only
  gen pow(const gen & base,const gen & exponent,giac::context *);
  gen giac_pow(const gen & base,const gen & exponent,giac::context *);
  gen iquo(const gen & a,const gen & b); // same
  gen irem(const gen & a,const gen & b,gen & q); // same
  gen smod(const gen & a,const gen & b); // same
  void smod(const giac::vecteur & v,const gen & g,giac::vecteur & w); 
  giac::vecteur smod(const giac::vecteur & a,const gen & b); // same
  // gen inv(const gen & a);
  gen inv(const gen & a,giac::context *);
  bool operator_equal(const gen & a,const gen & b,giac::context *);
  gen equal(const gen & a,const gen &b,giac::context *);
  int fastsign(const gen & a,giac::context *);   // 0 if unknown, 1 if >0, -1 if <0
  gen sign(const gen & a,giac::context *);
  bool is_greater(const gen & a,const gen &b,giac::context *);
  bool is_strictly_greater(const gen & a,const gen &b,giac::context *);
  bool is_positive(const gen & a,giac::context *);
  bool is_strictly_positive(const gen & a,giac::context *);
  // Large tests if strictly not precised, if sign is unknown make an error
  bool ck_is_greater(const gen & a,const gen &b,giac::context *);
  bool ck_is_strictly_greater(const gen & a,const gen &b,giac::context *);
  bool ck_is_positive(const gen & a,giac::context *);
  bool ck_is_strictly_positive(const gen & a,giac::context *);
  gen superieur_strict(const gen & a,const gen & b,giac::context *);
  gen superieur_egal(const gen & a,const gen & b,giac::context *);
  gen inferieur_strict(const gen & a,const gen & b,giac::context *);
  gen inferieur_egal(const gen & a,const gen & b,giac::context *);
  bool symb_size_less(const gen & a,const gen & b);

  gen min(const gen & a, const gen & b,giac::context *);
  gen max(const gen & a, const gen & b,giac::context *);
  // default context0 is required for instantiation in poly.h
  gen factorial(unsigned long int i);
  gen comb(unsigned long int i,unsigned long j);
  gen perm(unsigned long int i,unsigned long j);
  // gen pow(const gen & base, unsigned long int exponent);
  // gen pow(const gen & base, int exponent);
  // gen pow(unsigned long int base, unsigned long int exponent);

  // more advanced arithmetic
  gen gcd(const gen & A,const gen & B);
  gen lcm(const gen & a,const gen & b);
  gen simplify(gen & n, gen & d);
  void egcd(const gen &a,const gen &b, gen & u,gen &v,gen &d );
  gen ichinrem(const gen & a,const gen &b,const gen & amod, const gen & bmod);
  gen invmod(const gen & A,const gen & modulo);
  gen fracmod(const gen & a_orig,const gen & modulo); // -> p/q=a mod modulo
  bool fracmod(const gen & a_orig,const gen & modulo,gen & res);
  gen powmod(const gen &base,const gen & expo,const gen & modulo);
  gen isqrt(const gen & A);
  gen re(const gen & a,giac::context *);
  gen no_context_re(const gen & a);
  gen im(const gen & a,giac::context *);
  gen no_context_im(const gen & a);
  void reim(const gen & g,gen & r,gen & i,giac::context *);
  gen conj(const gen & a,giac::context *);
  gen no_context_conj(const gen & a);
  gen sq(const gen & a);
  gen abs(const gen & a,const context * contextptr=context0);
  // default context0 is required for instantiation in poly.h
  gen linfnorm(const gen & a,const context * contextptr=context0);
  // default context0 is required for instantiation in poly.h
  gen arg(const gen & a,giac::context *);
  gen arg_CPLX(const gen & a,giac::context *);
  int is_perfect_square(const gen & A);
  int is_probab_prime_p(const gen & A);
  gen nextprime(const gen & a); // more precisely next probably prime
  gen prevprime(const gen & a); // more precisely prev probably prime
  int jacobi(const gen & A, const gen &B);
  int legendre(const gen & A, const gen & B);
  giac::vecteur pascal_next_line(const giac::vecteur & v); 
  giac::vecteur pascal_nth_line(int n);
  // convert a __VECTOR__VECT vecteur to a normal vecteur
  gen vector2vecteur(const giac::vecteur & v);

  // if b is a _MOD, returns a as a b _MOD 
  gen chkmod(const gen& a,const gen & b);
  // make a _MOD a%b
  gen makemod(const gen & a,const gen & b);
  // same without evaluating %
  gen makemodquoted(const gen & a,const gen & b);

  // from a sum in x returns a list of [coeff monomial]
  // e.g. 5+2x+3*x*y -> [ [5 1] [2 x] [ 3 x*y] ]
  giac::vecteur symbolique2liste(const gen & x,giac::context *);
  // v should be sorted and shrinked
  gen liste2symbolique(const giac::vecteur & v);

  bool is_atomic(const gen & e);
  gen string2gen(const std::string & ss,bool remove_ss_quotes=true);
  // gen eval_VECT(const giac::vecteur & v,int subtype,int level,const context * context_ptr );
  gen no_context_evalf(const gen & e);
  gen evalf(const gen & e,int level,const context * contextptr );
  gen evalf2bcd_nock(const gen & g0,int level,const context * contextptr);
  gen evalf2bcd(const gen & g0,int level,const context * contextptr);
  // return true if g can be converted to a double or real or complex
  bool has_evalf(const gen & g,gen & res,int level,const context * contextptr);
  double int2double(int i);
  gen aplatir_fois_plus(const gen & g);
  gen collect(const gen & g,giac::context *);

  gen select_root(const giac::vecteur & v,giac::context * );
  bool is_known_rootof(const giac::vecteur & v,gen & symroot,giac::context * );
  gen alg_evalf(const gen & a,const gen &b,giac::context * );
  gen approx_rootof(const gen & e,giac::context * );
  gen common_EXT(gen & a,gen & b,const giac::vecteur * l,giac::context * );
  gen common_minimal_POLY(const gen & ga,const gen & gb, gen & a,gen & b,int &k,giac::context * );
  gen ext_add(const gen & a,const gen & b,giac::context * );
  gen ext_sub(const gen & a,const gen & b,giac::context * );
  gen ext_mul(const gen & a,const gen & b,giac::context * );
  gen symb_rootof(const gen & p,const gen &pmin,giac::context * );
  gen rootof(const gen & e,giac::context * );
  giac::vecteur sturm(const gen &g,const gen & x,giac::context * );
  int sturmab(const gen & g,const gen &x,const gen & a,const gen & b,giac::context * );
  gen _sturmab(const gen & g_orig,giac::context * );
  gen _sturm(const gen & g,giac::context * );
  gen _sturmseq(const gen & g,giac::context * );
  int sturmsign(const gen & a,bool strict,giac::context * );
  int find_range(const gen & g,giac::vecteur & a,giac::context * );
  gen fminmax(const gen & g,int minmax,giac::context * );
  gen _complexroot(const gen & g,giac::context * );
  gen _realroot(const gen & g,giac::context * );
  gen _crationalroot(const gen & g,giac::context * );
  gen _rationalroot(const gen & g,giac::context * );
  giac::vecteur symb2poly_num(const gen & g,giac::context * );
  gen eval_before_diff(const gen & expr,const gen & variable,giac::context * );
  gen derive(const gen & e,const giac::identificateur & i,giac::context * );
  gen derive(const gen & e,const gen & vars,giac::context * );
  gen derive(const gen & e,const gen & vars,const gen & nderiv,giac::context * );
  gen _derive(const gen & args,giac::context * );
  gen _function_diff(const gen & g,giac::context * );
  gen _grad(const gen & args,giac::context * );
  gen laplace(const gen & f,const gen & x,const gen & s,giac::context * );
  gen _laplace(const gen & args,giac::context * );
  gen diffeq_constante(int i,giac::context * );
  gen ilaplace(const gen & f,const gen & x,const gen & s,giac::context * );
  gen _ilaplace(const gen & args,giac::context * );
  gen _desolve(const gen & args,giac::context * );
  gen ztrans(const gen & f,const gen & x,const gen & s,giac::context * );
  gen _ztrans(const gen & args,giac::context * );
  gen invztrans(const gen & f,const gen & x,const gen & s,giac::context * );
  gen _invztrans(const gen & args,giac::context * );
  gen integrate_without_lnabs(const gen & e,const gen & x,giac::context * );
  gen _ezgcd(const gen & args,giac::context * );
  gen _modgcd(const gen & args,giac::context * );
  gen _heugcd(const gen & args,giac::context * );
  gen _psrgcd(const gen & args,giac::context * );
  giac::vecteur quad(int &b,const gen & q, const giac::vecteur & x,giac::context * );
  giac::vecteur qxa(const gen &q,const giac::vecteur & x,giac::context * );
  gen _q2a(const gen & args,giac::context * );
  giac::vecteur gauss(const gen & q,const giac::vecteur & x,giac::context * );
  gen _gauss(const gen & args,giac::context * );
  gen axq(const giac::vecteur &A,const giac::vecteur & x,giac::context * );
  gen _a2q(const gen & args,giac::context * );
  giac::vecteur qxac(const gen &q,const giac::vecteur & x,giac::context * );
  gen _conique_reduite(const gen & args,giac::context * );
  gen _quadrique_reduite(const gen & args,giac::context * );
  void settypeerr(giac::context * );
  void setsizeerr(giac::context * );
  void setdimerr(giac::context * );
  void divisionby0err(const gen &,giac::context * );
  void cksignerr(const gen &,giac::context * );
  void invalidserieserr(const std::string &,giac::context * );
  void toofewargs(const std::string & s,giac::context * );
  void toomanyargs(const std::string & s,giac::context * );
  void maxordererr(giac::context * );
  void setstabilityerr(giac::context * );
  gen gentypeerr(giac::context * );
  void gentypeerr(gen & g,giac::context * );
  gen gensizeerr(giac::context * );
  void gensizeerr(gen & g,giac::context * );
  gen gendimerr(giac::context * );
  void gendimerr(gen & g,giac::context * );
  gen gendivisionby0err(const gen &,giac::context * );
  gen gencksignerr(const gen &,giac::context * );
  gen geninvalidserieserr(const std::string &,giac::context * );
  gen gentoofewargs(const std::string & s,giac::context * );
  gen gentoomanyargs(const std::string & s,giac::context * );
  gen genmaxordererr(giac::context * );
  gen genstabilityerr(giac::context * );
  std::string print_the_type(int val,giac::context * );
  std::string printinner_VECT(const giac::vecteur & v, int subtype,giac::context * );
  std::string & add_printinner_VECT(std::string & s,const giac::vecteur &v,int subtype,giac::context * );
  std::string begin_VECT_string(int subtype,bool tex,giac::context * );
  std::string end_VECT_string(int subtype,bool tex,giac::context * );
  std::string print_VECT(const giac::vecteur & v,int subtype,giac::context * ); // subtype was 0 by default
  // std::string print_SPOL1(const giac::sparse_poly1 & p,giac::context * );
  std::string printint32(int val,int subtype,giac::context * );
  std::string print_FLOAT_(const giac_float & f,giac::context * );
  gen parsed_gen(giac::context * );
  void parsed_gen(const gen & g,giac::context * );
  unsigned archive_count(const gen & g,giac::context * );
  bool archive_save(void * f,const gen & g,giac::context * );
  gen archive_restore(FILE * f,giac::context * );
  gen _ithprime(const gen & args,giac::context * );
  gen _ifactors(const gen & args,giac::context * );
  gen _maple_ifactors(const gen & args,giac::context * );
  giac::vecteur ifactors(const gen & n0,giac::context * );
  gen ifactors(const gen & args,int maplemode,giac::context * );
  giac::vecteur factors(const gen & g,const gen & x,giac::context * );
  gen _factors(const gen & args,giac::context * );
  gen _divis(const gen & args,giac::context * );
  gen idivis(const gen & n,giac::context * );
  gen _idivis(const gen & args,giac::context * );
  giac::vecteur pfacprem(gen & n,bool addlast,giac::context * );
  gen ifactor(const gen & n,giac::context * );
  gen _ifactor(const gen & args,giac::context * );
  gen euler(const gen & e,giac::context * );
  gen _euler(const gen & args,giac::context * );
  gen pa2b2(const gen & p,giac::context * );
  gen _pa2b2(const gen & args,giac::context * );
  gen _propfrac(const gen & args,giac::context * );
  gen _iabcuv(const gen & args,giac::context * );
  gen abcuv(const gen & a,const gen & b,const gen & c,const gen & x,giac::context * );
  gen _abcuv(const gen & args,giac::context * );
  gen simp2(const gen & a,const gen & b,giac::context * );
  gen _simp2(const gen & args,giac::context * );
  gen _fxnd(const gen & args,giac::context * );
  int is_meromorphic(const gen & g,const gen & x,gen &a,gen &b,gen & P,giac::context * );
  int is_even_odd(const gen & f,const gen & x,giac::context * );
  gen residue(const gen & g,const gen & x,const gen & a,giac::context * );
  gen _residue(const gen & args,giac::context * );
  bool intgab(const gen & g,const gen & x,const gen & a,const gen & b,gen & res,giac::context * );
  giac::vecteur singular(const gen & g,const gen & x,giac::context * );
  gen _singular(const gen & args,giac::context * );
  gen complex_subst(const gen & e,const giac::vecteur & substin,const giac::vecteur & substout,giac::context * );
  gen complex_subst(const gen & e,const gen & x,const gen & newx,giac::context * );
  giac::vecteur lvarxwithinv(const gen &e,const gen & x,giac::context * );
  bool is_constant_wrt(const gen & e,const gen & x,giac::context * );
  bool is_linear_wrt(const gen & e,const gen &x,gen & a,gen & b,giac::context * );
  bool is_quadratic_wrt(const gen & e,const gen &x,gen & a,gen & b,gen & c,giac::context * );
  gen lnabs(const gen & x,giac::context * );
  gen surd(const gen & c,int n,giac::context * );
  gen _surd(const gen & args,giac::context * );
  gen invexptoexpneg(const gen& g,giac::context * );
  bool is_rewritable_as_f_of(const gen & fu,const gen & u,gen & fx,const gen & gen_x,giac::context * );
  bool intgab_ratfrac(const gen & e,const gen & x,gen & value,giac::context * );
  gen integrate_gen_rem(const gen & e, const gen & x, gen & remains_to_integrate,giac::context * );
  gen integrate_id_rem(const gen & e, const gen & x, gen & remains_to_integrate,giac::context * );
  gen linear_integrate(const gen & e,const gen & x,gen & remains_to_integrate,giac::context * );
  gen integrate_id(const gen & e,const giac::identificateur & x,giac::context * );
  gen integrate_gen(const gen & e,const gen & f,giac::context * );
  gen _integrate(const gen & args,giac::context * );
  double rombergo(const gen & f,const gen & x, const gen & a, const gen & b, int n,giac::context * );
  double rombergt(const gen & f,const gen & x, const gen & a, const gen & b, int n,giac::context * );
  gen _romberg(const gen & args,giac::context * );
  gen sum(const gen & e,const gen & x,gen & remains_to_sum,giac::context * );
  gen sum_loop(const gen & e,const gen & x,int i,int j,giac::context * );
  gen sum(const gen & e,const gen & x,const gen & a,const gen &b,giac::context * );
  gen _sum(const gen & args,giac::context * ) ;
  gen _Sum(const gen & args,giac::context * ) ;
  gen _bernoulli(const gen & args,giac::context * ) ;
  gen seqprod(const gen & g,int type,giac::context * );
  gen sum(const gen & e,const gen & x,gen & remains_to_sum,giac::context * );
  gen sum(const gen & e,const gen & x,const gen & a,const gen &b,giac::context * );
  gen _sum(const gen & args,giac::context * );
  gen sum_loop(const gen & e,const gen & x,int i,int j,giac::context * );
  gen _bernoulli(const gen & args,giac::context * );
  gen _odesolve(const gen & args,giac::context * );
  gen preval(const gen & f,const gen & x,const gen & a,const gen & b,giac::context * );
  gen _ibpdv(const gen & args,giac::context * );
  gen fourier_an(const gen & f,const gen & x,const gen & T,const gen & n,const gen & a,giac::context * );
  gen fourier_bn(const gen & f,const gen & x,const gen & T,const gen & n,const gen & a,giac::context * );
  gen fourier_cn(const gen & f,const gen & x,const gen & T,const gen & n,const gen & a,giac::context * );
  gen _fourier_an(const gen & args,giac::context * );
  gen _fourier_bn(const gen & args,giac::context * );
  gen _fourier_cn(const gen & args,giac::context * );
  gen _isom(const gen & args,giac::context * );
  gen _mkisom(const gen & args,giac::context * );
  giac::vecteur mkisom(const gen & n,int b,giac::context * );
  gen prod_expand(const gen & a,const gen & b,giac::context * );
  gen unlin(giac::vecteur & v,giac::context * );
  void convolution(const gen & coeff, const gen & arg,const giac::vecteur & w,giac::vecteur & res,giac::context * );
  void lin(const gen & e,giac::vecteur & v,giac::context * );
  gen _lin(const gen & e,giac::context * );
  gen lnexpand(const gen & e,giac::context * );
  gen tunlin(giac::vecteur & v,giac::context * );
  void tconvolution(const gen & coeff, const gen & arg,const giac::vecteur & w,giac::vecteur & res,giac::context * );
  void tlin(const gen & e,giac::vecteur & v,giac::context * );
  gen _tlin(const gen & e,giac::context * );
  gen _texpand(const gen & e,giac::context * );
  gen expand(const gen & e,giac::context * );
  gen ln_expand(const gen & e0,giac::context * );
  gen _texpand(const gen & args,giac::context * );
  gen expexpand(const gen & e,giac::context * );
  gen lnexpand(const gen & e,giac::context * );
  gen trigexpand(const gen & e,giac::context * );
  gen _about(const gen & g,giac::context * );
  gen _zip(const gen & args,giac::context * );
  gen _accumulate_head_tail(const gen & args,giac::context * );
  gen _divide(const gen & g,giac::context * );
  gen _ratnormal(const gen & g,giac::context * );
  gen _about(const gen & g,giac::context * );
  gen _inverse(const gen & a_orig,giac::context * );
  gen _Inverse(const gen & g,giac::context * );
  gen _igcdex(const gen & a_orig,giac::context * );
  gen _gcdex(const gen & a_orig,giac::context * );
  gen _revlist(const gen & a,giac::context * );
  gen _restart(const gen & args,giac::context * );
  gen _restart_modes(const gen & args,giac::context * );
  gen _time(const gen & a,giac::context * );
  gen _cat(const gen & a_orig,giac::context * );
  gen _pivot(const gen & a_orig,giac::context * );
  gen _rowspace(const gen & g,giac::context * );
  gen _colspace(const gen & g,giac::context * );
  gen _copy(const gen & g,giac::context * );
  gen _row(const gen & g,giac::context * );
  gen _col(const gen & g,giac::context * );
  gen _count_sup(const gen & args,giac::context * );
  gen _count_inf(const gen & args,giac::context * );
  gen _trunc(const gen & args,giac::context * );
  gen _evalc(const gen & g,giac::context * );
  gen _open(const gen & g,giac::context * );
  gen _fopen(const gen & g,giac::context * );
  gen _fprint(const gen & g,giac::context * );
  gen _close(const gen & g,giac::context * );
  gen _blockmatrix(const gen & g,giac::context * );
  gen _delcols(const gen & g,giac::context * );
  gen _delrows(const gen & g,giac::context * );
  gen _JordanBlock(const gen & g,giac::context * );
  gen _companion(const gen & g,giac::context * );
  gen _border(const gen & g,giac::context * );
  gen _pade(const gen & g,giac::context * );
  gen _lhs(const gen & g,giac::context * );
  gen _rhs(const gen & g,giac::context * );
  gen _reverse_rsolve(const gen & g,giac::context * );
  gen fft(const gen & g_orig,int direct,giac::context * );
  gen _fft(const gen & g,giac::context * );
  gen _ifft(const gen & g,giac::context * );
  gen _Resultant(const gen & g,giac::context * );
  gen _Nullspace(const gen & g,giac::context * );
  gen _assign(const gen & g,giac::context * );
  gen _implicitplot3d(const gen & g,giac::context * );
  gen _readwav(const gen & g,giac::context * );
  gen _writewav(const gen & g,giac::context * );
  gen _animate(const gen & g,giac::context * );
  gen _animate3d(const gen & g,giac::context * );
  gen _even(const gen & g,giac::context * );
  gen _odd(const gen & g,giac::context * );
  gen _writergb(const gen & g,giac::context * );
  gen _readrgb(const gen & g,giac::context * );
  gen product(const giac::polynome & P,const giac::vecteur & v,const gen & n,gen & remains,giac::context * );
  gen _seqsolve(const gen & args,giac::context * );
  gen _rsolve(const gen & args,giac::context * );
  gen _array(const gen & g,giac::context * );
  gen _makemod(const gen & args,giac::context * );
  gen _hexprint(const gen & g,giac::context * );
  gen _octprint(const gen & g,giac::context * );
  gen _binprint(const gen & g,giac::context * );
  std::string cprint(const gen & args,giac::context * );
  gen _mathml(const gen & g,giac::context * );
  gen _spread2mathml(const gen & g,giac::context * );
  std::string gen2mathml(const gen & e,giac::context * );
  std::string gen2mathml(const gen & e, std::string &svg,giac::context * );
  std::string gen2svg(const gen &e,giac::context * );
  gen _scalar_product(const gen & args,giac::context * );
  gen _compare(const gen & args,giac::context * );
  gen _preval(const gen & args,giac::context * );
  gen _lagrange(const gen & args,giac::context * );
  gen _reorder(const gen & args,giac::context * );
  gen _adjoint_matrix(const gen & args,giac::context * );
  gen _equal2diff(const gen & args,giac::context * );
  gen _equal2list(const gen & args,giac::context * );
  gen _rank(const gen & args,giac::context * );
  gen _csc(const gen & args,giac::context * );
  gen _cot(const gen & args,giac::context * );
  gen _acsc(const gen & args,giac::context * );
  gen _ibpu(const gen & args,giac::context * ) ;
  gen _changebase(const gen & args,giac::context * );
  gen _epsilon2zero(const gen & args,giac::context * );
  gen _suppress(const gen & args,giac::context * );
  gen _valuation(const gen & args,giac::context * );
  gen _degree(const gen & args,giac::context * );
  gen _lcoeff(const gen & args,giac::context * );
  gen _tcoeff(const gen & args,giac::context * );
  gen _sqrfree(const gen & args,giac::context * );
  gen _truncate(const gen & args,giac::context * );
  gen _canonical_form(const gen & args,giac::context * );
  gen _fcoeff(const gen & args,giac::context * );
  gen _froot(const gen & args,giac::context * );
  gen _roots(const gen & g,giac::context * );
  gen _divpc(const gen & args,giac::context * );
  gen _ptayl(const gen & args,giac::context * );
  gen _dfc(const gen & g_orig,giac::context * );
  gen _dfc2f(const gen & g,giac::context * );
  gen _float2rational(const gen & g,giac::context * );
  gen _gramschmidt(const gen & g,giac::context * );
  gen _pmin(const gen & g,giac::context * );
  gen _potential(const gen & g,giac::context * );
  gen _vpotential(const gen & g,giac::context * );
  gen _poly2symb(const gen & g,giac::context * );
  gen _symb2poly(const gen & g,giac::context * );
  gen _exp2trig(const gen & g,giac::context * );
  gen _nrows(const gen & g,giac::context * );
  gen _ncols(const gen & g,giac::context * );
  gen _l2norm(const gen & g0,giac::context * );
  gen _normalize(const gen & a,giac::context * );
  gen _lgcd(const gen & g,giac::context * );
  gen _float(const gen & g,giac::context * );
  gen _hold(const gen & g,giac::context * );
  gen _eigenvals(const gen & g,giac::context * );
  gen _Gcd(const gen & g,giac::context * );
  gen _Gcdex(const gen & g,giac::context * );
  gen _Factor(const gen & g,giac::context * );
  gen _Rref(const gen & g,giac::context * );
  gen _Rank(const gen & g,giac::context * );
  gen _Det(const gen & g,giac::context * );
  gen _Quo(const gen & g,giac::context * );
  gen _Rem(const gen & g,giac::context * );
  gen _Int(const gen & g,giac::context * );
  gen _divisors(const gen & g,giac::context * );
  gen _maxnorm(const gen & g0,giac::context * );
  gen _l1norm(const gen & g0,giac::context * );
  gen _dotprod(const gen & g,giac::context * );
  gen _diag(const gen & g,giac::context * );
  gen _input(const gen & args,giac::context * );
  gen _textinput(const gen & args,giac::context * );
  gen _primpart(const gen & g,giac::context * );
  gen _content(const gen & g,giac::context * );
  gen _coeff(const gen & g,giac::context * );
  gen _chrem(const gen & g,giac::context * );
  gen _genpoly(const gen & g,giac::context * );
  gen _median(const gen & g,giac::context * );
  gen _quartile1(const gen & g,giac::context * );
  gen _quartile3(const gen & g,giac::context * );
  gen _quantile(const gen & g,giac::context * );
  gen _quartiles(const gen & g,giac::context * );
  gen _moustache(const gen & g_orig,giac::context * );
  gen _mean(const gen & g,giac::context * );
  gen _stdDev(const gen & g,giac::context * );
  gen _stddev(const gen & g,giac::context * );
  gen _variance(const gen & g,giac::context * );
  gen _covariance_correlation(const gen & g,giac::context * );
  gen _covariance(const gen & g,giac::context * );
  gen _correlation(const gen & g,giac::context * );
  gen _interval2center(const gen & g,giac::context * );
  gen _linear_regression(const gen & g,giac::context * );
  gen _exponential_regression(const gen & g,giac::context * );
  gen _power_regression(const gen & g,giac::context * );
  gen regression_plot_attributs(const gen & g,giac::vecteur & attributs,bool & eq,bool & r,giac::context * );
  gen _linear_regression_plot(const gen & g,giac::context * );
  gen _exponential_regression_plot(const gen & g,giac::context * );
  gen _logarithmic_regression_plot(const gen & g,giac::context * );
  gen _power_regression_plot(const gen & g,giac::context * );
  gen _polynomial_regression(const gen & g,giac::context * );
  gen _polynomial_regression_plot(const gen & g,giac::context * );
  gen _logistic_regression(const gen & g,giac::context * );
  gen _logistic_regression_plot(const gen & g,giac::context * );
  gen _linear_interpolate(const gen & g,giac::context * );
  gen _parabolic_interpolate(const gen & g,giac::context * );
  gen _center2interval(const gen & g,giac::context * );
  gen _histogram(const gen & g,giac::context * );
  gen _cumulated_frequencies(const gen & g,giac::context * );
  gen _classes(const gen & g,giac::context * );
  gen _listplot(const gen & g,giac::context * );
  gen _scatterplot(const gen & g,giac::context * );
  gen _polygonplot(const gen & g,giac::context * );
  gen _polygonscatterplot(const gen & g,giac::context * );
  gen _diagramme_batons(const gen & g,giac::context * );
  gen _camembert(const gen & g,giac::context * );
  gen cross_prod(const gen & a,const gen & b,const gen & c,giac::context * );
  gen _convexhull(const gen & g,giac::context * );
  gen _simplex_reduce(const gen & g,giac::context * );
  gen _spline(const gen & g,giac::context * );
  gen _bitand(const gen & g,giac::context * );
  gen _bitor(const gen & g,giac::context * );
  gen _bitxor(const gen & g,giac::context * );
  gen _hamdist(const gen & g,giac::context * );
  gen _plotarea(const gen & g,giac::context * );
  gen _add_language(const gen & args,giac::context * );
  gen _remove_language(const gen & args,giac::context * );
  gen _show_language(const gen & args,giac::context * );
  gen _os_version(const gen & args,giac::context * );
  giac::vecteur gen2continued_fraction(const gen & g,int n,giac::context * );
  gen float2rational(double d_orig,double eps,giac::context * );
  gen _sec(const gen & args,giac::context * );
  gen _asec(const gen & args,giac::context * );
  gen _acsc(const gen & args,giac::context * );
  gen _csc(const gen & args,giac::context * );
  gen _acot(const gen & args,giac::context * );
  gen _cot(const gen & args,giac::context * );
  gen _horner(const gen & args,giac::context * );
  gen _moyal(const gen & args,giac::context * );
  gen _Airy_Ai(const gen & args,giac::context * );
  gen _Airy_Bi(const gen & args,giac::context * );
  gen _randNorm(const gen & args,giac::context * );
  gen _randexp(const gen & args,giac::context * );
  gen _UTPN(const gen & args,giac::context * );
  gen UTPC(const gen & n_orig,const gen & x0,giac::context * );
  gen _UTPC(const gen & args,giac::context * );
  gen UTPT(const gen & n_orig,const gen & x0,giac::context * );
  gen _UTPT(const gen & args,giac::context * );
  gen UTPF(const gen & num,const gen & den,const gen & x0,giac::context * );
  gen _UTPF(const gen & args,giac::context * );
  gen binomial(const gen & n,const gen & k,const gen & p,giac::context * );
  gen _binomial(const gen & args,giac::context * );
  gen binomial_cdf(const gen & n,const gen &p,const gen & x0,const gen & x,giac::context * );
  gen _binomial_cdf(const gen & args,giac::context * );
  gen binomial_icdf(const gen & n,const gen &p,const gen & x_orig,giac::context * );
  gen _binomial_icdf(const gen & args,giac::context * );
  gen poisson(const gen & m,const gen & k,giac::context * );
  gen _poisson(const gen & args,giac::context * );
  gen poisson_cdf(const gen & n,const gen & x,giac::context * );
  gen _poisson_cdf(const gen & args,giac::context * );
  gen poisson_icdf(const gen & m,const gen & t_orig,giac::context * );
  gen _poisson_icdf(const gen & args,giac::context * );
  gen _normald(const gen & g,giac::context * );
  gen _normal_cdf(const gen & args,giac::context * );
  gen _normal_icdf(const gen & args,giac::context * );
  gen student(const gen & n,const gen & x,giac::context * );
  gen _student(const gen & args,giac::context * );
  gen student_cdf(const gen & dof,const gen & x1,const gen & x2,giac::context * );
  gen _student_cdf(const gen & args,giac::context * );
  gen student_icdf(const gen & m,const gen & t_orig,giac::context * );
  gen _student_icdf(const gen & args,giac::context * );
  gen chisquare(const gen & n,const gen & x,giac::context * );
  gen _chisquare(const gen & args,giac::context * );
  gen chisquare_cdf(const gen & dof,const gen & x1,const gen & x2,giac::context * );
  gen _chisquare_cdf(const gen & args,giac::context * );
  gen chisquare_icdf(const gen & m,const gen & t_orig,giac::context * );
  gen _chisquare_icdf(const gen & args,giac::context * );
  gen snedecor(const gen & a,const gen & b,const gen & x,giac::context * );
  gen _snedecor(const gen & args,giac::context * );
  gen snedecor_cdf(const gen & ndof,const gen & ddof,const gen & x,giac::context * );
  gen _snedecor_cdf(const gen & args,giac::context * );
  gen snedecor_icdf(const gen & num,const gen & den,const gen & t_orig,giac::context * );
  gen _snedecor_icdf(const gen & args,giac::context * );
  gen Beta(const gen & a,const gen& b,giac::context * );
  gen _Beta(const gen & args,giac::context * );
  gen Bessel(const gen & g,int kind,giac::context * );
  gen _BesselI(const gen & args,giac::context * );
  gen _BesselJ(const gen & args,giac::context * );
  gen _BesselK(const gen & args,giac::context * );
  gen _BesselY(const gen & args,giac::context * );
  gen _constants_catalog(const gen & g,giac::context * );
  // std::string pariprint(const gen & e,int varnum,giac::context * );
  // GEN gen2GEN(const gen & e,const giac::vecteur & vars,giac::context * );
  gen _pari(const gen & args,giac::context * );
  gen _trn(const gen & args,giac::context * );
  gen _sizes(const gen & args,giac::context * );
  gen _permuorder(const gen & args,giac::context * );
  gen _randperm(const gen & args,giac::context * );
  gen _is_permu(const gen & args,giac::context * );
  gen _is_cycle(const gen & args,giac::context * );
  gen _cycle2perm(const gen & args,giac::context * );
  gen _p1op2(const gen & args,giac::context * );
  gen _c1oc2(const gen & args,giac::context * );
  gen _c1op2(const gen & args,giac::context * );
  gen _p1oc2(const gen & args,giac::context * );
  gen _cycles2permu(const gen & args,giac::context * );
  gen _permu2cycles(const gen & args,giac::context * );
  gen _perminv(const gen & args,giac::context * );
  gen _cycleinv(const gen & args,giac::context * );
  gen _signature(const gen & args,giac::context * );
  gen _randperm(const gen & args,giac::context * );
  gen _hilbert(const gen & args,giac::context * );
  gen _hadamard(const gen & args,giac::context * );
  gen _trn(const gen & args,giac::context * );
  gen _syst2mat(const gen & args,giac::context * );
  gen _vandermonde(const gen & args,giac::context * );
  gen _laplacian(const gen & args,giac::context * );
  gen _hessian(const gen & args,giac::context * );
  gen _divergence(const gen & args,giac::context * );
  gen _curl(const gen & args,giac::context * );
  gen _groupermu(const gen & args,giac::context * );
  gen _nextperm(const gen & args,giac::context * );
  gen _prevperm(const gen & args,giac::context * );
  gen _split(const gen & args,giac::context * );
  gen _sum_riemann(const gen & args,giac::context * );
  gen _divergence(const gen &,giac::context * );  
  gen _hermite(const gen & args,giac::context * );
  gen _laguerre(const gen & args,giac::context * );
  gen _tchebyshev1(const gen & args,giac::context * );
  gen _tchebyshev2(const gen & args,giac::context * );
  gen _legendre(const gen & args,giac::context * );
  gen _curl(const gen & args,giac::context * );
  gen _permu2mat(const gen & args,giac::context * ); // permutation vector -> matrix
  giac::vecteur interpolyedre(const giac::vecteur & p,const gen & bb,giac::context * );
  giac::vecteur interdroitehyperplan(const gen & a,const gen &b,giac::context * );
  giac::vecteur interhyperplan(const gen & p1,const gen & p2,giac::context * );
  giac::vecteur interhypersurfacecurve(const gen & a,const gen &b,giac::context * ); 
  giac::vecteur inter2hypersurface(const gen & a,const gen &b,giac::context * );
  giac::vecteur interplansphere(const gen & a,const gen & b,giac::context * );
  gen hypersurface_equation(const gen & g,const giac::vecteur & xyz,giac::context * );
  gen _plot3d(const gen & args,giac::context * );
  gen _hypersurface(const gen & args,giac::context * );
  gen hypersurface_equation(const gen & g,const giac::vecteur & xyz,giac::context * );
  gen _hyperplan(const gen & args,giac::context * );
  gen _plan(const gen & args,giac::context * );
  gen _cone(const gen & args,giac::context * );
  gen _demi_cone(const gen & args,giac::context * );
  gen _cylindre(const gen & args,giac::context * );
  gen _perpendiculaire_commune(const gen & args,giac::context * );
  gen _polyedre(const gen & args,giac::context * );
  gen _prisme(const gen & args,giac::context * );
  gen _parallelepipede(const gen & args,giac::context * );
  gen _pyramide(const gen & args,giac::context * );
  gen _tetraedre_centre(const gen & args,giac::context * );
  gen _cube(const gen & args,giac::context * );
  gen _cube_centre(const gen & args,giac::context * );
  gen _octaedre(const gen & args,giac::context * );
  gen _icosaedre(const gen & args,giac::context * );
  gen _dodecaedre(const gen & args,giac::context * );
  gen _aretes(const gen & args,giac::context * );
  gen _faces(const gen & args,giac::context * );
  gen _hypersphere(const gen & args,giac::context * );
  gen _sphere(const gen & args,giac::context * );
  gen _quadrique(const gen & args,giac::context * );
  gen _est_cospherique(const gen & args,giac::context * );
  gen get_point(const gen & g,int n,giac::context * );
  gen _point3d(const gen & args,giac::context * );
  gen _point2d(const gen & args,giac::context * );
  // gen mkrand2d3d(int dim,int nargs,gen (* f)(const gen &,const context *),giac::context * );
  gen droite_by_equation(const giac::vecteur & v,bool est_plan,giac::context * );
  bool point2abc(const gen & e,const gen & f,gen & a,gen & b,gen & c,giac::context * );
  gen abs_norm(const gen & g,giac::context * );
  gen abs_norm2(const gen & g,giac::context * );
  gen dotgiac::vecteur(const gen & a,const gen & b,giac::context * );
  giac::vecteur interpolygone(const giac::vecteur & p,const gen & bb,giac::context * );
  giac::vecteur interdroitecercle(const gen & a,const gen &b,giac::context * );
  giac::vecteur curveintercircle(const gen & curve,const gen &circle,bool iscircle,giac::context * );
  gen _avance(const gen & g,giac::context * );
  gen _recule(const gen & g,giac::context * );
  gen _position(const gen & g,giac::context * );
  gen _cap(const gen & g,giac::context * );
  gen _tourne_droite(const gen & g,giac::context * );
  gen _tourne_gauche(const gen & g,giac::context * );
  gen _leve_crayon(const gen & g,giac::context * );
  gen _baisse_crayon(const gen & g,giac::context * );
  gen _ecris(const gen & g,giac::context * );
  gen _signe(const gen & g,giac::context * );
  gen _saute(const gen & g,giac::context * );
  gen _pas_de_cote(const gen & g,giac::context * );
  gen _cache_tortue(const gen & g,giac::context * );
  gen _montre_tortue(const gen & g,giac::context * );
  gen _debut_enregistrement(const gen & g0,giac::context * );
  gen _fin_enregistrement(const gen & g0,giac::context * );
  gen _repete(const gen & g,giac::context * );
  gen _crayon(const gen & g,giac::context * );
  gen _efface(const gen & g,giac::context * );
  gen _vers(const gen & g,giac::context * );
  gen _rond(const gen & g,giac::context * );
  gen _disque(const gen & g,giac::context * );
  gen _disque_centre(const gen & g,giac::context * );
  gen _polygone_rempli(const gen & g,giac::context * );
  gen _rectangle_plein(const gen & g,giac::context * );
  gen _triangle_plein(const gen & g,giac::context * );
  gen _dessine_tortue(const gen & g,giac::context * );
  gen _arc(const gen & args,giac::context * );
  giac::vecteur plotpreprocess(const gen & args,giac::context * );
  bool chk_double_interval(const gen & g,double & inf,double & sup,giac::context * );
  void ck_parameter(const gen & ,giac::context * );
  gen _erase3d(const gen & args,giac::context * );
  void rewrite_with_t_real(gen & eq,const gen & t,giac::context * );
  bool on(const gen & e_orig,const gen & f,gen & t,giac::context * );
  gen plotcontour(const gen & f0,bool contour,giac::context * );
  gen _aire(const gen & args,giac::context * );
  gen _perimetre(const gen & args,giac::context * );
  gen _plotfunc(const gen &,giac::context * );
  gen _erase(const gen & args,giac::context * );
  gen _pixon(const gen & args,giac::context * );
  gen _pixoff(const gen & args,giac::context * );
  gen _droite(const gen & args,giac::context * );
  gen _slope(const gen & args,giac::context * );
  gen _demi_droite(const gen & args,giac::context * );
  gen _segment(const gen & args,giac::context * );
  gen symb_segment(const gen & x,const gen & y,const giac::vecteur & ,int ,giac::context * );
  gen symb_pnt_name(const gen & x,const gen & c,const gen & nom,giac::context * );
  gen symb_pnt(const gen & x,const gen & c,giac::context * );
  gen pnt_attrib(const gen & point,const giac::vecteur & attributs,giac::context * );
  gen symb_pnt(const gen & x,giac::context * );
  gen _pnt(const gen & args,giac::context * );
  gen _animation(const gen & args,giac::context * );
  gen _point(const gen & args,giac::context * );
  gen _affixe(const gen & args,giac::context * );
  gen _abscisse(const gen & args,giac::context * );
  gen _ordonnee(const gen & args,giac::context * );
  gen _cote(const gen & args,giac::context * );
  gen _coordonnees(const gen & args,giac::context * );
  gen _coordonnees_polaires(const gen & args,giac::context * );
  gen _coordonnees_rectangulaires(const gen & args,giac::context * );
  gen _point_polaire(const gen & args,giac::context * );
  gen _cercle(const gen & args,giac::context * );
  bool centre_rayon(const gen & cercle,gen & centre,gen & rayon,bool absrayon, giac::context * );
  gen _centre(const gen & args,giac::context * );
  gen _rayon(const gen & args,giac::context * );
  gen _milieu(const gen & args,giac::context * );
  gen _mediatrice(const gen & args,giac::context * );
  gen bissectrice(const gen & args,bool interieur,giac::context * );
  gen _bissectrice(const gen & args,giac::context * );
  gen _exbissectrice(const gen & args,giac::context * );
  gen _mediane(const gen & args,giac::context * );
  gen _circonscrit(const gen & args,giac::context * );
  gen _orthocentre(const gen & arg_orig,giac::context * );
  gen _inscrit(const gen & args,giac::context * );
  gen _exinscrit(const gen & args,giac::context * );
  gen _isobarycentre(const gen & args,giac::context * );
  gen _barycentre(const gen & args,giac::context * );
  gen perpendiculaire(const gen & args,bool suppl,giac::context * );
  gen _perpendiculaire(const gen & args,giac::context * );
  gen _orthogonal(const gen & args,giac::context * );
  gen _mediatrice(const gen & args,giac::context * );
  gen _parallele(const gen & args,giac::context * );
  gen distance2pp(const gen & ee,const gen & ff,giac::context * );
  gen distance2(const gen & f1,const gen & f2,giac::context * );
  gen _longueur2(const gen & args,giac::context * );
  // gen longueur(const gen & f1,const gen & f2,giac::context * );
  gen _longueur(const gen & args,giac::context * );
  // gen angle(const gen & f1,const gen & f2,giac::context * );
  gen _angle(const gen & args,giac::context * );
  gen scalar_product(const gen & a,const gen & b,giac::context * );
  gen projection(const gen & a,const gen & b,const gen & c,giac::context * );
  gen projection(const gen & e,const gen & p,giac::context * );
  gen parameter2point(const giac::vecteur & v,giac::context * );
  gen cercle2curve(const gen & f,giac::context * );
  giac::vecteur inter(const gen & a,const gen & b,giac::context * );
  gen _click(const gen & args,giac::context * );
  gen _element(const gen & args,giac::context * );
  gen _as_function_of(const gen & args,giac::context * );
  gen _lieu(const gen & args,giac::context * );
  gen _head(const gen & args,giac::context * );
  gen _tail(const gen & args,giac::context * );
  gen _sommets(const gen & args,giac::context * );
  gen _sommets_abca(const gen & args,giac::context * );
  gen _symetrie(const gen & args,giac::context * );
  gen _rotation(const gen & args,giac::context * );
  gen _projection(const gen & args,giac::context * );
  gen _homothetie(const gen & args,giac::context * );
  gen _est_coplanaire(const gen & args,giac::context * );
  gen _est_aligne(const gen & args,giac::context * );
  gen _est_cocyclique(const gen & args,giac::context * );
  gen _est_parallele(const gen & args,giac::context * );
  gen _est_perpendiculaire(const gen & args,giac::context * );
  gen _est_element(const gen & args,giac::context * );
  gen _inversion(const gen & args,giac::context * );
  gen _similitude(const gen & args,giac::context * );
  gen translation(const gen & a,const gen & bb,giac::context * );
  gen _translation(const gen & args,giac::context * );
  gen _curve(const gen & args,giac::context * );
  gen _plotparam(const gen & args,giac::context * );
  gen _plotpolar(const gen & args,giac::context * );
  gen _parameq(const gen & args,giac::context * );
  gen _equation(const gen & args,giac::context * );
  gen equation_homogene(const gen & eq,giac::context * );
  gen _tangent(const gen & args,giac::context * );
  gen _ellipse(const gen & args,giac::context * );
  gen _hyperbole(const gen & args,giac::context * );
  gen _parabole(const gen & args,giac::context * );
  gen _conique(const gen & args,giac::context * );
  gen _legende(const gen & args,giac::context * );
  gen _distanceat(const gen & args,giac::context * );
  gen _distanceatraw(const gen & args,giac::context * );
  gen _areaatraw(const gen & args,giac::context * );
  gen _areaat(const gen & args,giac::context * );
  gen _slopeatraw(const gen & args,giac::context * );
  gen _slopeat(const gen & args,giac::context * );
  gen _perimeterat(const gen & args,giac::context * );
  gen _perimeteratraw(const gen & args,giac::context * );
  gen _extract_measure(const gen & valeur,giac::context * );
  gen _angleat(const gen & args,giac::context * );
  gen _angleatraw(const gen & args,giac::context * );
  gen _couleur(const gen & args,giac::context * );
  gen _parameter(const gen & args,giac::context * );
  gen _hauteur(const gen & args,giac::context * );
  gen _triangle(const gen & args,giac::context * );
  gen _triangle_rectangle(const gen & args,giac::context * );
  gen _triangle_isocele(const gen & args,giac::context * );
  gen _triangle_equilateral(const gen & args,giac::context * );
  gen _parallelogramme(const gen & args,giac::context * );
  gen _isopolygone(const gen & args,giac::context * );
  gen _carre(const gen & args,giac::context * );
  gen _hexagone(const gen & args,giac::context * );
  gen _quadrilatere(const gen & args,giac::context * );
  gen _rectangle(const gen & args,giac::context * );
  gen _losange(const gen & args,giac::context * );
  gen _polygone(const gen & args,giac::context * );
  gen _polygone_ouvert(const gen & args,giac::context * );
  gen _bissectrice(const gen & args,giac::context * );
  gen _exbissectrice(const gen & args,giac::context * );
  gen _plotfield(const gen & args,giac::context * );
  gen _interactive_plotode(const gen & args,giac::context * );
  gen _plotode(const gen & args,giac::context * );
  gen archive_session(bool save_history,const std::string & s,giac::context * );
  gen unarchive_session_string(const std::string & s,int level, const gen & replace,giac::context * );
  // gen _archive(bool save_history,const gen & args,giac::context * );
  gen _unarchive(const gen & args,giac::context * );
  gen _xyztrange(const gen & args,giac::context * );
  gen _switch_axes(const gen & args,giac::context * );
  gen _plotseq(const gen & args,giac::context * );
  gen _plotimplicit(const gen & args,giac::context * );
  gen _plotcontour(const gen & args,giac::context * );
  gen _plotinequation(const gen & f0,giac::context * );
  gen _inter_droite(const gen & args,giac::context * );
  gen _dot_paper(const gen & args,giac::context * );
  gen _grid_paper(const gen & args,giac::context * );
  gen _triangle_paper(const gen & args,giac::context * );
  gen _line_paper(const gen & args,giac::context * );
  gen _bitmap(const gen & args,giac::context * );
  gen _Pictsize(const gen & args,giac::context * );
  gen _plot_style(const gen & args,giac::context * );
  gen _DrawInv(const gen & args,giac::context * );
  gen _Graph(const gen & g,giac::context * );
  gen _DrwCtour(const gen & g,giac::context * );
  int est_isocele(const gen & a,const gen & b,const gen & c,giac::context * );
  gen _est_isocele(const gen & args,giac::context * );
  bool est_equilateral(const gen & a,const gen & b,const gen & c,giac::context * );
  gen _est_equilateral(const gen & args,giac::context * );
  bool est_carre(const gen & a,const gen & b,const gen & c,const gen & d,giac::context * );
  gen _est_carre(const gen & args,giac::context * );
  int est_losange(const gen & a,const gen & b,const gen & c,const gen & d,giac::context * );
  gen _est_losange(const gen & args,giac::context * );
  int est_parallelogramme(const gen & a,const gen & b,const gen & c,const gen & d,giac::context * );
  gen _est_parallelogramme(const gen & args,giac::context * );
  int est_rect(const gen & a,const gen & b,const gen & c,const gen & d,giac::context * );
  gen _est_rectangle(const gen & args,giac::context * );
  gen _est_harmonique(const gen & args,giac::context * );
  gen _div_harmonique(const gen & args,giac::context * );
  gen _point_div(const gen & args,giac::context * );
  gen _birapport(const gen & args,giac::context * );
  gen _est_harmonique(const gen & args,giac::context * );
  gen _div_harmonique(const gen & args,giac::context * );
  gen _conj_harmonique(const gen & args,giac::context * );
  // gen _conj_harmoniques(const gen & args,giac::context * );
  gen _point_div(const gen & args,giac::context * );
  gen _birapport(const gen & args,giac::context * );
  gen _puissance(const gen & args,giac::context * );
  gen _axe_radical(const gen & args,giac::context * );
  gen _polaire(const gen & args,giac::context * );
  gen _pole(const gen & args,giac::context * );
  gen _polaire_reciproque(const gen & args,giac::context * );
  gen _est_orthogonal(const gen & args,giac::context * );
  gen _est_conjugue(const gen & args,giac::context * );
  gen _est_faisceau_cercle(const gen & args,giac::context * );
  gen _est_faisceau_droite(const gen & args,giac::context * );
  int est_faisceau_droite(const gen & a,const gen & b,const gen & c,giac::context * );
  gen _enveloppe(const gen & args,giac::context * );
  gen put_attributs(const gen & lieu_geo,const giac::vecteur & attributs,giac::context * );
  int est_aligne(const gen & a,const gen & b,const gen & c,giac::context * );
  bool est_coplanaire(const gen & a,const gen & b,const gen & c,const gen & d,giac::context * );
  bool est_cocyclique(const gen & a,const gen & b,const gen & c,const gen & d,giac::context * );
  bool est_parallele_giac::vecteur(const giac::vecteur & a,const giac::vecteur &b,gen & coeff,giac::context * );
  bool est_parallele(const gen & a,const gen & b,giac::context * );
  bool est_perpendiculaire(const gen & a,const gen & b,giac::context * );
  int est_element(const gen & a_orig,const gen & b_orig,giac::context * );
  bool est_carre(const gen & a,const gen & b,const gen & c,const gen & d,giac::context * );
  int est_isocele(const gen & a,const gen & b,const gen & c,giac::context * );
  bool est_equilateral(const gen & a,const gen & b,const gen & c,giac::context * );
  int est_trianglerect(const gen & a,const gen & b,const gen & c,giac::context * );
  bool est_orthogonal(const gen & c1,const gen & R1,const gen & c2,const gen & R2,giac::context * );
  bool est_harmonique(const gen & a,const gen & b,const gen & c,const gen & d,giac::context * );
  gen _vector(const gen & args,giac::context * );
  gen _hyperplan(const gen & args,giac::context * );
  gen _hypersphere(const gen & args,giac::context * );
  gen _hypersurface(const gen & args,giac::context * );
  gen _Ox_2d_unit_vector(const gen & args,giac::context * );
  gen _Oy_2d_unit_vector(const gen & args,giac::context * );
  gen _frame_2d(const gen & args,giac::context * );
  gen _Ox_3d_unit_vector(const gen & args,giac::context * );
  gen _Oy_3d_unit_vector(const gen & args,giac::context * );
  gen _Oz_3d_unit_vector(const gen & args,giac::context * );
  gen _frame_3d(const gen & args,giac::context * );
  void increment_instruction(const gen & arg,giac::context * );
  std::string check_local_assign(const gen & g,giac::context * );
  gen quote_program(const gen & args,giac::context * );
  gen _program(const gen & args,const gen & name,giac::context * );
  void adjust_sst_at(const gen & name,giac::context * ); //used in giac::symbolic.cc by nr_eval
  gen _bloc(const gen & prog,giac::context * );
  std::string printasfor(const gen & feuille,const char * sommetstr,giac::context * );
  std::string printasifte(const gen & feuille,const char * sommetstr,giac::context * );
  gen _ifte(const gen & e,giac::context * );
  gen _evalb(const gen & e,giac::context * );
  // gen _maple_if(const gen & args,giac::context * );
  gen _when(const gen & e,giac::context * );
  gen _for(const gen & e,giac::context * );
  gen _local(const gen & args,giac::context * );
  // gen _return(const gen & args,giac::context * );
  gen _try_catch(const gen & args,giac::context * );
  gen _check_type(const gen & args,giac::context * );
  gen _type(const gen & args,giac::context * );
  gen _nop(const gen & a,giac::context * );
  gen _feuille(const gen & args,giac::context * );
  gen _maple_op(const gen & args,giac::context * );
  gen _sommet(const gen & args,giac::context * );
  gen subsop(const gen & g,const giac::vecteur & v,giac::context * );
  gen subsop(const giac::vecteur & g,const giac::vecteur & v,const gen & sommet,giac::context * );
  gen _maple_subsop(const gen & args,giac::context * );
  gen _subsop(const gen & args,giac::context * );
  gen _append(const gen & args,giac::context * );
  gen _prepend(const gen & args,giac::context * );
  gen concat(const gen & g,bool glue_lines,giac::context * );
  gen _concat(const gen & args,giac::context * );
  gen _contains(const gen & args,giac::context * );
  gen _select(const gen & args,giac::context * );
  gen _remove(const gen & args,giac::context * );
  gen _option(const gen & args,giac::context * );
  gen _case(const gen & args,giac::context * );
  gen _rand(const gen & args,giac::context * );
  gen rand_interval(const giac::vecteur & v,bool entier,giac::context * );
  gen _srand(const gen & args,giac::context * );
  gen _char(const gen & args,giac::context * );
  gen _asc(const gen & args,giac::context * );
  gen _map(const gen & args,giac::context * );
  gen _apply(const gen & args,giac::context * );
  gen _makelist(const gen & args,giac::context * );
  gen _interval(const gen & args,giac::context * );
  gen _comment(const gen & args,giac::context * );
  gen _throw(const gen & args,giac::context * );
  gen _union(const gen & args,giac::context * );
  gen _intersect(const gen & args,giac::context * );
  gen _inter(const gen & args,giac::context * );
  gen _minus(const gen & args,giac::context * );
  gen _dollar(const gen & args,giac::context * );
  gen _makemat(const gen & args,giac::context * );
  gen _compose(const gen & args,giac::context * );
  gen _composepow(const gen & args,giac::context * );
  gen _has(const gen & args,giac::context * );
  gen _args(const gen & args,giac::context * );
  gen _lname(const gen & args,giac::context * );
  gen _halt(const gen & args,giac::context * );
  gen _kill(const gen & args,giac::context * );
  gen _cont(const gen & args,giac::context * );
  gen _sst(const gen & args,giac::context * );
  gen _sst_in(const gen & args,giac::context * );
  gen _debug(const gen & args,giac::context * );
  gen _watch(const gen & args,giac::context * );
  gen _rmwatch(const gen & args,giac::context * );
  gen _breakpoint(const gen & args,giac::context * );
  gen _rmbreakpoint(const gen & args,giac::context * );
  void debug_loop(gen &res,giac::context * );
  gen _backquote(const gen & args,giac::context * );
  gen _double_deux_points(const gen & args,giac::context * );
  gen _maple2mupad(const gen & args,giac::context * );
  gen _maple2xcas(const gen & args,giac::context * );
  gen _mupad2maple(const gen & args,giac::context * );
  gen _mupad2xcas(const gen & args,giac::context * );
  gen _cd(const gen & args,giac::context * );
  gen _pwd(const gen & args,giac::context * );
  gen _scientific_format(const gen & args,giac::context * );
  gen _integer_format(const gen & g,giac::context * );
  gen giac_eval_level(const gen & g,giac::context * ); // can not be declared as _eval_level, conflict
  gen _prog_eval_level(const gen & g,giac::context * );
  gen _with_sqrt(const gen & g,giac::context * );
  gen _xcas_mode(const gen & args,giac::context * );
  gen _all_trig_solutions(const gen & args,giac::context * );
  gen _ntl_on(const gen & args,giac::context * );
  gen _complex_mode(const gen & args,giac::context * );
  gen _angle_radian(const gen & args,giac::context * );
  gen _epsilon(const gen & args,giac::context * );
  gen _proba_epsilon(const gen & args,giac::context * );
  gen _complex_variables(const gen & args,giac::context * );
  gen _approx_mode(const gen & args,giac::context * );
  gen _threads(const gen & args,giac::context * );
  gen _threads_allowed(const gen & g,giac::context * );
  gen _mpzclass_allowed(const gen & g,giac::context * );
  gen _cas_setup(const gen & args,giac::context * );
  gen _Digits(const gen & args,giac::context * );
  gen _insmod(const gen & args,giac::context * );
  gen _xport(const gen & args,giac::context * );
  gen _rmmod(const gen & args,giac::context * );
  gen _lsmod(const gen & args,giac::context * );
  gen _virgule(const gen & args,giac::context * );
  gen _sort(const gen & args,giac::context * );
  gen _ans(const gen & args,giac::context * );
  gen _quest(const gen & args,giac::context * );
  gen _convert(const gen & args,giac::context * );
  gen _deuxpoints(const gen & args,giac::context * );
  gen quote_read(const gen & args,giac::context * ); // read in a file and return non evaled
  gen _read(const gen & args,giac::context * ); // read in a file and return evaled
  gen _write(const gen & args,giac::context * );
  gen _save_history(const gen & args,giac::context * );
  gen _findhelp(const gen & args,giac::context * );
  gen _member(const gen & args,giac::context * );
  gen _tablefunc(const gen & args,giac::context * );
  gen _tableseq(const gen & args,giac::context * );
  gen protecteval(const gen & g,int level,giac::context * );
  gen _unapply(const gen & args,giac::context * );
  gen _makevector(const gen & args,giac::context * );
  gen _matrix(const gen & args,giac::context * );
  gen _makesuite(const gen & args,giac::context * );
  gen _break(const gen & args,giac::context * );
  gen _continue(const gen & args,giac::context * );
  gen _label(const gen & args,giac::context * );
  gen _goto(const gen & args,giac::context * );
  gen _tilocal(const gen & args,giac::context * );
  gen inputform_post_analysis(const giac::vecteur & v,const gen & res,giac::context * );
  giac::vecteur inputform_pre_analysis(const gen & g,giac::context * );
  gen _inputform(const gen & args,giac::context * );
  gen _choosebox(const gen & args,giac::context * );
  gen _output(const gen & args,giac::context * );
  gen _input(const gen & args,bool textinput,giac::context * );
  gen _nop(const gen & args,giac::context * );
  std::string printastifunction(const gen & feuille,const char * sommetstr,giac::context * );
  gen _Dialog(const gen & args,giac::context * );
  gen _Title(const gen & args,giac::context * );
  gen _Text(const gen & args,giac::context * );
  gen _Request(const gen & args,giac::context * );
  gen _DropDown(const gen & args,giac::context * );
  gen _Popup(const gen & args,giac::context * );
  gen _expr(const gen & args,giac::context * );
  gen _string(const gen & args,giac::context * );
  gen _part(const gen & args,giac::context * );
  gen _Pause(const gen & args,giac::context * );
  gen _Row(const gen & args,giac::context * );
  gen _Col(const gen & args,giac::context * );
  // gen _DelVar(const gen & args,giac::context * );
  gen _pointprod(const gen & args,giac::context * );
  gen _pointdivision(const gen & args,giac::context * );
  gen _pointpow(const gen & args,giac::context * );
  gen _pourcent(const gen & args,giac::context * );
  gen _hash(const gen & args,giac::context * );
  gen _interactive(const gen & args,giac::context * );
  std::string printassuffix(const gen & feuille,const char * sommetstr,giac::context * );
  gen _ti_semi(const gen & args,giac::context * );
  gen widget_size(const gen & g,giac::context * );
  gen keyboard(const gen & g,giac::context * );
  gen current_sheet(const gen & g,giac::context * );
  gen window_switch(const gen & g,giac::context * );
  gen maple_lib(const gen & g,giac::context * );
  gen maple_root(const gen & g,giac::context * );
  gen symb_unit(const gen & a,const gen & b,giac::context * );
  std::string printasDigits(const gen & feuille,const char * sommetstr,giac::context * );
  gen user_operator(const gen & g,giac::context * );
  gen _SetFold(const gen & g,giac::context * );
  gen simplifier(const gen & g,giac::context * );
  gen _simplifier(const gen & g,giac::context * );
  gen mksa_reduce(const gen & g,giac::context * );
  gen find_or_make_symbol(const std::string & s,bool check38,giac::context * );
  giac::vecteur mksa_convert(const gen & g,giac::context * );
  gen _ufactor(const gen & g,giac::context * );
  gen _usimplify(const gen & g,giac::context * );
  gen _piecewise(const gen & g,giac::context * );
  gen when2piecewise(const gen & g,giac::context * );
  gen when2sign(const gen & g,giac::context * );
  gen piecewise2when(const gen & g,giac::context * );
  gen _geo2d(const gen & g,giac::context * );
  std::string printasinnerbloc(const gen & feuille,giac::context * );
  gen symb_local(const gen & a,const gen & b,giac::context * );
  gen symb_local(const gen & args,giac::context * );
  gen symb_check_type(const gen & args,giac::context * );
  gen _quaternion(const gen & args,giac::context * );
  gen risch(const gen & e_orig,const giac::identificateur & x,gen & remains_to_integrate,giac::context * );
  gen _risch(const gen & g,giac::context * );
  std::string printasconstant(const gen & feuille,const char * sommetstr,giac::context * );
  gen _rpn(const gen & args,giac::context * );
  gen _alg(const gen & args,giac::context * );
  gen _PERCENT(const gen & args,giac::context * );
  gen _ROLL(const gen & args,giac::context * );
  gen _ROLLD(const gen & args,giac::context * );
  gen _SWAP(const gen & args,giac::context * );
  gen _DUP(const gen & args,giac::context * );
  gen _OVER(const gen & args,giac::context * );
  gen _PICK(const gen & args,giac::context * );
  gen _DROP(const gen & args,giac::context * );
  gen _NOP(const gen & args,giac::context * );
  gen _IFTE(const gen & args,giac::context * );
  gen _RPN_LOCAL(const gen & args,giac::context * );
  gen _RPN_FOR(const gen & args,giac::context * );
  gen _RPN_WHILE(const gen & args,giac::context * );
  gen _RPN_UNTIL(const gen & args,giac::context * );
  gen _RPN_CASE(const gen & args,giac::context * );
  gen _RCL(const gen & args,giac::context * );
  gen _VARS(const gen & args,giac::context * );
  gen _purge(const gen & args,giac::context * );
  gen _rpn_prog(const gen & args,giac::context * );
  giac::vecteur rpn_eval(const gen & prog,giac::vecteur & pile,giac::context * );
  gen _division(const gen & args,giac::context * );
  gen _binary_minus(const gen & args,giac::context * );
  gen _hp38(const gen & args,giac::context * );
  gen _ABS(const gen & args,giac::context * );
  gen _MODULO(const gen & args,giac::context * );
  gen _RANDOM(const gen & g,giac::context * );
  gen _MAXREAL(const gen & g,giac::context * );
  gen _MINREAL(const gen & g,giac::context * );
  gen _EXPM1(const gen & g,giac::context * );
  gen _LNP1(const gen & g,giac::context * );
  gen _ADDROW(const gen & args,giac::context * );
  gen _ADDCOL(const gen & args,giac::context * );
  gen _SCALE(const gen & g,giac::context * );
  gen _SCALEADD(const gen & g,giac::context * );
  gen _SWAPCOL(const gen & args,giac::context * );
  gen _SUB(const gen & args,giac::context * );
  gen _RANDMAT(const gen & args,giac::context * );
  gen _REDIM(const gen & args,giac::context * );
  gen _REPLACE(const gen & args,giac::context * );
  gen _EIGENVAL(const gen & args,giac::context * );
  gen _EIGENVV(const gen & args,giac::context * );
  gen _SIZE(const gen& args,giac::context * );
  gen _INT(const gen & g,giac::context * );
  gen _HPDIFF(const gen & args,giac::context * );
  gen _HPINT(const gen & args,giac::context * );
  gen _HPSUM(const gen & args,giac::context * );
  gen _TAYLOR(const gen & args,giac::context * );
  gen _POLYFORM(const gen & args,giac::context * );
  gen _IS_LINEAR(const gen & args,giac::context * );
  gen _SVD(const gen & args0,giac::context * );
  gen _SVL(const gen & args0,giac::context * );
  gen _SPECRAD(const gen & args0,giac::context * );
  gen _SPECNORM(const gen & args0,giac::context * );
  gen _COND(const gen & args0,giac::context * );
  gen _rank(const gen & args,giac::context * );
  gen _SCHUR(const gen & args,giac::context * );
  gen _LQ(const gen & args0,giac::context * );
  gen _LU(const gen & args0,giac::context * );
  gen _QR(const gen & args0,giac::context * );
  gen _XPON(const gen & g,giac::context * );
  gen _MANT(const gen & g,giac::context * );
  gen _HMSX(const gen & g0,giac::context * );
  gen _XHMS(const gen & g0,giac::context * );
  gen _DEGXRAD(const gen & g,giac::context * );
  gen _RADXDEG(const gen & g,giac::context * );
  gen _PERCENT(const gen & g,giac::context * );
  gen _PERCENTCHANGE(const gen & g,giac::context * );
  gen _PERCENTTOTAL(const gen & g,giac::context * );
  gen _ITERATE(const gen & args,giac::context * );
  gen _MAKEMAT(const gen & args,giac::context * );
  gen _LSQ(const gen & args,giac::context * );
  gen _idivis(const gen & args0,giac::context * );
  gen _isprime(const gen & args0,giac::context * );
  gen _ithprime(const gen & args0,giac::context * );
  gen _euler(const gen & args0,giac::context * );
  gen _numer(const gen & args0,giac::context * );
  gen _denom(const gen & args0,giac::context * );
  gen _ifactors(const gen & args0,giac::context * );
  gen _binomial_icdf(const gen & args0,giac::context * );
  gen _poisson_icdf(const gen & args0,giac::context * );
  gen _NTHROOT(const gen & args,giac::context * );
  gen _Ans(const gen & args,giac::context * );
  // gen _EXPORT(const gen & args,giac::context * );
  gen _VIEWS(const gen & args,giac::context * );
  void qualify(gen & g,const giac::vecteur & v,const gen & prog,giac::context * );
  int seqapp_prepare(const gen & expr_un,gen & expr,giac::vecteur & vars,giac::context * ,int seqno=-1);
  // bool ptruncate(giac::sparse_poly1 & p,const gen & ordre,giac::context * );
  void lcmdeno(giac::vecteur &v,gen & e,giac::context * );
  gen series(const gen & e,const gen & vars,const gen & lim_point,const gen &ordre,giac::context * );
  gen _series(const gen & args,giac::context * );
  gen _revert(const gen & args,giac::context * );
  gen _bounded_function(const gen & args,giac::context * );
  gen bounded_function(giac::context * );
  gen limit(const gen & e,const giac::identificateur & x,const gen & lim_point,int direction,giac::context * );
  gen _limit(const gen & args,giac::context * );
  gen _bounded_function(const gen & args,giac::context * );
  gen bounded_function(giac::context * );
  gen _euler_mac_laurin(const gen & args,giac::context * );
  bool convert_to_euler_mac_laurin(const gen & g,gen & res,giac::context * );
  giac::vecteur find_singularities(const gen & e,const giac::identificateur & x,int cplxmode,giac::context * );
  giac::vecteur solve(const gen & e,const giac::identificateur & x,int isolate_mode,giac::context * );
  giac::vecteur solve(const gen & e,const gen & x,int isolate_mode,giac::context * );
  void solve(const gen & e,const giac::identificateur & x,giac::vecteur &v,int isolate_mode,giac::context * );
  void in_solve(const gen & e,const giac::identificateur & x,giac::vecteur &v,int isolate_mode,giac::context * );
  bool modpolyroot(const giac::vecteur & a,const gen & p,giac::vecteur & v,bool dogcd,giac::context * );
  gen solvepostprocess(const gen & g,const gen & x,giac::context * );
  gen _solve(const gen & args,giac::context * );
  gen in_fsolve(giac::vecteur & v,giac::context * );
  gen _fsolve(const gen & args,giac::context * );
  gen _linsolve(const gen & args,giac::context * );
  gen _newton(const gen & args,giac::context * );
  gen _greduce(const gen & args,giac::context * );
  gen _gbasis(const gen & args,giac::context * );
  gen _in_ideal(const gen & args,giac::context * );
  giac::vecteur solvepreprocess(const gen & args,bool complex_mode,giac::context * );
  gen checkanglemode(giac::context * );
  gen degtorad(const gen & g,giac::context * );
  gen radtodeg(const gen & g,giac::context * );
  giac::vecteur subst(const giac::vecteur & v,const gen & i,const gen & newi,bool quotesubst,giac::context * );
  gen subst(const gen & e,const gen & i,const gen & newi,bool quotesubst,giac::context * );
  gen subst(const gen & e,const giac::vecteur & i,const giac::vecteur & ewi,bool quotesubst,giac::context * );
  gen quotesubst(const gen & e,const gen & i,const gen & newi,giac::context * );
  gen halftan(const gen & e,giac::context * );
  gen _halftan(const gen & args,giac::context * );
  gen shift_phase(const gen & e,giac::context * );
  gen _shift_phase(const gen & args,giac::context * );
  gen hyp2exp(const gen & e,giac::context * );
  gen _hyp2exp(const gen & args,giac::context * );
  gen sincos(const gen & e,giac::context * );
  gen _sincos(const gen & args,giac::context * );
  gen trig2exp(const gen & e,giac::context * );
  gen _trig2exp(const gen & args,giac::context * );
  gen halftan_hyp2exp(const gen & e,giac::context * );
  gen _halftan_hyp2exp(const gen & args,giac::context * );
  gen rewrite_hyper(const gen & e,giac::context * );
  gen asin2acos(const gen & e,giac::context * );
  gen _asin2acos(const gen & args,giac::context * );
  gen asin2atan(const gen & e,giac::context * );
  gen _asin2atan(const gen & args,giac::context * );
  gen acos2asin(const gen & e,giac::context * );
  gen _acos2asin(const gen & args,giac::context * );
  gen acos2atan(const gen & e,giac::context * );
  gen _acos2atan(const gen & args,giac::context * );
  gen atan2acos(const gen & e,giac::context * );
  gen _atan2acos(const gen & args,giac::context * );
  gen atan2asin(const gen & e,giac::context * );
  gen _atan2asin(const gen & args,giac::context * );
  giac::vecteur as_linear_combination(const gen & g,giac::vecteur & wrt,giac::context * );
  gen tsimplify_noexpln(const gen & e,int s1,int s2,giac::context * );
  gen tsimplify_common(const gen & e,giac::context * );
  gen tsimplify(const gen & e,giac::context * );
  gen _tsimplify(const gen & args,giac::context * );
  gen simplify(const gen & e,giac::context * );
  gen _simplify(const gen & args,giac::context * );
  gen trigcos(const gen & e,giac::context * );
  gen _trigcos(const gen & args,giac::context * );
  gen trigsin(const gen & e,giac::context * );
  gen _trigsin(const gen & args,giac::context * );
  gen trigtan(const gen & e,giac::context * );
  gen _trigtan(const gen & args,giac::context * );
  gen tan2sincos(const gen & e,giac::context * );
  gen _tan2sincos(const gen & args,giac::context * );
  gen tan2sincos2(const gen & e,giac::context * );
  gen _tan2sincos2(const gen & args,giac::context * );
  gen tan2cossin2(const gen & e,giac::context * );
  gen _tan2cossin2(const gen & args,giac::context * );
  gen tcollect(const gen & e,giac::context * );
  gen _tcollect(const gen & args,giac::context * );
  gen lncollect(const gen & e,giac::context * );
  gen _lncollect(const gen & args,giac::context * );
  gen _powexpand(const gen & args,giac::context * );
  gen powexpand(const gen & e,giac::context * );
  gen exp2pow(const gen & e,giac::context * );
  gen _exp2pow(const gen & args,giac::context * );
  gen _pow2exp(const gen & e,giac::context * );
  gen pow2expln(const gen & e,giac::context * );
  gen simplifyfactorial(const gen & g,giac::context * );
  gen pow2expln(const gen & e,const giac::identificateur & x,giac::context * );
  gen gamma2factorial(const gen & g,giac::context * );
  gen gammatofactorial(const gen & g,giac::context * );
  gen factorial2gamma(const gen & g,giac::context * );
  gen factorialtogamma(const gen & g,giac::context * );
  gen factor_xn(const gen & args,const gen & x,giac::context * );
  gen factor_xn(const gen & args,giac::context * );
  gen _factor_xn(const gen & args,giac::context * );
  gen Heavisidetosign(const gen & args,giac::context * );
  gen _Heavisidetosign(const gen & args,giac::context * );
  gen expln2trig(const gen & g,giac::context * );
  gen _mult_conjugate(const gen & g0,giac::context * );
  gen _mult_c_conjugate(const gen & g0,giac::context * );
  gen sin2tan2(const gen & e,giac::context * );
  gen cos2tan2(const gen & e,giac::context * );
  gen tan2tan2(const gen & e,giac::context * );
  gen sinh2exp(const gen & e,giac::context * );
  gen cosh2exp(const gen & e,giac::context * );
  gen tanh2exp(const gen & e,giac::context * );
  gen inv_test_exp(const gen & e,giac::context * );
  gen sin2exp(const gen & e,giac::context * );
  gen cos2exp(const gen & e,giac::context * );
  gen tan2exp(const gen & e,giac::context * );
  gen exp2sincos(const gen & e,giac::context * );
  gen tantosincos(const gen & e,giac::context * );
  gen tantosincos2(const gen & e,giac::context * );
  gen tantocossin2(const gen & e,giac::context * );
  gen asintoacos(const gen & e,giac::context * );
  gen acostoasin(const gen & e,giac::context * );
  gen asintoatan(const gen & e,giac::context * );
  gen atantoasin(const gen & e,giac::context * );
  gen acostoatan(const gen & e,giac::context * );
  gen atantoacos(const gen & e,giac::context * );
  gen trigcospow(const gen & g,giac::context * );
  gen trigsinpow(const gen & g,giac::context * );
  gen trigtanpow(const gen & g,giac::context * );
  gen powtopowexpand(const gen & g,giac::context * );
  gen exptopower(const gen & g,giac::context * );
  gen asin2ln(const gen & g_orig,giac::context * );
  gen acos2ln(const gen & g_orig,giac::context * );
  gen atan2ln(const gen & g_orig,giac::context * );
  bool guess_program(gen & g,giac::context * );
  gen normal(const gen & e,giac::context * ); // rational simplifications
  gen normal(const gen & e,bool distribute_div,giac::context * );
  gen normalize_sqrt(const gen & e,giac::context * );
  gen recursive_normal(const gen & e,giac::context * );  
  gen _recursive_normal(const gen & e,giac::context * );  
  gen recursive_normal(const gen & e,bool distribute_div,giac::context * );
  gen factor(const gen & e,bool withsqrt,giac::context * ); // full factorization (alg ext)
  gen ratfactor(const gen & e,bool withsqrt,giac::context * ); // full factorization (rat)
  gen factor(const gen & e,const gen & f,bool withsqrt,giac::context * );
  gen _factor(const gen & args,giac::context * );
  gen _collect(const gen & args,giac::context * );
  gen factorcollect(const gen & args,bool with_sqrt,giac::context * );
  gen partfrac(const gen & e,const giac::vecteur & l,bool withsqrt,giac::context * );
  gen partfrac(const gen & e,bool withsqrt,giac::context * ); 
  gen partfrac(const gen & e,const giac::identificateur & x,bool withsqrt,giac::context * ); 
  gen partfrac(const gen & e,const gen & f,bool withsqrt,giac::context * );
  gen _partfrac(const gen & args,giac::context * );
  gen _resultant(const gen & args,giac::context * );
  gen cklvar(const gen & e,giac::context * );
  gen ckalgvar(const gen & e,giac::context * );

  bool sym2r (const gen &e,const giac::vecteur &l,const giac::vecteur & lv, const giac::vecteur & lvnum,const giac::vecteur & lvden, int l_size, gen & num,gen & den,giac::context * );
  bool sym2r (const gen &e,const giac::vecteur &l, int l_size, gen & num,gen & den,giac::context * );
  gen e2r(const gen & e,const giac::vecteur & l,giac::context * ); 
  gen e2r(const gen & e,const gen & x,giac::context * );
  gen _e2r(const gen & args,giac::context * );
  gen r2sym(const giac::polynome & p, const giac::vecteur & l,giac::context * );
  gen r2sym(const gen & p, const giac::vecteur & l,giac::context * );
  gen r2e(const gen & p,const giac::vecteur & l,giac::context * );
  gen r2e(const gen & r,const gen & x,giac::context * );
  gen _r2e(const gen & args,giac::context * );
  gen r2sym(const giac::fraction & f, const giac::vecteur & l,giac::context * );
  gen r2sym(const giac::factorization & vnum,const giac::vecteur & l,giac::context * );
  std::string & add_print(std::string & s,const gen & g,giac::context * );
  void evalfdouble2reim(const gen & a,gen & e,gen & f0,gen & f1,giac::context * );
  std::string gen2tex(const gen & e,giac::context * );
  gen graph2tex(const gen & args,const giac::vecteur & v,giac::context * );
  gen _graph2tex(const gen & args,giac::context * );
  gen _graph3d2tex(const gen & args,giac::context * );
  gen _latex(const gen & args,giac::context * );
  gen _seq(const gen & g,giac::context * );
  gen _logb(const gen & g,giac::context * );
  gen _getType(const gen & g,giac::context * );
  gen _Define(const gen & g,giac::context * );
  gen _isprime(const gen & args,giac::context * );
  gen _Input(const gen & args,giac::context * );
  gen _InputStr(const gen & g,giac::context * );
  gen _Prompt(const gen & g,giac::context * );
  gen _cSolve(const gen & g,giac::context * );
  gen _cFactor(const gen & g,giac::context * );
  gen _cpartfrac(const gen & g,giac::context * );
  gen _nSolve(const gen & g,giac::context * );
  gen _zeros(const gen & g,giac::context * );
  gen _cZeros(const gen & g,giac::context * );
  gen _getDenom(const gen & g,giac::context * );
  gen _denom(const gen & g,giac::context * );
  gen _getNum(const gen & g,giac::context * );
  gen _numer(const gen & g,giac::context * );
  gen _tExpand(const gen & g,giac::context * );
  gen _comDenom(const gen & g,giac::context * );
  gen _randPoly(const gen & g,giac::context * );
  gen _nInt(const gen & g,giac::context * );
  gen _nDeriv(const gen & g,giac::context * );
  gen _avgRC(const gen & g,giac::context * );
  gen _fMin(const gen & g,giac::context * );
  gen _fMax(const gen & g,giac::context * );
  gen _taylor(const gen & g,giac::context * );
  gen _arcLen(const gen & g,giac::context * );
  gen _dim(const gen & g,giac::context * );
  gen _format(const gen & g,giac::context * );
  gen _inString(const gen & g,giac::context * );
  gen _left(const gen & g,giac::context * );
  gen _right(const gen & g,giac::context * );
  gen _mid(const gen & g,giac::context * );
  gen _ord(const gen & g,giac::context * );
  gen _rotate(const gen & g,giac::context * );
  gen _shift(const gen & g,giac::context * );
  gen _augment(const gen & g,giac::context * );
  gen _semi_augment(const gen & g,giac::context * );
  gen _cumSum(const gen & g,giac::context * );
  gen _exp2list(const gen & g,giac::context * );
  gen _list2mat(const gen & g,giac::context * );
  gen _deltalist(const gen & g,giac::context * );
  gen _mat2list(const gen & g,giac::context * );
  gen _newList(const gen & g,giac::context * );
  gen _polyEval(const gen & g,giac::context * );
  gen _product(const gen & g,giac::context * );
  gen _SortA(const gen & g,giac::context * );
  gen _SortD(const gen & g,giac::context * );
  gen _int(const gen & g,giac::context * );
  gen _iPart(const gen & g,giac::context * );
  gen _Fill(const gen & g,giac::context * );
  gen _mRow(const gen & g,giac::context * );
  gen _mRowAdd(const gen & g,giac::context * );
  gen _rowAdd(const gen & g,giac::context * );
  gen _rowSwap(const gen & g,giac::context * );
  gen _LU(const gen & g,giac::context * );
  gen _QR(const gen & g,giac::context * );
  gen _newMat(const gen & g,giac::context * );
  gen _ref(const gen & a,giac::context * ) ;
  gen _subMat(const gen & g,giac::context * ) ;
  gen _unitV(const gen & g,giac::context * ) ;
  gen _rowNorm(const gen & g,giac::context * ) ;
  gen _colNorm(const gen & g,giac::context * ) ;
  gen _Archive(const gen & g,giac::context * );
  gen _Unarchiv(const gen & g,giac::context * );
  gen _ClrIO(const gen & g,giac::context * );
  gen _Output(const gen & g,giac::context * );
  gen _getKey(const gen & g,giac::context * );
  gen _DelFold(const gen & g,giac::context * );
  gen _DispG(const gen & g,giac::context * );
  gen _DispHome(const gen & g,giac::context * );
  gen _Exec(const gen & g,giac::context * );
  gen _NewFold(const gen & g,giac::context * );
  gen _GetFold(const gen & g,giac::context * );
  gen _StoPic(const gen & g,giac::context * );
  gen _RclPic(const gen & g,giac::context * );
  gen _RplcPic(const gen & g,giac::context * );
  gen _ClrGraph(const gen & g,giac::context * );
  gen _PtOn(const gen & g,giac::context * );
  gen _PtOff(const gen & g,giac::context * );
  gen _Line(const gen & g,giac::context * );
  gen _LineHorz(const gen & g,giac::context * );
  gen _LineVert(const gen & g,giac::context * );
  gen _DrawSlp(const gen & g,giac::context * );
  gen _Circle(const gen & g,giac::context * );
  gen _PtText(const gen & g,giac::context * );
  gen _NewPic(const gen & g,giac::context * );
  gen _ZoomSto(const gen & g,giac::context * );
  gen _ZoomRcl(const gen & g,giac::context * );
  gen _deSolve(const gen & g,giac::context * );
  gen _LineTan(const gen & g,giac::context * );
  gen _CyclePic(const gen & g,giac::context * );
  gen _RandSeed(const gen & g,giac::context * );
  gen exact(const gen & g,giac::context * );
  gen fPart(const gen & g,giac::context * );
  gen simult(const gen & g,giac::context * );
  gen make_symbol(const std::string & s,giac::context * );
  gen _unarchive_ti(const gen & g,giac::context * );
  gen _sialorssinon(const gen & g,giac::context * );
  gen _pour(const gen & g,giac::context * );
  gen _unarchive_ti(const gen & args,giac::context * );
  std::string printsommetasoperator(const gen & feuille,const char * sommetstr,giac::context * );
  std::string texprintsommetasoperator(const gen & feuille,const char * sommetstr,giac::context * );
  gen taylor(const gen & lim_point,int order,const giac::unary_function_ptr & D, int direction,gen & shift_coeff,giac::context * );
  gen apply(const gen & e,const giac::unary_function_ptr & f,giac::context * );
  gen frac_neg_out(const gen & g,giac::context * );
  gen _constant_one(const gen & args,giac::context * );
  gen _constant_zero(const gen & args,giac::context * );
  gen _rm_a_z(const gen & args,giac::context * );
  gen _id(const gen & args,giac::context * );
  gen _not(const gen & args,giac::context * );
  gen _neg(const gen & args,giac::context * );
  gen _inv(const gen & args,giac::context * );
  gen ln(const gen & e,giac::context * );
  gen log(const gen & e,giac::context * );
  gen log10(const gen & e,giac::context * );
  gen alog10(const gen & e,giac::context * );
  gen atan(const gen & e0,giac::context * );
  gen exp(const gen & e0,giac::context * );
  std::string gen2string(const gen & g,int format,giac::context * );
  std::string print_with_parenthesis_if_required(const gen & g,int format,giac::context * );
  void zint2simpldoublpos(const gen & e,gen & simpl,gen & doubl,bool & pos,int d,giac::context * );
  gen sqrt_noabs(const gen & e,giac::context * );
  gen sqrt(const gen & e,giac::context * );
  gen sqrt_mod(const gen & a,const gen & b,bool isprime,giac::context * ); // set isprime to true if b is prime
  gen sin(const gen & e,giac::context * );
  gen cos(const gen & e,giac::context * );
  gen tan(const gen & e,giac::context * );
  gen asin(const gen & e,giac::context * );
  gen acos(const gen & e,giac::context * );
  gen sinh(const gen & e,giac::context * );
  gen cosh(const gen & e,giac::context * );
  gen tanh(const gen & e,giac::context * );
  gen asinh(const gen & e,giac::context * );
  gen acosh(const gen & e,giac::context * );
  gen atanh(const gen & e,giac::context * );
  gen quote(const gen & e,giac::context * );
  gen unquote(const gen & e,giac::context * );
  gen re(const gen & a,giac::context * );
  gen im(const gen & a,giac::context * );
  gen conj(const gen & a,giac::context * );
  gen _sign(const gen & g,giac::context * );
  gen order_size(const gen & e,giac::context * );
  gen _and(const gen & args,giac::context * );
  gen _ou(const gen & args,giac::context * );
  gen _xor(const gen & args,giac::context * );
  gen xor2(const gen & a,const gen & b,giac::context * );
  gen _min(const gen & args,giac::context * );
  gen _max(const gen & args,giac::context * );
  gen _gcd(const gen & args,giac::context * );
  gen _lcm(const gen & args,giac::context * );
  gen _egcd(const gen & args,giac::context * );
  gen _iegcd(const gen & args,giac::context * );
  gen _iquo(const gen & args,giac::context * );
  gen _irem(const gen & args,giac::context * );
  // gen _mods(const gen & args,giac::context * );
  gen _quote_pow(const gen & args,giac::context * );
  gen _iquorem(const gen & args,giac::context * );
  gen _smod(const gen & args,giac::context * );
  gen _rdiv(const gen & args,giac::context * );
  gen _is_prime(const gen & args,giac::context * );
  gen _is_pseudoprime(const gen & args,giac::context * );
  gen nextprime1(const gen & a,giac::context * );
  gen prevprime1(const gen & a,giac::context * );
  // gen _nextprime(const gen & args,giac::context * );
  // gen _prevprime(const gen & args,giac::context * );
  gen _jacobi_symbol(const gen & args,giac::context * );
  gen _legendre_symbol(const gen & args,giac::context * );
  gen _floor(const gen & args,giac::context * );  
  gen _ceil(const gen & args,giac::context * );  
  gen ceil2floor(const gen & g,giac::context * );
  gen _round(const gen & args,giac::context * );  
  gen _print(const gen & args,giac::context * );  
  gen _ichinrem(const gen & args,giac::context * );
  gen double_is_int(const gen & g,giac::context * );
  gen _fracmod(const gen & args,giac::context * );
  gen _factorial(const gen & args,giac::context * );
  gen _perm(const gen & args,giac::context * );
  gen comb(const gen & n,const gen &k,giac::context * );
  gen _comb(const gen & args,giac::context * );
  gen _chinrem(const gen & args,giac::context * );
  gen _abs(const gen & args,giac::context * );
  gen _cyclotomic(const gen & a,giac::context * );
  gen _calc_mode(const gen & args,giac::context * );
  gen _quorem(const gen & args,giac::context * );
  gen _quo(const gen & args,giac::context * );
  gen _rem(const gen & args,giac::context * );
  gen sto(const gen & a,const gen & b,giac::context * );
  gen sto(const gen & a,const gen & b,bool in_place,giac::context * );  
  bool is_assumed_integer(const gen & g,giac::context * );
  gen giac_assume(const gen & a,giac::context * );
  gen giac_additionally(const gen & a,giac::context * );
  gen doubleassume_and(const giac::vecteur & v,const gen & a,int direction,bool or_assumption,giac::context * );
  gen _equal(const gen & args,giac::context * );
  gen _same(const gen & args,giac::context * );
  gen _inferieur_strict(const gen & args,giac::context * );
  gen _inferieur_egal(const gen & args,giac::context * );
  gen _superieur_strict(const gen & args,giac::context * );
  gen _superieur_egal(const gen & args,giac::context * );
  gen _different(const gen & args,giac::context * );
  gen check_symb_of(const gen & a,const gen & b,giac::context * );
  gen _of(const gen & args,giac::context * );
  gen _at(const gen & args,giac::context * );
  gen _table(const gen & args,giac::context * );
  gen _plus(const gen & args,giac::context * );
  gen _prod(const gen & args,giac::context * );
  std::string cprintaspow(const gen & feuille,const char * sommetstr_orig,giac::context * );
  gen _pow(const gen & args,giac::context * );
  gen _powmod(const gen & args,giac::context * );
  gen _eval(const gen & a,giac::context * );
  gen _evalf(const gen &,giac::context * );
  gen _subst(const gen & args,giac::context * );
  gen _subs(const gen & args,giac::context * );
  gen _maple_subs(const gen & args,giac::context * );
  gen _ampersand_times(const gen & g,giac::context * );
  gen _version(const gen & a,giac::context * );
  gen Gamma(const gen & x,giac::context * );
  gen _Gamma(const gen & args,giac::context * );
  gen lngamma(const gen & x,giac::context * );
  gen Psi(const gen & x,giac::context * );
  gen Psi(const gen & x,int n,giac::context * );
  gen _Psi(const gen & args,giac::context * );
  gen Zeta(const gen & x,giac::context * );
  gen _Zeta(const gen & args,giac::context * );
  gen _erf(const gen & args,giac::context * );
  gen erf(const gen & args,giac::context * );
  gen _erfc(const gen & args,giac::context * );
  gen erfc(const gen & args,giac::context * );
  gen _Ci(const gen & g,giac::context * );
  gen _Si(const gen & g,giac::context * );
  gen _Ei(const gen & g,giac::context * );
  gen _Heaviside(const gen & args,giac::context * );
  gen _Dirac(const gen & args,giac::context * );
  gen _multistring(const gen & args,giac::context * );
  gen Gamma(const gen & x,giac::context * );
  gen _Gamma(const gen & args,giac::context * ) ;
  gen Psi(const gen & x,giac::context * );
  gen Psi(const gen & x,int n,giac::context * );
  gen _Psi(const gen & args,giac::context * ) ;
  gen _normalmod(const gen & g,giac::context * );
  gen alternate_series(const gen & a,const gen & x,int n,giac::context * );
  gen Eta(const gen & s,int ndiff,giac::context * );
  gen Eta(const gen & s0,giac::context * );
  gen Zeta(const gen & x,int ndiff,giac::context * );
  gen Zeta(const gen & x,giac::context * );
  gen _Zeta(const gen & args,giac::context * ) ;
  gen _Eta(const gen & args,giac::context * ) ;
  gen _erfs(const gen & g,giac::context * );
  gen erf(const gen & x,giac::context * );
  gen _erf(const gen & args,giac::context * );
  gen erfc(const gen & x,giac::context * );
  gen _erfc(const gen & args,giac::context * );
  gen _SiCi_f(const gen & args,giac::context * );
  gen _SiCi_g(const gen & args,giac::context * );
  gen _Si(const gen & args,giac::context * );
  gen _Ci(const gen & args,giac::context * );
  gen _Ci0(const gen & args,giac::context * );
  gen _Ei_f(const gen & args,giac::context * );
  gen Ei(const gen & args,giac::context * );
  gen Ei(const gen & args,int n,giac::context * );
  gen _Ei(const gen & args,giac::context * );
  gen _Ei0(const gen & args,giac::context * );
  gen Ci_replace0(const gen & g,giac::context * );
  gen Ei_replace0(const gen & g,giac::context * );
  gen _Dirac(const gen & args,giac::context * );
  gen _Heaviside(const gen & args,giac::context * );
  std::string printassubs(const gen & feuille,const char * sommetstr,giac::context * );
  std::string printasmaple_subs(const gen & feuille,const char * sommetstr,giac::context * );
  std::string printassto(const gen & feuille,const char * sommetstr,giac::context * );
  gen _proot(const gen & e,giac::context * );
  gen _pcoeff(const gen & e,giac::context * );
  gen _peval(const gen & e,giac::context * );
  gen spread_convert(const gen & g,int g_row,int g_col,giac::context * );
  bool iscell(const gen & g,int & r,int & c,giac::context * );
  giac::matrice matrice_insert(const giac::matrice & m,int insert_row,int insert_col,int nrows,int ncols,const gen & fill,giac::context * );
  gen cross(const gen & g1,const gen & g2,giac::context * );
  gen _tran(const gen & a,giac::context * );
  gen ckmtrace(const gen & a,giac::context * );
  bool mrref(const giac::matrice & a, giac::matrice & res, giac::vecteur & pivots, gen & det,giac::context * );
  gen _rref(const gen & a,giac::context * ); // first non 0 elem in row is 1
  gen _idn(const gen & e,giac::context * );
  giac::vecteur vranm(int n,const gen & f,giac::context * ); 
  giac::matrice mranm(int n,int m,const gen & f,giac::context * ); // random matrix using f
  gen _ranm(const gen & e,giac::context * );
  gen _randvector(const gen & e,giac::context * );
  gen _padic_linsolve(const gen & g,giac::context * );
  gen mdet(const giac::matrice & a,giac::context * );
  gen _det(const gen & a,giac::context * );
  //gen _det_minor(const gen & g,bool convert_internal,giac::context * );
  gen _sylvester(const gen & a,giac::context * );
  gen _hessenberg(const gen & g,giac::context * );
  gen _pcar_hessenberg(const gen & g,giac::context * );
  gen _hessenberg(const gen & g0,giac::context * );
  gen _pcar(const gen & a,giac::context * );
  bool egv(const giac::matrice & m,giac::matrice & p,giac::vecteur & d, giac::context * , bool jordan,bool rational_jordan_form,bool eigenvalues_only);
  gen _egv(const gen & a,giac::context * );
  gen _svd(const gen & a,giac::context * );
  gen _egvl(const gen & a,giac::context * );
  gen _jordan(const gen & a,giac::context * );
  gen jordan(const gen & a,bool rational_jordan,giac::context * );
  gen _rat_jordan(const gen & a,giac::context * );
  gen _rat_jordan_block(const gen &args,giac::context * );
  giac::matrice diagonal_apply(const gen & g,const gen & x,const giac::matrice & m,giac::context * );
  giac::matrice analytic_apply(const gen &ux,const gen & x,const giac::matrice & m,giac::context * );
  giac::matrice matpow(const giac::matrice & m,const gen & n,giac::context * );
  gen _matpow(const gen & a,giac::context * );
  gen _ker(const gen & a,giac::context * );
  gen _image(const gen & a,giac::context * );
  gen _cross(const gen & a,giac::context * );
  gen _size(const gen & a,giac::context * );
  gen lu(const gen & a,giac::context * );
  gen qr(const gen & a,giac::context * );
  gen _cholesky(const gen & a,giac::context * );
  gen _svd(const gen & a,giac::context * );
  gen _basis(const gen & a,giac::context * );
  gen _ibasis(const gen & a,giac::context * );
  gen _cell(const gen & a,giac::context * );
  gen l2norm(const giac::vecteur & v,giac::context * );
  gen _lll(const gen & g,giac::context * );
  gen _ihermite(const gen & g,giac::context * );
  gen _ismith(const gen & g,giac::context * );
  gen _csv2gen(const gen & g,giac::context * );
  giac::matrice csv2gen(std::istream & i,char sep,char nl,char decsep,char eof,giac::context * );
}


