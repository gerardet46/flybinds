/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

/* macros */
#define INTERSECT(x, y, w, h, r) (MAX(0, MIN((x) + (w), (r).x_org + (r).width) - MAX((x), (r).x_org)) \
								  * MAX(0, MIN((y) + (h), (r).y_org + (r).height) - MAX((y), (r).y_org)))
#define LENGTH(X) (sizeof X / sizeof X[0])
#define TEXTW(X)  (drw_fontset_getwidth(drw, (X)) + lrpad)
#define ITEMW(X)  (drw_fontset_getwidth(drw, (X)->keyname) \
	              + drw_fontset_getwidth(drw, sep)         \
	              + drw_fontset_getwidth(drw, (X)->text) + lrpad)

/* enums */
enum {
	SchemeKey,
	SchemeDesc,
	SchemeSep,
	SchemeBorder,
	SchemeLast
}; /* color schemes */

typedef struct {
	unsigned int mod;
	KeySym keysym;
	char* name;
} Key;

typedef struct item item;
struct item {
	char* keyname;
	char* text;
	char* script;
	unsigned int keep;
	item* childs;
	int displayline;
	item* parent;
};

static char* embed;
static int bh, mw, mh;
static int inputw = 0, columnwidth, showncols, rows;
static int lrpad; /* sum of left and right padding */
static int total;
static int displayline = 0;
static item* parent;
static int mon = -1, screen;

static Atom utf8;
static Display* dpy;
static Window root, parentwin, win;
static XIC xic;

static Drw* drw;
static Clr* scheme[SchemeLast];

/* Xresources preferences */
enum resource_type {
	STRING  = 0,
	INTEGER = 1,
	FLOAT   = 2
};
typedef struct {
	char* name;
	enum resource_type type;
	void* dst;
} ResourcePref;

static void load_xresources(void);
static void resource_load(XrmDatabase db, char* name, enum resource_type rtype, void* dst);

#include "config.h"
#include "keys.h"

static void
calcoffsets()
{
	/* cw = column width, c = numcols */
	int c, max = 0;
	item* temp = parent;

	/* calc total items and column max width */
	total = 0;
	while (temp->keyname) {
		max = MAX(max, ITEMW(temp));
		temp++;
		total++;
	}

	/* set geometry */
	columnwidth = displayline == 1 ? mw - 2 * outpaddinghor : max + colpadding;
	c           = (mw - 2 * outpaddingvert) / columnwidth;
	showncols   = (c < columns || columns == 0) ? c : columns;
	rows        = (total % showncols ? 1 : 0) + total / showncols;
	mh          = rows * bh + 2 * outpaddingvert;
}

static void
cleanup(void)
{
	size_t i;

	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	for (i = 0; i < SchemeLast; i++)
		free(scheme[i]);
	drw_free(drw);
	XSync(dpy, False);
	XCloseDisplay(dpy);
}

static int
drawitem(item* item, int x, int y, int w)
{
	drw_setscheme(drw, scheme[SchemeKey]);
	drw_text(drw, x, y, w, bh, lrpad / 2, item->keyname, 0);

	x += TEXTW(maxkey);

	drw_setscheme(drw, scheme[SchemeSep]);
	drw_text(drw, x, y, w, bh, lrpad / 2, sep, 0);

	x += TEXTW(sep);

	drw_setscheme(drw, scheme[SchemeDesc]);
	return drw_text(drw, x, y, w, bh, lrpad / 2, item->text, 0);
}

static void
drawmenu(void)
{
	item* item;
	int x = outpaddinghor, y = outpaddingvert;

	drw_setscheme(drw, scheme[SchemeKey]);
	drw_rect(drw, 0, 0, mw, mh, 1, 1);

	drw_setscheme(drw, scheme[SchemeBorder]);
	drw_rect(drw, 0, topbar ? mh - borderpx : 0, mw, borderpx, 1, 1);

	item = parent;

	for (int i = 0; i < total; i++) {
		drawitem(item, x, y, mw - x);

		if ((i + 1) % showncols == 0) {
			y += bh;
			x = outpaddinghor;
		} else
			x += columnwidth;

		item++;
	}

	drw_map(drw, win, 0, 0, mw, mh);
}

