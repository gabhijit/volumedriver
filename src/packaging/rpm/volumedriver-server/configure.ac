AC_INIT([volumedriver-server], [3.6.0])
AC_CONFIG_MACRO_DIR([../../../m4])
AC_CONFIG_AUX_DIR([../../..])
AM_INIT_AUTOMAKE(m4_esyscmd([case `automake --version | head -n 1` in
                             *1.11*) echo "subdir-objects foreign";;
                             *) echo "subdir-objects foreign serial-tests";;
                             esac]))
dnl AM_INIT_AUTOMAKE([foreign])
AM_PATH_PYTHON([2.6])
AM_CONDITIONAL([USE_MD5_HASH], [test "x$USE_MD5_HASH" != xno])
AC_SUBST([VD_MAJOR_VERSION])
AC_SUBST([VD_MINOR_VERSION])
AC_SUBST([VD_PATCH_VERSION])
AC_SUBST([VD_EXTRA_VERSION])
AC_CONFIG_FILES([add-python-dirs.sh
                 Makefile
                 SPECS/volumedriver-no-dedup-server.spec
                 SPECS/volumedriver-server.spec], [chmod +x add-python-dirs.sh])
AC_OUTPUT
