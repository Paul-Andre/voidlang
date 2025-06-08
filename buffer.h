#pragma once

#include <sys/types.h>

// A automatically growing buffer
// Can be used as an ArrayList
struct Buffer {
  char *data;
  ssize_t size;
  ssize_t capacity;
};

// TODO: error checking for failed alloc
void buffer_append(struct Buffer *b, void *new_data, ssize_t size) {
  if (b->data == NULL) {
    b->data = malloc(size);
    b->size = size;
    b->capacity = size;
    memcpy(b->data, new_data, size);
    return;
  }
  while (b->size + size > b->capacity) {
    b->data = realloc(b->data, b->capacity * 2);
    b->capacity *= 2;
  }
  memcpy(&b->data[b->size], new_data, size);
  b->size += size;
}

// TODO: error checking for failed alloc
struct Buffer buffer_with_capacity(ssize_t capacity) {
  struct Buffer b;
  b.data = malloc(capacity);
  b.size = 0;
  b.capacity = capacity;
  return b;
}

