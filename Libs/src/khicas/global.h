/* -*- mode:C++;compile-command: "g++-3.4 -I.. -g -c global.cc" -*- */

/* Global definition and constants (see also dispatch.h)
 *  Copyright (C) 2000,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#ifndef _GIAC_GLOBAL_H
#define _GIAC_GLOBAL_H
//#define USE_OBJET_BIDON
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "first.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#define GIAC_CONTEXT const context * contextptr
#define GIAC_CONTEXT0 const context * contextptr=0

#if !defined(HAVE_NO_SYS_TIMES_H) && !defined(BESTA_OS) && !defined(__MINGW_H) && !defined(NSPIRE) && !defined(FXCG)
#include <sys/times.h>
#else
#if defined VISUALC || defined BESTA_OS || defined FREERTOS
typedef long pid_t;
#else // VISUALC
#if !defined(__MINGW_H) && !defined(HP39) && !defined(NSPIRE) && !defined(FXCG) && !defined(__ANDROID__) && !defined(NSPIRE_NEWLIB) && !defined(OSX) && !defined(IOS) && !defined(OSXIOS) && !defined(FIR_LINUX) && !defined(PRIMEWEBASM)
#include "wince_replacements.h"
#endif
#ifdef __MINGW_H
#include <sys/types.h>
#endif
#endif // VISUALC
#endif // HAVE_NO_SYS_TIMES_H
// #ifndef __APPLE__
#if defined VISUALC || defined BESTA_OS
#include <math.h>
#include <float.h>
#endif
#ifndef WIN32
#include <math.h>
//#define isnan __isnan
//#define isinf __isinf
#endif
// #endif

#ifdef SOFTMATH
#include "softmath.h"
#else
#include <cmath>
#endif

#ifdef _SOFTMATH_H
inline double giac_log(double d){
  return std::giac_gnuwince_log(d);
}
#else
inline double giac_log(double d){
  return std::log(d);
}
#endif


#ifdef HAVE_LIBPTHREAD
#include <semaphore.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include "vector.h"
#include <string>
#if !defined( NSPIRE) && !defined(FXCG)
#include <cstring>
#endif
#include <iostream>
//#include <fstream>
#include <map>

#ifdef GNUWINCE
#define SIGINT 2
#else
#ifndef HAVE_NO_SIGNAL_H
#include <signal.h>
#endif
#endif // GNUWINCE

#include <stdexcept>
#include "help.h"
#if !defined(FXCG) // !defined(GIAC_HAS_STO_38) // && !defined(ConnectivityKit) && !defined(BESTA_OS)
#include "tinymt32.h"
#endif

#if defined(HAVE_NO_SYS_TIMES_H) && defined(__MINGW_H)
#include <time.h>
#endif

extern "C" int ctrl_c_interrupted(int exception);
extern "C" void console_print(const char * s);
extern "C" const char * console_prompt(const char * s);

bool dfu_get_scriptstore_addr(size_t & start,size_t & taille);
bool dfu_get_scriptstore(const char * fname);
bool dfu_send_scriptstore(const char * fname);
bool dfu_send_rescue(const char * fname);
bool dfu_send_bootloader(const char * fname);
const int nwstoresize1=0x8000,nwstoresize2=0x8014;
// send to 0x90000000+offset*0x10000
bool dfu_send_firmware(const char * fname,int offset);
bool dfu_send_apps(const char * fname);
bool dfu_update_khicas(const char * fname); 

#if defined HAVE_LIBMICROPYTHON
#include <string>
// giac interface to micropython modules
std::string & python_console();
#endif
#ifdef QUICKJS
#include <string>
extern std::string js_vars;
#endif
int js_token(const char * list,const char * buf);
int js_token(const char * buf);
void update_js_vars();

extern bool freezeturtle;
extern "C" size_t pythonjs_stack_size,pythonjs_heap_size;
extern "C" void * bf_ctx_ptr;
extern "C" size_t bf_global_prec;

struct fileinfo_t {
  std::string filename;
  std::string type;
  size_t size;
  size_t header_offset;
  int mode;
};
size_t tar_totalsize(const char * buffer,size_t byteLength);
std::vector<fileinfo_t> tar_fileinfo(const char * buffer,size_t byteLength);
// tar file format: operations on a malloc-ed char * buffer of size buffersize
// (malloc is assumed if buffer needs to be resized by tar_adddata)
extern char * buf64k; // a 64k buffer in RAM for flash sector copy
extern int numworks_maxtarsize; // max tar size on the Numworks
extern size_t tar_first_modified_offset; // will be used to truncate the file sent to  the Numworks
int flash_adddata(const char * buffer_,const char * filename,const char * data,size_t datasize,int exec);
int tar_adddata(char * & buffer,size_t * buffersizeptr,const char * filename,const char * data,size_t datasize,int exec=0); // filename is only used to fill the header
int flash_addfile(const char * buffer,const char * filename);
int tar_addfile(char * & buffer,const char * filename,size_t * buffersizeptr);
const char * tar_loadfile(const char * buffer,const char * filename,size_t * len);
int tar_filebrowser(const char * buf,const char ** filenames,int maxrecords,const char * extension);
// by default removefile will mark the file as deleted (this requires 1 sector write)
// if mark_only==2, this will undelete the file
// int flash_removefile(const char * buffer,const char * filename,size_t * tar_first_modif_offsetptr,int mark_only=1);
// write all changes made in records (filename and readable attribut)
// returns 0 if there is a mismatch between buffer and finfo
int flash_synchronize(const char * buffer,const std::vector<fileinfo_t> & finfo,size_t * tar_first_modif_offsetptr);
ulonglong fromstring8(const char * ptr);
std::string toString8(longlong chksum);
void WriteMemory(char * target,const char * src,size_t length);
void erase_sector(const char * buf);

// empty trash: files marked as non readable are really removed
// this will do 1 sector write from first sector where a file is marked to be removed to the end 
int flash_emptytrash(const char * buffer,size_t * tar_first_modif_offsetptr);
int flash_emptytrash(const char * buffer,const std::vector<fileinfo_t> & finfo,size_t * tar_first_modif_offsetptr);

int tar_removefile(char * buffer,const char * filename,size_t * tar_first_modif_offsetptr);
int tar_savefile(char * buffer,const char * filename);
std::vector<fileinfo_t> tar_fileinfo(const char * buffer,size_t byteLength);
char * file_gettar(const char * filename);
// same as file_gettar but returns an aligned pointer and sets freeptr to the address to be free-ed
char * file_gettar_aligned(const char * filename,char * & freeptr);
int file_savetar(const char * filename,char * buffer,size_t buffersize);
#if !defined KHICAS && !defined USE_GMP_REPLACEMENTS && !defined GIAC_HAS_STO_38// 
// numworks_gettar return 0 or a buffer of size numworks_maxtarsize
// initialized with the calculator content
char * numworks_gettar(size_t & tar_first_modif_offset); 
bool numworks_sendtar(char * buffer,size_t buffersize,size_t tar_first_modif_offset=0);
#endif

//sha256 support
#if !defined USE_GMP_REPLACEMENTS && !defined GIAC_HAS_STO_38
#ifdef __cplusplus
extern "C" {
#endif
/*************************** HEADER FILES ***************************/
#include <stddef.h>

