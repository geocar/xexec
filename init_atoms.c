#include "atoms.h"

Atom XA_XEXEC_LOCK = 0;
Atom XA_XEXEC_COMMAND = 0;
Atom XA_XEXEC_RESPONSE = 0;
Atom XA_XEXEC_VERSION = 0;

void init_atoms(Display *dpy, const char *base)
{
	char *q;
#define WA(z) \
	if (!XA_XEXEC_ ## z) { \
		q = (char*)malloc(strlen(base)+10); \
		if (!q) { \
			perror("malloc"); \
			exit(255); \
		} \
		sprintf(q, "_%s_" #z, base); \
		XA_XEXEC_ ## z = XInternAtom(dpy, q, False); \
	}
	WA(VERSION)
	WA(LOCK)
	WA(COMMAND)
	WA(RESPONSE)
}
