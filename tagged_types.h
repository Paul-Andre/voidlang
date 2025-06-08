#pragma once

/**
 * "Tagged" structs are structs that have a "tag" at the beginning that indicates their type.
 */

enum Tag {
  TAG_NULL = 0,
#define TAGGED(Name) \
  TAG_##Name,
#include "tagged.def"
#undef TAGGED
};

#define TAGGED(Name) \
  struct Name;
#include "tagged.def"
#undef TAGGED

#define TAG(Name) TAG_##Name

struct Tagged {
  enum Tag tag;
};