static void
grabfocus(void)
{
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000 };
	Window focuswin;
	int i, revertwin;

	for (i = 0; i < 100; ++i) {
		XGetInputFocus(dpy, &focuswin, &revertwin);
		if (focuswin == win)
			return;
		XSetInputFocus(dpy, win, RevertToParent, CurrentTime);
		nanosleep(&ts, NULL);
	}
	die("cannot grab focus");
}

static void
grabkeyboard(void)
{
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 1000000 };
	int i;

	if (embed)
		return;
	/* try to grab keyboard, we may have to wait for another process to ungrab */
	for (i = 0; i < 1000; i++) {
		if (XGrabKeyboard(dpy, DefaultRootWindow(dpy), True, GrabModeAsync,
		        GrabModeAsync, CurrentTime)
		    == GrabSuccess)
			return;
		nanosleep(&ts, NULL);
	}
	die("cannot grab keyboard");
}

void spawn(char* sc, char** args)
{
	char command[512];
	sprintf(command, "%s %s %s %s %s %s %s %s %s", sc, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);

	char* v[] = {
		"/bin/sh",
		"-c",
		command,
		NULL
	};
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(v[0], v);
		fprintf(stderr, "flybinds: execvp %s", v[0]);
		perror(" failed");
	}
}

static void
navigate(char* keyname)
{
	if (keyname) {
		/* search and set parent to that option */
		item* temp = parent;
		int j;
		for (j = 0; j < total; j++) {
			if (!strcmp(keyname, temp->keyname)) {
				if (temp->childs) {
					/* if it has childs, navigate to them */
					temp->childs->parent = parent;
					parent = temp->childs;

					if (temp->displayline == 1 || temp->displayline == 0)
						displayline = temp->displayline;

					calcoffsets();
					XResizeWindow(dpy, win, mw, mh);
					drw_resize(drw, mw, mh);
				} else {
					/* if not, execute script */
					char* arg[8];
					int argc;
					for (argc = 0; argc < 8; argc++)
						arg[argc] = "";

					if (temp->script) {
						/* execute if this child has script */
						spawn(temp->script, arg);
						if (temp->keep != 1) {
							cleanup();
							exit(0);
						}
					}

					/* if not, search it */
					item* sc = NULL;
					item* aux = parent;
					char* nextarg;
					int keep = 0;
					argc     = -2;

					while (1) {
						/* search script name */
						nextarg = aux->keyname;
						keep    = aux->keep;

						if (aux->script && strlen(aux->script) > 0) {
							/* change to this script */
							sc = aux;
							for (argc = 0; argc < 8; argc++)
								arg[argc] = "";

							argc = -2;
						}

						if (++argc >= 0 && argc < 8 && nextarg)
							arg[argc] = nextarg;

						if (aux == aux->parent)
							break;
						aux = aux->parent;
					}

					if (sc) {
						arg[++argc] = temp->keyname;

						spawn(sc->script, arg);
						if (temp->keep != 1 && keep != 1) {
							cleanup();
							exit(0);
						}
					} else {
						/* no script path */
						cleanup();
						exit(1);
					}
				}
				break;
			}
			temp++;
		}
	}
}

static void
keypress(XKeyEvent* ev)
{
	char buf[64];
	KeySym ksym = NoSymbol;
	Status status;
	int i;

	XmbLookupString(xic, ev, buf, sizeof buf, &ksym, &status);

	if (ksym == XK_Escape) {
		cleanup();
		exit(1);
	} else if (ksym == backkey) {
		/* go backwards */
		parent = parent->parent;
		calcoffsets();
		XResizeWindow(dpy, win, mw, mh);
		drw_resize(drw, mw, mh);
	} else {
		for (i = 0; i < LENGTH(keys); i++) {
			if (ksym == keys[i].keysym && (keys[i].mod | 16) == (ev->state | 16)) {
				navigate(keys[i].name);
				break;
			}
		}
	}
	drawmenu();
}

static void
run(void)
{
	XEvent ev;

	while (!XNextEvent(dpy, &ev)) {
		if (XFilterEvent(&ev, win))
			continue;
		switch (ev.type) {
		case DestroyNotify:
			if (ev.xdestroywindow.window != win)
				break;
			cleanup();
			exit(1);
		case Expose:
			if (ev.xexpose.count == 0)
				drw_map(drw, win, 0, 0, mw, mh);
			break;
		case FocusIn:
			/* regrab focus from parent window */
			if (ev.xfocus.window != win)
				grabfocus();
			break;
		case KeyPress:
			keypress(&ev.xkey);
			break;
		case VisibilityNotify:
			if (ev.xvisibility.state != VisibilityUnobscured)
				XRaiseWindow(dpy, win);
			break;
		}
	}
}

