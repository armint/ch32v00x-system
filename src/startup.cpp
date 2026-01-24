/*
 * Copyright (c) 2015 Liviu Ionescu.
 *
 * was taken from the µOS++ distribution.
 * (https://github.com/micro-os-plus)
 * Modified for use with CH32V00x without µOS++ by Armin van der Togt
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "startup-hooks.h"
#include "system_ch32v00X.h"
// ----------------------------------------------------------------------------
// This file defines the system code for a portable embedded
// C/C++ application, built with newlib.
//
// Control reaches here from the reset handler via jump or call.
//
// The actual steps performed by _start are:
// - copy the initialised data region(s)
// - clear the BSS region(s)
// - initialise the system
// - run the preinit/init array (for the C++ static constructors)
// - branch to main()
//
// The normal configuration is standalone, with all support
// functions implemented locally.
//
// For this to be called, the project linker must be configured without
// the system sequence (-nostartfiles).
// ----------------------------------------------------------------------------

// Begin address for the initialisation values of the .data section.
// defined in linker script
extern unsigned int _sidata;
// Begin address for the .data section; defined in linker script
extern unsigned int _sdata;
// End address for the .data section; defined in linker script
extern unsigned int _edata;

// Begin address for the .bss section; defined in linker script
extern unsigned int __bss_start__;
// End address for the .bss section; defined in linker script
extern unsigned int __bss_end__;

extern unsigned int _Heap_Begin;
extern unsigned long int _Heap_Limit;
extern unsigned long int __stack;

extern "C" int main();

// ----------------------------------------------------------------------------

// Forward declarations

extern "C" {

static void initialize_data(unsigned int *from, unsigned int *region_begin, unsigned int *region_end);

static void initialize_bss(unsigned int *region_begin, unsigned int *region_end);

static void run_init_array(void);

// Not static since it is called from exit()
void run_fini_array(void);
}

// ----------------------------------------------------------------------------

[[gnu::always_inline]] inline void initialize_data(unsigned int *from, unsigned int *region_begin, unsigned int *region_end) {
   // Iterate and copy word by word.
   // It is assumed that the pointers are word aligned.
   unsigned int *p = region_begin;
   while(p < region_end) {
      *p++ = *from++;
   }
}

[[gnu::always_inline]] inline void initialize_bss(unsigned int *region_begin, unsigned int *region_end) {
   // Iterate and clear word by word.
   // It is assumed that the pointers are word aligned.
   unsigned int *p = region_begin;
   while(p < region_end) {
      *p++ = 0;
   }
}

// These magic symbols are provided by the linker.
[[gnu::weak]] extern void (*__preinit_array_start[])(void);
[[gnu::weak]] extern void (*__preinit_array_end[])(void);
[[gnu::weak]] extern void (*__init_array_start[])(void);
[[gnu::weak]] extern void (*__init_array_end[])(void);
[[gnu::weak]] extern void (*__fini_array_start[])(void);
[[gnu::weak]] extern void (*__fini_array_end[])(void);

// Iterate over all the preinit/init routines (mainly static constructors).
[[gnu::always_inline]] inline void run_init_array(void) {
   //  trace_printf ("%s()\n", __func__);

   int count = __preinit_array_end - __preinit_array_start;
   for(int i = 0; i < count; i++) {
      __preinit_array_start[i]();
   }

   count = __init_array_end - __init_array_start;
   for(int i = 0; i < count; i++) {
      __init_array_start[i]();
   }
}

// Run all the cleanup routines (mainly static destructors).
void run_fini_array(void) {
   int count = __fini_array_end - __fini_array_start;
   for(int i = count; i > 0; i--) {
      __fini_array_start[i - 1]();
   }
}

/**
 * @details
 * This is the place where the CPU core will go immediately
 * after reset (the `Reset_Handler` calls this function).
 *
 * To reach this location, the reset stack must point to a valid
 * internal RAM area.
 *
 * Debugging new system configurations usually begins with placing
 * a breakpoint at `_start()`, and stepping through the routine.
 */

extern "C" [[gnu::section(".after_vectors"), gnu::noreturn, gnu::weak, gnu::used]] void _start(void) {
   cpuInit();

   // Initialise hardware right after reset, to switch clock to higher
   // frequency and have the rest of the initialisations run faster.

   startup_initialize_hardware_early();

   // Copy the DATA segment from Flash to RAM (inlined).
   initialize_data(&_sidata, &_sdata, &_edata);

   // Zero fill the BSS section (inlined).
   initialize_bss(&__bss_start__, &__bss_end__);

   // Hook to continue the initialisations. Usually compute and store the
   // clock frequency in the global CMSIS variable, cleared above.
   startup_initialize_hardware();

   // Call the standard library initialisation (mandatory for C++ to
   // execute the constructors for the static objects).
   run_init_array();

   // set mepc to be main as the root app.
   __set_MEPC((uint32_t)main);

   asm volatile("mret\n");
   while(true)
      ;
   /* NOTREACHED */
}
