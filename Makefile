PACKAGE = 'ipc'
VERSION = '0.4.0'

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
SHAREDIR := $(PREFIX)/share
INCLUDEDIR := $(PREFIX)/include
MANDIR := $(SHAREDIR)/man
CFLAGS := -Wall -Wextra -g
CC := cc
CXX := c++
LD := ${CC}
CXXFLAGS :=  
LDFLAGS :=  

Q := @

all: libipc src/ipc.h man/libipc.7
	@:

libipc: libipc.so libipc.a 
	@:
libipc.install: libipc.so.install libipc.a.install

libipc.clean: libipc.so.clean libipc.a.clean

libipc.uninstall: libipc.so.uninstall libipc.a.uninstall

src/ipc.h.install: src/ipc.h src
	@echo '[01;31m  IN >    [01;37m$(INCLUDEDIR)/ipc.h[00m'
	$(Q)mkdir -p '$(DESTDIR)$(INCLUDEDIR)'
	$(Q)install -m0644 src/ipc.h $(DESTDIR)$(INCLUDEDIR)/ipc.h

src/ipc.h.clean: src/ipc.h
	$(Q):

src/ipc.h.uninstall:
	@echo '[01;37m  RM >    [01;37m$(INCLUDEDIR)/ipc.h[00m'
	$(Q)rm -f '$(DESTDIR)$(INCLUDEDIR)/ipc.h'

man/libipc.7: man/libipc.7.scd man
	@echo '[01;33m  MAN >   [01;37mman/libipc.7[00m'
	$(Q)scdoc < 'man/libipc.7.scd' > 'man/libipc.7'


man/libipc.7.install: man/libipc.7
	@echo '[01;31m  IN >    [01;37m$(MANDIR)/man7/libipc.7[00m'
	$(Q)mkdir -p '$(DESTDIR)$(MANDIR)/man7'
	$(Q)install -m0644 man/libipc.7 $(DESTDIR)$(MANDIR)/man7/libipc.7

man/libipc.7.clean:
	@echo '[01;37m  RM >    [01;37mman/libipc.7[00m'
	$(Q)rm -f man/libipc.7

man/libipc.7.uninstall:
	@echo '[01;37m  RM >    [01;37m$(MANDIR)/man7/libipc.7[00m'
	$(Q)rm -f '$(DESTDIR)$(MANDIR)/man7/libipc.7'

libipc.so: src/communication.o src/error.o src/logger.o src/message.o src/network.o src/usocket.o src/utils.o  
	@echo '[01;32m  LD >    [01;37mlibipc.so[00m'
	$(Q)$(CC) -o libipc.so -shared $(LDFLAGS) src/communication.o src/error.o src/logger.o src/message.o src/network.o src/usocket.o src/utils.o 

libipc.so.install: libipc.so
	@echo '[01;31m  IN >    [01;37m$(LIBDIR)/libipc.so.0.4.0[00m'
	$(Q)mkdir -p '$(DESTDIR)$(LIBDIR)'
	$(Q)install -m0755 libipc.so $(DESTDIR)$(LIBDIR)/libipc.so.0.4.0
	@echo '[01;35m  LN >    [01;37m$(LIBDIR)/libipc.so.0.4[00m'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.4.0' '$(DESTDIR)/$(LIBDIR)/libipc.so.0.4'
	@echo '[01;35m  LN >    [01;37m$(LIBDIR)/libipc.so.0[00m'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.4.0' '$(DESTDIR)/$(LIBDIR)/libipc.so.0'
	@echo '[01;35m  LN >    [01;37m$(LIBDIR)/libipc.so[00m'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.4.0' '$(DESTDIR)/$(LIBDIR)/libipc.so'

libipc.so.clean:
	@echo '[01;37m  RM >    [01;37mlibipc.so[00m'
	$(Q)rm -f libipc.so

