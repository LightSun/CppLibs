

#include <stdint.h>

int h_atomic_add(int volatile *a, int value);

int h_atomic_cas(int volatile *a, int oldvalue, int newvalue);

int h_atomic_get(int volatile *a);

void h_atomic_set(int volatile *a, int newvalue);
