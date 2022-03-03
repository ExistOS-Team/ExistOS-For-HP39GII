SUBDIRS = 	src check intl po debianold debian doc examples 
DIST_SUBDIRS = 	src check intl po debianold debian doc examples 
EXTRA_DIST = path.h.in setup inst TROUBLES Init.scm compte INSTALACION LEAME.WIN LEAME.ipaq giac.specs giac.specs32 fltk.1.1.changes README.MACOSX pariinl.h changeversion mkosx mkosx10 mkosx3 mkdebian mkdebian2 win2unix.cc addu.cc xcas.nsi xcas.desktop xcas.xpm xcas.applications README.visualcpp java.README config.h.win mkiphone build_for_iphoneos path.iphone simupath.iphone config.h.iphone config.h.64 config.h.mingw path.mingw configure.mingw README.mingw archive.mingw compile.android mkrpm icons/*

# Rule to build tar-bzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

## FIXME: installing autoconf-generated config.h is just PLAIN WRONG.

install-exec-hook:
	cp config.h $(DESTDIR)/$(includedir)/giac
	./mkinstalldirs $(DESTDIR)/$(prefix)/share/
	./mkinstalldirs $(DESTDIR)/$(prefix)/share/pixmaps
	cp xcas.xpm $(DESTDIR)/$(prefix)/share/pixmaps/xcas.xpm
	./mkinstalldirs $(DESTDIR)/$(prefix)/share/applications
	cp xcas.desktop $(DESTDIR)/$(prefix)/share/applications/xcas.desktop
	./mkinstalldirs $(DESTDIR)/$(prefix)/share/application-registry
	cp xcas.applications $(DESTDIR)/$(prefix)/share/application-registry/xcas.applications
	./mkinstalldirs $(DESTDIR)/$(prefix)/share/icons/
	./mkinstalldirs $(DESTDIR)/$(prefix)/share/icons/hicolor
	for SIZE in 256 128 64 32 16; do ./mkinstalldirs $(DESTDIR)/$(prefix)/share/icons/hicolor/$${SIZE}x$${SIZE} && ./mkinstalldirs $(DESTDIR)/$(prefix)/share/icons/hicolor/$${SIZE}x$${SIZE}/apps && cp icons/xcas_$${SIZE}.png $(DESTDIR)/$(prefix)/share/icons/hicolor/$${SIZE}x$${SIZE}/apps/xcas.png; done
	for SIZE in 256 128 64 32 16; do ./mkinstalldirs $(DESTDIR)/$(prefix)/share/icons/hicolor/$${SIZE}x$${SIZE}/mimetypes && cp icons/x-xcas_$${SIZE}.png $(DESTDIR)/$(prefix)/share/icons/hicolor/$${SIZE}x$${SIZE}/mimetypes/application-x-xcas.png;  done

ACLOCAL_AMFLAGS = -I m4

