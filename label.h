#pragma once

#include "basics.h"

#include "chunk.h"
#include "source.h"
#include "ast.h"

enum Vis {
  VIS_WHITE = 0,
  VIS_GREY,
  VIS_BLACK,
};

struct ReiLabel {
  int id;
  struct Chunk name;
  struct AstStLabel *ast;
  bool generated;
  enum Vis vis;
  bool infinite_loop;
  int num_users;
  struct ReiLabel *end_label;
};

int label_counter = 1;

void init_label(char *name, struct ReiLabel **l, struct AstStLabel **a) {
  *l = calloc(1, sizeof(**l));
  *a = ALLOC_TAGGED(AstStLabel);

  (*l)->id = ++label_counter;
  (*l)->name = as_chunk(name);
  (*l)->ast = *a;
  (*l)->generated = true;

  (*a)->label.rei = *l;
}
