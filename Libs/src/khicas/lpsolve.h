#ifndef __LPSOLVE_H
#define __LPSOLVE_H
//#define HAVE_LIBGLPK 1
//#define _GLIBCXX_USE_CXX11_ABI 0
#include "config.h"
#include "gen.h"
#include "unary.h"
#ifdef HAVE_LIBGLPK
#include <glpk.h>
#endif

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif //ndef NO_NAMESPACE_GIAC

#define LP_SCORE_FACTOR 0.1667
#define LP_MIN_AWAY 0.08
#define LP_MIN_PARALLELISM 0.86
#define LP_MAX_MAGNITUDE 1e6
#define LP_CONSTR_MAXSIZE 1e5

typedef std::vector<int> ints;

/*
//options below should be moved to dispatch.h
//8<-------------
enum lp_additional_options { // XCAS KEYWORD:         TYPE:
    _LP_MAX_CUTS,            // lp_maxcuts            option
    _LP_GAP_TOLERANCE,       // lp_gaptolerance       option
    _LP_NODESELECT,          // lp_nodeselect         option
    _LP_VARSELECT,           // lp_varselect          option
    _LP_FIRSTFRACTIONAL,     // lp_firstfractional    value
    _LP_LASTFRACTIONAL,      // lp_lastfractional     value
    _LP_MOSTFRACTIONAL,      // lp_mostfractional     value
    _LP_PSEUDOCOST,          // lp_pseudocost         value
    _LP_DEPTHFIRST,          // lp_depthfirst         value
    _LP_BREADTHFIRST,        // lp_breadthfirst       value
    _LP_BEST_PROJECTION,     // lp_bestprojection     value
    _LP_HYBRID,              // lp_hybrid             value
    _LP_ITERATION_LIMIT,     // lp_iterationlimit     option
    _LP_TIME_LIMIT,          // lp_timelimit          option
    _LP_VERBOSE              // lp_verbose            option
};
//8<-------------
*/

//the following enums should stay here as they're used only internally
enum lp_results {
    _LP_SOLVED,
    _LP_INFEASIBLE,
    _LP_UNBOUNDED,
    _LP_ERROR
};

enum lp_parsing_errors {
    _LP_ERR_SIZE = 1,
    _LP_ERR_TYPE = 2,
    _LP_ERR_DIM  = 3
};

enum lp_precision_types {
    _LP_EXACT,
    _LP_INEXACT,
    _LP_PROB_DEPENDENT
};

enum lp_relation_types {
    _LP_LEQ = -1,
    _LP_EQ  =  0,
    _LP_GEQ =  1
};

enum lp_variable_sign_types {
    _LP_VARSIGN_POS,
    _LP_VARSIGN_NEG,
    _LP_VARSIGN_POS_PART,
    _LP_VARSIGN_NEG_PART
};

struct lp_settings {
    //solver parameters
    int solver;
    int precision;
    bool presolve;
    bool maximize;
    int assumption;
    int iteration_limit;
    bool has_binary_vars;
    //branch&bound parameters
    int varselect;
    int nodeselect;
    double relative_gap_tolerance;
    int depth_limit;
    int node_limit;
    int time_limit; //in miliseconds
    int max_cuts;
    //message report parameters
    bool verbose;
    double status_report_freq;
    lp_settings();
};

struct lp_stats {
    int subproblems_examined;
    int cuts_applied;
    int max_active_nodes;
    double mip_gap;
    lp_stats();
};

struct lp_range {
    gen lbound;
    gen ubound;
    inline void tighten_lbound(const gen &l,GIAC_CONTEXT) { lbound=max(lbound,l,contextptr); }
    inline void tighten_ubound(const gen &u,GIAC_CONTEXT) { ubound=min(ubound,u,contextptr); }
    inline bool is_unrestricted_below() { return is_inf(lbound); }
    inline bool is_unrestricted_above() { return is_inf(ubound); }
    lp_range();
};

struct lp_variable {
    bool is_integral;
    int sign_type;
    lp_range range;
    std::string name;
    double pseudocost[2];
    int nbranch[2];
    lp_variable();
    void set_type(int t,GIAC_CONTEXT);
    void update_pseudocost(double delta,double fr,int dir);
    double score(double fr);
};

struct lp_constraints {
    matrice lhs;
    vecteur rhs;
    ints rv;
    std::vector<double> score;
    inline int nrows() { return lhs.size(); }
    inline int ncols() { return lhs.empty()?0:lhs.front()._VECTptr->size(); }
    void append(const vecteur &lh,const gen &rh,int relation_type);
    vecteur column(int index);
    void duplicate_column(int index);
    void negate_column(int index);
    void subtract_from_rhs_column(const vecteur &v);
    void set(int index,const vecteur &lh,const gen &rh,int relation_type);
    void get(int index,vecteur &lh,gen &rh,int &relation_type);
    void get_lr(int index,vecteur &lh,gen &rh);
    void div(int index,const gen &g,GIAC_CONTEXT);
    void subtract(int index,const vecteur &v,const gen &g);
    void remove(int index);
};

struct lp_node;

struct lp_problem {
    const context *ctx;
    std::pair<vecteur,gen> objective;
    double objective_norm;
    std::vector<double> obj_approx;
    std::vector<lp_variable> variables;
    vecteur variable_identifiers;
    lp_constraints constr;
    lp_constraints cuts;
    lp_settings settings;
    lp_stats stats;
    int nvars_initial;
    vecteur solution;
    gen optimum;
    lp_problem(GIAC_CONTEXT) {
        ctx=contextptr;
        settings=lp_settings();
    }
    inline int nc() { return constr.lhs.size(); }
    inline int nv() { return variables.size(); }
    void message(const char* msg,bool err=false);
    void report_status(const char* msg,int count);
    void add_identifiers_from(const gen &g);
    int get_variable_index(const identificateur &idnt);
    void set_objective(const vecteur &v,const gen &ft);
    void create_variables(int n);
    void make_problem_exact();
    void add_slack_variables();
    void tighten_variable_bounds(int i,const gen &l,const gen &u);
    void make_all_vars_bounded_below();
    bool has_integral_variables();
    bool has_approx_coefficients();
    bool lincomb_coeff(const gen &g,vecteur &varcoeffs,gen &freecoeff);
    int solve();
    vecteur output_solution();
    //GLPK routines
#ifdef HAVE_LIBGLPK
    glp_prob *glpk_initialize();
    int glpk_simplex(glp_prob *prob);
    int glpk_interior_point(glp_prob *prob);
    int glpk_branchcut(glp_prob *prob);
#endif
    int glpk_solve();
    bool glpk_load_from_file(const char *fname);
};

struct lp_node {
    lp_problem *prob;
    int depth;
    std::vector<lp_range> ranges;
    gen optimum;
    vecteur solution;
    double opt_approx;
    gen infeas;
    int most_fractional;
    std::map<int,double> fractional_vars;
    ints cut_indices;
    inline bool is_integer_feasible() { return is_zero(infeas); }
    bool is_var_fractional(int index);
    gen fracpart(const gen &g);
    lp_node create_child();
    int solve_relaxation();
};

gen _lpsolve(const gen &args,GIAC_CONTEXT);
extern const unary_function_ptr * const  at_lpsolve;

#ifndef NO_NAMESPACE_GIAC
} //namespace giac
#endif //ndef NO_NAMESPACE_GIAC
#endif //__LPSOLVE_H
