
#include "Interrupt.h"

#include "vectors_ch32v00X.h"

#include <cassert>
#include <cstddef>

extern uintptr_t __vectors_start;
namespace Interrupt {

// ram_vectors
static Handler_t &getInterruptHandler(size_t index) {
   [[gnu::section(".ram_vectors.interrupt_vector_table")]] static Handler_t handlers[INTERRUPT_VECTOR_TABLE_SIZE];
   return handlers[index];
}

static inline void prepareVectorTable() {
   auto *p = reinterpret_cast<uint32_t *>(&getInterruptHandler(1));
   uint32_t *end = p + INTERRUPT_VECTOR_TABLE_SIZE - 1;
   uint32_t *from = reinterpret_cast<uint32_t *>(&__vectors_start);
   while(p < end) {
      *p++ = *from++;
   }
   getInterruptHandler(0) = nullptr;
   CriticalSection ics;
   __asm volatile("fence.i" ::: "memory");
   __set_MTVEC(reinterpret_cast<uintptr_t>(&getInterruptHandler(0)) | 0b11);
}

struct AdvancedHandler {
   AdvancedHandler_t handler;
   void *argument;
};

static inline AdvancedHandler &getAdvancedHandler(unsigned int index) {
   static AdvancedHandler _advancedHandlers[INTERRUPT_VECTOR_TABLE_SIZE - 16]; // We don't support advanced handlers for
                                                                               // system interrupts
   return _advancedHandlers[index];
}

[[gnu::interrupt]] void handlerWithArgument() {
   auto &advanced_handler = getAdvancedHandler((__get_MCAUSE() & 0x3FF) - 16);
   advanced_handler.handler(advanced_handler.argument);
}

void connectHandler(IRQn_Type IRQn, AdvancedHandler_t handler, void *arg) {
   assert(static_cast<int>(IRQn) >= 16);
   auto &advanced_handler = getAdvancedHandler(static_cast<int>(IRQn) - 16);
   advanced_handler.handler = handler;
   advanced_handler.argument = arg;
   connectHandler(IRQn, &handlerWithArgument);
}

void connectHandler(IRQn_Type IRQn, Handler_t handler) {
   // auto *vTable = reinterpret_cast<Handler_t *>(&_sram_vectors);
   if((__get_MTVEC() & ~0b11) != reinterpret_cast<uintptr_t>(&getInterruptHandler(0))) {
      prepareVectorTable();
   }
   getInterruptHandler(IRQn) = handler;
   __asm volatile("fence.i" ::: "memory");
}

void releaseHandler(IRQn_Type IRQn) {
   auto *vTableOrig = reinterpret_cast<Handler_t *>(&__vectors_start);
   connectHandler(IRQn, vTableOrig[static_cast<int>(IRQn) - 1]);
}

AdvancedHandler getObjectHandler(IRQn_Type irq) {
   int i = static_cast<int>(irq);
   if(i < 16 || i >= INTERRUPT_VECTOR_TABLE_SIZE) {
      return {};
   }
   return getAdvancedHandler(i - 16);
}

Handler_t getHandler(IRQn_Type irq) {
   auto *handlers = reinterpret_cast<Handler_t *>(__get_MTVEC() & ~0b11);
   int index = static_cast<int>(irq);
   return (index < 0 || index >= INTERRUPT_VECTOR_TABLE_SIZE) ? nullptr : handlers[index];
}

BlockingSection::BlockingSection(IRQn_Type IRQn) {
   if(NVIC_GetStatusIRQ(IRQn)) {
      irqn = static_cast<int8_t>(IRQn);
      NVIC_DisableIRQ(IRQn);
   } else {
      irqn = INT8_MIN;
   }
}

BlockingSection::~BlockingSection() {
   if(irqn != INT8_MIN) {
      NVIC_EnableIRQ(static_cast<IRQn_Type>(irqn));
   }
}

CriticalSection::CriticalSection() {
   // Read the current mstatus, to be later restored
   _state = __get_MSTATUS();
   // Disable all interrupts
   __disable_irq();
}

CriticalSection::~CriticalSection() {
   // Restore mstatus to the saved value
   __set_MSTATUS(_state);
}
}; // namespace Interrupt
