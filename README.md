# flybinds
Create keyboard shortcuts (and show available combinations in top of screen) without holding
modifier keys (ctrl, super, meta, ...)

It's inspired in Doom Emacs key management and the code is based on dmenu.
## Usage
Map a key to open this program (maybe `\`, `¡` or `CapsLock` key, which are never used.
Then it will show on top of the screen available key combinations. For example,

```
x s -> shutdowns computer
x r -> reboots computer

l w -> launch browser
l e -> launch emacs

m h -> set monitor to HDMI
m d -> dual display monitor
...
```


Requirements
------------
In order to build flybinds you need the Xlib header files.


Installation
------------
Edit config.mk to match your local setup (dmenu is installed into
the /usr/local namespace by default).

Afterwards enter the following command to build and install dmenukey
(if necessary as root):

    make clean install
