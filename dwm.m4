dnl #------------------------------------------------------------------------
define(DWM_SET_PKGVARS,[
  AC_MSG_CHECKING([package variables])
  case $host_os in
    darwin*)
      OSNAME="darwin"
      OSVERSION=`uname -r`
      ;;
    freebsd*)
      OSNAME="freebsd"
      OSVERSION=`uname -r | cut -d- -f1`
      ;;
    linux*)
      OSNAME="linux"
      OSVERSION=`uname -r | cut -d'-' -f1`
      ;;
    *)
      ;;
  esac
  STAGING=`realpath ./staging`
  AC_SUBST(OSNAME)
  AC_SUBST(OSVERSION)
  AC_SUBST(STAGING)
  AC_MSG_RESULT([
    OSNAME=\"${OSNAME}\"
    OSVERSION=\"${OSVERSION}\"
    STAGING=\"${STAGING}\"
  ])
])

dnl #------------------------------------------------------------------------
define(MCLOC_CHECK_LIB_TRIVIAL_CPLUSPLUS,[
  AC_LANG_PUSH(C++)
  prev_CPPFLAGS="$CXXFLAGS"
  prev_LDFLAGS="$LDFLAGS"
  CXXFLAGS="$CXXFLAGS -std=c++17"
  LDFLAGS="${LDFLAGS} [$1]"
  AC_MSG_CHECKING([for [$1]])
  AC_TRY_COMPILE([$2],[$3],
                 [AC_MSG_RESULT(yes)
                  AC_SUBST(LDFLAGS)],
                 [AC_MSG_RESULT(no)
                  LDFLAGS="$prev_LDFLAGS"])
  CXXFLAGS="$prev_CPPFLAGS"
  AC_LANG_POP()
])

dnl #------------------------------------------------------------------------
define(MCLOC_NEED_LIBSTDCPPFS,[
  AC_MSG_CHECKING(if we need libstdc++fs)
  AC_LANG_PUSH(C++)
  prev_CPPFLAGS="$CXXFLAGS"
  prev_LIBS="$LIBS"
  CXXFLAGS="$CXXFLAGS -std=c++17"
  AC_TRY_LINK([#include <filesystem>],
              [std::filesystem::path  fspath(".");
	       std::filesystem::file_status  st =
	       std::filesystem::status(fspath);],
  	      [AC_MSG_RESULT(no)],
	      [LIBS="${LIBS} -lstdc++fs"
	       AC_TRY_LINK([#include <filesystem>],
		           [std::filesystem::path  fspath(".");
		            std::filesystem::file_status  st =
		            std::filesystem::status(fspath);],
                 [AC_MSG_RESULT(yes)
		  AC_SUBST(LIBS)],
		 [LIBS="$prev_LIBS"])])
  AC_LANG_POP()		  
])

dnl #-------------------------------------------------------------------------
define(DWM_GIT_TAG,[
  gittag=`git describe --tags --dirty 2>/dev/null`
  dirty=`echo "${gittag}" | awk -F '-' '$NF ~ /g[[a-f0-9]]+/ {if (NF > 2 && $(NF -1) ~ /[[0-9]]+/) {print $NF}}'`
  if test -z "${dirty}"; then
    GIT_TAG="${gittag}"
    GIT_VERSION=`echo "${gittag}" | awk -F '-' '{print $NF}'`
  else
    fakevers=`echo "${dirty}" | cut -d'g' -f2`
    fakevers=`echo $((0x${fakevers}))`
    GIT_TAG="[$1]-0.0.${fakevers}"
    GIT_VERSION="0.0.${fakevers}"
  fi
  AC_SUBST(GIT_TAG)
  AC_SUBST(GIT_VERSION)
])

dnl #-------------------------------------------------------------------------
define(DWM_SVN_TAG,[
  svntag=`svn info . 2>/dev/null | grep ^URL | grep 'tags/' | awk -F 'tags/' '{print [$]2}' | awk -F '/' '{print [$]1}'`
  if test -n "${svntag}"; then
    SVN_TAG="${svntag}"
    SVN_VERSION=`echo [${svntag}] | awk -F '-' '{print $NF}'`
  else
    svn_vers=`svnversion . | sed 's/[[0-9]]*://g'`
    if test "${svn_vers}" = "Unversioned directory"; then
      SVN_VERSION="0.0.0"
    else
      SVN_VERSION="0.0.${svn_vers}"
    fi
    SVN_TAG="[$1]-${SVN_VERSION}"
  fi
  AC_SUBST(SVN_TAG)
  AC_SUBST(SVN_VERSION)
])

dnl #-------------------------------------------------------------------------
define(DWM_GET_TAG,[
  DWM_GIT_TAG([$1])
  if test -n "${GIT_TAG}" ; then
    DWM_TAG="${GIT_TAG}"
    DWM_VERSION="${GIT_VERSION}"
  else
    DWM_SVN_TAG([$1])
    if test -n "${SVN_TAG}" ; then
      DWM_TAG="${SVN_TAG}"
      DWM_VERSION="${SVN_VERSION}"
    fi
  fi
  if test -z "${DWM_TAG}" ; then
    DWM_TAG="[$1]-0.0.0"
    DWM_VERSION="0.0.0"
  fi
  DWM_NAME=`echo "${DWM_TAG}" | sed s/\-${DWM_VERSION}//g`
  AC_SUBST(DWM_TAG)
  AC_SUBST(DWM_VERSION)
  AC_SUBST(DWM_NAME)
])
