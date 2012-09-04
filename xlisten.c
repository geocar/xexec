#include "atoms.h"
#include "vroot.h"

#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>

#include "display.h"
#include "version.h"

static int doit(unsigned char **res, unsigned int *len,
		const char *exec, const char **argv,
		unsigned char *data, unsigned int datalen)
{
	char buffer[32768];
	pid_t p;
	int fds[2];
	char *e;
	unsigned int nl;
	int r;

	if (pipe(fds) == -1) {
		perror("pipe");
		exit(255);
	}

	p = fork();
	if (p == -1) {
		perror("fork");
		exit(255);
	}
	if (p == 0) {
		e = (char*)malloc(datalen+5);
		if (!e) {
			perror("malloc");
			exit(255);
		}
		(void)close(fds[0]);
		if (fds[1] != 1) {
			(void)close(1);
			if (dup(fds[1]) != 1) {
				if (dup(fds[1]) != 1) {
					/* eh? */
					perror("dup");
					exit(255);
				}
			}
		}
		(void)close(0); /* no stdin */
		if (fds[1] != 1) {
			(void)close(fds[1]); /* no fd leak */
		}
		e[0] = 'C'; e[1]='M'; e[2]='D';e[3]='=';
		e[datalen+4]=0;
		memcpy(e+4,data,datalen);
		if (putenv(e) == -1) {
			perror("putenv");
			exit(255);
		}
		execvp(exec, argv);
		perror("execvp");
		exit(255);
	}
	(void)close(fds[1]);
	e = 0; nl = 0;
	for (;;) {
		do {
			r = read(fds[0], buffer, sizeof(buffer));
		} while (r == -1 && errno == EINTR);
		if (r > 0) {
			e = (char*)realloc(e, nl+r);
			if (!e) {
				perror("realloc");
				exit(255);
			}
			memcpy(e+nl,buffer,r);
			nl+=r;
		} else {
			break;
		}
	}
	while (wait(&r) != p);
	if (WIFEXITED(r) && WEXITSTATUS(r) == 0) {
		if (res)*res=e;
		if (len)*len=nl;
		return 1;
	} else {
		return 0;
	}
}

int main(int argc, const char **argv)
{
	XEvent ev;
	Display *dpy;
	Window win;
	int result;
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned char *res;
	unsigned int reslen;
	char *data = 0;
	int i;

	if (argc < 3) {
		fprintf(stderr, "Usage: xlisten ident exec...\n");
		exit(255);
	}
	dpy = setup_display();
	init_atoms(dpy, argv[1]);
	if (find_window(dpy)) {
		fprintf(stderr, "Already running\n");
		exit(255);
	}
	/* create window */
	win=XCreateWindow(dpy,RootWindowOfScreen(DefaultScreenOfDisplay(dpy)),
			0,0,1,1,0,CopyFromParent,InputOutput,
			CopyFromParent,0,0);
	if (!win) {
		perror("XCreateWindow");
		exit(255);
	}
	XChangeProperty(dpy,win,XA_XEXEC_VERSION,XA_STRING,8,PropModeReplace,
			xexec_version, sizeof(xexec_version)-1);
	memset(&ev,0,sizeof(ev));
	XSelectInput(dpy,win,(PropertyChangeMask|StructureNotifyMask));
	while (!XNextEvent(dpy,&ev)) {
		result = XGetWindowProperty(dpy,win,XA_XEXEC_COMMAND,
				0, (65536/sizeof(long)),
				True,
				XA_STRING,
				&actual_type, &actual_format,
				&nitems, &bytes_after,
				(unsigned char **)&data);
		if (result != Success || !data) continue;

		if (!doit(&res,&reslen,argv[2],argv+2,data,nitems)) {
			res = "";
			reslen = 0;
		} else {
			XChangeProperty(dpy,win,XA_XEXEC_RESPONSE,XA_STRING,
				8,PropModeReplace,
				res,reslen);
		}
	}
	puts("eh?");

}
