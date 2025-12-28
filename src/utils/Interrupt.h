#pragma once

#include <wch_device.h>

namespace Interrupt {

using Handler_t = void (*)();

using AdvancedHandler_t = void (*)(void *);

void connectHandler(IRQn_Type IRQn, Handler_t handler);

void connectHandler(IRQn_Type IRQn, AdvancedHandler_t handler, void *arg);

void releaseHandler(IRQn_Type IRQn);

AdvancedHandler_t getAdvancedHandler(IRQn_Type irq);

Handler_t getHandler(IRQn_Type irq);

// Helper to bind a class member function to an interrupt using
// AdvancedHandler_t
template <typename T, void (T::*Method)()> static void _memberThunk(void *arg) {
  (static_cast<T *>(arg)->*Method)();
}

template <typename T, void (T::*Method)()>
static void connectMemberHandler(IRQn_Type IRQn, T &instance) {
  connectHandler(IRQn, &_memberThunk<T, Method>,
                 static_cast<void *>(&instance));
}

class BlockingSection {
  int8_t irqn;

public:
  explicit BlockingSection(IRQn_Type IRQn);

  virtual ~BlockingSection();
};

class CriticalSection {
  uint32_t _state;

public:
  CriticalSection();
  ~CriticalSection();
};
} // namespace Interrupt
