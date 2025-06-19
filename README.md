# flybinds
Create keyboard shortcuts (and show available combinations in top of screen)
without holding modifier keys (ctrl, super, meta, ...)

It's inspired in `which-key` package for `Emacs` and the code is based on dmenu.
## Usage
Map a key to open this program (maybe `\`, `¡` or `CapsLock` key, which are
never used. Then it will show on top of the screen available key combinations.
For example,

```
x s -> shutdowns computer
x r -> reboots computer

l w -> launch browser
l e -> launch emacs

m h -> set monitor to HDMI
m d -> dual display monitor
...
```
You can press `Left Key` to go up to the menu.

Also, you can launch `flybinds` followed by some arguments `key1 key2 ...` to
navigate to those items at the very beginning.

For example, let's say we have a parent item **c (config)** and there a child
**m (monitors)** which have child to setup the monitors. Then, we can run
`flybinds c m` to open *flybinds* directly in the *monitors* section.

To see how the program works, just play around. The file `config.def.h` explains
how to use and configure flybinds easily.

## Xresources
It can be configured via the `~/.Xresources` file, using the class `flybinds`.
Check `config.def.h` to see what resources can be customized.

## Scripts
To avoid writing a script per item, you can group a script for a whole submenu. In the example of *shutdown, reboot, ...* we can write a simple script like this:
```bash
case "$@" in
    s) poweroff ;;
    r) reboot ;;
esac
```
and only set the path of the script to the parent.

## Example
Here we explain the example defined in `config.def.h`.

Here we have the following tree (see the submenus defined in `config.def.h`):

    {a} -> {. , ␣ \n n}
    {b} -> {A} -> {1 2}
           {B} -> {1 2 3}

- Children `{. , ␣}` in `{a}` have a custom script. Then, **this script is the
  one to be executed.**

- The items `{b}->{B}->{1 2}` have the _KEEPOPEN_ flag: **flybinds will keep
  opened** when this are selected.

- The item `{b}->{B}->{3}` have _DEFAULT_ behaviour but no script is provided.
  Then, it will search in its predecessors the first script it finds:
    - `{3}` doesn't have a script.
    - `{B}` doesn't have a script.
    - `{b}` has the script 'echo'. Therefore, we found the script 'echo', and
  the path we must follow from 'echo' to `{3}` is `{B}->{3}`, so the script
  executed will be **the command followed by the key names****: `echo B 3`

- Children `{\n n}` in `{a}` have the _CONCAT_ behaviour, which forces (like
  before) to search the script in the predecessors. Then, `{a}` is the first
  predecessor that has a script and doesn't have the flag _CONCAT_, and the
  script is `echo`. Thus, the scripts executed will be `echo 'enter pressed'`
  and `echo 'n pressed'`, because with the _CONCAT_ flag the **"script" field
  will be passed as an argument instead of the key name**.

- Another example of _CONCAT_: the submenu `{b}->{A}` has the _CONCAT_ flag with
  the "script" `'SUBMENU B -> A ->'`. Therefore, when selected `{b}->{A}->{1
  2}`, the result will be `echo 'SUBMENU B -> A ->' 1 (or 2)`. That it, we
  search from `{1 2}` the first predecessor with an actual script (`{b}`), and
  then we pass the "script" in `{A}` instead of the key name, "A".

- We can define multiple flags, as in `{b}->{B}->{1}`, which has _CONCAT_ and
  _KEEPOPEN_.

Requirements
------------
In order to build flybinds you need the Xlib header files.


Installation
------------
Edit config.mk to match your local setup (flybinds is installed into the
/usr/local namespace by default).

Afterwards enter the following command to build and install flybinds (if
necessary as root):

    make clean install
