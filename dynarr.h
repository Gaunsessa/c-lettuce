#ifndef DYNARR_H
#define DYNARR_H

/* -- dynarr.h --
   Type-safe header only dynamic array library.

   Functions:
      - dynarr_t(type) -> dynarr_t : Used as type for the dynamic array.

      - dyi(arr) -> array : Used to acsess the underline array for indexing.

      - dy_init(type, ...) -> dynarr_t : Creates dynamic array from type. Pushes extra args to array.

      - dy_push(arr, v) -> void : Pushes value to array.

      - dy_push_unsafe(arr, v) -> void : Pushes value to array without type checking.

      - dy_pop(arr) -> elem : Pops element from array.

      - dy_remove(arr, i) -> elem : Removes element from array.

      - dy_insert(arr, i, v) -> void : Inserts element to array at index.

      - dy_len(arr) -> size_t : Returns array length.

      - dy_toarr(arr) -> array : Frees dynamic array and returns normal array.

      - dy_free(arr) -> void : Frees dyamic array.

   Examples:
      - Basic:
         
   -------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define dynarr_t(type) type**                                                                                  \

#define dyi(arr) (*(arr))                                                                                      \

#define dy_init(type, ...)                                                                                     \
   ({                                                                                                          \
      dynarr_t(type) arr = (void *)_dynarr_init(sizeof(type));                                                 \
      type data[] = { __VA_ARGS__ };                                                                           \
      for (int i = 0; i < sizeof(data) / sizeof(type); i++)                                                    \
         dy_push_unsafe(arr, data[i]);                                                                         \
      (void *)arr;                                                                                             \
   })                                                                                                          \

#define dy_push(arr, v)                                                                                        \
   ({                                                                                                          \
      dyi(arr)[_dy_info((arr))->index++] = (v);                                                                \
      _dynarr_grow((void **)(arr));                                                                            \
   })                                                                                                          \

#define dy_push_unsafe(arr, v)                                                                                 \
   ({                                                                                                          \
      memcpy(dyi(arr) + _dy_info((arr))->index++, (__typeof__(v)[1]) { (v) }, sizeof(dyi(arr)[0]));            \
      _dynarr_grow((void **)(arr));                                                                            \
   })                                                                                                          \

#define dy_pop(arr)                                                                                            \
   ({                                                                                                          \
      __typeof__(dyi(arr)[0]) elm = dyi(arr)[--_dy_info((arr))->index];                                        \
      _dynarr_shrink((void **)(arr));                                                                          \
      elm;                                                                                                     \
   })                                                                                                          \

#define dy_remove(arr, i)                                                                                      \
   ({                                                                                                          \
      __typeof__(dyi(arr)[0]) elm = dyi(arr)[i];                                                               \
      memmove(dyi(arr) + (i), dyi(arr) + (i) + 1, (_dy_info((arr))->index - (i)) * _dy_info((arr))->elm_size); \
      _dy_info((arr))->index--;                                                                                \
      _dynarr_shrink((void **)(arr));                                                                          \
      elm;                                                                                                     \
   })                                                                                                          \

#define dy_insert(arr, i, v)                                                                                   \
   ({                                                                                                          \
      memmove(dyi(arr) + (i) + 1, dyi(arr) + (i), (_dy_info((arr))->index - (i)) * _dy_info((arr))->elm_size); \
      dyi(arr)[i] = (v);                                                                                       \
      _dy_info((arr))->index++;                                                                                \
      _dynarr_grow((void **)(arr));                                                                            \
   })                                                                                                          \

#define dy_len(arr) _dy_info((arr))->index                                                                     \

#define dy_toarr(arr)                                                                                          \
   ({                                                                                                          \
      uint8_t *narr = malloc(_dy_info((arr))->length * _dy_info((arr))->elm_size);                             \
      memcpy(narr, dyi((arr)), _dy_info((arr))->length * _dy_info((arr))->elm_size);                           \
      dy_free((arr));                                                                                          \
      (void *)narr;                                                                                            \
   })                                                                                                          \

#define dy_free(arr)                                                                                           \
   ({                                                                                                          \
      free(_dy_info((arr)));                                                                                   \
      free(arr);                                                                                               \
   })                                                                                                          \

#define _dy_info(arr) ((dynarr_info_t *)(*(arr)) - 1)                                                          \

typedef struct dynarr_info_t {
   size_t elm_size;
   size_t index;
   size_t length;
} dynarr_info_t;

static inline void **_dynarr_init(size_t size) {
   void **arr = malloc(sizeof(void *));

   *arr = (dynarr_info_t *)calloc(1, sizeof(dynarr_info_t) + size) + 1;

   _dy_info(arr)->elm_size = size;
   _dy_info(arr)->length   = 1;

   return arr;
}

static inline int _dynarr_resize(void **arr, int size) {
   if (arr == NULL) return -1;

   dynarr_info_t *info = _dy_info(arr);

   info->length = size;

   *arr = (dynarr_info_t *)realloc(info, sizeof(dynarr_info_t) + info->elm_size * info->length) + 1;

   return 0;
}

static inline int _dynarr_grow(void **arr) {
   if (arr == NULL) return -1;

   dynarr_info_t *info = _dy_info(arr);

   if (info->index == info->length)
      return _dynarr_resize(arr, info->length * 2);

   return 0;
}

static inline int _dynarr_shrink(void **arr) {
   if (arr == NULL) return -1;

   dynarr_info_t *info = _dy_info(arr);

   if (info->index <= info->length / 2 && info->length != 0)
      return _dynarr_resize(arr, info->length / 2 + 1);

   return 0;
}

#endif