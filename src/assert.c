#include <assert.h>
#include <wch_device.h>

void __attribute__((noreturn, weak)) __assert_func(const char *file, int line, const char *func, const char *failedexpr) {
   (void)file;
   (void)line;
   (void)func;
   (void)failedexpr;
   /*
      // Not atomic, but otherwise the entire string might get too long,
      // and temporary buffer used by trace_printf() will overflow.
   #if defined(TRACE)

      trace_printf("assertion \"%s\" failed\n", failedexpr);
      trace_printf("file: \"%s\"\n", file);
      trace_printf("line: %d\n", line);
      if(func != NULL) {
         trace_printf("function: %s\n", func);
      }
   #endif
   */
   //   abort();
   NVIC_SystemReset();
   while(1)
      ;
   /* NOTREACHED */
}
