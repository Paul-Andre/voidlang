#pragma once

#include "basics.h"

#include "chunk.h"
#include "source.h"

enum TokenType {
  TK_EOF = -1,
  TK_ERROR = -2,
  TK_NONE = 0,

  // 1-character tokens are represented using their ascii value

  TK_EQ = 128, // ==
  TK_NE,       // !=
  TK_LE,       // <=
  TK_GE,       // >=
  TK_ARROW,    // ->

  TK_INT,
  TK_FLOAT,

  TK_IDENT,

  TK_FN,
  TK_AND,
  TK_OR,
  TK_NOT,
  TK_RETURN,
  TK_WHILE,
  TK_IF,
  TK_ELSE,
  TK_GOTO,
  TK_BRANCH,
  TK_BREAK,
  TK_CONTINUE,
  TK_LET,
};

struct Token {
  enum TokenType type;
  struct TextPosition start;
  ssize_t length;
};

struct StrTk {
  char *str;
  enum TokenType tk;
};

struct StrTk OPS_MAPPING[] = {
    {"==", TK_EQ}, {"!=", TK_NE},    {"<=", TK_LE},
    {">=", TK_GE}, {"->", TK_ARROW},
};

struct StrTk KEYWORD_MAPPING[] = {
    {"fn", TK_FN},
    {"and", TK_AND},
    {"or", TK_OR},
    {"not", TK_NOT},
    {"return", TK_RETURN},
    {"while", TK_WHILE},
    {"if", TK_IF},
    {"else", TK_ELSE},
    {"goto", TK_GOTO},
    {"branch", TK_BRANCH},
    {"break", TK_BREAK},
    {"continue", TK_CONTINUE},
    {"let", TK_LET},
};

struct Chunk token_to_chunk(struct Source *s, struct Token *t) {
  return (struct Chunk){
      .data = source_get_ptr(s, t->start),
      .length = t->length,
  };
}
