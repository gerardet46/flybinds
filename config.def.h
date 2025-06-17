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
- behaviour
     * DEFAULT     default behaviour
     * KEEPOPEN    keep flybinds opened for this final option
     * ONEPERLINE  show one child per line
- children         submenu or NULL
*/

/* subsubmenu 2.1 */
static item menu21[] = {
    { "1", "Option 2.1.1" },
    { "2", "Option 2.1.2" },
	{ NULL } /* last element MUST be {NULL} */
};
/* subsubmenu 2.2 */
static item menu22[] = {
    { "1", "Option 2.2.1", NULL, KEEPOPEN }, /* keep open */
    { "2", "Option 2.2.2", NULL, KEEPOPEN }, /* keep open */
    { "3", "Option 2.2.3", NULL, DEFAULT },
	{ NULL }
};
/* submenu 2 */
static item menu2[] = {
    { "A", "Submenu 1", NULL, DEFAULT, menu21 }, /* capital A */
    { "B", "Submenu 2", NULL, DEFAULT, menu22 },
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
    /* keyname  description    path to script   behaviour     children */
    { "a",      "Menu A",      "/usr/bin/echo", DEFAULT,    menu1 },
    { "b",      "Menu B",      "echo"         , ONEPERLINE, menu2 }, /* one per line */
    { NULL } /* last element MUST be {NULL} */
};

/*
 Root of all the items (must be named "root"), pointing to the main items.
 keyname and description aren't processed.
*/
static item root = { "#", "ROOT", NULL, DEFAULT, items };
