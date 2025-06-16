/* See LICENSE file for copyright and license details. */
/* Default settings; can be overriden by command line. */
static int topbar = 1;                      /* -b  option; if 0, flybinds appears at bottom     */

/* -fn option overrides fonts[0]; default X11 font or font set */
static char font[] = "monospace:size=12";
static const char *fonts[] = {
	font,
    "monospace:size=12",
};
static char sep[]           = "->";      /* -s option; set the separator between key and text */
static char *maxkey         = "\\n";     /* max-length of key */
static char background[]    = "#000000"; /* global background */
static char keyfg[]         = "#00ff00"; /* key name foreground */
static char sepfg[]         = "#00ffff"; /* separator foreground */
static char descfg[]        = "#ffffff"; /* description foreground */
static char bordercol[]     = "#ff0000"; /* border color */
static unsigned int backkey = XK_Left;   /* backwards key */

static char *colors[SchemeLast][2] = {
	/*                 fg          bg       */
	[SchemeKey]    = { keyfg,      background }, /* fg for key */
	[SchemeSep]    = { sepfg,      background }, /* fg for separator (->) */
	[SchemeDesc]   = { descfg,     background }, /* fg for description */
	[SchemeBorder] = { background, bordercol  }, /* bg for border */
};
/* -c option, if nonzero flybinds will set this number of colums. If not, it's calculated */
static unsigned int columns        = 0;   /* [-c]  max-columns (0 for auto) */
static unsigned int colpadding     = 100; /* [-cs] separation between cols */
static unsigned int outpaddinghor  = 25;  /* [-ph] */
static unsigned int outpaddingvert = 15;  /* [-pv] */
static unsigned int borderpx       = 2;   /* [-bw] */

/*
 * Xresources preferences to load at startup (the class to use is "flybinds")
 */
ResourcePref resources[] = {
	{ "font",           STRING,  &font },
	{ "separator",      STRING,  &sep },
	{ "background",     STRING,  &background },
	{ "keyfg",          STRING,  &keyfg },
	{ "sepfg",          STRING,  &sepfg },
	{ "descfg",         STRING,  &descfg },
	{ "bordercol",      STRING,  &bordercol },
	{ "maxcolumns",     INTEGER, &columns },
	{ "colpadding",     INTEGER, &colpadding },
	{ "outpaddinghor",  INTEGER, &outpaddinghor },
	{ "outpaddingvert", INTEGER, &outpaddingvert },
	{ "borderpx",       INTEGER, &borderpx },
};

/* see keys.h to see the keynames */
/*
keyname          see keys.h
description      
script           path to script or NULL
keep open?       0 / 1
childs           submenu
one per line?    0 / 1
*/

/* subsubmenu 2.1 */
static item menu21[] = {
    { "1", "Option 2.1.1" },
    { "2", "Option 2.1.2" },
	{ NULL } /* last element MUST be {NULL} */
};
/* subsubmenu 2.2 */
static item menu22[] = {
    { "1", "Option 2.2.1" },
    { "2", "Option 2.2.2" },
    { "3", "Option 2.2.3" },
	{ NULL }
};
/* submenu 2 */
static item menu2[] = {
    { "A", "Submenu 1", NULL, 0, menu21 }, /* capital A */
    { "B", "Submenu 2", NULL, 1, menu22 },
	{ NULL }
};
/* submenu 1 */
static item menu1[] = {
    { ".",   "Option 1", "notify-send '. pressed'"     },
    { ",",   "Option 2", "notify-send ', pressed'"     },
    { "␣",   "Option 3", "notify-send 'space pressed'" },
    { "\\n", "Option 4", "notify-send 'enter pressed'" },
    { "n",   "Option 5", "notify-send 'n pressed'"     },
	{ NULL }
};
    
/* parent items */
static item items[] = {
    /* keyname  description    path to script          keep open?  childs  one per line */
    { "a",      "Menu A",      "/usr/bin/notify-send", 0,          menu1,  0 },
    { "b",      "Menu B",      "/usr/bin/notify-send", 1,          menu2,  1 },
	{ NULL }
};
