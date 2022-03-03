AC_INIT([symbolicc++],[3:35],[yhardy@uj.ac.za])
AM_INIT_AUTOMAKE

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_LANG(C++)

SAVE_CXXFLAGS=${CXXFLAGS}
CXXFLAGS="${CXXFLAGS} -fno-elide-constructors"
AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE([[using namespace std; int main(void){return 0;}]])],,
	[CXXFLAGS="${SAVE_CXXFLAGS}"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
