#pragma once

// Host stand-in for utils/Interrupt.h: the host tests run without
// interrupts, so the critical section does nothing.

namespace Interrupt {
class CriticalSection {
   public:
   CriticalSection() {
   }
   ~CriticalSection() {
   }
};
} // namespace Interrupt