/****************************** MACROS ******************************/
#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

/**************************** DATA TYPES ****************************/
typedef unsigned char BYTE;             // 8-bit byte
typedef unsigned int  WORD32;             // 32-bit word, change to "long" for 16-bit machines

typedef struct {
	BYTE data[64];
	WORD32 datalen;
	unsigned long long bitlen;
	WORD32 state[8];
} SHA256_CTX;

/*********************** FUNCTION DECLARATIONS **********************/
void giac_sha256_init(SHA256_CTX *ctx);
void giac_sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);
void giac_sha256_final(SHA256_CTX *ctx, BYTE hash[]);
#ifdef __cplusplus
}
#endif
#endif // sha256  

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  // 3 or 1 if a list of space separated commandnames includes buf
  int dichotomic_search(const char * const * tab,unsigned tab_size,const char * s);
  void opaque_double_copy(void * source,void * target);
  double opaque_double_val(const void * source);

  double giac_floor(double d);
  double giac_ceil(double d);
  unsigned int utf82unicode(const char * line,wchar_t * wline,unsigned int n);
  unsigned int unicode2utf8(const wchar_t * wline,char * line,unsigned int n);
  wchar_t * utf82unicode(const char * idname);
  char * unicode2utf8(const wchar_t * idname);

/* ---------------------------------------------------------------------
    The following 4 definitions are compiler-specific.
    The C standard does not guarantee that wchar_t has at least
    16 bits, so wchar_t is no less portable than unsigned short!
    All should be unsigned values to avoid sign extension during
    bit mask & shift operations.
------------------------------------------------------------------------ */

#ifdef GIAC_HAS_STO_38
typedef unsigned long UTF32; /* at least 32 bits */
typedef wchar_t UTF16; /* at least 16 bits */
typedef unsigned char UTF8; /* typically 8 bits */
typedef unsigned char Boolean; /* 0 or 1 */
#else
typedef unsigned long UTF32; /* at least 32 bits */
typedef unsigned short UTF16; /* at least 16 bits */
typedef unsigned char UTF8; /* typically 8 bits */
typedef unsigned char Boolean; /* 0 or 1 */
#endif

  int system_no_deprecation(const char *command);

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF

typedef enum {
    conversionOK = 0,   /* conversion successful */
    sourceExhausted = -1, /* partial character in source, but hit end */
    targetExhausted = -2, /* insuff. room in target for conversion */
    sourceIllegal = -3 /* source sequence is illegal/malformed */
} ConversionResult;

typedef enum {
    strictConversion = 0,
    lenientConversion
} ConversionFlags;

/* This is for C++ and does no harm in C */
#ifdef __cplusplus
extern "C" {
#endif

unsigned int ConvertUTF8toUTF16 (
        const UTF8* sourceStart, const UTF8* sourceEnd, 
        UTF16* targetStart, UTF16* targetEnd, ConversionFlags flags);

unsigned int ConvertUTF16toUTF8 (
        const UTF16* sourceStart, const UTF16* sourceEnd, 
        UTF8* targetStart, UTF8* targetEnd, ConversionFlags flags);

Boolean isLegalUTF8Sequence(const UTF8 *source, const UTF8 *sourceEnd);

#ifdef __cplusplus
}
#endif

  // convert position n in utf8-encoded line into the corresponding position
  // in the same string encoded with unicode
  unsigned int utf8pos2unicodepos(const char * line,unsigned int n,bool skip_added_spaces = true);
  unsigned int wstrlen(const char * line, unsigned int n = ~0u);
  unsigned int wstrlen(const wchar_t * wline);
  unsigned int utf8length(const wchar_t * wline);


#if defined VISUALC || defined BESTA_OS || defined FREERTOS
#ifndef R_OK
  extern int R_OK;
#endif
  int access(const char * ch,int mode);
  void usleep(int );
#endif
#if defined NSPIRE_NEWLIB || defined KHICAS
  void usleep(int );
#endif

#ifndef HAVE_NO_SYS_TIMES_H
  double delta_tms(struct tms tmp1,struct tms tmp2);
#elif defined(__MINGW_H)
  double delta_tms(clock_t tmp1,clock_t tmp2);
#endif

#define GIAC_DATA_BEGIN   ((char) 2)
#define GIAC_DATA_END     ((char) 5)
#define GIAC_DATA_ESCAPE  ((char) 27)

  std::vector<aide> * & vector_aide_ptr();
  std::vector<std::string> * & vector_completions_ptr();
  extern void (*fl_widget_delete_function)(void *);
#if !defined( NSPIRE) && !defined(FXCG)
  extern std::ostream & (*fl_widget_archive_function)(std::ostream &,void *);
#endif
  extern bool secure_run; // true if used in a non-trusted environment
  extern bool center_history,in_texmacs,block_signal,synchronize_history;
  extern bool threads_allowed;
  extern bool mpzclass_allowed;
  enum { 
    smallint=256, // max small int to make modular oply operations with int
    max_series_expansion_order=64, // max auto order for series expansion
    max_texpand_expansion_order=64,
    max_numexp=100
  };
  extern const char cas_suffixe[];
  extern const int BUFFER_SIZE;
  extern int history_begin_level;

  extern int debug_infolevel; // ==0 normal value
  extern int printprog; // ==0 normal value, used to force program print at parse
  // >0 log some informations
  // <0 for internal use
  // ==-1:
  // ==-2: icas.cc reads aide_cas even if STATIC_BUILTIN_LEXER is defined
  //       write static_lexer.h, static_extern.h
  // ==-3:
  // ==-4: write static_lexer.h, static_lexer_.h and static_extern.h
  // ==-5: do not throw on errors
  extern int threads;
  extern unsigned short int GIAC_PADIC;

  extern bool CAN_USE_LAPACK;
  extern bool simplify_sincosexp_pi;
