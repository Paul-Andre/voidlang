#include "basics.h"

#include "terminal_colors.h"

#include <stdlib.h>
#include <time.h>

#include <unordered_map>
using std::unordered_map;

#include <map>


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
    const int N = 10000000;
    printf("Testing hash_map: ");
    ReferencePair *a = (ReferencePair *) calloc(N, sizeof(struct ReferencePair));

    for (int i = 0; i < N; i++) {
      a[i].k = (uint64_t)i << 32 | (random_uint64() & ((1ull << 32)-1));
      //a[i].k = (uint64_t)i << 32;// | (random_uint64() & ((1ull << 32)-1));
      a[i].v = random_uint64();
    }

    std::map<uint64_t, uint64_t> map;

    size_t prev_size = 0;
    for (int i = 0; i < N; i++) {
      uint64_t k = a[i].k;
      uint64_t v = a[i].v;
      map[k] = v;
      /*
      if (map.bucket_count () != prev_size) {
        prev_size = map.bucket_count();
        printf("%ld, \n", prev_size);
      }
      */
    }

    for (int i = 0; i < N; i++) {
      uint64_t k = a[i].k;
      uint64_t v = a[i].v;
      uint64_t found = map[k];
      assert(found == v);
    }

    print_pass();
  }
}
