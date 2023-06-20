#ifndef HT_H
#define HT_H

/* -- ht.h --
   Header only typesafe Hash-Table Library.


   ---------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef HT_BUCKET_AMT
#define HT_BUCKET_AMT 32
#endif

#define _ht_valen(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _19
#define ht_valen(...) _ht_valen(0, ##__VA_ARGS__, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define ht_t(key_type, value_type) struct { key_type k; value_type v; }*

#define ht_new(key_type, value_type, key_string, value_string, ...) (_ht_new(sizeof(key_type), sizeof(value_type), key_string, value_string, (struct __attribute__((__packed__)) { key_type k; value_type v; }[]) { __VA_ARGS__ }, ht_valen(__VA_ARGS__)))

#define ht_set(ht, key, value) (_ht_set((ht_t *)(ht), (__typeof__((ht)->k)[1]) { key }, (__typeof__((ht)->v)[1]) { value }))

#define ht_get(ht, key) (*(__typeof__((ht)->v) *)_ht_get((ht_t *)(ht), (__typeof__((ht)->k)[1]) { key }))
#define ht_getp(ht, key) ((__typeof__((ht)->v) *)_ht_get((ht_t *)(ht), (__typeof__((ht)->k)[1]) { key }))

#define ht_exists(ht, key) (ht_getp((ht), (key)) != NULL)

#define ht_remove(ht, key) (_ht_remove((ht_t *)(ht), (__typeof__((ht)->k)[1]) { key }))

#define ht_free(ht) (_ht_free((ht_t *)(ht)))

typedef struct ht_bucket_t {
   void *key;
   void *value;

   struct ht_bucket_t *next;
} ht_bucket_t;

typedef struct ht_t {
   size_t key_size;
   size_t value_size;

   bool key_string;
   bool value_string;

   ht_bucket_t *buckets[HT_BUCKET_AMT];
} ht_t;

static inline int _ht_hash(ht_t *ht, void *data, size_t size);
static inline size_t _ht_size(ht_t *ht, void *data, bool key);

static inline ht_bucket_t *_ht_bucket_new(ht_t *ht, void *key, void *value);
static inline void _ht_bucket_free(ht_bucket_t *bucket, bool rec);

static inline void *_ht_new(size_t key_size, size_t value_size, bool key_string, bool value_string, void *data, size_t amt);

static inline void _ht_set(ht_t *ht, void *key, void *value);
static inline void *_ht_get(ht_t *ht, void *key);
static inline void _ht_remove(ht_t *ht, void *key);

static inline void _ht_free(ht_t *ht);

static inline int _ht_hash(ht_t *ht, void *data, size_t size) {
   uint64_t h = 0xcbf29ce484222325;

   for (size_t i = 0; i < size; i++)
       h = (h * 0x100000001b3) ^ ((uint8_t *)data)[i];

   return h % HT_BUCKET_AMT;
}

static inline size_t _ht_size(ht_t *ht, void *data, bool key) {
   bool string = key ? ht->key_string : ht->value_string;
   size_t size = key ? ht->key_size : ht->value_size;

   if (!string) return size;

   uint8_t *cmp = calloc(1, size);

   size_t length = 0;
   while (memcmp(((uint8_t *)data) + length, cmp, size) != 0) length += size;

   return length + size;
}

static inline ht_bucket_t *_ht_bucket_new(ht_t *ht, void *key, void *value) {
   ht_bucket_t *bucket = calloc(1, sizeof(ht_bucket_t));

   size_t key_size   = _ht_size(ht, key, true);
   size_t value_size = _ht_size(ht, value, false);

   bucket->key   = malloc(key_size);
   bucket->value = malloc(value_size);

   memcpy(bucket->key, key, key_size);
   memcpy(bucket->value, value, value_size);

   return bucket;
}

static inline void _ht_bucket_free(ht_bucket_t *bucket, bool rec) {
   ht_bucket_t *next = bucket->next;

   free(bucket->key);
   free(bucket->value);

   free(bucket);

   if (rec) _ht_bucket_free(next, true);
}

static inline void *_ht_new(size_t key_size, size_t value_size, bool key_string, bool value_string, void *data, size_t amt) {
   ht_t *ht = calloc(1, sizeof(ht_t));

   ht->key_size   = key_size;
   ht->value_size = value_size;

   ht->key_string   = key_string;
   ht->value_string = value_string;

   for (int i = 0; i < amt; i++) {
      // TODO
   }

   return ht;
}

static inline void _ht_set(ht_t *ht, void *key, void *value) {
   key   = ht->key_string ? *(void **)key : key;
   value = ht->value_string ? *(void **)value : value;

   int hash        = _ht_hash(ht, key, _ht_size(ht, key, true));
   size_t key_size = _ht_size(ht, key, true);

   ht_bucket_t *bucket = _ht_bucket_new(ht, key, value);;

   if (ht->buckets[hash] == NULL) {
      ht->buckets[hash] = bucket;

      return;
   }

   ht_bucket_t **pos = ht->buckets + hash;
   while (*pos != NULL && !(_ht_size(ht, (*pos)->key, true) == key_size && !memcmp(key, (*pos)->key, key_size)))
      pos = &(*pos)->next;

   if (*pos != NULL) {
      bucket->next = (*pos)->next;

      _ht_bucket_free(*pos, false);
   }

   *pos = bucket;
}

static inline void *_ht_get(ht_t *ht, void *key) {
   key = ht->key_string ? *(void **)key : key;

   int hash        = _ht_hash(ht, key, _ht_size(ht, key, true));
   size_t key_size = _ht_size(ht, key, true);

   ht_bucket_t *bucket = ht->buckets[hash];
   while (bucket != NULL && !(_ht_size(ht, bucket->key, true) == key_size && !memcmp(key, bucket->key, key_size)))
      bucket = bucket->next;

   return bucket == NULL ? NULL : ht->value_string ? &bucket->value : bucket->value;
}

static inline void _ht_remove(ht_t *ht, void *key) {
   key = ht->key_string ? *(void **)key : key;

   int hash        = _ht_hash(ht, key, _ht_size(ht, key, true));
   size_t key_size = _ht_size(ht, key, true);

   ht_bucket_t *pbucket = NULL; 
   ht_bucket_t *bucket  = ht->buckets[hash];
   while (bucket != NULL && !(_ht_size(ht, bucket->key, true) == key_size && !memcmp(key, bucket->key, key_size))) {
      pbucket = bucket;
      bucket  = bucket->next;
   }

   if (bucket == NULL) return;

   if (pbucket == NULL) ht->buckets[hash] = bucket->next;
   else pbucket->next = bucket->next;

   _ht_bucket_free(bucket, false);
}

static inline void _ht_free(ht_t *ht) {
   for (int i = 0; i < HT_BUCKET_AMT; i++)
      _ht_bucket_free(ht->buckets[i], true);

   free(ht);
}

#endif
