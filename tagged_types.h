#pragma once

/**
 * "Tagged" structs are structs that have a "tag" at the beginning that indicates their type.
 */

enum Tag {
  TAG_NULL = 0,
#define TAGGED(Name) \
  TAG_##Name,
#include "tagged.def"
};

#define TAGGED(Name) \
  struct Name;
#include "tagged.def"

#define TAG(Name) TAG_##Name

struct Tagged {
  enum Tag tag;
};

