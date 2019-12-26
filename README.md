# touchcursor-linux
This application was designed to remap the `uiophjklnmy` keys to different movement keys when the spacebar is pressed down, allowing you to keep your hands on the home row. It has grown to allow remapping all the keys, even the hyper key.

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
y - insert
```

# How to install
1. Clone or download this repo
2. 'make' to build the application
3. 'make install' to install the application
4. Update the config at /etc/touchcursor/touchcursor.conf

# Thanks to
[Thomas Bocek, Dvorak](https://github.com/tbocek/dvorak): Check him out and thanks for the starting point. Good examples for capturing and modifying keyboard input in Linux, specifically Wayland.  
  
[Martin Stone, Touch Cursor](https://github.com/martin-stone/touchcursor): Wonderful project for cursor movement when coding.

# Special note
This application works under Xorg and **Wayland**.

# Donate
[Feel free to buy me a beer!](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=M234HAXBDRRHY&currency_code=USD&source=url)
