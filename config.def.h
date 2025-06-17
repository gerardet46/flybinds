/* See LICENSE file for copyright and license details. */
/* Default settings; can be overriden by command line. */
static int topbar = 1; /* [-b] if 0, flybinds appears at bottom */

/* -fn option overrides fonts[0]; default X11 font or font set */
static char font[]         = "monospace:size=12";
static const char* fonts[] = {
	font,
	"monospace:size=12",
};
static char sep[]            = "->";      /* [-s] set the separator between key and text */
static char background[]     = "#000000"; /* [-bg] global background */
static char keyfg[]          = "#00ff00"; /* [-fk] key name foreground */
static char sepfg[]          = "#00ffff"; /* [-fs] separator foreground */
static char descfg[]         = "#ffffff"; /* [-fd] description foreground */
static char bordercol[]      = "#ff0000"; /* [-bc] border color */
static unsigned int keywidth = 25;        /* width reserved for the key name */
static unsigned int backkey  = XK_Left;   /* backwards key */

static char* colors[SchemeLast][2] = {
    /*                 fg          bg       */
	[SchemeKey]    = { keyfg,      background }, /* fg for key */
	[SchemeTitle]  = { keyfg,      background }, /* fg for key */
	[SchemeSep]    = { sepfg,      background }, /* fg for separator (->) */
	[SchemeDesc]   = { descfg,     background }, /* fg for description */
	[SchemeBorder] = { background, bordercol  }, /* bg for border */
};
/* -c option, if nonzero flybinds will set this number of colums. If not, it's calculated */
static unsigned int columns        = 0;   /* [-c]  max-columns (0 for auto) */
static unsigned int colpadding     = 100; /* [-cs] separation between cols */
static unsigned int outpaddinghor  = 25;  /* [-ph] */
static unsigned int outpaddingvert = 15;  /* [-pv] */
static unsigned int titlepadding   = 5;   /* [-pt] */
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
	{ "titlepadding",   INTEGER, &titlepadding },
	{ "borderpx",       INTEGER, &borderpx },
};

/*
ITEM STRUCT:
- keyname          see keys.h, or "#" for title sections
- description      description displayed
- script           script or NULL
- keep open?       1 to keep flybinds opened when pressing that particular item
- children         submenu
- one per line?    1 to display its children one by line
*/

/* subsubmenu 2.1 */
static item menu21[] = {
    { "1", "Option 2.1.1" },
    { "2", "Option 2.1.2" },
	{ NULL } /* last element MUST be {NULL} */
};
/* subsubmenu 2.2 */
static item menu22[] = {
    { "1", "Option 2.2.1", NULL, 1 }, /* keep open */
    { "2", "Option 2.2.2", NULL, 1 }, /* keep open */
    { "3", "Option 2.2.3", NULL, 0 },
	{ NULL }
};
/* submenu 2 */
static item menu2[] = {
    { "A", "Submenu 1", NULL, 0, menu21 }, /* capital A */
    { "B", "Submenu 2", NULL, 0, menu22 },
	{ NULL }
};
/* submenu 1 */
static item menu1[] = {
    { "#",   "TITLE 1"}, /* this is a title section */
    { ".",   "Option 1", "echo '. pressed'"     },
    { ",",   "Option 2", "echo ', pressed'"     },
    { "␣",   "Option 3", "echo 'space pressed'" },
	{ "#",   "TITLE 2"}, /* this is a title section */
    { "\\n", "Option 4", "echo 'enter pressed'" },
    { "n",   "Option 5", "echo 'n pressed'"     },
	{ NULL }
};

/* main items */
static item items[] = {
    /* keyname  description    path to script   keep open?  children  one per line */
    { "a",      "Menu A",      "/usr/bin/echo", 0,          menu1,    0 },
    { "b",      "Menu B",      "echo"         , 0,          menu2,    1 },
	{ NULL }
};

/* root of all the items (must be named "root"), pointing to the main items */
static item root = { "#", "ROOT", NULL, 0, items, 0 };
