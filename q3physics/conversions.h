#ifndef Q3_CONVERSIONS
#define Q3_CONVERSIONS

#include "classes.h"

Hitbox q3tobox(q3Box* qbox);
void boxtoq3(Hitbox box, q3Box* qbox);

#endif
