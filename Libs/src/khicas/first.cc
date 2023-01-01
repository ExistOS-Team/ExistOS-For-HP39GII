/* -*- compile-command: "g++ -g -c -I.. first.cc -DHAVE_CONFIG_H -DIN_GIAC  -DGIAC_CHECK_NEW" -*-
 *  Copyright (C) 2000,14 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "first.h"
int init_gmp_memory::refcount = 0;
init_gmp_memory init_gmp_memory_instance;

#ifdef HAVE_LIBGC
#include <new>
#define GC_DEBUG
#include <gc_cpp.h>
void* operator new(std::size_t size)
{
	return GC_MALLOC_UNCOLLECTABLE( size );
}
  
void operator delete(void* obj)
{
	GC_FREE(obj);
}
  
void* operator new[](std::size_t size)
{
	return GC_MALLOC_UNCOLLECTABLE(size);
}
  
void operator delete[](void* obj)
{
	GC_FREE(obj);
}

static void* RS_gmpalloc(size_t a)
{
	return GC_malloc_atomic(a);
}

static void* RS_gmprealloc(void* old_p, size_t old_size, size_t new_size)
{
	void* tmp = GC_realloc(old_p, new_size);
	return tmp;
}

static void RS_gmpfree(void * old_p,size_t old_size)
{

}

init_gmp_memory::init_gmp_memory()
{
	if (refcount++ == 0)
		mp_set_memory_functions(RS_gmpalloc, RS_gmprealloc, RS_gmpfree);
}

init_gmp_memory::~init_gmp_memory()
{
	if (--refcount == 0) {
		// XXX: do I need to clean up something here?
	}
}

#else
init_gmp_memory::init_gmp_memory() { }
init_gmp_memory::~init_gmp_memory() { }

#ifdef NSPIRE
#include <os.h> 
#else
#include <new>
#include <cstdlib>
#include <stdexcept>
#endif


#ifdef GIAC_CHECK_NEW 

#include <iostream>

size_t giac_allocated = 0;
void* operator new(std::size_t size)
{
  std::cerr << giac_allocated << " + " << size << '\n';
  giac_allocated += size;
  void * p =  std::malloc(size);  
  if(!p) {
    std::bad_alloc ba;
    throw ba;
  }
  return p;
}
  
void* operator new[](std::size_t size)
{
  std::cerr << giac_allocated << " + [] " << size << '\n';
  giac_allocated += size;
  void * p =  std::malloc(size);  
  if(!p) {
    std::bad_alloc ba;
    throw ba;
  }
  return p;
}
  
void operator delete[](void* obj)
{
  free(obj);
}
#else

#if 0 // defined KHICAS && defined DEVICE
// #include <unistd.h>
extern const void * _stack_end;

namespace giac {
  extern volatile bool ctrl_c,interrupted;
}

void* operator new(std::size_t size){
  void * p =  std::malloc(size);
  if ((size_t) p > (size_t) _stack_end)
    giac::ctrl_c=giac::interrupted=true;
  return p;
}
  
void* operator new[](std::size_t size){
  // if ( (0x20038000-(size_t)sbrk(0))<2*size) giac::ctrl_c=giac::interrupted=true;
  void * p =  std::malloc(size);  
  if ((size_t) p > (size_t) _stack_end)
    giac::ctrl_c=giac::interrupted=true;
  return p;
}
  
void operator delete(void* obj){
  free(obj);
}
  
void operator delete[](void* obj){
  free(obj);
}
#endif // KHICAS
#endif // GIAC_CHECK_NEW

#endif

