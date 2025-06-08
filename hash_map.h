#pragma once

#include "basics.h"

#include "chunk.h"
#include "source.h"

typedef uint64_t hash_t;
typedef uint64_t Key;
typedef uint64_t Value;

/*
struct Bucket {
  hash_t hash;
  Key key;
  Value value;
};
*/

struct HashMap {
  hash_t *hashes;
  Key *keys;
  Value *values;
  ssize_t size;
  ssize_t capacity;
};

hash_t hash64(uint64_t x) {
  x += 1;
  x ^= x >> 32;
  x *= UINT64_C(0xd6e8feb86659fd93);
  x ^= x >> 32;
  x *= UINT64_C(0xd6e8feb86659fd93);
  x ^= x >> 32;
  // return x & 0xff;
  return x;
}

bool _hm_has_capacity(ssize_t size, ssize_t capacity) {
  // This will exploit the x86_64 lea trick
  // return size * 2 < capacity;
  return size * 9 < capacity * 8;
}

struct HashMap hm_init(void) {
  struct HashMap map = {0};
  return map;
}

struct HashMap hm_with_capacity(ssize_t cap) {
  struct HashMap map = {0};
  map.capacity = cap;
  map.hashes = calloc(cap, sizeof(hash_t));
  map.keys = calloc(cap, sizeof(Key));
  map.values = calloc(cap, sizeof(Value));
  return map;
}

void _hm_shift_backward(struct HashMap *map, uint64_t to_overwrite,
                        uint64_t last) {
  assert(to_overwrite != last);
  if (to_overwrite < last) {
    uint64_t distance = last - to_overwrite;
    memcpy(&map->hashes[to_overwrite], &map->hashes[to_overwrite + 1],
           distance * sizeof(hash_t));
    memcpy(&map->keys[to_overwrite], &map->keys[to_overwrite + 1],
           distance * sizeof(Key));
    memcpy(&map->values[to_overwrite], &map->values[to_overwrite + 1],
           distance * sizeof(Value));
  } else {
    uint64_t distance = map->capacity - last - 1;
    memcpy(&map->hashes[to_overwrite], &map->hashes[to_overwrite + 1],
           distance * sizeof(hash_t));
    memcpy(&map->keys[to_overwrite], &map->keys[to_overwrite + 1],
           distance * sizeof(Key));
    memcpy(&map->values[to_overwrite], &map->values[to_overwrite + 1],
           distance * sizeof(Value));

    map->hashes[map->capacity - 1] = map->hashes[0];
    map->keys[map->capacity - 1] = map->keys[0];
    map->values[map->capacity - 1] = map->values[0];

    memcpy(&map->hashes[0], &map->hashes[1], last * sizeof(hash_t));
    memcpy(&map->keys[0], &map->keys[1], last * sizeof(Key));
    memcpy(&map->values[0], &map->values[1], last * sizeof(Value));
  }
}

void _hm_shift_forward(struct HashMap *map, uint64_t first,
                       uint64_t to_overwrite) {

  assert(to_overwrite != first);
  if (to_overwrite > first) {
    uint64_t distance = to_overwrite - first;
    memcpy(&map->hashes[first + 1], &map->hashes[first],
           distance * sizeof(hash_t));
    memcpy(&map->keys[first + 1], &map->keys[first], distance * sizeof(Key));
    memcpy(&map->values[first + 1], &map->values[first],
           distance * sizeof(Value));
  } else {
    memcpy(&map->hashes[1], &map->hashes[0], to_overwrite * sizeof(hash_t));
    memcpy(&map->keys[1], &map->keys[0], to_overwrite * sizeof(Key));
    memcpy(&map->values[1], &map->values[0], to_overwrite * sizeof(Value));

    map->hashes[0] = map->hashes[map->capacity - 1];
    map->keys[0] = map->keys[map->capacity - 1];
    map->values[0] = map->values[map->capacity - 1];

    uint64_t distance = map->capacity - first - 1;
    memcpy(&map->hashes[first + 1], &map->hashes[first],
           distance * sizeof(hash_t));
    memcpy(&map->keys[first + 1], &map->keys[first], distance * sizeof(Key));
    memcpy(&map->values[first + 1], &map->values[first],
           distance * sizeof(Value));
  }
}

