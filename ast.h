#pragma once

#include "basics.h"

#include "chunk.h"
#include "source.h"

#include "tagged_types.h"

struct ReiLabel;
struct Ast {
  struct Tagged;
  struct TextPosition begin;
  struct TextPosition end;
};
struct AstTy {
  struct Ast;
};
struct AstSt {
  struct Ast;
  struct AstSt *next;
};
struct AstTl {
  struct Ast;
  struct AstTl *next;
};
struct AstParam {
  struct Chunk name;
  struct AsTy *type;
  struct AstParam *next;
};
struct AstTlFn {
  struct AstTl;
  struct Chunk name;
  struct AstParam *params;
  struct AstTy *return_type;
  struct AstSt *body;
};
struct AstTyVar {
  struct AstTy;
  struct Chunk name;
};
struct AstEx {
  struct Ast;
};
struct AstExVar {
  struct AstEx;
  struct Chunk name;
};
struct AstExLit {
  struct AstEx;
  int64_t value;
};
struct AstStAssVar {
  struct AstSt;
  struct Chunk name;
  struct AstEx *right;
};
struct AstStRet {
  struct AstSt;
  struct AstEx *ex;
};
struct AstStEx {
  struct AstSt;
  struct AstEx *ex;
};
struct AstStIf {
  struct AstSt;
  struct AstEx *condition;
  struct AstSt *then_body;
  struct AstSt *else_body;
};
struct LabelInfo {
  struct Chunk name;
  struct ReiLabel *rei;
};
struct AstStLabel {
  struct AstSt;
  struct LabelInfo label;
};
struct AstStGoto {
  struct AstSt;
  struct LabelInfo label;
};
struct AstStBreak {
  struct AstSt;
  int label_unimplemented_sentry;
  // struct LabelInfo label;
};
struct AstStContinue {
  struct AstSt;
  int label_unimplemented_sentry;
  // struct LabelInfo label;
};
struct AstStBranch {
  struct AstSt;
  struct AstEx *condition;
  struct LabelInfo then_label;
  struct LabelInfo else_label;
};
struct AstStWhile{
  struct AstSt;
  struct AstEx *condition;
  struct AstSt *body;
};
struct AstArgument {
  struct AstEx *ex;
  struct AstArgument *next;
};
struct AstExCall {
  struct AstEx;
  struct AstEx *function;
  struct AstArgument *arguments;
};
struct AstExIndexing {
  struct AstEx;
  struct AstEx *base;
  struct AstEx *index;
};
struct AstExBinop {
  struct AstEx;
  enum TokenType op;
  struct AstEx *left;
  struct AstEx *right;
};
struct AstExUnop {
  struct AstEx;
  enum TokenType op;
  struct AstEx *right;
};

bool is_escape(enum Tag t) {
  return
    t==TAG(AstStRet) ||
    t==TAG(AstStGoto) ||
    t==TAG(AstStBreak) ||
    t==TAG(AstStContinue) ||
    t==TAG(AstStBranch);
}

#include "tagged_impls.h"
