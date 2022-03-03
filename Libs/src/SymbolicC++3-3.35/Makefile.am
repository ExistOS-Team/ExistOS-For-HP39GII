ACLOCAL_AMFLAGS=		-I m4
lib_LTLIBRARIES=		libsymbolicc++.la
libsymbolicc___la_CXXFLAGS=	-I include $(AM_CXXFLAGS)
libsymbolicc___la_LDFLAGS=	-version-info $(PACKAGE_VERSION) $(AM_LDFLAGS)

symbolicdir=			$(includedir)/symbolic

install-data-local:
	$(MKDIR_P) $(datadir)/doc
	$(INSTALL_DATA) README $(datadir)/doc/README.SymbolicC++
	$(INSTALL_DATA) doc/introsymb.pdf $(datadir)/doc/SymbolicC++.pdf
include_HEADERS= \
 include/array.h include/cloning.h include/derive.h include/identity.h \
 include/matnorm.h include/matrix.h include/multinomial.h include/polynomial.h \
 include/quatern.h include/rational.h include/symbolicc++.h include/vecnorm.h \
 include/vector.h include/verylong.h

symbolic_HEADERS= \
 include/symbolic/constants.h include/symbolic/equation.h \
 include/symbolic/functions.h include/symbolic/integrate.h \
 include/symbolic/number.h include/symbolic/product.h include/symbolic/solve.h \
 include/symbolic/sum.h include/symbolic/symbol.h include/symbolic/symbolic.h \
 include/symbolic/symbolicc++.h include/symbolic/symerror.h \
 include/symbolic/symmatrix.h

libsymbolicc___la_SOURCES= \
 src/array.cpp src/cloning.cpp src/derive.cpp src/identity.cpp \
 src/matnorm.cpp src/matrix.cpp src/multinomial.cpp src/polynomial.cpp \
 src/quatern.cpp src/rational.cpp src/vecnorm.cpp src/vector.cpp \
 src/verylong.cpp src/constants.cpp src/equation.cpp src/functions.cpp \
 src/integrate.cpp src/number.cpp src/product.cpp src/solve.cpp src/sum.cpp \
 src/symbol.cpp src/symbolic.cpp src/symbolicc++.cpp src/symerror.cpp \
 src/symmatrix.cpp