#ifndef RTOS_THREADX
  //#ifndef BESTA_OS
  extern int CALL_LAPACK; // lapack is used if dim of matrix is >= CALL_LAPACK
  // can be changed using shell variable GIAC_LAPACK in icas
  //#endif
#endif
  extern int FACTORIAL_SIZE_LIMIT;
  extern int GAMMA_LIMIT;
  extern int LIST_SIZE_LIMIT;
  extern int NEWTON_DEFAULT_ITERATION;
  extern int NEWTON_MAX_RANDOM_RESTART;
  extern int DEFAULT_EVAL_LEVEL;
  extern int PARENTHESIS_NWAIT;
  extern int MAX_PROD_EXPAND_SIZE;

  extern int TEST_PROBAB_PRIME; // probabilistic primality tests
  extern int GCDHEU_MAXTRY; // maximal number of retry for heuristic algorithms
  extern int GCDHEU_DEGREE; // max degree allowed inside gcdheu
  extern int MODFACTOR_PRIMES; // number of primes used for factorization
  extern int NTL_MODGCD; // lowest degree for NTL univariate modular GCD 
  extern int NTL_RESULTANT; // lowest degree for NTL univariate resultant 
  extern int NTL_XGCD; // lowest degree for NTL univariate extended GCD over Z
  extern int MODRESULTANT; // lowest degree for modular resultant 
  extern int HGCD; // lowest degree for half gcd call
  extern int HENSEL_QUADRATIC_POWER; // above #steps do quadratic Hensel lift
  extern int KARAMUL_SIZE; // Use Karatsuba multiplication if degree is >
  extern int INT_KARAMUL_SIZE; // Use Karatsuba multiplication if degree is >
  extern int FFTMUL_SIZE; // minimal size for fft mult of poly
  extern int FFTMUL_INT_MAXBITS; // max number of bits for fft mult of int poly 
  // Should be lower for larger coeff
  extern int MAX_ALG_EXT_ORDER_SIZE; // x^1/d extension not algebraic if d>
  extern int MAX_COMMON_ALG_EXT_ORDER_SIZE;
  extern int TRY_FU_UPRIME;
  extern int SOLVER_MAX_ITERATE;
  extern int MAX_PRINTABLE_ZINT;
  extern int MAX_RECURSION_LEVEL;
  extern int GBASIS_DETERMINISTIC;
  extern int GBASISF4_MAX_TOTALDEG;
  extern int GBASISF4_MAXITER;
  // extern int GBASISF4_BUCHBERGER;
  extern unsigned max_pairs_by_iteration; 
  extern unsigned simult_primes,simult_primes2,simult_primes_seuil2,simult_primes3,simult_primes_seuil3; 
  // see global.cc for explanations
  extern double gbasis_reinject_ratio;
  extern double gbasis_reinject_speed_ratio;
  extern int gbasis_logz_age_sort,gbasis_stop,rur_do_gbasis,rur_do_certify,rur_certify_maxthreads;
  extern bool rur_error_ifnot0dimensional; // error code or compute gbasis

  extern int PROOT_FACTOR_MAXDEG;
  extern int ABS_NBITS_EVALF;
  extern volatile bool ctrl_c,interrupted,kbd_interrupted;
  void ctrl_c_signal_handler(int signum);
#ifdef TIMEOUT
#if !defined(EMCC) && !defined(EMCC2)
  double time(int );
#endif
  extern time_t caseval_begin,caseval_current;
  extern double caseval_maxtime;
  extern int caseval_n,caseval_mod,caseval_unitialized;
#endif
  extern double powlog2float;
  extern int MPZ_MAXLOG2;

#ifdef WITH_MYOSTREAM
  // replacement for std::cerr
  extern my_ostream my_cerr;
#endif

  // void control_c();
  // note that ctrl_c=false was removed, should be done before calling eval
#if defined (NSPIRE) || defined(FXCG)
  void control_c();
#elif defined FIR
#define control_c()
#else
#if defined TIMEOUT && !defined POCKETCAS
  void control_c();
#else
#if 0
#define control_c() if (ctrl_c) { interrupted = true; CERR << "Throwing exception for user interruption." << '\n'; throw(std::runtime_error("Stopped by user interruption.")); }
#else
#define control_c() if (ctrl_c) { \
interrupted = true; \
std::string source_path = __FILE__; \
std::string source_filename = source_path.substr(source_path.find_last_of("/\\") + 1); \
CERR << "Throwing exception for user interruption (" << source_filename << ":" << __LINE__ << ")" << '\n'; \
throw(std::runtime_error("Stopped by user interruption.")); \
}
#endif
#endif // TIMEOUT
#endif // !NSPIRE, !FIR

  typedef void ( * void_function )();
  // set to non-0 if you want to hook a function call inside control_c()

#ifdef IMMEDIATE_VECTOR
  template <class T> class dbgprint_vector: public std::imvector<T> {
  public:
    // inherited constructors
    dbgprint_vector() : std::imvector<T>::imvector() { };
    dbgprint_vector(const T * b,const T * e) : std::imvector<T>::imvector(b,e) { };
    dbgprint_vector(size_t i) : std::imvector<T>::imvector(i) { };
    dbgprint_vector(size_t i,const T & t) : std::imvector<T>::imvector(i,t) { };
    // ~dbgprint_vector() { };
    // inherited destructors
    void dbgprint() const { COUT << *this << '\n'; }
  };
