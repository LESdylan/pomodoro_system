#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <sstream>

void setWindowOpacity(Display* display, Window win, unsigned long opacity)
{
	Atom property = XInternAtom(display, "_NET_WM_WINDOW_OPACITY", False);
	XChangeProperty(display, win, property, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&opacity, 1);
}

void drawCenteredText(Display* display, Window win, GC gc, int width, int height, const std::string& text)
{
	XFontStruct* font = XLoadQueryFont(display, "-*-helvetica-bold-r-*-*-80-*-*-*-*-*-*-*");
	if (!font)
		font = XLoadQueryFont(display, "fixed");
	XSetFont(display, gc, font->fid);
	int dir, ascent, descent;
	XCharStruct overall;
	XTextExtents(font, text.c_str(), text.length(), &dir, &ascent, &descent, &overall);
	int x = (width - overall.width) / 2;
	int y = (height + ascent) / 2;
	XDrawString(display, win, gc, x, y, text.c_str(), text.length());
	XFreeFont(display, font);
}

void drawPromptText(Display* display, Window win, GC gc, int width, int height, const std::string& text)
{
	XFontStruct* font = XLoadQueryFont(display, "-*-helvetica-bold-r-*-*-24-*-*-*-*-*-*-*");
	if (!font)
		font = XLoadQueryFont(display, "fixed");
	XSetFont(display, gc, font->fid);
	int dir, ascent, descent;
	XCharStruct overall;
	XTextExtents(font, text.c_str(), text.length(), &dir, &ascent, &descent, &overall);
	int x = (width - overall.width) / 2;
	int y = height - ascent - 20; // 20px margin from bottom
	XDrawString(display, win, gc, x, y, text.c_str(), text.length());
	XFreeFont(display, font);
}

struct OverlayWin {
	Window win;
	GC gc;
	int width, height;
};

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <seconds>\n", argv[0]);
		return 1;
	}
	int seconds = atoi(argv[1]);
	Display* display = XOpenDisplay(0);
	if (!display)
	{
		fprintf(stderr, "Cannot open display\n");
		return 1;
	}

	int numScreens = 1;
	XineramaScreenInfo* screens = 0;
	if (XineramaIsActive(display))
	{
		screens = XineramaQueryScreens(display, &numScreens);
	}

	std::vector<OverlayWin> overlays;

	for (int i = 0; i < numScreens; ++i)
	{
		int sw = screens ? screens[i].width : DisplayWidth(display, DefaultScreen(display));
		int sh = screens ? screens[i].height : DisplayHeight(display, DefaultScreen(display));
		int sx = screens ? screens[i].x_org : 0;
		int sy = screens ? screens[i].y_org : 0;
		int winWidth = sw / 3;
		int winHeight = sh / 4;
		int winX = sx + (sw - winWidth) / 2;
		int winY = sy + (sh - winHeight) / 2;

		Window win = XCreateSimpleWindow(display, RootWindow(display, DefaultScreen(display)), winX, winY, winWidth, winHeight, 0, 0, BlackPixel(display, DefaultScreen(display)));
		XSetWindowAttributes attrs;
		attrs.override_redirect = True;
		XChangeWindowAttributes(display, win, CWOverrideRedirect, &attrs);
		setWindowOpacity(display, win, 0xBFFFFFFF);
		XMapRaised(display, win);

		// Rounded rectangle mask
		int radius = 40;
		Pixmap mask = XCreatePixmap(display, win, winWidth, winHeight, 1);
		GC mask_gc = XCreateGC(display, mask, 0, 0);
		XSetForeground(display, mask_gc, 0);
		XFillRectangle(display, mask, mask_gc, 0, 0, winWidth, winHeight);
		XSetForeground(display, mask_gc, 1);
		XFillArc(display, mask, mask_gc, 0, 0, 2*radius, 2*radius, 0, 360*64);
		XFillArc(display, mask, mask_gc, winWidth-2*radius, 0, 2*radius, 2*radius, 0, 360*64);
		XFillArc(display, mask, mask_gc, 0, winHeight-2*radius, 2*radius, 2*radius, 0, 360*64);
		XFillArc(display, mask, mask_gc, winWidth-2*radius, winHeight-2*radius, 2*radius, 2*radius, 0, 360*64);
		XFillRectangle(display, mask, mask_gc, radius, 0, winWidth-2*radius, winHeight);
		XFillRectangle(display, mask, mask_gc, 0, radius, winWidth, winHeight-2*radius);
		XShapeCombineMask(display, win, ShapeBounding, 0, 0, mask, ShapeSet);
		XFreePixmap(display, mask);
		XFreeGC(display, mask_gc);

		GC gc = XCreateGC(display, win, 0, 0);
		XColor color;
		Colormap colormap = DefaultColormap(display, DefaultScreen(display));
		XParseColor(display, colormap, "#222222", &color);
		XAllocColor(display, colormap, &color);
		XSetForeground(display, gc, color.pixel);

		OverlayWin ow;
		ow.win = win;
		ow.gc = gc;
		ow.width = winWidth;
		ow.height = winHeight;
		overlays.push_back(ow);
	}

	// Grab input on all overlays
	for (size_t i = 0; i < overlays.size(); ++i)
	{
		XGrabKeyboard(display, overlays[i].win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
		XGrabPointer(display, overlays[i].win, True, 0, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	}

	time_t end = time(0) + seconds;
	bool askConfirm = false;
	bool confirmed = false;
	XEvent ev;
	while (1)
	{
		while (XPending(display))
		{
			XNextEvent(display, &ev);
			if (ev.type == KeyPress)
			{
				KeySym ks = XLookupKeysym(&ev.xkey, 0);
				if (!askConfirm && ks == XK_Escape)
				{
					askConfirm = true;
				}
				else if (askConfirm && (ks == XK_y || ks == XK_Y))
				{
					confirmed = true;
					break;
				}
				else if (askConfirm && (ks == XK_n || ks == XK_N))
				{
					askConfirm = false;
				}
			}
		}
		if (confirmed)
			break;
		time_t now = time(0);
		int remain = (int)(end - now);
		if (remain < 0)
			break;
		for (size_t i = 0; i < overlays.size(); ++i)
		{
			XClearWindow(display, overlays[i].win);
			XSetForeground(display, overlays[i].gc, 0x222222);
			XFillRectangle(display, overlays[i].win, overlays[i].gc, 0, 0, overlays[i].width, overlays[i].height);
			XSetForeground(display, overlays[i].gc, WhitePixel(display, DefaultScreen(display)));
			std::ostringstream oss;
			oss << (remain / 60 < 10 ? "0" : "") << (remain / 60) << ":" << (remain % 60 < 10 ? "0" : "") << (remain % 60);
			drawCenteredText(display, overlays[i].win, overlays[i].gc, overlays[i].width, overlays[i].height, oss.str());
			if (askConfirm)
			{
				std::string msg = "Really urgent to skip? (Y/N)";
				XSetForeground(display, overlays[i].gc, 0xff6666);
				drawPromptText(display, overlays[i].win, overlays[i].gc, overlays[i].width, overlays[i].height, msg);
				XSetForeground(display, overlays[i].gc, WhitePixel(display, DefaultScreen(display)));
			}
			XFlush(display);
		}
		usleep(100000);
	}
	// Ungrab and cleanup
	for (size_t i = 0; i < overlays.size(); ++i)
	{
		XUngrabKeyboard(display, CurrentTime);
		XUngrabPointer(display, CurrentTime);
		XDestroyWindow(display, overlays[i].win);
	}
	if (screens)
		XFree(screens);
	XCloseDisplay(display);
	return 0;
}
