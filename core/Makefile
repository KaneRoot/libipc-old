PACKAGE = 'ipc'
VERSION = '0.0.1'

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
SHAREDIR := $(PREFIX)/share
INCLUDEDIR := $(PREFIX)/include
MANDIR := $(SHAREDIR)/man

CC := cc
AR := ar
RANLIB := ranlib
CFLAGS := 
LDFLAGS := 

Q := @

all: libipc
	@:

libipc: libipc.so libipc.a
	@:
libipc.install: libipc.so.install libipc.a.install

libipc.clean: libipc.so.clean libipc.a.clean

libipc.uninstall: libipc.so.uninstall libipc.a.uninstall

libipc.so: communication.o logger.o msg.o process.o usocket.o utils.o 
	@echo '[01;32m  LD >    [01;37mlibipc.so[00m'
	$(Q)$(CC) -o libipc.so -shared $(LDFLAGS) communication.o logger.o msg.o process.o usocket.o utils.o 

libipc.so.install: libipc.so
	@echo '[01;31m  IN >    [01;37m$(LIBDIR)/libipc.so.0.0.1[00m'
	$(Q)mkdir -p '$(DESTDIR)$(LIBDIR)'
	$(Q)install -m0755 libipc.so $(DESTDIR)$(LIBDIR)/libipc.so.0.0.1
	@echo '[01;33m  LN >    [01;37m$(LIBDIR)/libipc.so.0.0[00m'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.0.1' '$(DESTDIR)/$(LIBDIR)/libipc.so.0.0'
	@echo '[01;33m  LN >    [01;37m$(LIBDIR)/libipc.so.0[00m'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.0.1' '$(DESTDIR)/$(LIBDIR)/libipc.so.0'
	@echo '[01;33m  LN >    [01;37m$(LIBDIR)/libipc.so[00m'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.0.1' '$(DESTDIR)/$(LIBDIR)/libipc.so'

libipc.so.clean:
	@echo '[01;37m  RM >    [01;37mlibipc.so[00m'
	$(Q)rm -f libipc.so

