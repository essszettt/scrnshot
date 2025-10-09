// Need enough pages to cover main bank in logical->physical table
#pragma output DOTN_LAST_PAGE = 11

// limit the size of printf
// #pragma printf = "%s %c %d %ld %u %lu %X %lX"
#pragma printf = "%s %d %u"

// non-zero closes open FILEs on exit
#pragma output CRT_ENABLE_CLOSE = 1   

// room for one exit function
#pragma output CLIB_EXIT_STACK_SIZE = 1

// create heap of given site
#pragma output CLIB_MALLOC_HEAP_SIZE = 512
