# Dvorak <> Qwerty - Keyboard remapping for Linux when pressing CTRL or ALT

Since I type with the "Dvorak" keyboard layout, the shortcuts such as ctrl-c, ctrl-x, or ctrl-v are 
not comfortable anymore for the left hand. And even one of them require two hands to press.

Furthermore, applications such as Intellij have their shortcuts, which I'm used to. So 
for these shortcuts I prefer "Querty". Since there is no way to configure this, it is necessary to intercept the keys and remap the keys from "Dvorak" to "Querty" when pressing CTRL, ALT, WIN or any of those combinations.
   
With X.org I was relying on the wonderful project from Kenton Varda. And then came Wayland. 

## Keyboard remapping with dvorak that works reliably with Wayland - make ctrl-c ctrl-c again

XGrabKey() works partially with some application but not with others (e.g., gedit is not working). Since XGrabKey() is an X.org function with some support in Wayland, I was looking for a more stable solution. After a quick look to this [repo](https://github.com/kentonv/dvorak-qwerty), I saw that Kenton added a systemtap script to implement the mapping. It scared me a bit to follow that path, so I implemented an other solution based on /dev/uinput. The idea is to read /dev/input, grab keys with EVIOCGRAB, create a virtual device that can emit the keys and pass the keys from /dev/input to /dev/uinput. If CTRL/ALT/WIN is pressed it will map the keys back to "Qwerty".

Kenton Varda reported that this project also works with Chrome OS if started as root.

## Problems

The mapping does not work with Eclipse. Regular typing uses Dvorak, while the shortcuts are using Qwerty (I have not figured out why). This results in a situation where a Qwerty key gets remapped according to the Dvorak mapping.

## Installation

create binary with ```make dvorak```

Make sure your user belongs to the group "input" as ```ls -la /dev/input``` is "input" group read-writeable.

This also applies for /dev/uinput. In case its not uinput group read-writeable, add this [rule](https://github.com/tuomasjjrasanen/python-uinput/blob/master/udev-rules/40-uinput.rules) to /etc/udev/rules.d/. 

Start it in startup applications (gnome-shell) by pointing to your keyboard device, i.e. `dvorak /dev/input/event2`. Or, if your device event number keeps changing, call the device id directly, i.e. `dvorak /dev/input/by-id/usb-Microsoft_Wired_Keyboard_400-event-kbd`, using your keyboard device id.

## Related Links
I used the following sites for inspiration:

 * https://www.kernel.org/doc/html/v4.12/input/uinput.html
 * https://www.linuxquestions.org/questions/programming-9/uinput-any-complete-example-4175524044/
 * https://stackoverflow.com/questions/20943322/accessing-keys-from-linux-input-device
 * https://gist.github.com/toinsson/7e9fdd3c908b3c3d3cd635321d19d44d
