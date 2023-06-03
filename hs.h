#ifndef HS_H
#define HS_H

/* -- hs.h --
   Header only Hash-Set library.

   Notes:
      - You cannot use untyped numbers, eg: hs_add(hs, 32). Instead do: hs_add(hs, (uint8_t)32).
      - Only the first 19 init values will be added.

   Functions:
      - hs_new(type, ...) -> hs_t   : Creates new Hash-Set with non-string inputs.
      - hs_new_s(type, ...) -> hs_t : Creates new Hash-Set with string inputs (type should be non-pointer, eg: hs_new_s(char)).

      - hs_add(hs, data) -> void   : Adds new non-string input.
      - hs_add_s(hs, data) -> void : Adds new string input.

      - hs_remove(hs, data) -> void   : Removes a non-string input.
      - hs_remove_s(hs, data) -> void : Removes a string input.

      - hs_exists(hs, data) -> bool   : Checks if non-string input has been added.
      - hs_exists_s(hs, data) -> bool : Checks if string input has been added.

      - hs_free(hs) -> void : Frees a given Hash-Set.

   Examples:
      - Baisc:
         hs_t *hs = hs_new(uint8_t);

         hs_add(hs, (uint8_t)32);
         hs_add(hs, (uint8_t)64);

         hs_remove(hs, 32);

         printf("%d\n", hs_exists(hs, (uint8_t)32));
         printf("%d\n", hs_exists(hs, (uint8_t)64));

         hs_free(hs);

      - String:
         hs_t *hs = hs_new_s(wchar_t);

         hs_add_s(hs, L"Foo");
         hs_add_s(hs, L"Bar");

         hs_remove_s(hs, L"Bar");

         printf("%d\n", hs_exists_s(hs, L"Foo"));
         printf("%d\n", hs_exists_s(hs, L"Bar"));

         hs_free(hs);

      - Init Data:
         hs_t *hs = hs_new(uint8_t, (uint8_t)32, (uint8_t)64);

         hs_remove(hs, (uint8_t)32);

         printf("%d\n", hs_exists(hs, (uint8_t)32));
         printf("%d\n", hs_exists(hs, (uint8_t)64));

         hs_free(hs);
   ---------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef HS_BUCKET_AMT
#define HS_BUCKET_AMT 32
#endif

#define _hs_valen(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _19
#define hs_valen(...) _hs_valen(0, ##__VA_ARGS__, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define hs_new(type, ...) (_hs_new(sizeof(type), false, (type[]) { __VA_ARGS__ }, hs_valen(__VA_ARGS__)))
#define hs_new_s(type, ...) (_hs_new(sizeof(type), true, (type *[]) { __VA_ARGS__ }, hs_valen(__VA_ARGS__)))

#define hs_add(hs, data) (_hs_add((hs), (__typeof__((data))[]) { data }))
#define hs_add_s(hs, data) (_hs_add((hs), (data)))

#define hs_remove(hs, data) (_hs_remove((hs), (__typeof__((data))[]) { data }))
#define hs_remove_s(hs, data) (_hs_remove((hs), (data)))

#define hs_exists(hs, data) (_hs_exists((hs), (__typeof__((data))[]) { data }))
#define hs_exists_s(hs, data) (_hs_exists((hs), data))

#define hs_free(hs) (_hs_free((hs)))

typedef struct hs_bucket_t {
   void *data;

   struct hs_bucket_t *next;
} hs_bucket_t;

typedef struct hs_t {
   size_t data_size;
   bool string;

   hs_bucket_t *buckets[HS_BUCKET_AMT];
} hs_t;

static inline size_t _hs_size(hs_t *hs, void *data) {
   if (!hs->string) return hs->data_size;

   uint8_t *d = data;
   uint8_t cmp[hs->data_size];

   memset(cmp, 0, hs->data_size);

   size_t i = 0;
   for (;; i += hs->data_size)
      if (!memcmp(d + i, cmp, hs->data_size))
         break;

   return i + hs->data_size;
}

static inline int _hs_hash(hs_t *hs, void *data, size_t size) {
   uint8_t *d = data;
   uint64_t h = 0xcbf29ce484222325;

   for (size_t i = 0; i < size; i++)
       h = (h * 0x100000001b3) ^ d[i];

   return h % HS_BUCKET_AMT;
}

static inline void _hs_add(hs_t *hs, void *data) {
   size_t size = _hs_size(hs, data);

   void *d = malloc(size);
   memcpy(d, data, size);

   int hash = _hs_hash(hs, data, size);

   hs_bucket_t *bucket = malloc(sizeof(hs_bucket_t));

   bucket->data = d;
   bucket->next = NULL;

   if (hs->buckets[hash] == NULL) {
      hs->buckets[hash] = bucket;
   } else {
      hs_bucket_t *prev = hs->buckets[hash];

      if (!memcmp(data, prev->data, size)) return;

      while (prev->next != NULL) {
         if (!memcmp(data, prev->data, size)) return;

         prev = prev->next;
      }

      prev->next = bucket;
   }
}

static inline void _hs_remove(hs_t *hs, void *data) {
   size_t size = _hs_size(hs, data);

   hs_bucket_t **bucket = hs->buckets + _hs_hash(hs, data, size);

   while (*bucket != NULL) {
      if (!memcmp(data, (*bucket)->data, size)) {
         free((*bucket)->data);

         hs_bucket_t *tmp = *bucket;

         *bucket = (*bucket)->next;

         free(tmp);

         return;
      }

      bucket = &(*bucket)->next;
   }
}

static inline bool _hs_exists(hs_t *hs, void *data) {
   size_t size = _hs_size(hs, data);

   hs_bucket_t *bucket = hs->buckets[_hs_hash(hs, data, size)];

   while (bucket != NULL) {
      if (_hs_size(hs, bucket->data) == size && 
          !memcmp(data, bucket->data, size)) return true;

      bucket = bucket->next;
   }

   return false;
}

static inline hs_t *_hs_new(size_t key_size, bool string, void *data, size_t amt) {
   hs_t *hs = calloc(1, sizeof(hs_t));

   hs->data_size = key_size;
   hs->string    = string;

   if (amt == 0) return hs;

   if (string) {
      void **d = data;

      for (size_t i = 0; i < amt; i++)
         _hs_add(hs, d[i]);
   } else {
      uint8_t *d = data;

      for (size_t i = 0; i < amt * key_size; i += key_size)
         _hs_add(hs, d + i);
   }

   return hs;
}

static inline void _hs_free_bucket(hs_bucket_t *bucket) {
   free(bucket->data);

   if (bucket->next != NULL)
      _hs_free_bucket(bucket->next);

   free(bucket);
}

static inline void _hs_free(hs_t *hs) {
   for (size_t i = 0; i < HS_BUCKET_AMT; i++)
      if (hs->buckets[i] != NULL)
         _hs_free_bucket(hs->buckets[i]);

   free(hs);
}

#endif