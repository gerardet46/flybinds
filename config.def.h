/* See LICENSE file for copyright and license details. */
/* Default settings; can be overriden by command line. */
static int topbar = 1;                      /* -b  option; if 0, flybinds appears at bottom     */

/* -fn option overrides fonts[0]; default X11 font or font set */
static const char *fonts[] = {
	"monospace:size=12"
};
static const char *sep         = "->";      /* -s option; set the separator between key and text */
static const char *maxkey      = "\\n";     /* max-length of key */
static const char background[] = "#000000"; /* global background */

static const char *colors[SchemeLast][2] = {
	/*                 fg           bg       */
	[SchemeKey]    = { "#00ff00"  , background }, /* fg for key */
	[SchemeSep]    = { "#00ffff"  , background }, /* fg for separator (->) */
	[SchemeDesc]   = { "#ffffff"  , background }, /* fg for description */
	[SchemeBorder] = { background , "#ffffff"  }, /* bg for border */
};
/* -c option, if nonzero flybinds will set this number of colums. If not, it's calculated */
static unsigned int columns        = 0;   /* [-c]  max-columns (0 for auto) */
static unsigned int colpadding     = 100; /* [-cs] separation between cols */
static unsigned int outpaddinghor  = 25;  /* [-ph] */
static unsigned int outpaddingvert = 15;  /* [-pv] */
static unsigned int borderpx       = 2;   /* [-bw] */


/* see keys.h to see the keynames */
/*
keyname          see keys.h
description      
script           path to script or NULL
keep open?       0 / 1
childs           submenu
childs length    LENGTH(submenu)
one per line?    0 / 1
*/

/* subsubmenu 2.1 */
static item menu21[] = {
    { "1", "Option 2.1.1" },
    { "2", "Option 2.1.2" },
};
/* subsubmenu 2.2 */
static item menu22[] = {
    { "1", "Option 2.2.1" },
    { "2", "Option 2.2.2" },
    { "3", "Option 2.2.3" },
};
/* submenu 2 */
static item menu2[] = {
    { "A", "Submenu 1", NULL, 0, menu21, LENGTH(menu21) }, /* capital A */
    { "B", "Submenu 2", NULL, 1, menu22, LENGTH(menu22) },
};
/* submenu 1 */
static item menu1[] = {
    { ".",   "Option 1", "notify-send '. pressed'"     },
    { ",",   "Option 2", "notify-send ', pressed'"     },
    { "␣",   "Option 3", "notify-send 'space pressed'" },
    { "\\n", "Option 4", "notify-send 'enter pressed'" },
    { "n",   "Option 5", "notify-send 'n pressed'"     },
};
    
/* parent items */
static item items[] = {
    /* keyname  description    path to script          keep open?  childs  childs length    one per line */
    { "a",      "Menu A",      "/usr/bin/notify-send", 0,          menu1,  LENGTH(menu1)    ,0 },
    { "b",      "Menu B",      "/usr/bin/notify-send", 1,          menu2,  LENGTH(menu2)    ,1 },
};
