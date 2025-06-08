#pragma once

#include "basics.h"
#include <ctype.h>

#include "basic_string.h"
#include "print_source.h"
#include "reader.h"
#include "token.h"

bool begins_word(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

bool part_of_word(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || (c == '_');
}

struct Token tokenize_word(struct Reader *r);
struct Token tokenize_int(struct Reader *r);
struct Token tokenize_operator(struct Reader *r);

struct Token next_token(struct Reader *r) {
  skip_whitespace(r);
  if (reader_done(r)) {
    struct Token ret = {
        .type = TK_EOF,
        .start = r->position,
        .length = 0,
    };
    return ret;
  }
  char c = reader_peek(r);
  if (begins_word(c)) {
    return tokenize_word(r);
  }
  if (isdigit(c)) {
    return tokenize_int(r);
  }
  if (is_visible_ascii(c)) {
    return tokenize_operator(r);
  }
  print_position(&r->source, r->position);
  print_error();
  printf("non-ascii character\n");
  print_line(&r->source, r->position.line);
  print_arrow(&r->source, r->position);
  struct Token ret = {
      .type = TK_ERROR,
      .start = r->position,
      .length = 0,
  };
  return ret;
}

struct Token tokenize_word(struct Reader *r) {
  assert(begins_word(reader_peek(r)));
  struct TextPosition start = r->position;
  ssize_t length = 0;
  while (part_of_word(reader_peek(r))) {
    length++;
    r->position.column++;
  }
  struct Chunk word =  {
    .data = source_get_ptr(&r->source, start),
    .length = length
  };
  enum TokenType type = TK_IDENT;
  for (int i = 0; i < NELEM(KEYWORD_MAPPING); i++) {
    if (chunk_str_cmp(word, KEYWORD_MAPPING[i].str) == 0) {
      type = KEYWORD_MAPPING[i].tk;
    }
  }
  struct Token ret = {
      .type = type,
      .start = start,
      .length = length,
  };
  return ret;
}

struct Token tokenize_int(struct Reader *r) {
  assert(isdigit(reader_peek(r)));
  struct TextPosition start = r->position;
  ssize_t length = 0;
  while (isdigit(reader_peek(r))) {
    length++;
    r->position.column++;
  }
  if (begins_word(reader_peek(r))) {
    print_position(&r->source, r->position);
    print_error();
    printf("non-numeric characters after number\n");
    print_line(&r->source, r->position.line);
    print_arrow(&r->source, r->position);
    struct Token ret = {
        .type = TK_ERROR,
        .start = r->position,
        .length = 0,
    };
    return ret;
  }

  struct Token ret = {
      .type = TK_INT,
      .start = start,
      .length = length,
  };
  return ret;
}

struct Token tokenize_operator(struct Reader *r) {
  for (int i = 0; i < NELEM(OPS_MAPPING); i++) {
    char *ref = OPS_MAPPING[i].str;
    ssize_t len = strlen(ref);
    if (memcmp(ref, reader_get_ptr(r), len) == 0) {
      struct Token ret = {
          .type = OPS_MAPPING[i].tk,
          .start = r->position,
          .length = len,
      };
      r->position.column += len;
      return ret;
    }
  }
  struct Token ret = {
      .type = reader_peek(r),
      .start = r->position,
      .length = 1,
  };
  r->position.column += 1;
  return ret;
}

struct TokenStream {
  struct Reader reader;
  struct Token current;
};

void token_stream_advance(struct TokenStream *s) {
  s->current = next_token(&s->reader);
  /*
  if (s->current.type != TK_EOF) {
    printf("advanced to ");
    print_position(&s->reader.source, s->current.start);
    print_chunk(token_to_chunk(&s->reader.source, &s->current));
    printf("\n");
  }
  */
}
