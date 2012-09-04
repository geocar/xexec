CFLAGS=-I/usr/X11R6/include
LDFLAGS=-L/usr/X11R6/lib -lXmu -lX11

all: xlisten xexec
xlisten: xlisten.o init_atoms.o setup_display.o find_window.o
xexec: init_atoms.o find_window.o obtain_lock.o free_lock.o send_command.o \
		setup_display.o xexec.o

install: xlisten xexec
	-[ -w /usr/local/bin ] && install -m 0755 xlisten xexec /usr/local/bin/.
	-[ ! -w /usr/local/bin ] && install -m 0755 xlisten xexec $$HOME/bin/.

clean:;rm -f *.o xlisten xexec
