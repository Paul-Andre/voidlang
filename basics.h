#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#define WARN_UNUSED __attribute__((warn_unused_result))
#define NELEM(a)  (sizeof(a) / sizeof(a)[0]) 
