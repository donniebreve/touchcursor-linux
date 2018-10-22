# Thanks to
[Thomas Bocek](https://github.com/tbocek) check him out and thanks for the starting point.  
[Martin Stone, Touch Cursor](https://github.com/martin-stone/touchcursor) wonderful project for cursor movement when coding.

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

# To do
* Add a keybuffer to correctly handle quick typing
* Pull logic out of the main file for portability to other platforms
* Make the keymapping configurable
* Make the application launch when the computer is booted up
* Change setuid(0) to setcap in the Makefile
```
sudo setcap cap_sys_admin+ep binary
```