#else // IMMEDIATE_VECTOR
  template <class T> class dbgprint_vector: public std::vector<T> {
  public:
    // inherited constructors
    dbgprint_vector() : std::vector<T>::vector() { };
#ifndef GIAC_VECTOR
    dbgprint_vector(const typename std::vector<T>::const_iterator & b,const typename std::vector<T>::const_iterator & e) : std::vector<T>::vector(b,e) { };
#endif
    dbgprint_vector(const T * b,const T * e) : std::vector<T>::vector(b,e) { };
    dbgprint_vector(size_t i) : std::vector<T>::vector(i) { };
    dbgprint_vector(size_t i,const T & t) : std::vector<T>::vector(i,t) { };
    // ~dbgprint_vector() { };
    // inherited destructors
    void dbgprint() const { COUT << *this << '\n'; }
  };
#endif // IMMEDIATE_VECTOR
  
  template <class T> class std_matrix: public std::vector< dbgprint_vector<T> > {
  public:
    // inherited constructors
    std_matrix() : std::vector< dbgprint_vector<T> >::vector() { };
    std_matrix(size_t i) : std::vector< dbgprint_vector<T> >::vector(i) { };
    std_matrix(size_t i,const dbgprint_vector<T> & v) : std::vector< dbgprint_vector<T> >::vector(i,v) { };
    std_matrix(size_t i,size_t j) : std::vector< dbgprint_vector<T> >::vector(i,dbgprint_vector<T>(j)) { };
    std_matrix(size_t i,size_t j,const T & t) : std::vector< dbgprint_vector<T> >::vector(i,dbgprint_vector<T>(j,t)) { };
    // ~dbgprint_vector() { };
    // inherited destructors
    std_matrix<T> transpose() const {
      if (std::vector< dbgprint_vector<T> >::empty())
	return *this;
      int n=int(std::vector< dbgprint_vector<T> >::size());
      int m=int(std::vector< dbgprint_vector<T> >::front().dbgprint_vector<T>::size());
      std_matrix<T> res(m,n);
      typename std_matrix<T>::const_iterator it=std::vector< dbgprint_vector<T> >::begin();
      for (int i=0;i<n;++i,++it){
	for (int j=0;j<m;++j)
	  res[j][i]=(*it)[j];
      }
      return res;
    }
    std_matrix<T> transconjugate() const {
      if (std::vector< dbgprint_vector<T> >::empty())
	return *this;
      int n=std::vector< dbgprint_vector<T> >::size();
      int m=std::vector< dbgprint_vector<T> >::front().dbgprint_vector<T>::size();
      std_matrix<T> res(m,n);
      typename std_matrix<T>::const_iterator it=std::vector< dbgprint_vector<T> >::begin();
      for (int i=0;i<n;++i,++it){
	for (int j=0;j<m;++j)
	  res[j][i]=conj((*it)[j],0);
      }
      return res;
    }
    void dbgprint() { COUT << *this << '\n'; }
  };

  struct user_function {
    std::string s;
    int parser_token;
    user_function():s(""),parser_token(-1) {};
    user_function(const std::string & mys,int i):s(mys),parser_token(i){};
  };

  class gen;
  // vecteurs and dense 1-d polynomilas

  typedef dbgprint_vector<gen> vecteur; // debugging support

  vecteur * keywords_vecteur_ptr(); // idnt assigned to a commandname for localization, like mediatrice for perpen_bissector

  class context;
  extern const giac::context * python_contextptr;
  
  struct debug_struct {
    int indent_spaces;
    vecteur args_stack;
    vecteur debug_breakpoint; // alternate _IDNT and instruction number
    // count 1 for a normal instruction, 3 for ifte, 4 for a for loop
    // breakpoint(_IDNT,int) to set a breakpoint at _IDNT, instruction int
    // rmbreakpoint(int) removes breakpoint number int
    vecteur debug_watch; 
    // the value of each element of debug_watch is signaled
    // to the parent process each time the execution stops
    // watch(_IDNT) to add _IDNT to the watch
    // rmwatch(int) or rmwatch(_IDNT) removes _IDNT
    // halt inside a prog starts debug mode, debug(instruction) 
    // starts prog in SST mode
    // kill reset the protection level, instruction_stack and debug_mode to false
    bool debug_mode;
    bool sst_mode; // true to single step in this function
    bool sst_in_mode; // true to single step inside next instruction
    bool debug_allowed;
    std::vector<int> current_instruction_stack;
    int current_instruction;
    std::vector< std::vector<int> > sst_at_stack;
    std::vector<int> sst_at;
    gen * debug_info_ptr, * fast_debug_info_ptr,* debug_prog_name, * debug_localvars;
    bool debug_refresh;
    context * debug_contextptr;
    debug_struct();
    ~debug_struct();
    debug_struct & operator =(const debug_struct & dbg);
  };

  typedef void (* giac_callback)(const giac::gen & ,void * );

  struct thread_param {
    bool _kill_thread;
    int thread_eval_status;
    giac_callback f;
    void * f_param;
    giac::vecteur v;
#ifdef HAVE_LIBPTHREAD
    pthread_t eval_thread;
    pthread_attr_t attr;
    size_t stacksize;
    void * stackaddr;
#endif
    size_t stack;
    thread_param();
  };

#if !defined( NSPIRE) && !defined(FXCG)
  extern gen (*fl_widget_unarchive_function)(std::istream &);
#endif
  extern std::string (*fl_widget_texprint_function)(void * ptr);
  extern gen (*fl_widget_updatepict_function)(const gen & g);
  // name -> gen table
  struct ltstring
  {
    bool operator()(const std::string & s1, const std::string & s2) const
    {
      return strcmp(s1.c_str(), s2.c_str()) < 0;
    }
  };
  typedef std::map<std::string, gen,ltstring> sym_string_tab;
  struct ltstr
  {
    bool operator()(const char* s1, const char* s2) const
    {
      return strcmp(s1, s2) < 0;
    }
  };
  
  typedef std::map<const char *, gen,ltstr> map_charptr_gen;
  typedef map_charptr_gen sym_tab;

  struct nwsrec {
    unsigned char type;
    std::vector<unsigned char> data;
  };
  typedef std::map<std::string,nwsrec,ltstring> nws_map;
  std::string dos2unix(const std::string & res);
  bool scriptstore2map(const char * fname,nws_map & m);
  bool map2scriptstore(const nws_map & m,const char * fname);
  // check that filename content matches a file content signed in sigfilename
  bool sha256_check(const char * sigfilename,const char * filename);
  bool nws_certify_firmware(bool with_overwrite,GIAC_CONTEXT); // Numworks certification

  struct parser_lexer {
    int _index_status_; // 0 if [ -> T_VECT_DISPATCH, 1 if [ -> T_INDEX_BEGIN
    int _opened_quote_; // 1 if we are inside a quote
    int _in_rpn_; // 1 inside RPN expression
    int _lexer_line_number_;
    int _lexer_column_number_;
    int _spread_formula_; // beginning = and meaning of :
    int _initialisation_done_;
    std::string _comment_s_;
    std::string _parser_filename_;
    std::string _parser_error_;
    int _first_error_line_;
    std::string _error_token_name_;
    int _i_sqrt_minus1_;
  };
  std::string gen2string(const gen & g);
  const int turtle_length=10;
