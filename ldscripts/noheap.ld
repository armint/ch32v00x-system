_Heap_Begin = _end_noinit;
_Heap_Limit = _Heap_Begin;
ASSERT(_Heap_Limit - _Heap_Begin == 0, "Heap must be disabled (size != 0)");