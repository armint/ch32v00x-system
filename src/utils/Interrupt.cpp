
#include "Interrupt.h"

#include "vectors_ch32v00X.h"

#include <cassert>
#include <cstddef>

extern uintptr_t __vectors_start;
extern uintptr_t _sram_vectors;
namespace Interrupt {

// ram_vectors
static Handler_t& getInterruptHandler(size_t index) {
   [[gnu::section(".ram_vectors.interrupt_vector_table")]] static Handler_t handlers[INTERRUPT_VECTOR_TABLE_SIZE];
   return handlers[index];
}
static inline void prepareVectorTable(void) {
   auto *p = reinterpret_cast<uint32_t *>(&_sram_vectors);
   uint32_t *end = p + INTERRUPT_VECTOR_TABLE_SIZE;
   uint32_t *from = reinterpret_cast<uint32_t *>(&__vectors_start);
   while(p < end) {
      *p++ = *from++;
   }
   CriticalSection ics;
   __DSB();
   SCB->VTOR = &_sram_vectors;
}

struct AdvancedHandler {
   AdvancedHandler_t handler;
   void *argument;
};

static inline AdvancedHandler &getAdvancedHandler(unsigned int index) {
   static AdvancedHandler _advancedHandlers[INTERRUPT_VECTOR_TABLE_SIZE - 16]; // We only don't support object handlers for system interrupts
   return _advancedHandlers[index];
}


inline void handlerWithArgument() {
   auto &advanced_handler = getAdvancedHandler(__get_IPSR() - 16);
   advanced_handler.handler(advanced_handler.argument);
}

void connectHandler(IRQn_Type IRQn, AdvancedHandler_t handler, void *arg) {
   assert(static_cast<int>(IRQn) >= 0);
   auto &advanced_handler = getAdvancedHandler(static_cast<int>(IRQn));
   advanced_handler.handler = handler;
   advanced_handler.argument = arg;
   connectHandler(IRQn, &handlerWithArgument);
}


void connectHandler(IRQn_Type IRQn, Handler_t handler) {
   auto *vTable = reinterpret_cast<Handler_t *>(&_sram_vectors);
   if(SCB->VTOR != _sram_vectors) {
      prepareVectorTable();
   }
   vTable[IRQn + 16] = handler;
   __DSB();
}

void releaseHandler(IRQn_Type IRQn) {
   auto *vTableOrig = reinterpret_cast<Handler_t *>(&__vectors_start);
   connectHandler(IRQn, vTableOrig[IRQn + 16]);
}

AdvancedHandler getObjectHandler(IRQn_Type irq) {
   int i = static_cast<int>(irq);
   if(i < 0 || i >= INTERRUPT_VECTOR_TABLE_SIZE) {
      return {};
   }
   return getAdvancedHandler(i);
}

Handler_t getHandler(IRQn_Type irq) {
   auto *handlers = reinterpret_cast<Handler_t *>(SCB->VTOR);
   int index = static_cast<int>(irq) + 16;
   return (index < 0 || index > INTERRUPT_VECTOR_TABLE_SIZE) ? nullptr : handlers[index];
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
   // Read the current PRIMASK, to be later restored
   _state = __get_PRIMASK();
   // Disable all interrupts
   __disable_irq();
}

CriticalSection::~CriticalSection() {
   // Restore PRIMASK to the saved value
   __set_PRIMASK(_state);
}
}; // namespace Interrupt
