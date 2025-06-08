#include "basics.h"

#include "source.h"
#include "terminal_colors.h"

#include "hash_map.h"

#include <stdlib.h>
#include <time.h>

void print_pass(void) { printf(COLOR_GREEN "PASS" RESET "\n"); }
void print_fail(void) { printf(COLOR_RED "FAIL" RESET "\n"); }

struct ReferencePair {
  uint64_t k;
  uint64_t v;
};

uint64_t random_uint64(void) {
  int r = (uint64_t)rand() * ((uint64_t)RAND_MAX + 1) + (uint64_t)rand();
  return r;
}

int main() {
  srand(5); // Initialization, should only be called once.

  {
    printf("Testing split_lines: ");
    char *input = "hey \n"                 // 5
                  "this is a\n\r"          // 11
                  "this multiline\r"       // 15
                  "source # (comment)\r\n" // 20
                  "file";                  // 4
    ssize_t num_lines;
    char **lines = split_lines(input, strlen(input), &num_lines);
    assert(num_lines == 5);
    assert(lines[0] == &input[0]);
    assert(lines[1] == &input[5]);
    assert(lines[2] == &input[16]);
    assert(lines[3] == &input[31]);
    assert(lines[4] == &input[51]);

    print_pass();
  }
  {
    printf("Testing hash_map: ");

    struct HashMap map = hm_init();

    assert(hm_insert(&map, 0, 1, 1, NULL) == HM_SUCCESS);
    assert(hm_insert(&map, 0, 2, 2, NULL) == HM_SUCCESS);
    assert(hm_insert(&map, 1, 3, 3, NULL) == HM_SUCCESS);
    assert(hm_insert(&map, 1, 4, 4, NULL) == HM_SUCCESS);
    assert(hm_insert(&map, 0, 5, 5, NULL) == HM_SUCCESS);
    assert(hm_insert(&map, 3, 6, 5, NULL) == HM_SUCCESS);
    assert(hm_insert(&map, 1, 7, 5, NULL) == HM_SUCCESS);

    /*
    printf("\n");
    for (int i = 0; i < map.capacity; i++) {
      if (map.buckets[i].hash == 0) {
        printf("E ");
      } else {
        printf("%ld ", map.buckets[i].value);
      }
    }
    printf("\n");
    for (int i = 0; i < map.capacity; i++) {
      if (map.buckets[i].hash == 0) {
        printf("E ");
      } else {
        printf("%ld ", map.buckets[i].hash%16);
      }
    }
    printf("\n");
    */

    uint64_t *found;
    assert(hm_find(&map, 0, 1, &found));
    assert(hm_find(&map, 0, 2, &found));
    assert(hm_find(&map, 1, 3, &found));
    assert(hm_find(&map, 1, 4, &found));
    assert(hm_find(&map, 0, 5, &found));

    assert(!hm_find(&map, 0, 60, &found));
    assert(!hm_find(&map, 1, 70, &found));
    assert(!hm_find(&map, 2, 123, &found));

    assert(hm_insert(&map, 1, 4, 4, NULL) == HM_ENTRY_EXISTS);
    assert(hm_insert(&map, 0, 5, 5, NULL) == HM_ENTRY_EXISTS);

    print_pass();
  }

  {
    const int N = 10000000;
    printf("Testing hash_map: ");
    struct ReferencePair *a = calloc(N, sizeof(struct ReferencePair));

    for (int i = 0; i < N; i++) {
      a[i].k = (uint64_t)i << 32 | (random_uint64() & ((1ull << 32)-1));
      //a[i].k = (uint64_t)i << 32;// | (random_uint64() & ((1ull << 32)-1));
      a[i].v = random_uint64();
    }

    struct HashMap map = hm_init();

    for (int i = 0; i < N; i++) {
      uint64_t k = a[i].k;
      uint64_t v = a[i].v;
      enum HmResult r = hm_insert(&map, hash64(k), k, v, NULL);
      assert(r == HM_SUCCESS);
    }

    for (int i = 0; i < N; i++) {
      uint64_t k = a[i].k;
      uint64_t v = a[i].v;
      uint64_t *found;
      bool f = hm_find(&map, hash64(k), k, &found);
      assert(f);
      assert(*found == v);
    }

    print_pass();
  }
}
