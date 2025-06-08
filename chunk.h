#pragma once

#include "basics.h"
#include "string.h"

struct Chunk{
  char *data;
  ssize_t length;
};

int chunk_str_cmp(struct Chunk c, char *str) {
  int i=0;
  for (; i<c.length; i++) {
    int diff = c.data[i] - str[i];
    if (diff != 0) {
      return diff;
    }
  }
  return 0 - str[i];
}

void print_chunk(struct Chunk c) {
  printf("%.*s", (int)c.length, c.data);
}

struct Chunk as_chunk(char *str) {
  ssize_t len = strlen(str);
  return (struct Chunk) {
    .data = str,
    .length = len,
  };
}
