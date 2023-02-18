#ifndef emit_h
#define emit_h

#include "config.h"
/**
 * Emits a key event.
 * */
int emit(int type, int code, int value);

int emit_codes(int type, struct mapped_keycodes codes, int value);

#endif
