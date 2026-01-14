#include <wch_device.h>

#include "delay.h"
#include <peripherals/systick.h>

// This number is processor specific. For the QingKe V2 core (CH32V00x) it is 6
// cycles per loop
constexpr uint32_t CyclesPerLoop = 6;
// This is the amount of cycles spent in the delayuS method before entering the wait loop. This is also processor specific.
constexpr uint32_t OneTimeCycles = 630;


void delaymS(uint32_t msec) {
   auto end = getSysTick() + msec;
   while(getSysTick() < end)
      ;
}

void delayuS(uint32_t usec) {
   uint32_t loops = SystemCoreClock / (CyclesPerLoop * 1000);
   loops *= usec;
   loops = loops / 1000;

   if(loops <= (OneTimeCycles / CyclesPerLoop) + 1) {
      return;
   }
   loops -= (OneTimeCycles / CyclesPerLoop); // cycle count for entry/exit and above calculations
   asm volatile("loop:"
                "SUB %[count], %[count], %[dec];"
                "BNEZ %[count], loop;"
                : [count] "+r"(loops)
                : [dec] "r"(1)
                :);
}
