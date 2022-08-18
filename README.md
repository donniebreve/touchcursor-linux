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

# Removing previous installation
Either use `make uninstall` or perform the steps manually:
1. Disable the old service  
`systemctl stop touchcusor.path`  
`systemctl disable touchcusor.path`
2. Remove the old service files
`rm ~/.config/systemd/user/touchcursor.service`
`rm ~/.config/systemd/user/touchcursor.path`
`rm ~/.config/systemd/user/default.target.wants/touchcursor.path`
3. Remove the old configuration files
`rm -r ~/.config/touchcursor`

# How to install
1. Clone or download this repo
2. 'make' to build the application
3. 'make install' to install the application
4. Modify the config file (`~/.config/touchcursor/touchcursor.conf`) to your liking
5. Restart the service `systemctl --user restart touchcursor.service`

# Thanks to
[Thomas Bocek, Dvorak](https://github.com/tbocek/dvorak): Check him out and thanks for the starting point. Good examples for capturing and modifying keyboard input in Linux, specifically Wayland.  
  
[Martin Stone, Touch Cursor](https://github.com/martin-stone/touchcursor): Wonderful project for cursor movement when coding.

# Special note
This application works under Xorg and **Wayland**.
