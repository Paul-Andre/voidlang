#pragma once

#include "basics.h"

#include "ast.h"
#include "chunk.h"
#include "label.h"

int indent_level = 0;

void print_indent(void) {
  for (int i = 0; i < indent_level; i++) {
    printf(" ");
  }
}

#define INDENT_WIDTH 4

void increase_indent(void) { indent_level += INDENT_WIDTH; }

void decrease_indent(void) {
  indent_level -= INDENT_WIDTH;
  assert(indent_level >= 0);
}

void print_ty(struct AstTy *a) {
  if (IS_TAGGED(AstTyVar, a)) {
    struct AstTyVar *aa = IS_TAGGED(AstTyVar, a);
    print_chunk(aa->name);
  }
}

void print_op(enum TokenType t) {

  if (is_visible_ascii(t)) {
    putchar(t);
    return;
  }

  if (t == TK_LE)
    printf("<=");
  else if (t == TK_GE)
    printf(">=");
  else if (t == TK_EQ)
    printf("==");
  else if (t == TK_NE)
    printf("!=");
  else if (t == TK_AND)
    printf("and");
  else if (t == TK_OR)
    printf("or");
  else if (t == TK_NOT)
    printf("not");
  else {
    printf("?%d?", t);
  }
}

void print_ex(struct AstEx *a) {
  if (IS_TAGGED(AstExCall, a)) {
    struct AstExCall *aa = IS_TAGGED(AstExCall, a);
    print_ex(aa->function);
    printf("(");
    struct AstArgument *p = aa->arguments;
    while (p != NULL) {
      print_ex(p->ex);
      if (p->next != NULL) {
        printf(", ");
      }
      p = p->next;
    }
    printf(")");
  } else if (IS_TAGGED(AstExLit, a)) {
    struct AstExLit *aa = IS_TAGGED(AstExLit, a);
    printf("%ld", aa->value);
  } else if (IS_TAGGED(AstExVar, a)) {
    struct AstExVar *aa = IS_TAGGED(AstExVar, a);
    print_chunk(aa->name);
  } else if (IS_TAGGED(AstExBinop, a)) {
    struct AstExBinop *aa = IS_TAGGED(AstExBinop, a);
    printf("(");

    print_ex(aa->left);
    printf(" ");

    print_op(aa->op);

    printf(" ");
    print_ex(aa->right);

    printf(")");
  } else if (a->tag == TAG(AstExUnop)) {
    struct AstExUnop *aa = (struct AstExUnop *)a;
    printf("(");

    print_op(aa->op);

    printf(" ");
    print_ex(aa->right);

    printf(")");
  }
}

void print_st_all(struct AstSt *a);

void print_if_chain(struct AstSt *a) {
    struct AstStIf *aa = IS_TAGGED(AstStIf, a);
    assert(aa != NULL);
    printf("if ");
    print_ex(aa->condition);
    printf(" {\n");

    increase_indent();
    print_st_all(aa->then_body);
    decrease_indent();

    print_indent();
    printf("}");
    if (aa->else_body && aa->else_body->tag == TAG(AstStIf) && aa->else_body->next == NULL) {
      printf(" else ");
      print_if_chain(aa->else_body);
    } else if (aa->else_body) {
      printf(" else {\n");

      increase_indent();
      print_st_all(aa->else_body);
      decrease_indent();

      print_indent();
      printf("}");
    }
    printf("\n");
}

void print_label_info(struct LabelInfo *l) {
  if (l->rei == NULL) {
    print_chunk(l->name);
  } else {
    //printf("@");
    print_chunk(l->rei->name);
    printf("@_%d" , l->rei->id);
  }
}

void print_st(struct AstSt *a) {
  if (a->tag == TAG(AstStAss)) {
    struct AstStAss *aa = (struct AstStAss *)a;
    print_indent();
    print_ex(aa->left);
    printf(" = ");
    increase_indent();
    print_ex(aa->right);
    printf(";\n");
    decrease_indent();
  } else if (a->tag == TAG(AstStEx)) {
    struct AstStEx *aa = (struct AstStEx *)a;
    print_indent();
    print_ex(aa->ex);
    printf(";\n");
  } else if (a->tag == TAG(AstStRet)) {
    struct AstStRet *aa = (struct AstStRet *)a;
    print_indent();
    printf("return ");
    if (aa->ex != NULL) {
      print_ex(aa->ex);
    }
    printf(";\n");
  } else if (a->tag == TAG(AstStIf)) {
    print_indent();
    print_if_chain(a);
  } else if (a->tag == TAG(AstStWhile)) {
    struct AstStWhile *aa = (struct AstStWhile *)a;
    print_indent();
    printf("while ");
    print_ex(aa->condition);
    printf(" {\n");

    increase_indent();
    print_st_all(aa->body);
    decrease_indent();

    print_indent();
    printf("}\n");
  } else if (a->tag == TAG(AstStLabel)) {
    struct AstStLabel *aa = (struct AstStLabel *)a;
    decrease_indent();
    indent_level+=1;
    print_indent();
    print_label_info(&aa->label);
    printf(":\n");
    indent_level-=1;
    increase_indent();
  } else if (a->tag == TAG(AstStGoto)) {
    struct AstStGoto *aa = (struct AstStGoto *)a;
    print_indent();
    printf("goto ");
    print_label_info(&aa->label);
    printf(";\n");
  } else if (a->tag == TAG(AstStBranch)) {
    struct AstStBranch *aa = (struct AstStBranch *)a;
    print_indent();
    printf("branch ");
    print_ex(aa->condition);
    printf(" ");
    print_label_info(&aa->then_label);
    printf(" ");
    print_label_info(&aa->else_label);
    printf(";\n");
  } else {
    print_indent();
    printf("?????;\n");
  }
}

void print_st_all(struct AstSt *a) {
  while (a != NULL) {
    print_st(a);
    a = a->next;
  }
}

void print_tl(struct AstTl *a) {
  if (a->tag == TAG(AstTlFn)) {
    struct AstTlFn *aa = (struct AstTlFn *)a;
    print_indent();
    printf("fn %.*s(", (int)aa->name.length, aa->name.data);
    struct AstParam *p = aa->params;
    while (p != NULL) {
      print_chunk(p->name);
      if (p->next != NULL) {
        printf(", ");
      }
      p = p->next;
    }
    printf(")");
    if (aa->return_type != NULL) {
      printf(" -> ");
      print_ty(aa->return_type);
    }
    printf(" {\n");
    increase_indent();
    print_st_all(aa->body);

    decrease_indent();
    print_indent();
    printf("}\n\n");
  }
}

void print_tl_all(struct AstTl *a) {
  while (a != NULL) {
    print_tl(a);
    a = a->next;
  }
}
