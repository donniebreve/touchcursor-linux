# Thanks to
[Thomas Bocek](https://github.com/tbocek): check him out and thanks for the starting point.  
[Martin Stone, Touch Cursor](https://github.com/martin-stone/touchcursor): wonderful project for cursor movement when coding.

# What this application does
This application remaps the uiophjklnm keys to different movement keys when the spacebar is pressed down, allowing you to keep your hands on the home row.

```
i - up
j - left
k - down
l - right
u - home
o - end
p - backspace
h - page up
n - page down
m - del
```

# How to install
1. Clone or download this repo
2. 'make' to build the application
3. 'make install' to install the application

# Bluetooth keyboards
Bluetooth keyboards do not show up in /dev/input/by-id/ to be easily identified.

1. type 'less /proc/bus/input/devices'
2. Find your bluetooth keyboard by looking at the Name or looking for 'bluetooth' in Sysfs
3. Once you have identified your keyboard, take note of the event in the Handlers variable (e.g. 'event21')
4. Add your keyboard to the touchcursor configuration under /etc/touchcursor/touchcursor.conf
    1. keyboard=/dev/input/event21

# Special note
This application works under Xorg and **Wayland**
