#include <X11/Xlib.h>
#include <X11/Xatom.h>

extern Atom XA_XEXEC_LOCK;
extern Atom XA_XEXEC_COMMAND;
extern Atom XA_XEXEC_RESPONSE;
extern Atom XA_XEXEC_VERSION;

void init_atoms(Display *dpy, const char *base);
