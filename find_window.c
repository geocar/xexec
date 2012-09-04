#include "atoms.h"

#include <stdio.h>

#include "vroot.h"

#include "version.h"

static const char *expected_version = xexec_version;

/* must have called init_atoms() appropriately */
Window find_window(Display *dpy)
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
  int i;
  Window root = RootWindowOfScreen (DefaultScreenOfDisplay (dpy));
  Window root2, parent, *kids;
  unsigned int nkids;
  Window result = 0;
  Window tenative = 0;
  unsigned char *tenative_version = 0;

  if (! XQueryTree (dpy, root, &root2, &parent, &kids, &nkids))
    {
      fprintf (stderr, "XQueryTree failed on display %s\n",
	       DisplayString (dpy));
      exit (2);
    }

  /* root != root2 is possible with virtual root WMs. */

  if (! (kids && nkids))
    {
      fprintf (stderr, "root window has no children on display %s\n",
	       DisplayString (dpy));
      exit (2);
    }

  for (i = nkids-1; i >= 0; i--)
    {
      Atom type;
      int format;
      unsigned long nitems, bytesafter;
      unsigned char *version = 0;
      Window w = XmuClientWindow (dpy, kids[i]);
      int status = XGetWindowProperty (dpy, w, XA_XEXEC_VERSION,
				       0, (65536 / sizeof (long)),
				       False, XA_STRING,
				       &type, &format, &nitems, &bytesafter,
				       &version);
      if (! version)
	continue;
      if (strcmp ((char *) version, expected_version) &&
	  !tenative)
	{
	  tenative = w;
	  tenative_version = version;
	  continue;
	}
      XFree (version);
      if (status == Success && type != None)
	{
	  result = w;
	  break;
	}
    }

  if (result && tenative)
    {
      fprintf (stderr,
	       "warning: both version %s (0x%x) and version\n"
	       "\t%s (0x%x) are running.  Using version %s.\n",
	       tenative_version, (unsigned int) tenative,
	       expected_version, (unsigned int) result,
	       expected_version);
      XFree (tenative_version);
      return result;
    }
  else if (tenative)
    {
      fprintf (stderr,
	       "warning: expected version %s but found version\n"
	       "\t%s (0x%x) instead.\n",
	       expected_version,
	       tenative_version, (unsigned int) tenative);
      XFree (tenative_version);
      return tenative;
    }
  else if (result)
    {
      return result;
    }
  else
    {
      return 0;
    }
}