void resource_load(XrmDatabase db, char* name, enum resource_type rtype, void* dst)
{
	char* sdst  = NULL;
	int* idst   = NULL;
	float* fdst = NULL;
	sdst        = dst;
	idst        = dst;
	fdst        = dst;
	char fullname[256];
	char* type;
	XrmValue ret;
	snprintf(fullname, sizeof(fullname), "%s.%s", "flybinds", name);
	fullname[sizeof(fullname) - 1] = '\0';
	XrmGetResource(db, fullname, "*", &type, &ret);
	if (!(ret.addr == NULL || strncmp("String", type, 64))) {
		switch (rtype) {
		case STRING:
			strcpy(sdst, ret.addr);
			break;
		case INTEGER:
			*idst = strtoul(ret.addr, NULL, 10);
			break;
		case FLOAT:
			*fdst = strtof(ret.addr, NULL);
			break;
		}
	}
}

void load_xresources(void)
{
	Display* display;
	char* resm;
	XrmDatabase db;
	ResourcePref* p;
	display = XOpenDisplay(NULL);
	resm    = XResourceManagerString(display);
	if (!resm)
		return;
	db = XrmGetStringDatabase(resm);
	for (p = resources; p < resources + LENGTH(resources); p++)
		resource_load(db, p->name, p->type, p->dst);
	XCloseDisplay(display);
}

static void
setup(void)
{
	int x, y, i, j;
	unsigned int du;
	XSetWindowAttributes swa;
	XIM xim;
	Window w, dw, *dws;
	XWindowAttributes wa;
	XClassHint ch = { "flybinds", "flybinds" };
#ifdef XINERAMA
	XineramaScreenInfo* info;
	Window pw;
	int a, di, n, area = 0;
#endif
	/* init appearance */
	for (j = 0; j < SchemeLast; j++)
		scheme[j] = drw_scm_create(drw, colors[j], 2);

	utf8 = XInternAtom(dpy, "UTF8_STRING", False);

	/* calculate menu geometry */
	bh      = drw->fonts->h + 2;
	columns = MAX(columns, 0);

#ifdef XINERAMA
	i = 0;
	if (parentwin == root && (info = XineramaQueryScreens(dpy, &n))) {
		XGetInputFocus(dpy, &w, &di);
		if (mon >= 0 && mon < n)
			i = mon;
		else if (w != root && w != PointerRoot && w != None) {
			/* find top-level window containing current input focus */
			do {
				if (XQueryTree(dpy, (pw = w), &dw, &w, &dws, &du) && dws)
					XFree(dws);
			} while (w != root && w != pw);
			/* find xinerama screen with which the window intersects most */
			if (XGetWindowAttributes(dpy, pw, &wa))
				for (j = 0; j < n; j++)
					if ((a = INTERSECT(wa.x, wa.y, wa.width, wa.height, info[j])) > area) {
						area = a;
						i    = j;
					}
		}
		/* no focused window is on screen, so use pointer location instead */
		if (mon < 0 && !area && XQueryPointer(dpy, root, &dw, &dw, &x, &y, &di, &di, &du))
			for (i = 0; i < n; i++)
				if (INTERSECT(x, y, 1, 1, info[i]) != 0)
					break;

		mw = info[i].width;
		calcoffsets();
		x = info[i].x_org;
		y = info[i].y_org + (topbar ? 0 : info[i].height - mh);
		XFree(info);
	} else
#endif
	{
		if (!XGetWindowAttributes(dpy, parentwin, &wa))
			die("could not get embedding window attributes: 0x%lx",
			    parentwin);
		x  = 0;
		y  = topbar ? 0 : wa.height - mh;
		mw = wa.width;
	}
	inputw = MIN(inputw, mw / 3);

	/* create menu window */
	swa.override_redirect = True;
	swa.background_pixel  = scheme[SchemeKey][ColBg].pixel;
	swa.event_mask        = ExposureMask | KeyPressMask | VisibilityChangeMask;
	win                   = XCreateWindow(dpy, parentwin, x, y, mw, mh, 0,
	                      CopyFromParent, CopyFromParent, CopyFromParent,
	                      CWOverrideRedirect | CWBackPixel | CWEventMask, &swa);
	XSetClassHint(dpy, win, &ch);

	/* input methods */
	if ((xim = XOpenIM(dpy, NULL, NULL, NULL)) == NULL)
		die("XOpenIM failed: could not open input device");

	xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
	    XNClientWindow, win, XNFocusWindow, win, NULL);

	XMapRaised(dpy, win);
	if (embed) {
		XSelectInput(dpy, parentwin, FocusChangeMask | SubstructureNotifyMask);
		if (XQueryTree(dpy, parentwin, &dw, &w, &dws, &du) && dws) {
			for (i = 0; i < du && dws[i] != win; ++i)
				XSelectInput(dpy, dws[i], FocusChangeMask);
			XFree(dws);
		}
		grabfocus();
	}
	drw_resize(drw, mw, mh);
	drawmenu();
}

