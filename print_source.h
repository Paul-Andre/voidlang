#pragma once

#include "basics.h"

#include "reader.h"
#include "source.h"
#include "terminal_colors.h"

#define TAB_WIDTH 4
#define TAB_REPLACEMENT "    "

void print_position(struct Source *s, struct TextPosition p) {
  printf(STYLE_BOLD "%s:%d:%d: " RESET, s->file_name, p.line, p.column);
}

// Calculates horizontal position of character assuming tabs are replaced by
int get_drift(struct Source *s, struct TextPosition p) {
  char *line = s->lines[p.line - 1];
  ssize_t col = 1;
  ssize_t drift = 0;
  while (line[col - 1] != '\r' && line[col - 1] != '\n') {
    if (col == p.column) {
      return drift;
    }
    char c = line[col - 1];
    if (c == '\t') {
      drift += TAB_WIDTH;
    } else {
      drift += 1;
    }
    col++;
  }
  assert(false && "Position not located within line");
}

void print_line(struct Source *s, int line_num) {
  char *line = s->lines[line_num - 1];
  ssize_t col = 1;
  while (line[col - 1] != '\r' && line[col - 1] != '\n') {
    char c = line[col - 1];
    if (c == '\t') {
      printf(TAB_REPLACEMENT);
    } else {
      putchar(c);
    }
    col++;
  }
  putchar('\n');
}

void print_arrow(struct Source *s, struct TextPosition p) {
  int drift = get_drift(s, p);

  for (int i = 0; i < drift; i++) {
    putchar(' ');
  }
  putchar('^');
  putchar('\n');
}

// TODO: make this work on multi-line spans
void print_line_with_highlight(struct Source *s, struct TextPosition p,
                               ssize_t length, char *highlight) {
  char *line = s->lines[p.line - 1];
  ssize_t col = 1;
  while (line[col - 1] != '\r' && line[col - 1] != '\n') {
    if (col == p.column) {
      printf("%s", highlight);
    }
    char c = line[col - 1];
    if (c == '\t') {
      printf(TAB_REPLACEMENT);
    } else {
      putchar(c);
    }
    col++;
    if (col == p.column + length) {
      printf(RESET);
    }
  }
  putchar('\n');
}

void print_line_with_highlight_and_arrow(struct Source *s, struct TextPosition p,
                               ssize_t length, char *highlight) {
    print_line_with_highlight(s, p, length, highlight);
    print_arrow(s, p);
}

void print_from_source(struct Source *s, struct TextPosition p,
                       ssize_t length) {
  char *line = s->lines[p.line - 1];
  for (int i = 0; i < length; i++) {
    char c = line[p.column - 1 + i];
    putchar(c);
  }
}
