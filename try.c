#include "basics.h"

typedef int Zero[0];

struct Pod {
	int hash;
	int key;
	Zero value;
};


int main(void) {
	printf("sizeof int[0] %ld\n", sizeof(Zero));
	printf("sizeof Pod%ld\n", sizeof(struct Pod));
}
