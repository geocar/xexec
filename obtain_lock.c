#include "atoms.h"

#include <stdio.h>

#include "lock.h"

char *lock_data = 0;

void obtain_lock(Display *dpy, Window window)
{
	/* this comes largely from remote.c
	 * which bears the following mark:
 * Copyright © 1996 Netscape Communications Corporation, all rights reserved.
 * Created: Jamie Zawinski <jwz@netscape.com>, 24-Dec-94.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
	 *
	 * BUGS: this file is too long
	 */
  Bool locked = False;
  Bool waited = False;

  if (! lock_data)
    {
      lock_data = (char *) malloc (255);
      sprintf (lock_data, "pid%d@", getpid ());
      if (gethostname (lock_data + strlen (lock_data), 100))
	{
	  perror ("gethostname");
	  exit (-1);
	}
    }

  do
    {
      int result;
      Atom actual_type;
      int actual_format;
      unsigned long nitems, bytes_after;
      unsigned char *data = 0;

      XGrabServer (dpy);   /* ################################# DANGER! */

      result = XGetWindowProperty (dpy, window, XA_XEXEC_LOCK,
				   0, (65536 / sizeof (long)),
				   False, /* don't delete */
				   XA_STRING,
				   &actual_type, &actual_format,
				   &nitems, &bytes_after,
				   &data);
      if (result != Success || actual_type == None)
	{
	  /* It's not now locked - lock it. */
#ifdef DEBUG_PROPS
	  fprintf (stderr, "(writing lock property "
		   " \"%s\" to 0x%x)\n",
		   lock_data, (unsigned int) window);
#endif
	  XChangeProperty (dpy, window, XA_XEXEC_LOCK, XA_STRING, 8,
			   PropModeReplace, (unsigned char *) lock_data,
			   strlen (lock_data));
	  locked = True;
	}

      XUngrabServer (dpy); /* ################################# danger over */
      XSync (dpy, False);

      if (! locked)
	{
	  /* We tried to grab the lock this time, and failed because someone
	     else is holding it already.  So, wait for a PropertyDelete event
	     to come in, and try again. */

	  fprintf (stderr, "window 0x%x is locked by %s; waiting...\n",
		   (unsigned int) window, data);
	  waited = True;

	  while (1)
	    {
	      XEvent event;
	      XNextEvent (dpy, &event);
	      if (event.xany.type == DestroyNotify &&
		  event.xdestroywindow.window == window)
		{
		  fprintf (stderr, "window 0x%x unexpectedly destroyed.\n",
			   (unsigned int) window);
		  exit (6);
		}
	      else if (event.xany.type == PropertyNotify &&
		       event.xproperty.state == PropertyDelete &&
		       event.xproperty.window == window &&
		       event.xproperty.atom == XA_XEXEC_LOCK)
		{
		  /* Ok!  Someone deleted their lock, so now we can try
		     again. */
#ifdef DEBUG_PROPS
		  fprintf (stderr, "(0x%x unlocked, trying again...)\n",
			   (unsigned int) window);
#endif
		  break;
		}
	    }
	}
      if (data)
	XFree (data);
    }
  while (! locked);

  if (waited)
    fprintf (stderr, "obtained lock.\n");
}
