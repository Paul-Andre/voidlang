#pragma once

#include "basics.h"
#include <stdarg.h>

#include "ast.h"
#include "chunk.h"
#include "print_message.h"
#include "print_source.h"
#include "token.h"
#include "tokenize.h"

#define RETURN_IF_NULL(a)                                                      \
  if ((a) == NULL) {                                                           \
    return NULL;                                                               \
  }


void print_highlighting_current_token(struct TokenStream *s) {
  struct Token t = s->current;
  if (t.type > 0) {
    print_line_with_highlight(&s->reader.source, t.start, t.length, COLOR_RED);
    print_arrow(&s->reader.source, t.start);
  }
}

void print_parse_error(struct TokenStream *s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
void print_parse_error(struct TokenStream *s, const char *fmt, ...) {

  print_position(&s->reader.source, s->current.start);
  print_error();

  va_list myargs;
  va_start(myargs, fmt);
  vprintf(fmt, myargs);
  va_end(myargs);
  printf("\n");

  print_highlighting_current_token(s);
}

struct AstParam *parse_param(struct TokenStream *s) {
  if (s->current.type != TK_IDENT) {
    print_parse_error(s, "expected function parameter name");
    return NULL;
  }
  struct Chunk ch = token_to_chunk(&s->reader.source, &s->current);
  token_stream_advance(s);
  struct AstParam *ret = calloc(1, sizeof(*ret));
  ret->name = ch;
  return ret;
}

struct AstTy *parse_simple_ty(struct TokenStream *s) {
  if (s->current.type != TK_IDENT) {
    print_parse_error(s, "expected type name");
    return NULL;
  }
  struct Chunk ch = token_to_chunk(&s->reader.source, &s->current);
  token_stream_advance(s);
  struct AstTyVar *ret = ALLOC_TAGGED(AstTyVar);
  ret->name = ch;
  return ret;
}

bool WARN_UNUSED parse_body(struct TokenStream *s, struct AstSt **ret);

struct AstTlFn *parse_tl_fn(struct TokenStream *s) {
  // TODO: assert that it starts with "fn"
  token_stream_advance(s);

  if (s->current.type != TK_IDENT) {
    print_parse_error(s, "'fn' should be followed by the name of the function");
    return NULL;
  }
  struct Chunk name = token_to_chunk(&s->reader.source, &s->current);
  token_stream_advance(s);

  if (s->current.type != '(') {
    print_parse_error(s, "expected '(' after function name");
    return NULL;
  }
  token_stream_advance(s);

  struct AstParam *param_root = NULL;
  struct AstParam **param_last_next = &param_root;
  while (true) {
    if (s->current.type == ')') {
      token_stream_advance(s);
      break;
    }
    struct AstParam *p = parse_param(s);
    if (p == NULL) {
      return NULL;
    }

    *param_last_next = p;
    param_last_next = &p->next;
    if (s->current.type == ')') {
      token_stream_advance(s);
      break;
    }
    if (s->current.type != ',') {
      print_parse_error(s, "expected ',' or ')'");
      return NULL;
    }
    token_stream_advance(s);
  }

  struct AstTy *return_type = NULL;
  if (s->current.type == TK_ARROW) {
    token_stream_advance(s);
    return_type = parse_simple_ty(s);
    if (return_type == NULL) {
      return NULL;
    }
  } else if (s->current.type != '{') {
    print_parse_error(s, "expected '->' followed by the return type or '{' "
                         "followed by the function's body");
    return NULL;
  }
  if (s->current.type != '{') {
    print_parse_error(s, "expected '{' followed by the function's body");
    return NULL;
  }

  struct AstSt *body;
  if (!parse_body(s, &body)) {
    return NULL;
  }

  struct AstTlFn *ret = ALLOC_TAGGED(AstTlFn);
  ret->name = name;
  ret->params = param_root;
  ret->return_type = return_type;
  ret->body = body;
  return ret;
}

bool _expect_semicolon(struct TokenStream *s) {
  if (s->current.type != ';') {
    // TODO: better line number
    print_parse_error(s, "expected ';'");
    return false;
  }
  token_stream_advance(s);
  return true;
}

#define EXPECT_SEMICOLON(s)                                                    \
  if (!_expect_semicolon(s)) {                                                 \
    return NULL;                                                               \
  }

bool is_unop(enum TokenType t) {
  return t == '-' || t == TK_NOT || t == '&' || t == '*';
}
int precedence_unop(enum TokenType t) {
  assert(is_unop(t));
  return 90;
}

bool is_binop(enum TokenType t) {
  return t == '(' || t == '[' || t == '.' || t == '*' || t == '/' || t == '%' ||
         t == '+' || t == '-' || t == '<' || t == '>' || t == TK_LE ||
         t == TK_GE || t == TK_EQ || t == TK_NE || t == TK_AND || t == TK_OR;
}
int precedence_binop(enum TokenType t) {
  assert(is_binop(t));
  if (t == '(' || t == '[' || t == '.')
    return 100;
  else if (t == '*' || t == '/' || t == '%')
    return 80;
  else if (t == '+' || t == '-')
    return 70;
  else if (t == '<' || t == '>' || t == TK_LE || t == TK_GE)
    return 60;
  else if (t == TK_EQ || t == TK_NE)
    return 50;
  else if (t == TK_AND)
    return 40;
  else if (t == TK_OR)
    return 30;

  assert(false && "Didn't find precedence of binop");
}

struct AstEx *parse_ex(struct TokenStream *s);
struct AstEx *parse_prec_bin(struct TokenStream *s, struct AstEx *left,
                             int left_group_prec);

struct AstEx *parse_primary(struct TokenStream *s) {
  if (s->current.type == TK_IDENT) {
    struct Chunk ch = token_to_chunk(&s->reader.source, &s->current);
    token_stream_advance(s);
    struct AstExVar *ret = ALLOC_TAGGED(AstExVar);
    ret->name = ch;
    return ret;
  } else if (s->current.type == TK_INT) {
    struct Chunk ch = token_to_chunk(&s->reader.source, &s->current);
    // TODO: parse int
    errno = 0;
    long long int n = strtoll(ch.data, NULL, 10);
    if (errno == ERANGE) {
      print_parse_error(s, "integer is too big");
      return NULL;
    }
    token_stream_advance(s);
    struct AstExLit *ret = ALLOC_TAGGED(AstExLit);
    ret->value = n;
    return ret;
  } else if (s->current.type == '(') {
    token_stream_advance(s);
    struct AstEx *e = parse_ex(s);
    RETURN_IF_NULL(e);
    if (s->current.type != ')') {
      print_parse_error(s, "expected ')'");
      return NULL;
    }
    token_stream_advance(s);
    return e;
  } else if (is_unop(s->current.type)) {
    struct Token op = s->current;
    token_stream_advance(s);
    struct AstEx *e = parse_primary(s);
    if (e == NULL) {
      return NULL;
    }
    struct AstEx *ee = parse_prec_bin(s, e, precedence_unop(op.type));
    if (ee == NULL) {
      return NULL;
    }
    struct AstExUnop *ret = ALLOC_TAGGED(AstExUnop);
    ret->op = op.type;
    ret->right = ee;
    return ret;
  }
  print_parse_error(s, "expected identifier, literal, paranthesized expression "
                       "or unary operator");
  return NULL;
}

struct AstEx *parse_prec_bin(struct TokenStream *s, struct AstEx *left,
                             int left_group_prec) {
  if (s->current.type == '(') {
    token_stream_advance(s);
    struct AstArgument *arg_root = NULL;
    struct AstArgument **arg_last_next = &arg_root;
    while (true) {
      if (s->current.type == ')') {
        token_stream_advance(s);
        break;
      }
      struct AstEx *ae = parse_ex(s);
      if (ae == NULL) {
        return NULL;
      }

      struct AstArgument *a = calloc(1, sizeof(*a));
      a->ex = ae;

      *arg_last_next = a;
      arg_last_next = &a->next;
      if (s->current.type == ')') {
        token_stream_advance(s);
        break;
      }
      if (s->current.type != ',') {
        print_parse_error(s, "expected ',' or ')'");
        return NULL;
      }
      token_stream_advance(s);
    }
    struct AstExCall *ret = ALLOC_TAGGED(AstExCall);
    ret->function = left;
    ret->arguments = arg_root;

    return parse_prec_bin(s, ret, left_group_prec);
  } else if (s->current.type == '[') {
    print_parse_error(s, "parsing of indexing is unimplemented");
    return NULL;
    /*
  } else if (s->current.type == '.') {
    print_parse_error(s, "parsing of field access is unimplemented");
    return NULL;
    */
  } else if (is_binop(s->current.type)) {
    struct Token left_op = s->current;
    token_stream_advance(s);
    struct AstEx *middle = parse_primary(s);
    if (middle == NULL) {
      return NULL;
    }
    if (!is_binop(s->current.type)) {
      struct AstExBinop *ret = ALLOC_TAGGED(AstExBinop);
      ret->op = left_op.type;
      ret->left = left;
      ret->right = middle;
      return ret;
    }
    struct Token right_op = s->current;

    int lp = precedence_binop(left_op.type);
    int rp = precedence_binop(right_op.type);

    if (lp >= rp) {
      struct AstExBinop *ret = ALLOC_TAGGED(AstExBinop);
      ret->op = left_op.type;
      ret->left = left;
      ret->right = middle;
      if (left_group_prec >= rp) {
        return ret;
      } else {
        return parse_prec_bin(s, ret, left_group_prec);
      }
    } else {
      struct AstEx *rest = parse_prec_bin(s, middle, lp);
      if (rest == NULL) {
        return NULL;
      }
      struct AstExBinop *ret = ALLOC_TAGGED(AstExBinop);
      ret->op = left_op.type;
      ret->left = left;
      ret->right = rest;
      return parse_prec_bin(s, ret, left_group_prec);
    }
  }

  return left;
}

struct AstEx *parse_ex(struct TokenStream *s) {
  struct AstEx *e = parse_primary(s);
  RETURN_IF_NULL(e);
  return parse_prec_bin(s, e, 0);
}

struct AstStIf *parse_st_if(struct TokenStream *s) {
  assert(s->current.type == TK_IF);
  token_stream_advance(s);

  struct AstEx *e = parse_ex(s);
  RETURN_IF_NULL(e);

  struct AstSt *then_body;
  if (!parse_body(s, &then_body)) {
    return NULL;
  }

  struct AstStIf *ret = ALLOC_TAGGED(AstStIf);
  ret->condition = e;
  ret->then_body = then_body;
  return ret;
}

struct AstSt *parse_if_chain(struct TokenStream *s) {
  struct AstSt *root;
  struct AstSt **last_else = &root;
  while (true) {
    struct AstStIf *a = parse_st_if(s);
    RETURN_IF_NULL(a);
    *last_else = a;
    last_else = &a->else_body;
    if (s->current.type != TK_ELSE) {
      break;
    }
    token_stream_advance(s);
    if (s->current.type == TK_IF) {
      continue;
    } else if (s->current.type == '{') {
      struct AstSt *else_body;
      if (!parse_body(s, &else_body)) {
        return NULL;
      }
      *last_else = else_body;
      break;
    } else {
      print_parse_error(s, "expected 'if' or '{' after 'else'");
      return NULL;
    }
  }
  return root;
}

struct AstSt *parse_st(struct TokenStream *s) {
  if (s->current.type == TK_RETURN) {
    token_stream_advance(s);
    struct AstEx *e = NULL;
    if (s->current.type != ';') {
      e = parse_ex(s);
      RETURN_IF_NULL(e);
    }
    EXPECT_SEMICOLON(s);
    struct AstStRet *ret = ALLOC_TAGGED(AstStRet);
    ret->ex = e;
    return ret;
  }

  if (s->current.type == TK_IF) {
    return parse_if_chain(s);
  }
  if (s->current.type == TK_WHILE) {
    token_stream_advance(s);

    struct AstEx *e = parse_ex(s);
    RETURN_IF_NULL(e);

    struct AstSt *then_body;
    if (!parse_body(s, &then_body)) {
      return NULL;
    }

    struct AstStWhile *ret = ALLOC_TAGGED(AstStWhile);
    ret->condition = e;
    ret->body = then_body;
    return ret;
  }
  if (s->current.type == TK_GOTO) {
    token_stream_advance(s);

    if (s->current.type != TK_IDENT) {
      print_parse_error(s, "'goto' must be followed by label name");
      return NULL;
    }
    struct Chunk name = token_to_chunk(&s->reader.source, &s->current);
    token_stream_advance(s);
    EXPECT_SEMICOLON(s);

    struct AstStGoto *ret = ALLOC_TAGGED(AstStGoto);
    ret->label.name = name;
    return ret;
  }
  if (s->current.type == TK_BRANCH) {
    token_stream_advance(s);

    struct AstEx *e = parse_ex(s);
    RETURN_IF_NULL(e);

    if (s->current.type != TK_IDENT) {
      print_parse_error(s, "'branch' condition must be followed by two label names");
      return NULL;
    }
    struct Chunk l1 = token_to_chunk(&s->reader.source, &s->current);
    token_stream_advance(s);

    if (s->current.type != TK_IDENT) {
      print_parse_error(s, "'branch' condition must be followed by two label names");
      return NULL;
    }
    struct Chunk l2 = token_to_chunk(&s->reader.source, &s->current);
    token_stream_advance(s);

    EXPECT_SEMICOLON(s);

    struct AstStBranch *ret = ALLOC_TAGGED(AstStBranch);
    ret->condition = e;
    ret->then_label.name = l1;
    ret->else_label.name = l2;
    return ret;
  }

  // TODO: better error message. It tries to parse expression and complains that
  // it expects tokens that start an expression. Maybe look ahead to see if
  // looks like an expression
  struct AstEx *e = parse_ex(s);
  RETURN_IF_NULL(e);

  if (s->current.type == '=') {
    token_stream_advance(s);

    struct AstEx *ee = parse_ex(s);
    RETURN_IF_NULL(ee);

    EXPECT_SEMICOLON(s);

    struct AstStAss *ret = ALLOC_TAGGED(AstStAss);
    ret->left = e;
    ret->right = ee;
    return ret;
  }
  if (s->current.type == ':') {
    if (IS_TAGGED(AstExVar, e)) {
      token_stream_advance(s);
      struct Chunk name;
      {
        struct AstExVar *ee = IS_TAGGED(AstExVar, e);
        name = ee->name;
      }
      free(e);
      e = NULL;
      struct AstStLabel *ret = ALLOC_TAGGED(AstStLabel);
      ret->label.name = name;
      return ret;
    }
  }

  EXPECT_SEMICOLON(s);
  struct AstStEx *ret = ALLOC_TAGGED(AstStEx);
  ret->ex = e;
  return ret;
}

// parses list of statement inside braces
bool WARN_UNUSED parse_body(struct TokenStream *s, struct AstSt **ret) {
  if (s->current.type != '{') {
    print_parse_error(s, "expected '{'");
    return false;
  }
  token_stream_advance(s);
  struct AstSt *root = NULL;
  struct AstSt **last_next = &root;
  while (true) {
    if (s->current.type == '}') {
      token_stream_advance(s);
      break;
    }
    struct AstSt *st = parse_st(s);
    if (st == NULL) {
      return false;
    }
    *last_next = st;
    last_next = &st->next;
  }
  *ret = root;
  return true;
}

struct AstTl *parse_tl(struct TokenStream *s) {
  struct Token t = s->current;
  if (t.type == TK_FN) {
    return parse_tl_fn(s);
  }
  print_parse_error(
      s, "top level declaration must start with 'fn', 'type' or 'let'");
  return NULL;
}

bool WARN_UNUSED parse_all_tl(struct TokenStream *s, struct AstTl **ret) {
  struct AstTl *root = NULL;
  struct AstTl **last_next = &root;
  while (true) {
    if (s->current.type == TK_EOF) {
      break;
    }
    struct AstTl *a = parse_tl(s);
    if (a == NULL) {
      return false;
    }
    *last_next = a;
    last_next = &a->next;
  }
  *ret = root;
  return true;
}
