#include "display.h"

Display *setup_display(void)
{
	Display *dpy;
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		perror("XOpenDisplay");
		exit(255);
	}
	XSynchronize(dpy,True);
	return dpy;
}