#ifdef KHICAS
  struct logo_turtle {
    double x,y;
    double theta; // theta is given in degrees or radians dep. on angle_mode
    bool visible; // true if turtle visible
    bool mark; // true if moving marks
    bool direct; // true if rond/disque is done in the trigonometric direction
    char turtle_width;
    short int s;//std::string s;
    int color;
    int radius; // 0 nothing, >0 -> draw a plain disk 
    // bit 0-8=radius, bit9-17 angle1, bit 18-26 angle2, bit 27=1 filled  or 0 
    // <0 fill a polygon from previous turtle positions
    logo_turtle(): x(100),y(100),theta(0),visible(true),mark(true),direct(true),color(0),turtle_width(1),radius(0) {}
    inline bool equal_except_nomark(const logo_turtle &t) const {
      return x==t.x && y==t.y && turtle_width==t.turtle_width && s==t.s && radius==t.radius;
    }
  };
#else // KHICAS
  struct logo_turtle {
    double x,y;
    double theta; // theta is given in degrees or radians dep. on angle_mode
    bool visible; // true if turtle visible
    bool mark; // true if moving marks
    bool direct; // true if rond/disque is done in the trigonometric direction
    int color;
    int turtle_width;
    int radius; // 0 nothing, >0 -> draw a plain disk 
    // bit 0-8=radius, bit9-17 angle1, bit 18-26 angle2, bit 27=1 filled  or 0 
    // <0 fill a polygon from previous turtle positions
    std::string s;
    void * widget;
    inline bool equal_except_nomark(const logo_turtle &t) const {
      return x==t.x && y==t.y  && turtle_width==t.turtle_width && s==t.s &&  radius==t.radius;
    }
#ifdef IPAQ
    logo_turtle(): x(70),y(70),theta(0),visible(true),mark(true),direct(true),color(0),turtle_width(1),radius(0),widget(0) {}
#else
    logo_turtle(): x(100),y(100),theta(0),visible(true),mark(true),direct(true),color(0),turtle_width(1),radius(0),widget(0) {}
#endif
  };
