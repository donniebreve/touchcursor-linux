/*
 * Copyright 2018 Thomas Bocek
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */
 
 /*
  * Why is this tool useful?
  * ========================
  * 
  * Since I type with the "Dvorak" keyboard layout, the shortcuts such
  * as ctrl-c, ctrl-x, or ctrl-v are not comfortable anymore and one of them
  * require two hands to press.
  * 
  * Furthermore, applications such as Intellij and Eclipse have their
  * shortcuts, which I'm used to. So for these shortcuts I prefer "Querty".
  * Since there is no way to configure this, I had to intercept the
  * keys and remap the keys from "Dvorak" to "Querty" once CTRL, ALT, 
  * WIN or any of those combinations are pressed.
  * 
  * With X.org I was reling on the wonderful tool from Kenton Varda,
  * which I modified a bit, to make it work when Numlock is active. Other
  * than that, it worked as expected.
  * 
  * And then came Wayland. XGrabKey() works partially with some application
  * but not with others (e.g., gedit is not working). Since XGrabKey() is
  * an X.org function with some support in Wayland, I was looking for a more
  * stable solution. After a quick look to the repo https://github.com/kentonv/dvorak-qwerty
  * I saw that Kenton added a systemtap script to implement the mapping. This
  * scared me a bit to follow that path, so I implemented an other solution
  * based on /dev/uinput. The idea is to read /dev/input, grab keys with
  * EVIOCGRAB, create a virtual device that can emit the keys and pass 
  * the keys from /dev/input to /dev/uinput. If CTRL/ALT/WIN is
  * pressed it will map the keys back to "Qwerty".
  * 
  * Intallation
  * ===========
  * 
  * make dvorak
  * //make sure your user belongs to the group "input" -> ls -la /dev/input
  * //this also applies for /dev/uinput -> https://github.com/tuomasjjrasanen/python-uinput/blob/master/udev-rules/40-uinput.rules
  * //start it in startup applications
  * 
  * Related Links
  * =============
  * I used the following sites for inspiration:
  * https://www.kernel.org/doc/html/v4.12/input/uinput.html
  * https://www.linuxquestions.org/questions/programming-9/uinput-any-complete-example-4175524044/
  * https://stackoverflow.com/questions/20943322/accessing-keys-from-linux-input-device
  * https://gist.github.com/toinsson/7e9fdd3c908b3c3d3cd635321d19d44d
  * 
  */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdio.h>

/*static const char *const evval[3] = {
    "RELEASED",
    "PRESSED",
    "REPEATED"
};*/

int emit(int fd, int type, int code, int val)
{
   struct input_event ie;
   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   return write(fd, &ie, sizeof(ie));
}

//from: https://github.com/kentonv/dvorak-qwerty/tree/master/unix
static int modifier_bit(int key) {
  switch (key) {
    case 29: return 1;     // l-ctrl
    case 97: return 2;     // r-ctrl
    case 56: return 4;     // l-alt
    case 125: return 8;   // win
  }
  return 0;
}

//from: https://github.com/kentonv/dvorak-qwerty/tree/master/unix
static int qwerty2dvorak(int key) {
  switch (key) {
    case 12: return 40;
    case 13: return 27;
    case 16: return 45;
    case 17: return 51;
    case 18: return 32;
    case 19: return 24;
    case 20: return 37;
    case 21: return 20;
    case 22: return 33;
    case 23: return 34;
    case 24: return 31;
    case 25: return 19;
    case 26: return 12;
    case 27: return 13;
    case 30: return 30;
    case 31: return 39;
    case 32: return 35;
    case 33: return 21;
    case 34: return 22;
    case 35: return 36;
    case 36: return 46;
    case 37: return 47;
    case 38: return 25;
    case 39: return 44;
    case 40: return 16;
    case 44: return 53;
    case 45: return 48;
    case 46: return 23;
    case 47: return 52;
    case 48: return 49;
    case 49: return 38;
    case 50: return 50;
    case 51: return 17;
    case 52: return 18;
    case 53: return 26;
  }
  return key;
}

