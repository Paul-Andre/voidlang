#pragma once

#include "basics.h"

#include "ast.h"
#include "chunk.h"
#include "label.h"


// Turn if and while into goto and branch
struct AstSt *flatten_control_flow(struct AstSt *a, struct AstSt *append) {
  struct AstSt *root = a;
  struct AstSt **prev_next = &root;
  while (a != NULL) {
    if (a->tag == TAG(AstStIf)) {
      struct AstStIf *aa = (struct AstStIf *)a;

      struct ReiLabel *l1;
      struct ReiLabel *l2;
      struct ReiLabel *end;

      struct AstStLabel *l1_ast;
      struct AstStLabel *l2_ast;
      struct AstStLabel *end_ast;

      init_label("then", &l1, &l1_ast);
      init_label("else", &l2, &l2_ast);
      init_label("end", &end, &end_ast);

      struct AstStBranch *branch = ALLOC_TAGGED(AstStBranch);
      struct AstStGoto *goto1 = ALLOC_TAGGED(AstStGoto);

      *prev_next = branch;
      branch->condition = aa->condition;
      branch->then_label.rei = l1;
      l1->num_users++;
      branch->else_label.rei = l2;
      l2->num_users++;
      branch->next = l1_ast;
      l1_ast->next = flatten_control_flow(aa->then_body, goto1);
      goto1->label.rei = end;
      end->num_users++;
      goto1->next = l2_ast;
      l2_ast->next = flatten_control_flow(aa->else_body, end_ast);
      end_ast->next = aa->next;

      prev_next = &end_ast->next;
      a = aa->next;

    } else if (a->tag == TAG(AstStWhile)) {
      struct AstStWhile *aa = (struct AstStWhile *)a;

      struct ReiLabel *head_l;
      struct ReiLabel *body_l;
      struct ReiLabel *end_l;

      struct AstStLabel *head_ast;
      struct AstStLabel *body_ast;
      struct AstStLabel *end_ast;

      init_label("head", &head_l, &head_ast);
      init_label("body", &body_l, &body_ast);
      init_label("end", &end_l, &end_ast);

      struct AstStBranch *branch = ALLOC_TAGGED(AstStBranch);
      struct AstStGoto *goto_head = ALLOC_TAGGED(AstStGoto);

      *prev_next = head_ast;
      head_ast->next = branch;
      branch->condition = aa->condition;
      branch->then_label.rei = body_l;
      body_l->num_users++;
      branch->else_label.rei = end_l;
      end_l->num_users++;
      branch->next = body_ast;
      body_ast->next = flatten_control_flow(aa->body, goto_head);
      goto_head->label.rei = head_l;
      head_l->num_users++;
      goto_head->next = end_ast;
      end_ast->next = aa->next;

      prev_next = &end_ast->next;
      a = aa->next;
    } else {
      prev_next = &a->next;
      a = a->next;
    }
  }
  *prev_next = append;
  return root;
}

void vis_label(struct ReiLabel *l) {
  l->vis = VIS_GREY;
  struct ReiLabel *ll = NULL;
  struct AstStLabel *a = l->ast;
  if (a->next != NULL) {
    if (a->next->tag == TAG(AstStGoto)) {
      struct AstStGoto *ag = (struct AstStGoto *)a->next;
      ll = ag->label.rei;
    } else if (a->next->tag == TAG(AstStLabel)) {
      struct AstStLabel *ag = (struct AstStLabel *)a->next;
      ll = ag->label.rei;
    }
  }

  if (ll != NULL) {
    if (ll->vis == VIS_WHITE) {
      vis_label(ll);
    }
    assert(ll->vis != VIS_WHITE);
    if (ll->vis == VIS_BLACK) {
      l->infinite_loop = ll->infinite_loop;
      l->end_label = ll->end_label;
    } else if (ll->vis == VIS_GREY) {
      l->infinite_loop = true;
    }
  } else {
    l->infinite_loop = false;
    l->end_label = l;
  }
  l->vis = VIS_BLACK;
}

struct ReiLabel *get_last_label(struct ReiLabel *l) {
  if (l == NULL) {
    return NULL;
  }
  if (l->vis == VIS_WHITE) {
    vis_label(l);
  }
  assert(l->vis == VIS_BLACK);
  if (l->infinite_loop) {
    return l;
  } else {
    return l->end_label;
  }
}

void streamline_label(struct LabelInfo *l) {
  if (l->rei != NULL) {
    struct ReiLabel *oldL = l->rei;
    struct ReiLabel *newL = get_last_label(l->rei);
    l->rei = newL;
    newL->num_users++;
    oldL->num_users--;
  }
}

void streamline_labels(struct AstSt *a) {
  while (a != NULL) {
    if (a->tag == TAG(AstStGoto)) {
      struct AstStGoto *aa = (struct AstStGoto *)a;
      streamline_label(&aa->label);
    } else if (a->tag == TAG(AstStBranch)) {
      struct AstStBranch *aa = (struct AstStBranch *)a;
      streamline_label(&aa->then_label);
      streamline_label(&aa->else_label);
    }
    a = a->next;
  }
}

void remove_unused_labels(struct AstSt *a) {
  if (a == NULL) {
    return;
  }
  while (a != NULL && a->next != NULL) {
    if (a->next->tag == TAG(AstStLabel)) {
      struct AstStLabel *aa = (struct AstStLabel *)a->next;
      if (aa->label.rei != NULL && aa->label.rei->num_users == 0) {
        a->next = aa->next;
        continue;
      }
    } else if (is_escape(a->tag) && a->next->tag != TAG(AstStLabel)) {
      a->next = a->next->next;
      continue;
    }
    a = a->next;
  }
}

void remove_dead_statements(struct AstSt *a) {
  if (a == NULL) {
    return;
  }
  while (a != NULL && a->next != NULL) {
    if (a->next->tag == TAG(AstStLabel)) {
      struct AstStLabel *aa = (struct AstStLabel *)a->next;
      if (aa->label.rei != NULL && aa->label.rei->num_users == 0) {
        a->next = aa->next;
        continue;
      }
    }
    a = a->next;
  }
}

void flatten_all_functions(struct AstTl *a) {
  while (a != NULL) {
    if (a->tag == TAG(AstTlFn)) {
      struct AstTlFn *aa = (struct AstTlFn *)a;
      aa->body = flatten_control_flow(aa->body, NULL);
      streamline_labels(aa->body);
      remove_unused_labels(aa->body);
    }
    a = a->next;
  }
}
