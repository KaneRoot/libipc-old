PACKAGE = 'ipc-examples'
VERSION = '0.5.0'

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
SHAREDIR := $(PREFIX)/share
INCLUDEDIR := $(PREFIX)/include
MANDIR := $(SHAREDIR)/man
CC := cc
CXX := c++
LD := ${CC}
CFLAGS := -Wall -Wextra -g
LDFLAGS := -I../src -L../ ../src/ipc.h -lipc

Q := @

all: fs-function-tests fd-exchange-providing fd-exchange-receiving fs-experimentations pong pongd pongspam simple-tcp-client simple-tcpd test-ask-for-fd-to-networkd test-networkd-provide-fd wsserver
	@:

fs-function-tests: fs-function-tests.o  
	@echo '[01;32m  LD >    [01;37mfs-function-tests[00m'
	$(Q)$(CC) -o fs-function-tests fs-function-tests.o $(LDFLAGS)
fs-function-tests.install: fs-function-tests
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/fs-function-tests[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 fs-function-tests $(DESTDIR)$(BINDIR)/fs-function-tests

fs-function-tests.clean:  fs-function-tests.o.clean
	@echo '[01;37m  RM >    [01;37mfs-function-tests[00m'
	$(Q)rm -f fs-function-tests

fs-function-tests.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/fs-function-tests[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/fs-function-tests'

fd-exchange-providing: fd-exchange-providing.o  
	@echo '[01;32m  LD >    [01;37mfd-exchange-providing[00m'
	$(Q)$(CC) -o fd-exchange-providing fd-exchange-providing.o $(LDFLAGS)
fd-exchange-providing.install: fd-exchange-providing
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/fd-exchange-providing[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 fd-exchange-providing $(DESTDIR)$(BINDIR)/fd-exchange-providing

fd-exchange-providing.clean:  fd-exchange-providing.o.clean
	@echo '[01;37m  RM >    [01;37mfd-exchange-providing[00m'
	$(Q)rm -f fd-exchange-providing

fd-exchange-providing.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/fd-exchange-providing[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/fd-exchange-providing'

fd-exchange-receiving: fd-exchange-receiving.o  
	@echo '[01;32m  LD >    [01;37mfd-exchange-receiving[00m'
	$(Q)$(CC) -o fd-exchange-receiving fd-exchange-receiving.o $(LDFLAGS)
fd-exchange-receiving.install: fd-exchange-receiving
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/fd-exchange-receiving[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 fd-exchange-receiving $(DESTDIR)$(BINDIR)/fd-exchange-receiving

fd-exchange-receiving.clean:  fd-exchange-receiving.o.clean
	@echo '[01;37m  RM >    [01;37mfd-exchange-receiving[00m'
	$(Q)rm -f fd-exchange-receiving

fd-exchange-receiving.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/fd-exchange-receiving[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/fd-exchange-receiving'

fs-experimentations: fs-experimentations.o  
	@echo '[01;32m  LD >    [01;37mfs-experimentations[00m'
	$(Q)$(CC) -o fs-experimentations fs-experimentations.o $(LDFLAGS)
fs-experimentations.install: fs-experimentations
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/fs-experimentations[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 fs-experimentations $(DESTDIR)$(BINDIR)/fs-experimentations

fs-experimentations.clean:  fs-experimentations.o.clean
	@echo '[01;37m  RM >    [01;37mfs-experimentations[00m'
	$(Q)rm -f fs-experimentations

fs-experimentations.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/fs-experimentations[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/fs-experimentations'

pong: pong.o  
	@echo '[01;32m  LD >    [01;37mpong[00m'
	$(Q)$(CC) -o pong pong.o $(LDFLAGS)
pong.install: pong
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/pong[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 pong $(DESTDIR)$(BINDIR)/pong

pong.clean:  pong.o.clean
	@echo '[01;37m  RM >    [01;37mpong[00m'
	$(Q)rm -f pong

pong.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/pong[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/pong'

pongd: pongd.o  
	@echo '[01;32m  LD >    [01;37mpongd[00m'
	$(Q)$(CC) -o pongd pongd.o $(LDFLAGS)
pongd.install: pongd
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/pongd[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 pongd $(DESTDIR)$(BINDIR)/pongd

pongd.clean:  pongd.o.clean
	@echo '[01;37m  RM >    [01;37mpongd[00m'
	$(Q)rm -f pongd

pongd.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/pongd[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/pongd'

pongspam: pongspam.o  
	@echo '[01;32m  LD >    [01;37mpongspam[00m'
	$(Q)$(CC) -o pongspam pongspam.o $(LDFLAGS)
pongspam.install: pongspam
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/pongspam[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 pongspam $(DESTDIR)$(BINDIR)/pongspam

pongspam.clean:  pongspam.o.clean
	@echo '[01;37m  RM >    [01;37mpongspam[00m'
	$(Q)rm -f pongspam

pongspam.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/pongspam[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/pongspam'

simple-tcp-client: simple-tcp-client.o  
	@echo '[01;32m  LD >    [01;37msimple-tcp-client[00m'
	$(Q)$(CC) -o simple-tcp-client simple-tcp-client.o $(LDFLAGS)
simple-tcp-client.install: simple-tcp-client
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/simple-tcp-client[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 simple-tcp-client $(DESTDIR)$(BINDIR)/simple-tcp-client

simple-tcp-client.clean:  simple-tcp-client.o.clean
	@echo '[01;37m  RM >    [01;37msimple-tcp-client[00m'
	$(Q)rm -f simple-tcp-client

simple-tcp-client.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/simple-tcp-client[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/simple-tcp-client'

simple-tcpd: simple-tcpd.o  
	@echo '[01;32m  LD >    [01;37msimple-tcpd[00m'
	$(Q)$(CC) -o simple-tcpd simple-tcpd.o $(LDFLAGS)
simple-tcpd.install: simple-tcpd
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/simple-tcpd[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 simple-tcpd $(DESTDIR)$(BINDIR)/simple-tcpd

simple-tcpd.clean:  simple-tcpd.o.clean
	@echo '[01;37m  RM >    [01;37msimple-tcpd[00m'
	$(Q)rm -f simple-tcpd

simple-tcpd.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/simple-tcpd[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/simple-tcpd'

test-ask-for-fd-to-networkd: test-ask-for-fd-to-networkd.o  
	@echo '[01;32m  LD >    [01;37mtest-ask-for-fd-to-networkd[00m'
	$(Q)$(CC) -o test-ask-for-fd-to-networkd test-ask-for-fd-to-networkd.o $(LDFLAGS)
test-ask-for-fd-to-networkd.install: test-ask-for-fd-to-networkd
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/test-ask-for-fd-to-networkd[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 test-ask-for-fd-to-networkd $(DESTDIR)$(BINDIR)/test-ask-for-fd-to-networkd

test-ask-for-fd-to-networkd.clean:  test-ask-for-fd-to-networkd.o.clean
	@echo '[01;37m  RM >    [01;37mtest-ask-for-fd-to-networkd[00m'
	$(Q)rm -f test-ask-for-fd-to-networkd

test-ask-for-fd-to-networkd.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/test-ask-for-fd-to-networkd[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/test-ask-for-fd-to-networkd'

test-networkd-provide-fd: test-networkd-provide-fd.o  
	@echo '[01;32m  LD >    [01;37mtest-networkd-provide-fd[00m'
	$(Q)$(CC) -o test-networkd-provide-fd test-networkd-provide-fd.o $(LDFLAGS)
test-networkd-provide-fd.install: test-networkd-provide-fd
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/test-networkd-provide-fd[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 test-networkd-provide-fd $(DESTDIR)$(BINDIR)/test-networkd-provide-fd

test-networkd-provide-fd.clean:  test-networkd-provide-fd.o.clean
	@echo '[01;37m  RM >    [01;37mtest-networkd-provide-fd[00m'
	$(Q)rm -f test-networkd-provide-fd

test-networkd-provide-fd.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/test-networkd-provide-fd[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/test-networkd-provide-fd'

wsserver: wsserver.o  
	@echo '[01;32m  LD >    [01;37mwsserver[00m'
	$(Q)$(CC) -o wsserver wsserver.o $(LDFLAGS)
wsserver.install: wsserver
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/wsserver[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 wsserver $(DESTDIR)$(BINDIR)/wsserver

wsserver.clean:  wsserver.o.clean
	@echo '[01;37m  RM >    [01;37mwsserver[00m'
	$(Q)rm -f wsserver

wsserver.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/wsserver[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/wsserver'

fs-function-tests.o: fs-function-tests.c  ./../src/ipc.h ./../src/fs.h
	@echo '[01;34m  CC >    [01;37mfs-function-tests.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c fs-function-tests.c  -std=c11 -o fs-function-tests.o

fs-function-tests.o.install:

fs-function-tests.o.clean:
	@echo '[01;37m  RM >    [01;37mfs-function-tests.o[00m'
	$(Q)rm -f fs-function-tests.o

fs-function-tests.o.uninstall:

fd-exchange-providing.o: fd-exchange-providing.c  ./../src/ipc.h ./../src/usocket.h
	@echo '[01;34m  CC >    [01;37mfd-exchange-providing.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c fd-exchange-providing.c  -std=c11 -o fd-exchange-providing.o

fd-exchange-providing.o.install:

fd-exchange-providing.o.clean:
	@echo '[01;37m  RM >    [01;37mfd-exchange-providing.o[00m'
	$(Q)rm -f fd-exchange-providing.o

fd-exchange-providing.o.uninstall:

fd-exchange-receiving.o: fd-exchange-receiving.c  ./../src/ipc.h ./../src/usocket.h
	@echo '[01;34m  CC >    [01;37mfd-exchange-receiving.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c fd-exchange-receiving.c  -std=c11 -o fd-exchange-receiving.o

fd-exchange-receiving.o.install:

fd-exchange-receiving.o.clean:
	@echo '[01;37m  RM >    [01;37mfd-exchange-receiving.o[00m'
	$(Q)rm -f fd-exchange-receiving.o

fd-exchange-receiving.o.uninstall:

fs-experimentations.o: fs-experimentations.c 
	@echo '[01;34m  CC >    [01;37mfs-experimentations.o[00m'
	$(Q)$(CC) $(CFLAGS) -c fs-experimentations.c -o fs-experimentations.o

fs-experimentations.o.install:

fs-experimentations.o.clean:
	@echo '[01;37m  RM >    [01;37mfs-experimentations.o[00m'
	$(Q)rm -f fs-experimentations.o

fs-experimentations.o.uninstall:

pong.o: pong.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mpong.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c pong.c  -std=c11 -o pong.o

pong.o.install:

pong.o.clean:
	@echo '[01;37m  RM >    [01;37mpong.o[00m'
	$(Q)rm -f pong.o

pong.o.uninstall:

pongd.o: pongd.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mpongd.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c pongd.c  -std=c11 -o pongd.o

pongd.o.install:

pongd.o.clean:
	@echo '[01;37m  RM >    [01;37mpongd.o[00m'
	$(Q)rm -f pongd.o

pongd.o.uninstall:

pongspam.o: pongspam.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mpongspam.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c pongspam.c  -std=c11 -o pongspam.o

pongspam.o.install:

pongspam.o.clean:
	@echo '[01;37m  RM >    [01;37mpongspam.o[00m'
	$(Q)rm -f pongspam.o

pongspam.o.uninstall:

simple-tcp-client.o: simple-tcp-client.c  ./../src/ipc.h ./../src/utils.h
	@echo '[01;34m  CC >    [01;37msimple-tcp-client.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c simple-tcp-client.c  -std=c11 -o simple-tcp-client.o

simple-tcp-client.o.install:

simple-tcp-client.o.clean:
	@echo '[01;37m  RM >    [01;37msimple-tcp-client.o[00m'
	$(Q)rm -f simple-tcp-client.o

simple-tcp-client.o.uninstall:

simple-tcpd.o: simple-tcpd.c  ./../src/ipc.h ./../src/utils.h
	@echo '[01;34m  CC >    [01;37msimple-tcpd.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c simple-tcpd.c  -std=c11 -o simple-tcpd.o

simple-tcpd.o.install:

simple-tcpd.o.clean:
	@echo '[01;37m  RM >    [01;37msimple-tcpd.o[00m'
	$(Q)rm -f simple-tcpd.o

simple-tcpd.o.uninstall:

test-ask-for-fd-to-networkd.o: test-ask-for-fd-to-networkd.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mtest-ask-for-fd-to-networkd.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c test-ask-for-fd-to-networkd.c  -std=c11 -o test-ask-for-fd-to-networkd.o

test-ask-for-fd-to-networkd.o.install:

test-ask-for-fd-to-networkd.o.clean:
	@echo '[01;37m  RM >    [01;37mtest-ask-for-fd-to-networkd.o[00m'
	$(Q)rm -f test-ask-for-fd-to-networkd.o

test-ask-for-fd-to-networkd.o.uninstall:

test-networkd-provide-fd.o: test-networkd-provide-fd.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mtest-networkd-provide-fd.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c test-networkd-provide-fd.c  -std=c11 -o test-networkd-provide-fd.o

test-networkd-provide-fd.o.install:

test-networkd-provide-fd.o.clean:
	@echo '[01;37m  RM >    [01;37mtest-networkd-provide-fd.o[00m'
	$(Q)rm -f test-networkd-provide-fd.o

test-networkd-provide-fd.o.uninstall:

wsserver.o: wsserver.c  ./../src/ipc.h ./../src/utils.h
	@echo '[01;34m  CC >    [01;37mwsserver.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c wsserver.c  -std=c11 -o wsserver.o

wsserver.o.install:

wsserver.o.clean:
	@echo '[01;37m  RM >    [01;37mwsserver.o[00m'
	$(Q)rm -f wsserver.o

wsserver.o.uninstall:

$(DESTDIR)$(PREFIX):
	@echo '[01;35m  DIR >   [01;37m$(PREFIX)[00m'
	$(Q)mkdir -p $(DESTDIR)$(PREFIX)
$(DESTDIR)$(BINDIR):
	@echo '[01;35m  DIR >   [01;37m$(BINDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(BINDIR)
$(DESTDIR)$(LIBDIR):
	@echo '[01;35m  DIR >   [01;37m$(LIBDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(LIBDIR)
$(DESTDIR)$(SHAREDIR):
	@echo '[01;35m  DIR >   [01;37m$(SHAREDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(SHAREDIR)
$(DESTDIR)$(INCLUDEDIR):
	@echo '[01;35m  DIR >   [01;37m$(INCLUDEDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(INCLUDEDIR)
$(DESTDIR)$(MANDIR):
	@echo '[01;35m  DIR >   [01;37m$(MANDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(MANDIR)
install: fs-function-tests.install fd-exchange-providing.install fd-exchange-receiving.install fs-experimentations.install pong.install pongd.install pongspam.install simple-tcp-client.install simple-tcpd.install test-ask-for-fd-to-networkd.install test-networkd-provide-fd.install wsserver.install fs-function-tests.o.install fd-exchange-providing.o.install fd-exchange-receiving.o.install fs-experimentations.o.install pong.o.install pongd.o.install pongspam.o.install simple-tcp-client.o.install simple-tcpd.o.install test-ask-for-fd-to-networkd.o.install test-networkd-provide-fd.o.install wsserver.o.install
	@:

uninstall: fs-function-tests.uninstall fd-exchange-providing.uninstall fd-exchange-receiving.uninstall fs-experimentations.uninstall pong.uninstall pongd.uninstall pongspam.uninstall simple-tcp-client.uninstall simple-tcpd.uninstall test-ask-for-fd-to-networkd.uninstall test-networkd-provide-fd.uninstall wsserver.uninstall fs-function-tests.o.uninstall fd-exchange-providing.o.uninstall fd-exchange-receiving.o.uninstall fs-experimentations.o.uninstall pong.o.uninstall pongd.o.uninstall pongspam.o.uninstall simple-tcp-client.o.uninstall simple-tcpd.o.uninstall test-ask-for-fd-to-networkd.o.uninstall test-networkd-provide-fd.o.uninstall wsserver.o.uninstall
	@:

clean: fs-function-tests.clean fd-exchange-providing.clean fd-exchange-receiving.clean fs-experimentations.clean pong.clean pongd.clean pongspam.clean simple-tcp-client.clean simple-tcpd.clean test-ask-for-fd-to-networkd.clean test-networkd-provide-fd.clean wsserver.clean fs-function-tests.o.clean fd-exchange-providing.o.clean fd-exchange-receiving.o.clean fs-experimentations.o.clean pong.o.clean pongd.o.clean pongspam.o.clean simple-tcp-client.o.clean simple-tcpd.o.clean test-ask-for-fd-to-networkd.o.clean test-networkd-provide-fd.o.clean wsserver.o.clean
distclean: clean
dist: dist-gz dist-xz dist-bz2
	$(Q)rm -- $(PACKAGE)-$(VERSION)

distdir:
	$(Q)rm -rf -- $(PACKAGE)-$(VERSION)
	$(Q)ln -s -- . $(PACKAGE)-$(VERSION)

dist-gz: $(PACKAGE)-$(VERSION).tar.gz
$(PACKAGE)-$(VERSION).tar.gz: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.gz[00m'
	$(Q)tar czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/test-networkd-provide-fd.c \
		$(PACKAGE)-$(VERSION)/test-ask-for-fd-to-networkd.c \
		$(PACKAGE)-$(VERSION)/fd-exchange-receiving.c \
		$(PACKAGE)-$(VERSION)/fs-function-tests.c \
		$(PACKAGE)-$(VERSION)/pongd.c \
		$(PACKAGE)-$(VERSION)/wsserver.c \
		$(PACKAGE)-$(VERSION)/pong.c \
		$(PACKAGE)-$(VERSION)/fd-exchange-providing.c \
		$(PACKAGE)-$(VERSION)/fs-experimentations.c \
		$(PACKAGE)-$(VERSION)/pongspam.c \
		$(PACKAGE)-$(VERSION)/simple-tcpd.c \
		$(PACKAGE)-$(VERSION)/simple-tcp-client.c

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.xz[00m'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/test-networkd-provide-fd.c \
		$(PACKAGE)-$(VERSION)/test-ask-for-fd-to-networkd.c \
		$(PACKAGE)-$(VERSION)/fd-exchange-receiving.c \
		$(PACKAGE)-$(VERSION)/fs-function-tests.c \
		$(PACKAGE)-$(VERSION)/pongd.c \
		$(PACKAGE)-$(VERSION)/wsserver.c \
		$(PACKAGE)-$(VERSION)/pong.c \
		$(PACKAGE)-$(VERSION)/fd-exchange-providing.c \
		$(PACKAGE)-$(VERSION)/fs-experimentations.c \
		$(PACKAGE)-$(VERSION)/pongspam.c \
		$(PACKAGE)-$(VERSION)/simple-tcpd.c \
		$(PACKAGE)-$(VERSION)/simple-tcp-client.c

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.bz2[00m'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/test-networkd-provide-fd.c \
		$(PACKAGE)-$(VERSION)/test-ask-for-fd-to-networkd.c \
		$(PACKAGE)-$(VERSION)/fd-exchange-receiving.c \
		$(PACKAGE)-$(VERSION)/fs-function-tests.c \
		$(PACKAGE)-$(VERSION)/pongd.c \
		$(PACKAGE)-$(VERSION)/wsserver.c \
		$(PACKAGE)-$(VERSION)/pong.c \
		$(PACKAGE)-$(VERSION)/fd-exchange-providing.c \
		$(PACKAGE)-$(VERSION)/fs-experimentations.c \
		$(PACKAGE)-$(VERSION)/pongspam.c \
		$(PACKAGE)-$(VERSION)/simple-tcpd.c \
		$(PACKAGE)-$(VERSION)/simple-tcp-client.c

help:
	@echo '[01;37m :: ipc-examples-0.5.0[00m'
	@echo ''
	@echo '[01;37mGeneric targets:[00m'
	@echo '[00m    - [01;32mhelp          [37m Prints this help message.[00m'
	@echo '[00m    - [01;32mall           [37m Builds all targets.[00m'
	@echo '[00m    - [01;32mdist          [37m Creates tarballs of the files of the project.[00m'
	@echo '[00m    - [01;32minstall       [37m Installs the project.[00m'
	@echo '[00m    - [01;32mclean         [37m Removes compiled files.[00m'
	@echo '[00m    - [01;32muninstall     [37m Deinstalls the project.[00m'
	@echo ''
	@echo '[01;37mCLI-modifiable variables:[00m'
	@echo '    - [01;34mCC            [37m ${CC}[00m'
	@echo '    - [01;34mCXX           [37m ${CXX}[00m'
	@echo '    - [01;34mLD            [37m ${LD}[00m'
	@echo '    - [01;34mCFLAGS        [37m ${CFLAGS}[00m'
	@echo '    - [01;34mCXXFLAGS      [37m ${CXXFLAGS}[00m'
	@echo '    - [01;34mLDFLAGS       [37m ${LDFLAGS}[00m'
	@echo '    - [01;34mPREFIX        [37m ${PREFIX}[00m'
	@echo '    - [01;34mBINDIR        [37m ${BINDIR}[00m'
	@echo '    - [01;34mLIBDIR        [37m ${LIBDIR}[00m'
	@echo '    - [01;34mSHAREDIR      [37m ${SHAREDIR}[00m'
	@echo '    - [01;34mINCLUDEDIR    [37m ${INCLUDEDIR}[00m'
	@echo '    - [01;34mMANDIR        [37m ${MANDIR}[00m'
	@echo ''
	@echo '[01;37mProject targets: [00m'
	@echo '    - [01;33mfs-function-tests             [37m binary[00m'
	@echo '    - [01;33mfd-exchange-providing         [37m binary[00m'
	@echo '    - [01;33mfd-exchange-receiving         [37m binary[00m'
	@echo '    - [01;33mfs-experimentations           [37m binary[00m'
	@echo '    - [01;33mpong                          [37m binary[00m'
	@echo '    - [01;33mpongd                         [37m binary[00m'
	@echo '    - [01;33mpongspam                      [37m binary[00m'
	@echo '    - [01;33msimple-tcp-client             [37m binary[00m'
	@echo '    - [01;33msimple-tcpd                   [37m binary[00m'
	@echo '    - [01;33mtest-ask-for-fd-to-networkd   [37m binary[00m'
	@echo '    - [01;33mtest-networkd-provide-fd      [37m binary[00m'
	@echo '    - [01;33mwsserver                      [37m binary[00m'
	@echo ''
	@echo '[01;37mMakefile options:[00m'
	@echo '    - gnu:           false'
	@echo '    - colors:        true'
	@echo ''
	@echo '[01;37mRebuild the Makefile with:[00m'
	@echo '    zsh ./build.zsh -c'
.PHONY: all  clean distclean dist install uninstall help