// Shifts until finds empty space
void _hm_shift_forward_until_empty(struct HashMap *map, uint64_t pos) {
  uint64_t ii = 0;
  bool found = false;
  uint64_t first_empty;
  for (; ii < map->capacity; ii++) {
    uint64_t i = (ii + pos) & (map->capacity - 1);
    if (map->hashes[i] == 0) {
      first_empty = i;
      found = true;
      break;
    }
  }
  assert(found);

  _hm_shift_forward(map, pos, first_empty);
}

enum HmResult {
  HM_ENTRY_EXISTS,
  HM_SUCCESS,
};

// if entry already exists, sets previous_value to that value and returns
// HM_ENTRY_EXISTS
enum HmResult _hm_insert_inner(struct HashMap *map, hash_t h, Key k, Value v,
                               Value **previous_value) {
  uint64_t h_trancated = h & (map->capacity - 1);

  uint64_t ii = 0;
  for (; ii < map->capacity; ii++) {
    uint64_t i = (ii + h_trancated) & (map->capacity - 1);
    if (map->hashes[i] == 0) { // is empty
      map->hashes[i] = h;
      map->keys[i] = k;
      map->values[i] = v;
      map->size++;
      return HM_SUCCESS;
    }
    if (map->hashes[i] == h) {
      // hash collision
      if (map->keys[i] == k) {
        if (previous_value != NULL) {
          *previous_value = &map->values[i];
        }
        return HM_ENTRY_EXISTS;
      }
      continue;
    }
    hash_t hh = map->hashes[i];
    uint64_t hht = hh & (map->capacity - 1);
    uint64_t hh_diff = (i - hht) & (map->capacity - 1);
    if (ii > hh_diff) {
      _hm_shift_forward_until_empty(map, i);
      map->hashes[i] = h;
      map->keys[i] = k;
      map->values[i] = v;
      map->size++;
      return HM_SUCCESS;
    }
  }
  assert(false && "Found no space to insert");
}

void _hm_increase_size(struct HashMap *map) {
  if (map->capacity == 0) {
    *map = hm_with_capacity(2);
    return;
  } else {
    struct HashMap newMap = hm_with_capacity(map->capacity * 2);
    for (int i = 0; i < map->capacity; i++) {
      if (map->hashes[i] != 0) {
        enum HmResult r = _hm_insert_inner(&newMap, map->hashes[i],
                                           map->keys[i], map->values[i], NULL);
        assert(r == HM_SUCCESS);
      }
    }
    *map = newMap;
  }
}

bool hm_find(struct HashMap *map, hash_t h, Key k, Value **previous_value) {
  if (h == 0) {
    h = (1ull << 63);
  }
  uint64_t h_trancated = h & (map->capacity - 1);
  uint64_t ii = 0;
  for (; ii < map->capacity; ii++) {
    uint64_t i = (ii + h_trancated) & (map->capacity - 1);

    if (map->hashes[i] == 0) { // is empty
      return false;
    }
    if (map->hashes[i] == h) {
      // hash collision
      if (map->keys[i] == k) {
        *previous_value = &map->values[i];
        return true;
      }
      continue;
    }
    hash_t hh = map->hashes[i];
    uint64_t hht = hh & (map->capacity - 1);
    uint64_t hh_diff = (i - hht) & (map->capacity - 1);
    if (ii > hh_diff) {
      return false;
    }
  }
  assert(false);
}

enum HmResult hm_insert(struct HashMap *map, hash_t h, Key k, Value v,
                        Value **previous_value) {
  if (!_hm_has_capacity(map->size, map->capacity)) {
    _hm_increase_size(map);
  }
  if (h == 0) {
    h = (1ull << 63);
  }
  return _hm_insert_inner(map, h, k, v, previous_value);
}

bool nm_remove(struct HashMap *map, hash_t h, Key k) { return true; }