int main(int argc, char* argv[]) {
	
	setuid(0);
	
	if(argc < 2) {
		fprintf(stderr, "error: specify input device, e.g., found in "
		 "/dev/input/by-id/.\n");
        return EXIT_FAILURE;
	}
	
    struct input_event ev;
    ssize_t n;
    int fdi, fdo, i, mod_state, mod_current, emit_counter;
    struct uinput_user_dev uidev;

	//find the first input that exists
	for (i = 1; i < argc; i++) {
		fdi = open(argv[i], O_RDONLY);
		if (fdi != -1) {
			break;
		}
	}
	if (fdi == -1) {
		fprintf(stderr, "Cannot open any of the devices: %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}
    
    fdo = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fdo == -1) {
		fprintf(stderr, "Cannot open /dev/uinput: %s.\n", strerror(errno));
        return EXIT_FAILURE;
	}
	
	//grab the key, from the input
	//https://unix.stackexchange.com/questions/126974/where-do-i-find-ioctl-eviocgrab-documented/126996
	
	//fix is implemented, will make it to ubuntu sometimes in 1.9.4
	//https://bugs.freedesktop.org/show_bug.cgi?id=101796
	//quick workaround, sleep for 100ms...
	usleep(200 * 1000);
	
	if(ioctl(fdi, EVIOCGRAB, 1) == -1){
		fprintf(stderr, "Cannot grab key: %s.\n", strerror(errno));
        return EXIT_FAILURE;
	}
	
	// Keyboard
	if (ioctl(fdo, UI_SET_EVBIT, EV_KEY) == -1) {
		fprintf(stderr, "Cannot set ev bits, key: %s.\n", strerror(errno));
        return EXIT_FAILURE;
	}
	/*if(ioctl(fdo, UI_SET_EVBIT, EV_SYN) == -1) {
		fprintf(stderr, "Cannot set ev bits, syn: %s.\n", strerror(errno));
        return EXIT_FAILURE;
	}
	if(ioctl(fdo, UI_SET_EVBIT, EV_MSC) == -1) {
		fprintf(stderr, "Cannot set ev bits, msc: %s.\n", strerror(errno));
        return EXIT_FAILURE;
	}*/
    
	// All keys
    for (i = 0; i < KEY_MAX; i++) {
        if (ioctl(fdo, UI_SET_KEYBIT, i) == -1) {
            fprintf(stderr, "Cannot set ev bits: %s.\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	
	memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Virtual Dvorak Keyboard");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1234;
    uidev.id.product = 0x5678;
    uidev.id.version = 0;

    if (write(fdo, &uidev, sizeof(uidev)) == -1) {
        fprintf(stderr, "Cannot set device data: %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}

    if (ioctl(fdo, UI_DEV_CREATE) == -1) {
		fprintf(stderr, "Cannot create device: %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	emit_counter = 0;

	while (1) {
        n = read(fdi, &ev, sizeof ev);
        if (n == (ssize_t)-1) {
            if (errno == EINTR) {
                continue;
			} else {
                break;
			}
        } else
        if (n != sizeof ev) {
            errno = EIO;
            break;
        }
		if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2) {
			//printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);
			//map the keys
			mod_current = modifier_bit(ev.code);
			if(mod_current > 0) {
				if(ev.value == 1) { //pressed
					mod_state |= mod_current;
				} else if(ev.value == 0) {//released
					mod_state &= ~mod_current;
				}	
			}
			//if emit_counter is larger than 0, we have pending
			//pressed events, which needs to be handled in the dvorak
			//layout, otherwise we have stuck keys
			//there is still a theorectical chance to have stuck keys
			//if a key release of an non-dvorak mapping is seen before
			//a key release of a dvorak mapping. However, I have not
			//seen this in practice, so it should not happen. If it does,
			//then instead of an emit_counter, an array with the pressed
			//dvorak keys have to be kept in order to release the right
			//key
			if(mod_state > 0 || emit_counter > 0) {
				//printf("dvorak %d\n", emit_counter);
				if(ev.value==1) { //pressed
					emit_counter++;
				} else if(ev.value==0) { //released
					emit_counter--;
				}
				emit(fdo, ev.type, qwerty2dvorak(ev.code), ev.value);
			} else {
				//printf("non dvorak %d\n", emit_counter);
				emit(fdo, ev.type, ev.code, ev.value);
			}
		} else {
			//printf("%d 0x%04x (%d)\n", ev.value, (int)ev.code, (int)ev.code);
			emit(fdo, ev.type, ev.code, ev.value);
		}
    }
    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
    return EXIT_FAILURE;
}
