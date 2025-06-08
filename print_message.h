#pragma once

#include "basics.h"

#include "terminal_colors.h"

void print_error(void) {
  printf(COLOR_RED STYLE_BOLD "error: " RESET);
}