static void
usage(void)
{
	fputs("usage: flybinds [-bvh] [-c columns] [-fn font] [-m monitor]\n"
	      "                [-bg color] [-kf color] [-sf color] [-df color] [-bc color]\n"
	      "                [-cs separation] [-ph paddingH] [-pv paddingV] [-bw border width]\n"
	      "                [-w windowid] key1 key2 ...\n",
	    stderr);
	exit(1);
}

int main(int argc, char* argv[])
{
	XWindowAttributes wa;
	int i, j = 0;

	XrmInitialize();
	load_xresources();

	parent = items;
	parent->parent = parent;

	for (i = 1; i < argc; i++)
		/* these options take no arguments */
		if (!strcmp(argv[i], "-v")) { /* prints version information */
			puts("flybinds-" VERSION);
			exit(0);
		} else if (!strcmp(argv[i], "-b")) /* appears at the bottom of the screen */
			topbar = 0;
		else if (!strcmp(argv[i], "-h")) /* displays help */
			usage();
		/* these options take one argument */
		else if (!strcmp(argv[i], "-c")) /* number of columns in vertical list */
			columns = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-cs")) /* column separation */
			colpadding = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-ph")) /* outside hor. padding */
			outpaddinghor = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-pv")) /* outside vert. padding */
			outpaddingvert = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-bw")) /* border width */
			borderpx = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-m"))
			mon = atoi(argv[++i]);
		else if (!strcmp(argv[i], "-fn")) /* font or font set */
			fonts[0] = argv[++i];
		else if (!strcmp(argv[i], "-bg")) { /* global background color */
			colors[SchemeKey][ColBg]  = argv[++i];
			colors[SchemeSep][ColBg]  = argv[i];
			colors[SchemeDesc][ColBg] = argv[i];
		} else if (!strcmp(argv[i], "-kf")) /* key foreground color */
			colors[SchemeKey][ColFg] = argv[++i];
		else if (!strcmp(argv[i], "-sf")) /* separator foreground color */
			colors[SchemeSep][ColFg] = argv[++i];
		else if (!strcmp(argv[i], "-df")) /* description foreground color */
			colors[SchemeDesc][ColFg] = argv[++i];
		else if (!strcmp(argv[i], "-bc")) /* border color */
			colors[SchemeBorder][ColBg] = argv[++i];
		else if (!strcmp(argv[i], "-w")) /* embedding window id */
			embed = argv[++i];
		/* naviagate from this arg (set initial item diferent from parent) */
		else {
			j = i;
			break;
		}

	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("cannot open display");
	screen = DefaultScreen(dpy);
	root   = RootWindow(dpy, screen);
	if (!embed || !(parentwin = strtol(embed, NULL, 0)))
		parentwin = root;
	if (!XGetWindowAttributes(dpy, parentwin, &wa))
		die("could not get embedding window attributes: 0x%lx",
		    parentwin);
	drw = drw_create(dpy, screen, root, wa.width, wa.height);
	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");
	lrpad = drw->fonts->h / 2;

#ifdef __OpenBSD__
	if (pledge("stdio rpath", NULL) == -1)
		die("pledge");
#endif
	grabkeyboard();
	setup();

	for (; j < argc; j++)
		navigate(argv[j]);

	run();

	return 1; /* unreachable */
}
