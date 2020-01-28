PACKAGE = 'ipc-tests'
VERSION = '0.5.0'

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
SHAREDIR := $(PREFIX)/share
INCLUDEDIR := $(PREFIX)/include
MANDIR := $(SHAREDIR)/man
CFLAGS := -Wall -Wextra -g
LDFLAGS := -I../src -L../ ../src/ipc.h -lipc
CC := cc
CXX := c++
LD := ${CC}
CXXFLAGS :=  

Q := @

all: func_01_connection_establishment func_01_connection_establishmentd func_02_pong func_02_pongd func_03_multiple-communications-client func_03_multiple-communications-server func_04_empty_message func_05_read-write-loop unit_01_service-path unit_02_usock-remove unit_03_connection-add-remove
	@:

func_01_connection_establishment: func_01_connection_establishment.o  
	@echo '[01;32m  LD >    [01;37mfunc_01_connection_establishment[00m'
	$(Q)$(CC) -o func_01_connection_establishment func_01_connection_establishment.o $(LDFLAGS)
func_01_connection_establishment.install: func_01_connection_establishment
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/func_01_connection_establishment[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 func_01_connection_establishment $(DESTDIR)$(BINDIR)/func_01_connection_establishment

func_01_connection_establishment.clean:  func_01_connection_establishment.o.clean
	@echo '[01;37m  RM >    [01;37mfunc_01_connection_establishment[00m'
	$(Q)rm -f func_01_connection_establishment

func_01_connection_establishment.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/func_01_connection_establishment[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/func_01_connection_establishment'

func_01_connection_establishmentd: func_01_connection_establishmentd.o  
	@echo '[01;32m  LD >    [01;37mfunc_01_connection_establishmentd[00m'
	$(Q)$(CC) -o func_01_connection_establishmentd func_01_connection_establishmentd.o $(LDFLAGS)
func_01_connection_establishmentd.install: func_01_connection_establishmentd
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/func_01_connection_establishmentd[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 func_01_connection_establishmentd $(DESTDIR)$(BINDIR)/func_01_connection_establishmentd

func_01_connection_establishmentd.clean:  func_01_connection_establishmentd.o.clean
	@echo '[01;37m  RM >    [01;37mfunc_01_connection_establishmentd[00m'
	$(Q)rm -f func_01_connection_establishmentd

func_01_connection_establishmentd.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/func_01_connection_establishmentd[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/func_01_connection_establishmentd'

func_02_pong: func_02_pong.o  
	@echo '[01;32m  LD >    [01;37mfunc_02_pong[00m'
	$(Q)$(CC) -o func_02_pong func_02_pong.o $(LDFLAGS)
func_02_pong.install: func_02_pong
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/func_02_pong[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 func_02_pong $(DESTDIR)$(BINDIR)/func_02_pong

func_02_pong.clean:  func_02_pong.o.clean
	@echo '[01;37m  RM >    [01;37mfunc_02_pong[00m'
	$(Q)rm -f func_02_pong

func_02_pong.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/func_02_pong[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/func_02_pong'

func_02_pongd: func_02_pongd.o  
	@echo '[01;32m  LD >    [01;37mfunc_02_pongd[00m'
	$(Q)$(CC) -o func_02_pongd func_02_pongd.o $(LDFLAGS)
func_02_pongd.install: func_02_pongd
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/func_02_pongd[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 func_02_pongd $(DESTDIR)$(BINDIR)/func_02_pongd

func_02_pongd.clean:  func_02_pongd.o.clean
	@echo '[01;37m  RM >    [01;37mfunc_02_pongd[00m'
	$(Q)rm -f func_02_pongd

func_02_pongd.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/func_02_pongd[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/func_02_pongd'

func_03_multiple-communications-client: func_03_multiple-communications-client.o  
	@echo '[01;32m  LD >    [01;37mfunc_03_multiple-communications-client[00m'
	$(Q)$(CC) -o func_03_multiple-communications-client func_03_multiple-communications-client.o $(LDFLAGS)
func_03_multiple-communications-client.install: func_03_multiple-communications-client
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/func_03_multiple-communications-client[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 func_03_multiple-communications-client $(DESTDIR)$(BINDIR)/func_03_multiple-communications-client

func_03_multiple-communications-client.clean:  func_03_multiple-communications-client.o.clean
	@echo '[01;37m  RM >    [01;37mfunc_03_multiple-communications-client[00m'
	$(Q)rm -f func_03_multiple-communications-client

func_03_multiple-communications-client.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/func_03_multiple-communications-client[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/func_03_multiple-communications-client'

func_03_multiple-communications-server: func_03_multiple-communications-server.o  
	@echo '[01;32m  LD >    [01;37mfunc_03_multiple-communications-server[00m'
	$(Q)$(CC) -o func_03_multiple-communications-server func_03_multiple-communications-server.o $(LDFLAGS)
func_03_multiple-communications-server.install: func_03_multiple-communications-server
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/func_03_multiple-communications-server[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 func_03_multiple-communications-server $(DESTDIR)$(BINDIR)/func_03_multiple-communications-server

func_03_multiple-communications-server.clean:  func_03_multiple-communications-server.o.clean
	@echo '[01;37m  RM >    [01;37mfunc_03_multiple-communications-server[00m'
	$(Q)rm -f func_03_multiple-communications-server

func_03_multiple-communications-server.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/func_03_multiple-communications-server[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/func_03_multiple-communications-server'

func_04_empty_message: func_04_empty_message.o  
	@echo '[01;32m  LD >    [01;37mfunc_04_empty_message[00m'
	$(Q)$(CC) -o func_04_empty_message func_04_empty_message.o $(LDFLAGS)
func_04_empty_message.install: func_04_empty_message
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/func_04_empty_message[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 func_04_empty_message $(DESTDIR)$(BINDIR)/func_04_empty_message

func_04_empty_message.clean:  func_04_empty_message.o.clean
	@echo '[01;37m  RM >    [01;37mfunc_04_empty_message[00m'
	$(Q)rm -f func_04_empty_message

func_04_empty_message.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/func_04_empty_message[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/func_04_empty_message'

func_05_read-write-loop: func_05_read-write-loop.o  
	@echo '[01;32m  LD >    [01;37mfunc_05_read-write-loop[00m'
	$(Q)$(CC) -o func_05_read-write-loop func_05_read-write-loop.o $(LDFLAGS)
func_05_read-write-loop.install: func_05_read-write-loop
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/func_05_read-write-loop[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 func_05_read-write-loop $(DESTDIR)$(BINDIR)/func_05_read-write-loop

func_05_read-write-loop.clean:  func_05_read-write-loop.o.clean
	@echo '[01;37m  RM >    [01;37mfunc_05_read-write-loop[00m'
	$(Q)rm -f func_05_read-write-loop

func_05_read-write-loop.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/func_05_read-write-loop[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/func_05_read-write-loop'

unit_01_service-path: unit_01_service-path.o  
	@echo '[01;32m  LD >    [01;37munit_01_service-path[00m'
	$(Q)$(CC) -o unit_01_service-path unit_01_service-path.o $(LDFLAGS)
unit_01_service-path.install: unit_01_service-path
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/unit_01_service-path[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 unit_01_service-path $(DESTDIR)$(BINDIR)/unit_01_service-path

unit_01_service-path.clean:  unit_01_service-path.o.clean
	@echo '[01;37m  RM >    [01;37munit_01_service-path[00m'
	$(Q)rm -f unit_01_service-path

unit_01_service-path.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/unit_01_service-path[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/unit_01_service-path'

unit_02_usock-remove: unit_02_usock-remove.o  
	@echo '[01;32m  LD >    [01;37munit_02_usock-remove[00m'
	$(Q)$(CC) -o unit_02_usock-remove unit_02_usock-remove.o $(LDFLAGS)
unit_02_usock-remove.install: unit_02_usock-remove
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/unit_02_usock-remove[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 unit_02_usock-remove $(DESTDIR)$(BINDIR)/unit_02_usock-remove

unit_02_usock-remove.clean:  unit_02_usock-remove.o.clean
	@echo '[01;37m  RM >    [01;37munit_02_usock-remove[00m'
	$(Q)rm -f unit_02_usock-remove

unit_02_usock-remove.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/unit_02_usock-remove[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/unit_02_usock-remove'

unit_03_connection-add-remove: unit_03_connection-add-remove.o  
	@echo '[01;32m  LD >    [01;37munit_03_connection-add-remove[00m'
	$(Q)$(CC) -o unit_03_connection-add-remove unit_03_connection-add-remove.o $(LDFLAGS)
unit_03_connection-add-remove.install: unit_03_connection-add-remove
	@echo '[01;31m  IN >    [01;37m$(BINDIR)/unit_03_connection-add-remove[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 unit_03_connection-add-remove $(DESTDIR)$(BINDIR)/unit_03_connection-add-remove

unit_03_connection-add-remove.clean:  unit_03_connection-add-remove.o.clean
	@echo '[01;37m  RM >    [01;37munit_03_connection-add-remove[00m'
	$(Q)rm -f unit_03_connection-add-remove

unit_03_connection-add-remove.uninstall:
	@echo '[01;37m  RM >    [01;37m$(BINDIR)/unit_03_connection-add-remove[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/unit_03_connection-add-remove'

func_01_connection_establishment.o: func_01_connection_establishment.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mfunc_01_connection_establishment.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c func_01_connection_establishment.c  -std=c11 -o func_01_connection_establishment.o

func_01_connection_establishment.o.install:

func_01_connection_establishment.o.clean:
	@echo '[01;37m  RM >    [01;37mfunc_01_connection_establishment.o[00m'
	$(Q)rm -f func_01_connection_establishment.o

func_01_connection_establishment.o.uninstall:

func_01_connection_establishmentd.o: func_01_connection_establishmentd.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mfunc_01_connection_establishmentd.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c func_01_connection_establishmentd.c  -std=c11 -o func_01_connection_establishmentd.o

func_01_connection_establishmentd.o.install:

func_01_connection_establishmentd.o.clean:
	@echo '[01;37m  RM >    [01;37mfunc_01_connection_establishmentd.o[00m'
	$(Q)rm -f func_01_connection_establishmentd.o

func_01_connection_establishmentd.o.uninstall:

func_02_pong.o: func_02_pong.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mfunc_02_pong.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c func_02_pong.c  -std=c11 -o func_02_pong.o

func_02_pong.o.install:

func_02_pong.o.clean:
	@echo '[01;37m  RM >    [01;37mfunc_02_pong.o[00m'
	$(Q)rm -f func_02_pong.o

func_02_pong.o.uninstall:

func_02_pongd.o: func_02_pongd.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mfunc_02_pongd.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c func_02_pongd.c  -std=c11 -o func_02_pongd.o

func_02_pongd.o.install:

func_02_pongd.o.clean:
	@echo '[01;37m  RM >    [01;37mfunc_02_pongd.o[00m'
	$(Q)rm -f func_02_pongd.o

func_02_pongd.o.uninstall:

func_03_multiple-communications-client.o: func_03_multiple-communications-client.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mfunc_03_multiple-communications-client.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c func_03_multiple-communications-client.c  -std=c11 -o func_03_multiple-communications-client.o

func_03_multiple-communications-client.o.install:

func_03_multiple-communications-client.o.clean:
	@echo '[01;37m  RM >    [01;37mfunc_03_multiple-communications-client.o[00m'
	$(Q)rm -f func_03_multiple-communications-client.o

func_03_multiple-communications-client.o.uninstall:

func_03_multiple-communications-server.o: func_03_multiple-communications-server.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mfunc_03_multiple-communications-server.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c func_03_multiple-communications-server.c  -std=c11 -o func_03_multiple-communications-server.o

func_03_multiple-communications-server.o.install:

func_03_multiple-communications-server.o.clean:
	@echo '[01;37m  RM >    [01;37mfunc_03_multiple-communications-server.o[00m'
	$(Q)rm -f func_03_multiple-communications-server.o

func_03_multiple-communications-server.o.uninstall:

func_04_empty_message.o: func_04_empty_message.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mfunc_04_empty_message.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c func_04_empty_message.c  -std=c11 -o func_04_empty_message.o

func_04_empty_message.o.install:

func_04_empty_message.o.clean:
	@echo '[01;37m  RM >    [01;37mfunc_04_empty_message.o[00m'
	$(Q)rm -f func_04_empty_message.o

func_04_empty_message.o.uninstall:

func_05_read-write-loop.o: func_05_read-write-loop.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37mfunc_05_read-write-loop.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c func_05_read-write-loop.c  -std=c11 -o func_05_read-write-loop.o

func_05_read-write-loop.o.install:

func_05_read-write-loop.o.clean:
	@echo '[01;37m  RM >    [01;37mfunc_05_read-write-loop.o[00m'
	$(Q)rm -f func_05_read-write-loop.o

func_05_read-write-loop.o.uninstall:

unit_01_service-path.o: unit_01_service-path.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37munit_01_service-path.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c unit_01_service-path.c  -std=c11 -o unit_01_service-path.o

unit_01_service-path.o.install:

unit_01_service-path.o.clean:
	@echo '[01;37m  RM >    [01;37munit_01_service-path.o[00m'
	$(Q)rm -f unit_01_service-path.o

unit_01_service-path.o.uninstall:

unit_02_usock-remove.o: unit_02_usock-remove.c  ./../src/ipc.h ./../src/usocket.h
	@echo '[01;34m  CC >    [01;37munit_02_usock-remove.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c unit_02_usock-remove.c  -std=c11 -o unit_02_usock-remove.o

unit_02_usock-remove.o.install:

unit_02_usock-remove.o.clean:
	@echo '[01;37m  RM >    [01;37munit_02_usock-remove.o[00m'
	$(Q)rm -f unit_02_usock-remove.o

unit_02_usock-remove.o.uninstall:

unit_03_connection-add-remove.o: unit_03_connection-add-remove.c  ./../src/ipc.h
	@echo '[01;34m  CC >    [01;37munit_03_connection-add-remove.o[00m'
	$(Q)$(CC) $(CFLAGS) -std=c11 -c unit_03_connection-add-remove.c  -std=c11 -o unit_03_connection-add-remove.o

unit_03_connection-add-remove.o.install:

unit_03_connection-add-remove.o.clean:
	@echo '[01;37m  RM >    [01;37munit_03_connection-add-remove.o[00m'
	$(Q)rm -f unit_03_connection-add-remove.o

unit_03_connection-add-remove.o.uninstall:

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
install: func_01_connection_establishment.install func_01_connection_establishmentd.install func_02_pong.install func_02_pongd.install func_03_multiple-communications-client.install func_03_multiple-communications-server.install func_04_empty_message.install func_05_read-write-loop.install unit_01_service-path.install unit_02_usock-remove.install unit_03_connection-add-remove.install func_01_connection_establishment.o.install func_01_connection_establishmentd.o.install func_02_pong.o.install func_02_pongd.o.install func_03_multiple-communications-client.o.install func_03_multiple-communications-server.o.install func_04_empty_message.o.install func_05_read-write-loop.o.install unit_01_service-path.o.install unit_02_usock-remove.o.install unit_03_connection-add-remove.o.install
	@:

uninstall: func_01_connection_establishment.uninstall func_01_connection_establishmentd.uninstall func_02_pong.uninstall func_02_pongd.uninstall func_03_multiple-communications-client.uninstall func_03_multiple-communications-server.uninstall func_04_empty_message.uninstall func_05_read-write-loop.uninstall unit_01_service-path.uninstall unit_02_usock-remove.uninstall unit_03_connection-add-remove.uninstall func_01_connection_establishment.o.uninstall func_01_connection_establishmentd.o.uninstall func_02_pong.o.uninstall func_02_pongd.o.uninstall func_03_multiple-communications-client.o.uninstall func_03_multiple-communications-server.o.uninstall func_04_empty_message.o.uninstall func_05_read-write-loop.o.uninstall unit_01_service-path.o.uninstall unit_02_usock-remove.o.uninstall unit_03_connection-add-remove.o.uninstall
	@:

clean: func_01_connection_establishment.clean func_01_connection_establishmentd.clean func_02_pong.clean func_02_pongd.clean func_03_multiple-communications-client.clean func_03_multiple-communications-server.clean func_04_empty_message.clean func_05_read-write-loop.clean unit_01_service-path.clean unit_02_usock-remove.clean unit_03_connection-add-remove.clean func_01_connection_establishment.o.clean func_01_connection_establishmentd.o.clean func_02_pong.o.clean func_02_pongd.o.clean func_03_multiple-communications-client.o.clean func_03_multiple-communications-server.o.clean func_04_empty_message.o.clean func_05_read-write-loop.o.clean unit_01_service-path.o.clean unit_02_usock-remove.o.clean unit_03_connection-add-remove.o.clean
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
		$(PACKAGE)-$(VERSION)/unit_01_service-path.c \
		$(PACKAGE)-$(VERSION)/func_02_pong.c \
		$(PACKAGE)-$(VERSION)/func_02_pongd.c \
		$(PACKAGE)-$(VERSION)/func_04_empty_message.c \
		$(PACKAGE)-$(VERSION)/func_01_connection_establishment.c \
		$(PACKAGE)-$(VERSION)/func_03_multiple-communications-server.c \
		$(PACKAGE)-$(VERSION)/unit_03_connection-add-remove.c \
		$(PACKAGE)-$(VERSION)/func_05_read-write-loop.c \
		$(PACKAGE)-$(VERSION)/func_03_multiple-communications-client.c \
		$(PACKAGE)-$(VERSION)/unit_02_usock-remove.c \
		$(PACKAGE)-$(VERSION)/func_01_connection_establishmentd.c

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.xz[00m'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/unit_01_service-path.c \
		$(PACKAGE)-$(VERSION)/func_02_pong.c \
		$(PACKAGE)-$(VERSION)/func_02_pongd.c \
		$(PACKAGE)-$(VERSION)/func_04_empty_message.c \
		$(PACKAGE)-$(VERSION)/func_01_connection_establishment.c \
		$(PACKAGE)-$(VERSION)/func_03_multiple-communications-server.c \
		$(PACKAGE)-$(VERSION)/unit_03_connection-add-remove.c \
		$(PACKAGE)-$(VERSION)/func_05_read-write-loop.c \
		$(PACKAGE)-$(VERSION)/func_03_multiple-communications-client.c \
		$(PACKAGE)-$(VERSION)/unit_02_usock-remove.c \
		$(PACKAGE)-$(VERSION)/func_01_connection_establishmentd.c

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '[01;33m  TAR >   [01;37m$(PACKAGE)-$(VERSION).tar.bz2[00m'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/unit_01_service-path.c \
		$(PACKAGE)-$(VERSION)/func_02_pong.c \
		$(PACKAGE)-$(VERSION)/func_02_pongd.c \
		$(PACKAGE)-$(VERSION)/func_04_empty_message.c \
		$(PACKAGE)-$(VERSION)/func_01_connection_establishment.c \
		$(PACKAGE)-$(VERSION)/func_03_multiple-communications-server.c \
		$(PACKAGE)-$(VERSION)/unit_03_connection-add-remove.c \
		$(PACKAGE)-$(VERSION)/func_05_read-write-loop.c \
		$(PACKAGE)-$(VERSION)/func_03_multiple-communications-client.c \
		$(PACKAGE)-$(VERSION)/unit_02_usock-remove.c \
		$(PACKAGE)-$(VERSION)/func_01_connection_establishmentd.c

help:
	@echo '[01;37m :: ipc-tests-0.5.0[00m'
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
	@echo '    - [01;34mLDFLAGS       [37m ${LDFLAGS}[00m'
	@echo '    - [01;34mCC            [37m ${CC}[00m'
	@echo '    - [01;34mCXX           [37m ${CXX}[00m'
	@echo '    - [01;34mLD            [37m ${LD}[00m'
	@echo '    - [01;34mCXXFLAGS      [37m ${CXXFLAGS}[00m'
	@echo '    - [01;34mPREFIX        [37m ${PREFIX}[00m'
	@echo '    - [01;34mBINDIR        [37m ${BINDIR}[00m'
	@echo '    - [01;34mLIBDIR        [37m ${LIBDIR}[00m'
	@echo '    - [01;34mSHAREDIR      [37m ${SHAREDIR}[00m'
	@echo '    - [01;34mINCLUDEDIR    [37m ${INCLUDEDIR}[00m'
	@echo '    - [01;34mMANDIR        [37m ${MANDIR}[00m'
	@echo ''
	@echo '[01;37mProject targets: [00m'
	@echo '    - [01;33mfunc_01_connection_establishment[37m binary[00m'
	@echo '    - [01;33mfunc_01_connection_establishmentd[37m binary[00m'
	@echo '    - [01;33mfunc_02_pong                  [37m binary[00m'
	@echo '    - [01;33mfunc_02_pongd                 [37m binary[00m'
	@echo '    - [01;33mfunc_03_multiple-communications-client[37m binary[00m'
	@echo '    - [01;33mfunc_03_multiple-communications-server[37m binary[00m'
	@echo '    - [01;33mfunc_04_empty_message         [37m binary[00m'
	@echo '    - [01;33mfunc_05_read-write-loop       [37m binary[00m'
	@echo '    - [01;33munit_01_service-path          [37m binary[00m'
	@echo '    - [01;33munit_02_usock-remove          [37m binary[00m'
	@echo '    - [01;33munit_03_connection-add-remove [37m binary[00m'
	@echo ''
	@echo '[01;37mMakefile options:[00m'
	@echo '    - gnu:           false'
	@echo '    - colors:        true'
	@echo ''
	@echo '[01;37mRebuild the Makefile with:[00m'
	@echo '    zsh ./build.zsh -c'
.PHONY: all  clean distclean dist install uninstall help

