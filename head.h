/* --------------
   C-LETTUCE HEAD

   This module allow the use of no headers and
   allows variable shadowing.

   Variable shadowing can be removed by defining "CLETTUCE_NO_SHADOW"

   Example:
   
   # test.c
   #ifndef TEST_C
   #define TEST_C

   #include <c-lettuce/head.h>

   #include <stdio.h>

   void test() $ ({
       printf("HELLO!");
   })

   # test2.c
   #ifndef TEST2_C
   #define TEST2_C

   #include <c-lettuce/head.h>

   #include "test.c"

   void test2() $({
       test();
   })

   #endif

   # main.c
   #include "test.c"
   #include "test2.c"

   int main(int argc, char **argv) {
       test();
       test2();
   }

   #endif
   
   -------------- */

#undef $

#ifndef LEVEL
#define LEVEL 1
#endif

#if __INCLUDE_LEVEL__ == LEVEL
#ifdef CLETTUCE_NO_SHADOW
#define $(...) __VA_ARGS__
#else
#define $(...) {__VA_ARGS__}
#endif
#else 
#define $(...) ; 
#endif

#undef LEVEL