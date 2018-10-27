#include <unistd.h>
#include <linux/input.h>

#include "binding.h"
#include "emit.h"

/**
* Emits a key event.
*/
int emit(int type, int code, int value)
{
   struct input_event e;
   e.time.tv_sec = 0;
   e.time.tv_usec = 0;
   // emit key
   e.type = type;
   e.code = code;
   e.value = value;
   write(output, &e, sizeof(e));
   // emit syn event
   e.type = EV_SYN;
   e.code = 0;
   e.value = 0;
   write(output, &e, sizeof(e));
   return 0;
}
