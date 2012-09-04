#include "atoms.h"
#include "lock.h"
#include "display.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	Display *dpy;
	Window win;
	int i;

	if (argc < 3) {
		fprintf(stderr, "Usage: xexec ident arg...\n");
		exit(255);
	}
	dpy = setup_display();
	init_atoms(dpy, argv[1]);
	win = find_window(dpy);
	if (!win) {
		fprintf(stderr, "%s not running on display\n", argv[1]);
		exit(255);
	}
	XSelectInput(dpy, win, (PropertyChangeMask|StructureNotifyMask));
	obtain_lock(dpy,win);
	for (i = 2; i < argc; i++) {
		send_command(dpy,win,argv[i]);
	}
	free_lock(dpy,win);
	exit(0);
}