libipc.so.uninstall:
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.so.0.4.0[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0.4.0'
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.so.0.4[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0.4'
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.so.0[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0'
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.so[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so'

libipc.a: src/communication.o src/error.o src/logger.o src/message.o src/network.o src/usocket.o src/utils.o  
	@echo '[01;32m  LD >    [01;37mlibipc.a[00m'
	$(Q)$(AR) rc 'libipc.a' src/communication.o src/error.o src/logger.o src/message.o src/network.o src/usocket.o src/utils.o

libipc.a.install: libipc.a
	@echo '[01;31m  IN >    [01;37m$(LIBDIR)/libipc.a[00m'
	$(Q)mkdir -p '$(DESTDIR)$(LIBDIR)'
	$(Q)install -m0755 libipc.a $(DESTDIR)$(LIBDIR)/libipc.a

libipc.a.clean:
	@echo '[01;37m  RM >    [01;37mlibipc.a[00m'
	$(Q)rm -f libipc.a

libipc.a.uninstall:
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.a[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.a'

src/communication.o: src/communication.c  src/ipc.h src/utils.h src/message.h
	@echo '[01;34m  CC >    [01;37msrc/communication.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC -std=c11 -c src/communication.c  -fPIC -std=c11 -o src/communication.o

src/communication.o.install:

src/communication.o.clean:
	@echo '[01;37m  RM >    [01;37msrc/communication.o[00m'
	$(Q)rm -f src/communication.o

src/communication.o.uninstall:

src/error.o: src/error.c  src/ipc.h
	@echo '[01;34m  CC >    [01;37msrc/error.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC -std=c11 -c src/error.c  -fPIC -std=c11 -o src/error.o

src/error.o.install:

src/error.o.clean:
	@echo '[01;37m  RM >    [01;37msrc/error.o[00m'
	$(Q)rm -f src/error.o

src/error.o.uninstall:

src/logger.o: src/logger.c  src/logger.h src/ipc.h
	@echo '[01;34m  CC >    [01;37msrc/logger.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC -std=c11 -c src/logger.c  -fPIC -std=c11 -o src/logger.o

src/logger.o.install:

src/logger.o.clean:
	@echo '[01;37m  RM >    [01;37msrc/logger.o[00m'
	$(Q)rm -f src/logger.o

src/logger.o.uninstall:

src/message.o: src/message.c  src/message.h src/usocket.h
	@echo '[01;34m  CC >    [01;37msrc/message.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC -std=c11 -c src/message.c  -fPIC -std=c11 -o src/message.o

src/message.o.install:

src/message.o.clean:
	@echo '[01;37m  RM >    [01;37msrc/message.o[00m'
	$(Q)rm -f src/message.o

src/message.o.uninstall:

src/network.o: src/network.c  src/ipc.h
	@echo '[01;34m  CC >    [01;37msrc/network.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC -std=c11 -c src/network.c  -fPIC -std=c11 -o src/network.o

src/network.o.install:

src/network.o.clean:
	@echo '[01;37m  RM >    [01;37msrc/network.o[00m'
	$(Q)rm -f src/network.o

src/network.o.uninstall:

src/usocket.o: src/usocket.c  src/usocket.h src/utils.h
	@echo '[01;34m  CC >    [01;37msrc/usocket.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC -std=c11 -c src/usocket.c  -fPIC -std=c11 -o src/usocket.o

src/usocket.o.install:

src/usocket.o.clean:
	@echo '[01;37m  RM >    [01;37msrc/usocket.o[00m'
	$(Q)rm -f src/usocket.o

src/usocket.o.uninstall:

src/utils.o: src/utils.c  src/utils.h src/ipc.h
	@echo '[01;34m  CC >    [01;37msrc/utils.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC -std=c11 -c src/utils.c  -fPIC -std=c11 -o src/utils.o

src/utils.o.install:

src/utils.o.clean:
	@echo '[01;37m  RM >    [01;37msrc/utils.o[00m'
	$(Q)rm -f src/utils.o

src/utils.o.uninstall:

src:
	$(Q)mkdir -p src
man:
	$(Q)mkdir -p man
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
install: libipc.install src/ipc.h.install man/libipc.7.install libipc.so.install libipc.a.install src/communication.o.install src/error.o.install src/logger.o.install src/message.o.install src/network.o.install src/usocket.o.install src/utils.o.install src/communication.o.install src/error.o.install src/logger.o.install src/message.o.install src/network.o.install src/usocket.o.install src/utils.o.install
	@:

uninstall: libipc.uninstall src/ipc.h.uninstall man/libipc.7.uninstall libipc.so.uninstall libipc.a.uninstall src/communication.o.uninstall src/error.o.uninstall src/logger.o.uninstall src/message.o.uninstall src/network.o.uninstall src/usocket.o.uninstall src/utils.o.uninstall src/communication.o.uninstall src/error.o.uninstall src/logger.o.uninstall src/message.o.uninstall src/network.o.uninstall src/usocket.o.uninstall src/utils.o.uninstall
	@:

test: all
	@:

clean: libipc.clean src/ipc.h.clean man/libipc.7.clean libipc.so.clean libipc.a.clean src/communication.o.clean src/error.o.clean src/logger.o.clean src/message.o.clean src/network.o.clean src/usocket.o.clean src/utils.o.clean src/communication.o.clean src/error.o.clean src/logger.o.clean src/message.o.clean src/network.o.clean src/usocket.o.clean src/utils.o.clean
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
		$(PACKAGE)-$(VERSION)/src/ipc.h \
		$(PACKAGE)-$(VERSION)/man/libipc.7.scd \
		$(PACKAGE)-$(VERSION)/src/communication.c \
		$(PACKAGE)-$(VERSION)/src/error.c \
		$(PACKAGE)-$(VERSION)/src/logger.c \
		$(PACKAGE)-$(VERSION)/src/message.c \
		$(PACKAGE)-$(VERSION)/src/network.c \
		$(PACKAGE)-$(VERSION)/src/usocket.c \
		$(PACKAGE)-$(VERSION)/src/utils.c \
		$(PACKAGE)-$(VERSION)/src/logger.h \
		$(PACKAGE)-$(VERSION)/src/message.h \
		$(PACKAGE)-$(VERSION)/src/usocket.h \
		$(PACKAGE)-$(VERSION)/src/utils.h

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.xz[00m'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/src/ipc.h \
		$(PACKAGE)-$(VERSION)/man/libipc.7.scd \
		$(PACKAGE)-$(VERSION)/src/communication.c \
		$(PACKAGE)-$(VERSION)/src/error.c \
		$(PACKAGE)-$(VERSION)/src/logger.c \
		$(PACKAGE)-$(VERSION)/src/message.c \
		$(PACKAGE)-$(VERSION)/src/network.c \
		$(PACKAGE)-$(VERSION)/src/usocket.c \
		$(PACKAGE)-$(VERSION)/src/utils.c \
		$(PACKAGE)-$(VERSION)/src/logger.h \
		$(PACKAGE)-$(VERSION)/src/message.h \
		$(PACKAGE)-$(VERSION)/src/usocket.h \
		$(PACKAGE)-$(VERSION)/src/utils.h

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.bz2[00m'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/src/ipc.h \
		$(PACKAGE)-$(VERSION)/man/libipc.7.scd \
		$(PACKAGE)-$(VERSION)/src/communication.c \
		$(PACKAGE)-$(VERSION)/src/error.c \
		$(PACKAGE)-$(VERSION)/src/logger.c \
		$(PACKAGE)-$(VERSION)/src/message.c \
		$(PACKAGE)-$(VERSION)/src/network.c \
		$(PACKAGE)-$(VERSION)/src/usocket.c \
		$(PACKAGE)-$(VERSION)/src/utils.c \
		$(PACKAGE)-$(VERSION)/src/logger.h \
		$(PACKAGE)-$(VERSION)/src/message.h \
		$(PACKAGE)-$(VERSION)/src/usocket.h \
		$(PACKAGE)-$(VERSION)/src/utils.h

help:
	@echo '[01;37m :: ipc-0.4.0[00m'
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
	@echo '    - [01;34mCFLAGS        [37m ${CFLAGS}[00m'
	@echo '    - [01;34mCC            [37m ${CC}[00m'
	@echo '    - [01;34mCXX           [37m ${CXX}[00m'
	@echo '    - [01;34mLD            [37m ${LD}[00m'
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
	@echo '    - [01;33mlibipc        [37m library[00m'
	@echo '    - [01;33msrc/ipc.h     [37m header[00m'
	@echo '    - [01;33mman/libipc.7  [37m scdocman[00m'
	@echo ''
	@echo '[01;37mMakefile options:[00m'
	@echo '    - gnu:           false'
	@echo '    - colors:        true'
	@echo ''
	@echo '[01;37mRebuild the Makefile with:[00m'
	@echo '    zsh ./build.zsh -c'
.PHONY: all  clean distclean dist install uninstall help

