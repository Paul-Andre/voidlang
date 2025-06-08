#pragma once

#include "basics.h"

#include "buffer.h"
#include "print_message.h"

// Both line and column start at 1
struct TextPosition {
  int line;
  int column;
};

struct Source {
  char *file_name;
  char *contents;
  ssize_t size;
  char **lines;
  ssize_t num_lines;
};

char *source_get_ptr(struct Source *s, struct TextPosition p) {
  assert(1 <= p.line && p.line <= s->num_lines);
  char *ptr = &s->lines[p.line - 1][p.column - 1];
  assert((uintptr_t)ptr >= (uintptr_t)s->contents &&
         (uintptr_t)ptr < (uintptr_t)s->contents + (uintptr_t)s->size);
  return ptr;
}

char **split_lines(char *data, ssize_t size, ssize_t *num_lines) {
  struct Buffer lines = {0};
  ssize_t ptr = 0;
  *num_lines = 0;

  while (ptr < size) {

    char *line_start = data + ptr;
    buffer_append(&lines, &line_start, sizeof(line_start));
    (*num_lines)++;

    while (ptr < size && data[ptr] != '\n' && data[ptr] != '\r') {
      ptr++;
    }
    if (data[ptr] == '\n') {
      ptr++;
      if (ptr < size && data[ptr] == '\r') {
        ptr++;
      }
      continue;
    }
    if (data[ptr] == '\r') {
      ptr++;
      if (ptr < size && data[ptr] == '\n') {
        ptr++;
      }
      continue;
    }
  }
  return (char **)lines.data;
}

bool WARN_UNUSED read_file(char *file_name, struct Source *ret) {
  FILE *file = fopen(file_name, "rb");
  if (file == NULL) {
    int en = errno;
    print_error();
    printf(STYLE_BOLD "%s:" RESET " %s\n", file_name, strerror(en));
    return false;
  }

  // TODO: every single io operation could fail.
  // Maybe I should use Zig after all
  fseek(file, 0L, SEEK_END);
  ssize_t size = ftell(file);

  char *data = malloc(size + 2);
  fseek(file, 0L, SEEK_SET);

  ssize_t ss = fread(data, 1, size, file);
  assert(ss == size);
  fclose(file);

  data[size] = '\n';
  data[size + 1] = '\0';

  ssize_t num_lines;
  char **lines = split_lines(data, size, &num_lines);

  *ret = (struct Source){
      .file_name = file_name,
      .contents = data,
      .size = size,
      .lines = lines,
      .num_lines = num_lines,
  };
  return true;
}