libipc.so.uninstall:
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.so.0.0.1[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0.0.1'
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.so.0.0[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0.0'
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.so.0[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0'
	@echo '[01;37m  RM >    [01;37m$(LIBDIR)/libipc.so[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so'

libipc.a: communication.o logger.o msg.o process.o usocket.o utils.o 
	@echo '[01;32m  LD >    [01;37mlibipc.a[00m'
	$(Q)$(AR) rc 'libipc.a' communication.o logger.o msg.o process.o usocket.o utils.o

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

communication.o: communication.c ./communication.h ./usocket.h ./utils.h ./error.h
	@echo '[01;34m  CC >    [01;37mcommunication.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c communication.c  -fPIC  -o communication.o

communication.o.install:

communication.o.clean:
	@echo '[01;37m  RM >    [01;37mcommunication.o[00m'
	$(Q)rm -f communication.o

communication.o.uninstall:

logger.o: logger.c ./logger.h
	@echo '[01;34m  CC >    [01;37mlogger.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c logger.c  -fPIC  -o logger.o

logger.o.install:

logger.o.clean:
	@echo '[01;37m  RM >    [01;37mlogger.o[00m'
	$(Q)rm -f logger.o

logger.o.uninstall:

msg.o: msg.c ./msg.h ./error.h ./usocket.h
	@echo '[01;34m  CC >    [01;37mmsg.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c msg.c  -fPIC  -o msg.o

msg.o.install:

msg.o.clean:
	@echo '[01;37m  RM >    [01;37mmsg.o[00m'
	$(Q)rm -f msg.o

msg.o.uninstall:

process.o: process.c ./process.h
	@echo '[01;34m  CC >    [01;37mprocess.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c process.c  -fPIC  -o process.o

process.o.install:

process.o.clean:
	@echo '[01;37m  RM >    [01;37mprocess.o[00m'
	$(Q)rm -f process.o

process.o.uninstall:

usocket.o: usocket.c ./usocket.h ./utils.h ./error.h
	@echo '[01;34m  CC >    [01;37musocket.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c usocket.c  -fPIC  -o usocket.o

usocket.o.install:

usocket.o.clean:
	@echo '[01;37m  RM >    [01;37musocket.o[00m'
	$(Q)rm -f usocket.o

usocket.o.uninstall:

utils.o: utils.c ./utils.h
	@echo '[01;34m  CC >    [01;37mutils.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c utils.c  -fPIC  -o utils.o

utils.o.install:

utils.o.clean:
	@echo '[01;37m  RM >    [01;37mutils.o[00m'
	$(Q)rm -f utils.o

utils.o.uninstall:

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
install: subdirs.install libipc.install libipc.so.install libipc.a.install communication.o.install logger.o.install msg.o.install process.o.install usocket.o.install utils.o.install communication.o.install logger.o.install msg.o.install process.o.install usocket.o.install utils.o.install
	@:

subdirs.install:

uninstall: subdirs.uninstall libipc.uninstall libipc.so.uninstall libipc.a.uninstall communication.o.uninstall logger.o.uninstall msg.o.uninstall process.o.uninstall usocket.o.uninstall utils.o.uninstall communication.o.uninstall logger.o.uninstall msg.o.uninstall process.o.uninstall usocket.o.uninstall utils.o.uninstall
	@:

subdirs.uninstall:

test: all subdirs subdirs.test
	@:

subdirs.test:

clean: libipc.clean libipc.so.clean libipc.a.clean communication.o.clean logger.o.clean msg.o.clean process.o.clean usocket.o.clean utils.o.clean communication.o.clean logger.o.clean msg.o.clean process.o.clean usocket.o.clean utils.o.clean

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
		$(PACKAGE)-$(VERSION)/error.h \
		$(PACKAGE)-$(VERSION)/communication.c \
		$(PACKAGE)-$(VERSION)/logger.c \
		$(PACKAGE)-$(VERSION)/msg.c \
		$(PACKAGE)-$(VERSION)/process.c \
		$(PACKAGE)-$(VERSION)/usocket.c \
		$(PACKAGE)-$(VERSION)/utils.c \
		$(PACKAGE)-$(VERSION)/communication.h \
		$(PACKAGE)-$(VERSION)/logger.h \
		$(PACKAGE)-$(VERSION)/msg.h \
		$(PACKAGE)-$(VERSION)/process.h \
		$(PACKAGE)-$(VERSION)/usocket.h \
		$(PACKAGE)-$(VERSION)/utils.h

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.xz[00m'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/error.h \
		$(PACKAGE)-$(VERSION)/communication.c \
		$(PACKAGE)-$(VERSION)/logger.c \
		$(PACKAGE)-$(VERSION)/msg.c \
		$(PACKAGE)-$(VERSION)/process.c \
		$(PACKAGE)-$(VERSION)/usocket.c \
		$(PACKAGE)-$(VERSION)/utils.c \
		$(PACKAGE)-$(VERSION)/communication.h \
		$(PACKAGE)-$(VERSION)/logger.h \
		$(PACKAGE)-$(VERSION)/msg.h \
		$(PACKAGE)-$(VERSION)/process.h \
		$(PACKAGE)-$(VERSION)/usocket.h \
		$(PACKAGE)-$(VERSION)/utils.h

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.bz2[00m'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/error.h \
		$(PACKAGE)-$(VERSION)/communication.c \
		$(PACKAGE)-$(VERSION)/logger.c \
		$(PACKAGE)-$(VERSION)/msg.c \
		$(PACKAGE)-$(VERSION)/process.c \
		$(PACKAGE)-$(VERSION)/usocket.c \
		$(PACKAGE)-$(VERSION)/utils.c \
		$(PACKAGE)-$(VERSION)/communication.h \
		$(PACKAGE)-$(VERSION)/logger.h \
		$(PACKAGE)-$(VERSION)/msg.h \
		$(PACKAGE)-$(VERSION)/process.h \
		$(PACKAGE)-$(VERSION)/usocket.h \
		$(PACKAGE)-$(VERSION)/utils.h

help:
	@echo '[01;37m :: ipc-0.0.1[00m'
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
	@echo '    - [01;34mCFLAGS        [37m ${CFLAGS}[00m'
	@echo '    - [01;34mLDFLAGS       [37m ${LDFLAGS}[00m'
	@echo '    - [01;34mDESTDIR       [37m ${DESTDIR}[00m'
	@echo '    - [01;34mPREFIX        [37m ${PREFIX}[00m'
	@echo '    - [01;34mBINDIR        [37m ${BINDIR}[00m'
	@echo '    - [01;34mLIBDIR        [37m ${LIBDIR}[00m'
	@echo '    - [01;34mSHAREDIR      [37m ${SHAREDIR}[00m'
	@echo '    - [01;34mINCLUDEDIR    [37m ${INCLUDEDIR}[00m'
	@echo '    - [01;34mMANDIR        [37m ${MANDIR}[00m'
	@echo ''
	@echo '[01;37mProject targets: [00m'
	@echo '    - [01;33mlibipc        [37m library[00m'
	@echo ''
	@echo '[01;37mMakefile options:[00m'
	@echo '    - gnu:           false'
	@echo '    - colors:        true'
	@echo ''
	@echo '[01;37mRebuild the Makefile with:[00m'
	@echo '    zsh ./build.zsh -c'
.PHONY: all subdirs clean distclean dist install uninstall help

