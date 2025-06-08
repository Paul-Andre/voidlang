#pragma once

#include "tagged_types.h"
#include "ast.h"

#define TAGGED(Name) \
  struct Name *alloc_##Name(void) { \
    struct Name *ret = calloc(1, sizeof(*ret)); \
    ret->tag = TAG_##Name; \
    return ret; \
  }
#include "tagged.def"
#undef TAGGED

#define TAGGED(Name) \
  struct Name *is_##Name(struct Tagged *t) { \
    if (t == NULL) return NULL; \
    if (t->tag == TAG_##Name) { \
      return (struct Name *)t; \
    } \
    return NULL; \
  }
#include "tagged.def"
#undef TAGGED

#define ALLOC_TAGGED(Name) alloc_##Name()

#define IS_TAGGED(Name, a) is_##Name(a)
