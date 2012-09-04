#include <X11/Xlib.h>
#include <X11/Xatom.h>

extern char *lock_data;

void obtain_lock(Display *dpy, Window window);
void free_lock(Display *dpy, Window window);

