#ifndef __LPSOLVE_H
#define __LPSOLVE_H
#include "config.h"
#include "gen.h"
#include "unary.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

#define LP_AFFINE_SCALING_BETA 0.997

gen _lpsolve(const gen & args,GIAC_CONTEXT);

extern const unary_function_ptr * const  at_lpsolve;

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
#endif // __LPSOLVE_H