#endif // KHICAS
    
  // a structure that should contain all global variables
  class global {
  public:
    int _xcas_mode_;
    int _calc_mode_;
    int _decimal_digits_;
    int _minchar_for_quote_as_string_;
    int _scientific_format_;
    int _integer_format_;
    int _latex_format_;
#ifdef BCD
    u32 _bcd_decpoint_;
    u32 _bcd_mantissa_;
    u32 _bcd_flags_;
    bool _bcd_printdouble_;
#endif
    bool _expand_re_im_;
    bool _do_lnabs_;
    bool _eval_abs_;
    bool _eval_equaltosto_;
    bool _integer_mode_;
    bool _complex_mode_;
    bool _escape_real_;
    bool _complex_variables_;
    bool _increasing_power_;
    bool _approx_mode_;
    bool _variables_are_files_;
    bool _local_eval_;
    bool _withsqrt_;
    bool _show_point_; // show 3-d point 
    bool _io_graph_; // show 2-d point in io
    bool _all_trig_sol_;
    bool _ntl_on_;
    bool _lexer_close_parenthesis_;
    bool _rpn_mode_;
    bool _try_parse_i_;
    bool _specialtexprint_double_;
    bool _atan_tan_no_floor_;
    bool _keep_acosh_asinh_;
    bool _keep_algext_;
    bool _auto_assume_;
    bool _parse_e_;
    bool _convert_rootof_;
    int _python_compat_;
    int _angle_mode_;
    int _bounded_function_no_;
    int _series_flags_; // 1= full simplify, 2=1 for truncation, bit3=atan does not rewrite sin/cos to tan, bit4=no back conversion, bit5=write<<1,1>> with series_variable_name, bit 6=write O() instead of order_size, bit7= 1 diff in subst does not variable substitution
    int _step_infolevel_; 
    int _default_color_;
    double _epsilon_;
    double _proba_epsilon_; // if not 0, probabilistic algo may be used
    // the proba should be less than proba_epsilon for giac to return an answer
    int _show_axes_;
    int _spread_Row_,_spread_Col_;
    int _printcell_current_row_,_printcell_current_col_;
#ifdef NSPIRE
    nio::console * _logptr_;
#else
#if 1 // def WITH_MYOSTREAM
    my_ostream * _logptr_;
#else
    std::ostream * _logptr_;
#endif
#endif
    debug_struct * _debug_ptr;
    gen * _parsed_genptr_;
    parser_lexer _pl;
    int _prog_eval_level_val ;
    int _eval_level;
#if defined(FXCG) // defined(GIAC_HAS_STO_38) || defined(ConnectivityKit)
    unsigned int _rand_seed;
#else
    tinymt32_t _rand_seed;
#endif
    thread_param * _thread_param_ptr;
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_t * _mutexptr,* _mutex_eval_status_ptr ;
#endif
    int _language_;
    const char * _last_evaled_function_name_;
    const gen * _last_evaled_argptr_;
    int _max_sum_sqrt_;
    int _max_sum_add_;
    logo_turtle _turtle_;
    std::string _autoname_;
    std::string _format_double_;
    std::string _autosimplify_;
    std::string _lastprog_name_;
    const char * _currently_scanned_;
#ifndef KHICAS
    std::vector<logo_turtle> _turtle_stack_; 
#endif
    double _total_time_;
    void * _evaled_table_;
    void * _extra_ptr_;
    char _series_variable_name_;
    unsigned short _series_default_order_;
    global();  
    ~global();
    global & operator = (const global & g);
  };

  // Context type to be used for evaluation without global variables
  // tabptr is the current evaluation context, 
  // it is a global context if globalcontextptr=0, local otherwise
  // previous is the upper local or global evaluation context
  // globalcontextptr points to the current global evaluation context
  // If globalcontextptr=0 and previous=0, we are at the top folder
  // The top level local context should have previous=globalcontextptr
  class context {
  public:
    sym_tab * tabptr ;
    context * globalcontextptr ;
    context * previous ;
    global * globalptr; 
    const context * parent;
    vecteur * quoted_global_vars, * rootofs;
    vecteur * history_in_ptr, * history_out_ptr,*history_plot_ptr;
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

  context * clone_context(const context *);
  void init_context(context * ptr);
  void clear_context(context * ptr);

  extern const context * context0;
  std::vector<context *> & context_list();
#ifdef HAVE_LIBPTHREAD
  extern pthread_mutex_t context_list_mutex;
#endif
  
#if !defined(RTOS_THREADX) && !defined(BESTA_OS) && !defined(NSPIRE) && !defined(FXCG) && !defined KHICAS
  extern std::map<std::string,context *> * context_names ;
#endif

  const char * & last_evaled_function_name(GIAC_CONTEXT);
  const char * & currently_scanned(GIAC_CONTEXT);
  const gen * & last_evaled_argptr(GIAC_CONTEXT);

  bool make_thread(const giac::gen & g,int level,const giac_callback & f,void * f_param,const context * contextptr);

  std::string autoname(GIAC_CONTEXT);
  std::string autoname(const std::string & s,GIAC_CONTEXT);

  std::string autosimplify(GIAC_CONTEXT);
  std::string autosimplify(const std::string & s,GIAC_CONTEXT);

  bool csv_guess(const char * data,int count,char & sep,char & nl,char & decsep);
  std::string & format_double(GIAC_CONTEXT);

  int check_thread(context * contextptr);
  int check_threads(int i=0);

  void * & evaled_table(GIAC_CONTEXT);
  void * & extra_ptr(GIAC_CONTEXT);

  int & xcas_mode(GIAC_CONTEXT);
  void xcas_mode(int b,GIAC_CONTEXT);

  int & python_compat(GIAC_CONTEXT);
  void python_compat(int b,GIAC_CONTEXT);
  int array_start(GIAC_CONTEXT);
  extern bool python_color; // global variable for syntax highlighting
  extern bool os_shell; // true if Numworks called from shell

  int & calc_mode(GIAC_CONTEXT);
  int abs_calc_mode(GIAC_CONTEXT);
  void calc_mode(int b,GIAC_CONTEXT);

  int & scientific_format(GIAC_CONTEXT);
  void scientific_format(int b,GIAC_CONTEXT);

  int & decimal_digits(GIAC_CONTEXT);
  void decimal_digits(int b,GIAC_CONTEXT);

  int & minchar_for_quote_as_string(GIAC_CONTEXT);
  void minchar_for_quote_as_string(int b,GIAC_CONTEXT);

  int & integer_format(GIAC_CONTEXT);
  void integer_format(int b,GIAC_CONTEXT);
  int & latex_format(GIAC_CONTEXT);
#ifdef BCD
  u32 & bcd_decpoint(GIAC_CONTEXT);
  u32 & bcd_mantissa(GIAC_CONTEXT);
  u32 & bcd_flags(GIAC_CONTEXT);
  bool & bcd_printdouble(GIAC_CONTEXT);
#endif
  bool & expand_re_im(GIAC_CONTEXT);
  void expand_re_im(bool b,GIAC_CONTEXT);

  bool & integer_mode(GIAC_CONTEXT);
  void integer_mode(bool b,GIAC_CONTEXT);

  bool & complex_mode(GIAC_CONTEXT);
  void complex_mode(bool b,GIAC_CONTEXT);

  bool & escape_real(GIAC_CONTEXT); // default=true, if false sqrt(-1) errorsglo
  void escape_real(bool b,GIAC_CONTEXT);

  bool & try_parse_i(GIAC_CONTEXT);
  void try_parse_i(bool b,GIAC_CONTEXT);

  bool & specialtexprint_double(GIAC_CONTEXT);
  void specialtexprint_double(bool b,GIAC_CONTEXT);

  bool & atan_tan_no_floor(GIAC_CONTEXT);
  void atan_tan_no_floor(bool b,GIAC_CONTEXT);

  bool & keep_acosh_asinh(GIAC_CONTEXT);
  void keep_acosh_asinh(bool b,GIAC_CONTEXT);

  bool & keep_algext(GIAC_CONTEXT);
  void keep_algext(bool b,GIAC_CONTEXT);

  bool & auto_assume(GIAC_CONTEXT);
  void auto_assume(bool b,GIAC_CONTEXT);

  bool & parse_e(GIAC_CONTEXT);
  void parse_e(bool b,GIAC_CONTEXT);

  bool & convert_rootof(GIAC_CONTEXT);
  void convert_rootof(bool b,GIAC_CONTEXT);

  bool & do_lnabs(GIAC_CONTEXT);
  void do_lnabs(bool b,GIAC_CONTEXT);

  bool & eval_abs(GIAC_CONTEXT);
  void eval_abs(bool b,GIAC_CONTEXT);

  bool & eval_equaltosto(GIAC_CONTEXT);
  void eval_equaltosto(bool b,GIAC_CONTEXT);

  bool & complex_variables(GIAC_CONTEXT);
  void complex_variables(bool b,GIAC_CONTEXT);

  bool & increasing_power(GIAC_CONTEXT);
  void increasing_power(bool b,GIAC_CONTEXT);

  bool & approx_mode(GIAC_CONTEXT);
  void approx_mode(bool b,GIAC_CONTEXT);

  char & series_variable_name(GIAC_CONTEXT);
  void series_variable_name(char b,GIAC_CONTEXT);

  unsigned short & series_default_order(GIAC_CONTEXT);
  void series_default_order(unsigned short b,GIAC_CONTEXT);

  vecteur & history_in(GIAC_CONTEXT);
  vecteur & history_out(GIAC_CONTEXT);
  vecteur & history_plot(GIAC_CONTEXT);

  // True if we factor 2nd order polynomials using sqrt
  bool & withsqrt(GIAC_CONTEXT);
  void withsqrt(bool b,GIAC_CONTEXT);

  bool & all_trig_sol(GIAC_CONTEXT);
  void all_trig_sol(bool b,GIAC_CONTEXT);

  bool & ntl_on(GIAC_CONTEXT);
  void ntl_on(bool b,GIAC_CONTEXT);

  bool & lexer_close_parenthesis(GIAC_CONTEXT);
  void lexer_close_parenthesis(bool b,GIAC_CONTEXT);

  bool & rpn_mode(GIAC_CONTEXT);
  void rpn_mode(bool b,GIAC_CONTEXT);

  std::string lastprog_name(GIAC_CONTEXT);
  std::string lastprog_name(const std::string & b,GIAC_CONTEXT);

#ifdef KHICAS
  logo_turtle & turtle();
  std::vector<logo_turtle> & turtle_stack();
#else
  logo_turtle & turtle(GIAC_CONTEXT);
  std::vector<logo_turtle> & turtle_stack(GIAC_CONTEXT);
#endif

  int & angle_mode(GIAC_CONTEXT);
  int get_mode_set_radian(GIAC_CONTEXT);
  void angle_mode(int m,GIAC_CONTEXT);
  bool angle_radian(GIAC_CONTEXT);
  void angle_radian(bool b,GIAC_CONTEXT);
  bool angle_degree(GIAC_CONTEXT);

  bool & show_point(GIAC_CONTEXT);
  void show_point(bool b,GIAC_CONTEXT);

  int & show_axes(GIAC_CONTEXT);
  void show_axes(int b,GIAC_CONTEXT);

  bool & io_graph(GIAC_CONTEXT);
  void io_graph(bool b,GIAC_CONTEXT);

  bool & variables_are_files(GIAC_CONTEXT);
  void variables_are_files(bool b,GIAC_CONTEXT);

  int & bounded_function_no(GIAC_CONTEXT);
  void bounded_function_no(int b,GIAC_CONTEXT);

  int & series_flags(GIAC_CONTEXT);
  void series_flags(int b,GIAC_CONTEXT);

  int & step_infolevel(GIAC_CONTEXT);
  void step_infolevel(int b,GIAC_CONTEXT);

  bool & local_eval(GIAC_CONTEXT);
  void local_eval(bool b,GIAC_CONTEXT);

  int & default_color(GIAC_CONTEXT);
  void default_color(int c,GIAC_CONTEXT);

  int & spread_Row(GIAC_CONTEXT);
  void spread_Row(int c,GIAC_CONTEXT);

  int & spread_Col(GIAC_CONTEXT);
  void spread_Col(int c,GIAC_CONTEXT);

  int & printcell_current_row(GIAC_CONTEXT);
  void printcell_current_row(int c,GIAC_CONTEXT);

  int & printcell_current_col(GIAC_CONTEXT);
  void printcell_current_col(int c,GIAC_CONTEXT);

  double & total_time(GIAC_CONTEXT);
  double & epsilon(GIAC_CONTEXT);
  void epsilon(double c,GIAC_CONTEXT);
  double & proba_epsilon(GIAC_CONTEXT);
  extern double min_proba_time; // in seconds, minimal time for proba early termination

#ifdef NSPIRE
  nio::console * logptr(GIAC_CONTEXT);
  void logptr(nio::console *,GIAC_CONTEXT);
#else
  my_ostream * logptr(GIAC_CONTEXT);
  void logptr(my_ostream *,GIAC_CONTEXT);
#endif

  int & eval_level(GIAC_CONTEXT);
  // void eval_level(int b,GIAC_CONTEXT);
  thread_param * thread_param_ptr(const context * contextptr);

#if defined(FXCG) // defined(GIAC_HAS_STO_38) || defined(ConnectivityKit)
  unsigned int & rand_seed(GIAC_CONTEXT);
#else
  tinymt32_t * rand_seed(GIAC_CONTEXT);
#endif
  void rand_seed(unsigned int b,GIAC_CONTEXT);
  int giac_rand(GIAC_CONTEXT);
  int std_rand(); // a congruential random generator without context

  int & prog_eval_level_val(GIAC_CONTEXT);
  void prog_eval_level_val(int b,GIAC_CONTEXT);

  int & max_sum_sqrt(GIAC_CONTEXT);
  void max_sum_sqrt(int b,GIAC_CONTEXT);

  int & max_sum_add(GIAC_CONTEXT);

  int & language(GIAC_CONTEXT);
  void language(int b,GIAC_CONTEXT);

  int & lexer_line_number(GIAC_CONTEXT);
  int & lexer_column_number(GIAC_CONTEXT);
  void lexer_line_number(int b,GIAC_CONTEXT);
  void increment_lexer_line_number(GIAC_CONTEXT);

  std::string parser_error(GIAC_CONTEXT);
  void parser_error(const std::string & s,GIAC_CONTEXT);

  int & index_status(GIAC_CONTEXT);
  void index_status(int b,GIAC_CONTEXT);

  int & i_sqrt_minus1(GIAC_CONTEXT);
  void i_sqrt_minus1(int b,GIAC_CONTEXT);

  int & opened_quote(GIAC_CONTEXT);
  void opened_quote(int b,GIAC_CONTEXT);

  int & in_rpn(GIAC_CONTEXT);
  void in_rpn(int b,GIAC_CONTEXT);

  int & spread_formula(GIAC_CONTEXT);
  void spread_formula(int b,GIAC_CONTEXT);

  int & initialisation_done(GIAC_CONTEXT);
  void initialisation_done(int b,GIAC_CONTEXT);

  std::string comment_s(GIAC_CONTEXT);
  void comment_s(const std::string & s,GIAC_CONTEXT);
  void increment_comment_s(const std::string & s,GIAC_CONTEXT);
  void increment_comment_s(char ch,GIAC_CONTEXT);

  std::string parser_filename(GIAC_CONTEXT);
  void parser_filename(const std::string & s,GIAC_CONTEXT);

  int & first_error_line(GIAC_CONTEXT);
  void first_error_line(int s,GIAC_CONTEXT);

  std::string error_token_name(GIAC_CONTEXT);
  void error_token_name(const std::string & s,GIAC_CONTEXT);

  gen parsed_gen(GIAC_CONTEXT);
  void parsed_gen(const gen & g,GIAC_CONTEXT);

  debug_struct * debug_ptr(GIAC_CONTEXT);

  // gen_op is the type of all functions taking 1 or more gen args 
  // and returning 1 arg of type gen
  // gen argument has atomic type for an unary op, 
  // and vecteur type (compttr) otherwise
  typedef gen ( * gen_op ) (const gen & arg);
  typedef gen ( * gen_op_context ) (const gen & arg,const context * context_ptr);

  extern pid_t child_id;
  extern pid_t parent_id;
#ifdef HAVE_SIGNAL_H_OLD
  // all this stuff is obsolete, will be removed soon
  void kill_and_wait_sigusr2(); // called by child for interm. data
  // subprocess evaluation
  // return true if entree has been sent to evalation by child process
  bool child_eval(const std::string & entree,bool numeric,bool is_run_file);
  bool child_reeval(int history_begin_level);
  bool update_data(gen & entree,gen & sortie,GIAC_CONTEXT);
  void updatePICT(const vecteur & args);
  bool read_data(gen & entree,gen & sortie,std::string & message,GIAC_CONTEXT);

  gen wait_parent(); // wait a SIGUSR2 from parent process, return a gen

  pid_t make_child(); // forks and return child id
  extern bool running_file;
  extern volatile bool child_busy;
  extern volatile bool data_ready;
  extern volatile bool signal_plot_child;
  extern volatile bool signal_plot_parent; 
  extern int run_modif_pos;
  // end of obsolete section 
#endif

  void read_config(const std::string & name,GIAC_CONTEXT,bool verbose=true);
  void protected_read_config(GIAC_CONTEXT,bool verbose=true);
  vecteur remove_multiples(vecteur & v); // sort v and return list without multiple occurrences
  int equalposcomp(const std::vector<int> v,int i);
  int equalposcomp(const std::vector<short int> v,int i);
  int equalposcomp(int tab[],int f);
  // replace c1 by c2 in s
  std::string replace(const std::string & s,char c1,char c2);
  // attempt to convert Python-like programming structures to Xcas
  std::string python2xcas(const std::string & s_orig,GIAC_CONTEXT);
  std::string find_doc_prefix(int i);
  std::string find_lang_prefix(int i);
  int string2lang(const std::string & s); // convert "fr" to 1, "es" to 3 etc.
  void update_completions();
  void add_language(int i,GIAC_CONTEXT);
  void remove_language(int i,GIAC_CONTEXT);
  std::string set_language(int i,GIAC_CONTEXT);
  std::string read_env(GIAC_CONTEXT,bool verbose=true); // return doc prefix
  std::string home_directory();
  std::string cas_entree_name();
  std::string cas_sortie_name();
  std::string cas_setup_string(GIAC_CONTEXT);
  std::string geo_setup_string();
  std::string giac_aide_dir(); // PATH to the directory of aide_cas
  bool is_file_available(const char * ch);
  bool file_not_available(const char * ch);
  bool check_file_path(const std::string & s); // true if file is in path
  std::string browser_command(const std::string & orig_file);
  bool system_browser_command(const std::string & file);
  // convert doc name to an absolute path name
  std::string absolute_path(const std::string & orig_file);
  std::string & xcasrc();
  std::string & xcasroot();
  std::string add_extension(const std::string & s,const std::string & ext,const std::string & def);
  std::string remove_filename(const std::string & s);
  bool my_isnan(double d);
  bool my_isinf(double d);

  /* launch a new thread for evaluation only,
     no more readqueue, readqueue is done by the "parent" thread
     Ctrl-C will kill the "child" thread
     wait_001 is a function that should wait 0.001 s and update thinks
     for example it could remove idle callback of a GUI
     then call the wait function of the GUI and readd callbacks
  */
  giac::gen thread_eval(const giac::gen & g,int level,giac::context * contextptr,void (* wait_001)(giac::context *));
#ifdef HAVE_LIBPTHREAD
  // pointer to the context mutex so that thread_eval can be locked
  // Check this in wait_001 function if you don't want the main thread to
  // be blocked by a call to thread_eval inside wait_001
  pthread_mutex_t * mutexptr(GIAC_CONTEXT);
  extern pthread_mutex_t interactive_mutex,turtle_mutex;
  
#endif
  // Check if a thread_eval is active
  bool is_context_busy(GIAC_CONTEXT);
  // Check and set the kill thread flag
  bool kill_thread(GIAC_CONTEXT);
  void kill_thread(bool b,GIAC_CONTEXT);
  // Thread eval status = 0 finished, =1 eval, =2 debug_wait_main
  int thread_eval_status(GIAC_CONTEXT);
  void thread_eval_status(int c,GIAC_CONTEXT);

  void clear_prog_status(GIAC_CONTEXT);
  void cleanup_context(GIAC_CONTEXT);

  // count how many bytes are required to save g in a file
  unsigned archive_count(const gen & g,GIAC_CONTEXT);
  // save g in a opened file
  bool archive_save(void * f,const gen & g,size_t writefunc(void const* p, size_t nbBytes,size_t NbElements, void *file),GIAC_CONTEXT, bool noRecurse=false);

  bool archive_save(void * f,const gen & g,GIAC_CONTEXT);
  // restore a gen from an opened file
  gen archive_restore(void * f,size_t readfunc(void * p, size_t nbBytes,size_t NbElements, void *file),GIAC_CONTEXT);
  gen archive_restore(FILE * f,GIAC_CONTEXT);
  void init_geogebra(bool on,GIAC_CONTEXT);
  vecteur giac_current_status(bool save_history,GIAC_CONTEXT);
  bool unarchive_session(const gen & g,int level,const gen & replace,GIAC_CONTEXT,bool with_history=true);

  gen add_autosimplify(const gen & g,GIAC_CONTEXT);

  extern void (*my_gprintf)(unsigned special,const std::string & format,const vecteur & v,GIAC_CONTEXT);
  void gprintf(const std::string & format,const vecteur & v,GIAC_CONTEXT);
  void gprintf(const std::string & format,const vecteur & v,int step_info,GIAC_CONTEXT);
  void gprintf(unsigned special,const std::string & format,const vecteur & v,GIAC_CONTEXT);
  void gprintf(unsigned special,const std::string & format,const vecteur & v,int step_info,GIAC_CONTEXT);
  gen make_symbolic(const gen & op,const gen & args);
  // optional, call it just before exiting
  int release_globals();
  
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC


#endif // _GIAC_GLOBAL_H
