#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>

#include "binding.h"
#include "emit.h"

/**
 * Emits a key event.
 * */
void emit(int type, int code, int value)
{
   //printf("emit: code=%i value=%i\n", code, value);
   struct input_event e;
   e.time.tv_sec = 0;
   e.time.tv_usec = 0;
   // Set the virtual key code / value
   e.type = type;
   e.code = code;
   e.value = value;
   write(output_file_descriptor, &e, sizeof(e));

   if (type == EV_KEY)
   {
      // TODO: I don't like this here
      output_device_keystate[code] = value;
   }

   // Emit a syn event
   e.type = EV_SYN;
   e.code = SYN_REPORT;
   e.value = 0;
   write(output_file_descriptor, &e, sizeof(e));
}

/**
 * Emits a key event.
 * */
void emit_codes(int type, struct mapped_keycodes codes, int value)
{
   for (int i = 0; i < MAX_CHORDS && codes.codes[i]; i++) {
      emit(type, codes.codes[i], value);
   }
}
