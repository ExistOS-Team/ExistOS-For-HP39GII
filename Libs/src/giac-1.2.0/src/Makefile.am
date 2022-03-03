lib_LTLIBRARIES = libgiac.la
libgiac_la_SOURCES = input_lexer.ll sym2poly.cc gausspol.cc threaded.cc \
  moyal.cc maple.cc ti89.cc mathml.cc\
  misc.cc permu.cc quater.cc desolve.cc input_parser.yy \
  symbolic.cc index.cc modpoly.cc modfactor.cc \
  ezgcd.cc derive.cc solve.cc intg.cc intgab.cc risch.cc lin.cc series.cc \
  subst.cc vecteur.cc sparse.cc csturm.cc tex.cc global.cc \
  ifactor.cc alg_ext.cc gauss.cc isom.cc plot.cc plot3d.cc rpn.cc prog.cc \
  pari.cc cocoa.cc unary.cc usual.cc identificateur.cc gen.cc tinymt32.cc first.cc \
  TmpLESystemSolver.C TmpFGLM.C \
  TmpFGLM.H TmpLESystemSolver.H \
  path.h lexer.h \
  input_lexer.h input_parser.h threaded.h vector.h \
  help.cc  # Ugh..

libgiac_la_LIBADD = $(NTL_LIBS) $(COCOA_LIBS) $(PARI_LIBS) $(GSL_LIBS)

giacincludedir = $(includedir)/giac
giacinclude_HEADERS = dispatch.h fraction.h gen.h desolve.h misc.h ti89.h \
mathml.h identificateur.h unary.h symbolic.h index.h modpoly.h modfactor.h permu.h \
monomial.h poly.h threaded.h gausspol.h sym2poly.h ezgcd.h derive.h \
solve.h quater.h subst.h usual.h intg.h intgab.h risch.h lin.h series.h \
vecteur.h sparse.h vector.h csturm.h moyal.h tex.h global.h \
ifactor.h alg_ext.h gauss.h isom.h plot.h plot3d.h rpn.h prog.h \
pari.h cocoa.h giac.h first.h maple.h help.h tinymt32.h tinymt32_license.h \
static.h static_extern.h static_lexer.h static_lexer_.h\
lexer_tab_int.h \
static_help.h giacPCH.h giacintl.h gmp_replacements.h myostream.h

noinst_LTLIBRARIES = libxcas.la
libxcas_la_LIBADD = libgiac.la $(GUI_LIBS)

libxcas_la_SOURCES = History.cc Input.cc Xcas1.cc Equation.cc Print.cc \
	Tableur.cc Editeur.cc Graph.cc Graph3d.cc Help1.cc Cfg.cc \
	Flv_CStyle.cc \
	Flve_Check_Button.cc Flve_Input.cc Flv_Style.cc Flv_Data_Source.cc \
	Flve_Combo.cc Flv_List.cc Flv_Table.cc gl2ps.c \
	History.h Input.h Xcas1.h Equation.h Print.h Tableur.h \
	Editeur.h Graph.h Graph3d.h Help1.h Cfg.h \
	Flv_CStyle.H  Flve_Check_Button.H  Flve_Input.H  Flv_Style.H \
	Flv_Data_Source.H  Flve_Combo.H Flv_List.H Flv_Table.H gl2ps.h

libjavagiac_la_LIBADD = libgiac.la 

libjavagiac_la_SOURCES = giac_wrap.cxx


bin_PROGRAMS = icas xcas aide
icas_SOURCES = icas.cc 
icas_LDADD = libxcas.la $(GINSH_LIBS) $(GSL_LIBS) $(LAPACK_LIBS)
aide_SOURCES = aide.cc
aide_LDADD = libxcas.la $(GINSH_LIBS) $(GSL_LIBS) $(LAPACK_LIBS)

AM_CPPFLAGS = -DIN_GIAC -I$(srcdir) -I$(top_srcdir) -I$(builddir) \
	      -I$(top_builddir) $(GMP_CFLAGS) $(NTL_CFLAGS) \
	      $(COCOA_CFLAGS) $(PARI_CFLAGS)

xcas_SOURCES = xcas.cxx 
xcas_LDADD = libxcas.la $(GSL_LIBS) $(LAPACK_LIBS)

AM_YFLAGS = -p giac_yy -d
EXTRA_DIST = .gdbinit xgiac pgiac prime.table gen_user \
	input_parser.h softmath.h softmath.cc\
	mkxcas mkwcas mkgcas mkaout Makefile.wince Makefile.osx\
	Makefile.rpm Makefile.ipaq Makefilenew.ipaq ipaq_usr.tgz \
	Makefile.fast Makefile.win Makefile.win64 Makefile.O3 Makefile.64 \
	Makefile.nspire khicas.lua luagiac.c luabridge.cc luabridge.h \
	giacnspire.cc config.h.nspire missing.c ndlessconfigure\
	normalize.cc integrate.cc partfrac.cc factor.cc xcas.cc \
	hist.fl hist.h hist.cxx hist.menus\
	renee.cc renee.h cas2.cc README.ipaq README.WIN   \
	readme.doc latex.bat xdvi.bat dvips.bat xcasfr.bat xcases.bat \
	xcasfr.sh xcases.sh xcasen.sh mkstat mkhist mkdyn mkdyn10 mkxdyn10 \
        update.sh update.bat \
	giac.pl cas2html.cc all_global_var find_global_var.cc static_init \
	Fl_GDI_Printer.cxx  Fl_PS_Printer.cxx Fl_Gl_Choice.H \
	casce.cc xcasce.cc xcasctrl.cc casctrl.cc \
	Makefile.vcc config.h.vcc giac_oo.cpp giaclib.vcproj \
	Makefile.android compile.android.arm compile.android.x86 config.h.android Makefile.mingw \
	config.h.iphone Makefile.iphone \
	f2c.h clapack.h blaswrap.h mklocal64 pnt_gen \
	giac.i runme.java archive_java javascript.cxx giacjs.i giacjs.js README.javascript \
	Makefile.jni README.jni README.nacl mkdylib mkdylib10 mkgiacpy giac_wrap.cxx

bin_SCRIPTS = pgiac

install-exec-hook:
	rm -f $(DESTDIR)$(bindir)/cas_help
	mv $(DESTDIR)/$(bindir)/aide$(EXEEXT) $(DESTDIR)$(bindir)/cas_help
	ln -sf cas_help $(DESTDIR)$(bindir)/en_cas_help
	ln -sf cas_help $(DESTDIR)$(bindir)/es_cas_help
	ln -sf cas_help $(DESTDIR)$(bindir)/fr_cas_help
	ln -sf icas $(DESTDIR)$(bindir)/giac
	ln -sf xcas $(DESTDIR)$(bindir)/xcasnew
##	$(INSTALL_DATA) $(libgiac_la_SOURCES)  $(DESTDIR)$(docdir)/src
##	$(INSTALL_DATA) $(top_srcdir)/config.h $(DESTDIR)$(includedir)/giac

