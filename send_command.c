#include "atoms.h"

#include <stdio.h>

void send_command(Display *dpy, Window window, const char *command)
{
  XChangeProperty (dpy, window, XA_XEXEC_COMMAND, XA_STRING, 8,
		   PropModeReplace, (unsigned char *) command,
		   strlen (command));
  for (;;)
    {
      XEvent event;
      XNextEvent (dpy, &event);
      if (event.xany.type == DestroyNotify &&
	  event.xdestroywindow.window == window)
	{
	  /* Print to warn user...*/
	  fprintf (stderr, "window 0x%x was destroyed.\n",
		   (unsigned int) window);
	  exit(255);
	}
      else if (event.xany.type == PropertyNotify &&
	       event.xproperty.state == PropertyNewValue &&
	       event.xproperty.window == window &&
	       event.xproperty.atom == XA_XEXEC_RESPONSE)
	{
	  Atom actual_type;
	  int actual_format;
	  unsigned long nitems, bytes_after;
	  unsigned char *data = 0;
	  int result;

	  result = XGetWindowProperty (dpy, window, XA_XEXEC_RESPONSE,
				       0, (65536 / sizeof (long)),
				       True, /* atomic delete after */
				       XA_STRING,
				       &actual_type, &actual_format,
				       &nitems, &bytes_after,
				       &data);
	if (result != Success || !data) {
		fprintf(stderr, "(server sent invalid response)\n");
		exit(255);
	}
	if (fwrite(data,nitems,1,stdout) != 1 && nitems > 0) {
		perror("fwrite");
		exit(255);
	}
	if (fflush(stdout)) {
		perror("fflush");
		exit(255);
	}
	XFree(data);
	break;
	}
    }
}
