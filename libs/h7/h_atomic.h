

#include <stdint.h>

int atomic_add(int volatile *a, int value);

int atomic_cas(int volatile *a, int oldvalue, int newvalue);

int atomic_get(int volatile *a);

void atomic_set(int volatile *a, int newvalue);
