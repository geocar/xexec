#include "atoms.h"
#include "lock.h"

#include <stdio.h>

void free_lock(Display *dpy, Window window)
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
  int result;
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *data = 0;

#ifdef DEBUG_PROPS
	  fprintf (stderr, "(deleting " MOZILLA_LOCK_PROP
		   " \"%s\" from 0x%x)\n",
		   lock_data, (unsigned int) window);
#endif

  result = XGetWindowProperty (dpy, window, XA_XEXEC_LOCK,
			       0, (65536 / sizeof (long)),
			       True, /* atomic delete after */
			       XA_STRING,
			       &actual_type, &actual_format,
			       &nitems, &bytes_after,
			       &data);
  if (result != Success)
    {
      fprintf (stderr, "unable to read and delete lock property\n");
      return;
    }
  else if (!data || !*data)
    {
      fprintf (stderr, "invalid data on lock property of window 0x%x.\n",
	       (unsigned int) window);
      return;
    }
  else if (strcmp ((char *) data, lock_data))
    {
      fprintf (stderr, "lock was stolen!  Expected \"%s\", saw \"%s\"!\n",
	       lock_data, data);
      return;
    }

  if (data)
    XFree (data);
}
