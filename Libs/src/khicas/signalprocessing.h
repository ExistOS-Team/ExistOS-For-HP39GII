/*
 * signalprocessing.h
 *
 * (c) 2018 Luka Marohnić
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __SIGNALPROCESSING_H
#define __SIGNALPROCESSING_H
#include <giac/config.h>
#include <giac/gen.h>
#include <giac/unary.h>

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

gen _cross_correlation(const gen &g,GIAC_CONTEXT);
gen _auto_correlation(const gen &g,GIAC_CONTEXT);
gen _lowpass(const gen &g,GIAC_CONTEXT);
gen _highpass(const gen &g,GIAC_CONTEXT);
gen _convolution(const gen &g,GIAC_CONTEXT);
gen _threshold(const gen &g,GIAC_CONTEXT);
gen _bartlett_hann_window(const gen &g,GIAC_CONTEXT);
gen _blackman_harris_window(const gen &g,GIAC_CONTEXT);
gen _blackman_window(const gen &g,GIAC_CONTEXT);
gen _bohman_window(const gen &g,GIAC_CONTEXT);
gen _cosine_window(const gen &g,GIAC_CONTEXT);
gen _gaussian_window(const gen &g,GIAC_CONTEXT);
gen _hamming_window(const gen &g,GIAC_CONTEXT);
gen _hann_poisson_window(const gen &g,GIAC_CONTEXT);
gen _hann_window(const gen &g,GIAC_CONTEXT);
gen _parzen_window(const gen &g,GIAC_CONTEXT);
gen _poisson_window(const gen &g,GIAC_CONTEXT);
gen _riemann_window(const gen &g,GIAC_CONTEXT);
gen _triangle_window(const gen &g,GIAC_CONTEXT);
gen _tukey_window(const gen &g,GIAC_CONTEXT);
gen _welch_window(const gen &g,GIAC_CONTEXT);

extern const unary_function_ptr * const at_cross_correlation;
extern const unary_function_ptr * const at_auto_correlation;
extern const unary_function_ptr * const at_lowpass;
extern const unary_function_ptr * const at_highpass;
extern const unary_function_ptr * const at_convolution;
extern const unary_function_ptr * const at_threshold;
extern const unary_function_ptr * const at_bartlett_hann_window;
extern const unary_function_ptr * const at_blackman_harris_window;
extern const unary_function_ptr * const at_blackman_window;
extern const unary_function_ptr * const at_bohman_window;
extern const unary_function_ptr * const at_cosine_window;
extern const unary_function_ptr * const at_gaussian_window;
extern const unary_function_ptr * const at_hamming_window;
extern const unary_function_ptr * const at_hann_poisson_window;
extern const unary_function_ptr * const at_hann_window;
extern const unary_function_ptr * const at_parzen_window;
extern const unary_function_ptr * const at_poisson_window;
extern const unary_function_ptr * const at_riemann_window;
extern const unary_function_ptr * const at_triangle_window;
extern const unary_function_ptr * const at_tukey_window;
extern const unary_function_ptr * const at_welch_window;

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
#endif // __SIGNALPROCESSING_H
