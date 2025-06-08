#pragma once

#include "basics.h"
#include <ctype.h>

#include "source.h"

struct Reader {
  struct Source source;
  struct TextPosition position;
};

bool reader_done(struct Reader *r) {
  return r->position.line > r->source.num_lines;
}

char reader_peek(struct Reader *r) {
  assert(!reader_done(r));
  return *source_get_ptr(&r->source, r->position);
}

char *reader_get_ptr(struct Reader *r) {
  assert(!reader_done(r));
  return source_get_ptr(&r->source, r->position);
}

void skip_whitespace(struct Reader *r) {
  while (true) {
    if (reader_done(r)) {
      return;
    }
    char c = reader_peek(r);
    if (c == '#' || c == '\r' || c == '\n') {
      r->position.line++;
      r->position.column = 1;
    } else if (isspace(c)) {
      r->position.column++;
    } else {
      break;
    }
  }
}
