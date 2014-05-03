#
# Copyright (C) 2013 Mamadou DIOP
# Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
# License: GPLv3 or commercial
# This file is part of Doubango Open Source SIP TelePresence System <https://code.google.com/p/telepresence/>
#

AC_PREREQ([2.1])
AC_INIT(telepresence, 2.1, opentelepresence(at)googlegroups(dot)com)
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([source/main.cc])

dnl find suitable C and C++ compilers
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O

AC_PROG_INSTALL

AC_PROG_SED

AC_PROG_MAKE_SET

PKG_PROG_PKG_CONFIG

AC_SUBST(DOUBANGO_MINVER, 2.0.1050)
AC_SUBST(LIBAVCODEC_MINVER, 54.92.100)
AC_SUBST(LIBAVFILTER_MINVER, 3.42.103)
AC_SUBST(LIBAVFORMAT_MINVER, 54.63.104)
AC_SUBST(LIBAVUTIL_MINVER, 52.18.100)
AC_SUBST(LIBSWRESAMPLE_MINVER, 0.17.102)
AC_SUBST(LIBSWSCAL_MINVER, 2.2.100)
AC_SUBST(OPENAL_MINVER, 1.15.1)

target_cpu=`uname -m`
target_os=`uname -s`
#echo Setting up build environment for ${target_cpu}${target_os}

AC_CHECK_HEADERS([unistd.h spawn.h sys/wait.h errno.h])
AC_CHECK_LIB(c, dlopen, AC_SUBST(LIBDL,""), AC_CHECK_LIB(dl, dlopen, AC_SUBST(LIBDL,-ldl)))
AC_CHECK_FUNCS([fork vfork execl posix_spawn waitpid setpgid popen pclose])

### ENABLE-DEBUG
debug_default="no"
AC_ARG_ENABLE(debug, 
[  --enable-debug[=no/yes] turn on/off debugging 
                       [[default=$debug_default]]],
[ if test "x$enableval" = "xyes" ; then
	CFLAGS="$CFLAGS -O0 -g3 -DDEBUG"
	AC_MSG_RESULT(yes)
  else
	CFLAGS="$CFLAGS -O3 -NDEBUG"
	AC_MSG_RESULT(no)
  fi
],
[])

# FFMPEG
AC_SUBST(have_libavcodec, no)
AC_SUBST(have_libavfilter, no)
AC_SUBST(have_libavformat, no)
AC_SUBST(have_libavutil, no)
AC_SUBST(have_libswresample, no)
AC_SUBST(have_libswscale, no)
PKG_CHECK_MODULES(LIBAVCODEC, libavcodec >= ${LIBAVCODEC_MINVER}, AC_SUBST(have_libavcodec, yes), AC_MSG_WARN(Cannot find libavcodec >= ${LIBAVCODEC_MINVER} using pkg-config))
PKG_CHECK_MODULES(LIBAVFILTER, libavfilter >= ${LIBAVFILTER_MINVER}, AC_SUBST(have_libavfilter, yes), AC_MSG_WARN(Cannot find libavfilter >= ${LIBAVFILTER_MINVER} using pkg-config))
PKG_CHECK_MODULES(LIBAVFORMAT, libavformat >= ${LIBAVFORMAT_MINVER}, AC_SUBST(have_libavformat, yes), AC_MSG_WARN(Cannot find libavformat >= ${LIBAVFORMAT_MINVER} using pkg-config))
PKG_CHECK_MODULES(LIBAVUTIL, libavutil >= ${LIBAVUTIL_MINVER}, AC_SUBST(have_libavutil, yes), AC_MSG_WARN(Cannot find libavutil >= ${LIBAVUTIL_MINVER} using pkg-config))
PKG_CHECK_MODULES(LIBSWRESAMPLE, libswresample >= ${LIBSWRESAMPLE_MINVER}, AC_SUBST(have_libswresample, yes), AC_MSG_WARN(Cannot find libswresample >= ${LIBSWRESAMPLE_MINVER} using pkg-config))
PKG_CHECK_MODULES(LIBSWSCAL, libswscale >= ${LIBSWSCAL_MINVER}, AC_SUBST(have_libswscale, yes), AC_MSG_WARN(Cannot find libswscale >= ${LIBSWSCAL_MINVER} using pkg-config))

if test $have_libavcodec:$have_libavfilter:$have_libavformat:$have_libavutil:$have_libswresample:$have_libswscale != yes:yes:yes:yes:yes:yes ; then
	AC_MSG_WARN([
***
*** Failed to find all FFmpeg libraries using pkg-config.
*** This most likely means the FFmpeg pkg files (*.pc) are not 
*** in the default path where pkg-config expect them.
***
*** Please set your PKG_CONFIG_PATH environment variable to include the
*** directory in which the *.pc files are installed. Most likely
*** these files are installed in /usr/local/lib/pkgconfig; if that is the
*** case, try:
***
***     export PKG_CONFIG_PATH="\$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
***
*** then run this ./configure <your options here> again.
***
*** or, try:
***
***   PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./configure <your options here>
***
*** Anyways, we're trying another way to find FFmpeg.
***
]);

AC_SUBST(FFMPEG_INCLUDES_FALLBACK,-I/usr/local/include)
AC_SUBST(FFMPEG_LIBS_FALLBACK,-L/usr/local/lib)
AC_SUBST(LIBAVCODEC_LIBS,-lavcodec)
AC_SUBST(LIBAVFILTER_LIBS,-lavfilter)
AC_SUBST(LIBAVFORMAT_LIBS,-lavformat)
AC_SUBST(LIBAVUTIL_LIBS,-lavutil)
AC_SUBST(LIBSWRESAMPLE_LIBS,-lswresample)
AC_SUBST(LIBSWSCAL_LIBS,-lswscale)
fi

if test $have_libavcodec != yes; then
	AC_CHECK_HEADERS(libavcodec/avcodec.h, 
		AC_CHECK_LIB(avcodec, avcodec_find_encoder, AC_SUBST(have_libavcodec, yes) LIBAVCODEC_LIBS="-lavcodec", AC_MSG_ERROR(Failed to find libavcodec), [-lavutil])
	)
fi
if test $have_libavfilter != yes; then
	AC_CHECK_HEADERS(libavfilter/avfilter.h, 
		AC_CHECK_LIB(avfilter, avfilter_get_by_name, AC_SUBST(have_libavfilter, yes) LIBAVFILTER_LIBS="-lavfilter", AC_MSG_ERROR(Failed to find libavfilter), [-lavcodec -lswscale -lavformat -lswresample])
	)
fi
if test $have_libavformat != yes; then
	AC_CHECK_HEADERS(libavformat/avformat.h, 
		AC_CHECK_LIB(avformat, avformat_new_stream, AC_SUBST(have_libavformat, yes) LIBAVFORMAT_LIBS="-lavformat", AC_MSG_ERROR(Failed to find libavformat), [-lavutil -lavcodec])
	)
fi
if test $have_libavutil != yes; then
	AC_CHECK_HEADERS(libavutil/avutil.h, 
		AC_CHECK_LIB(avutil, av_gettime, AC_SUBST(have_libavutil, yes) LIBAVUTIL_LIBS="-lavutil", AC_MSG_ERROR(Failed to find libavutil))
	)
fi
if test $have_libswresample != yes; then
	AC_CHECK_HEADERS(libswresample/swresample.h, 
		AC_CHECK_LIB(swresample, swr_convert, AC_SUBST(have_libswresample, yes) LIBSWRESAMPLE_LIBS="-lswresample", AC_MSG_ERROR(Failed to find libswresample), [-lavutil])
	)
fi
if test $have_libswscale != yes; then
	AC_CHECK_HEADERS(libswscale/swscale.h, 
		AC_CHECK_LIB(swscale, sws_scale, AC_SUBST(have_libswscale, yes) LIBSWSCAL_LIBS="-lswscale", AC_MSG_ERROR(Failed to find libswscale), [-lavutil])
	)
fi
test $have_libavcodec:$have_libavfilter:$have_libavformat:$have_libavutil:$have_libswresample:$have_libswscale = yes:yes:yes:yes:yes:yes &&
	AC_DEFINE_UNQUOTED(HAVE_FFMPEG, 1, HAVE_FFMPEG)



# OPENAL
AC_SUBST(have_openal, no)
PKG_CHECK_MODULES(OPENAL, openal >= ${OPENAL_MINVER}, AC_SUBST(have_openal, yes), 
	AC_MSG_WARN(Cannot find libopenal >= ${OPENAL_MINVER} using pkg-config)

	AC_SUBST(OPENAL_INCLUDES_FALLBACK,-I/usr/local/include)
	AC_SUBST(OPENAL_LIBS_FALLBACK,-L/usr/local/lib)
	AC_SUBST(OPENAL_LIBS,)
)
if test $have_openal != yes; then
	AC_CHECK_HEADERS(AL/alc.h AL/al.h AL/alext.h, 
		AC_CHECK_LIB(openal, alcLoopbackOpenDeviceSOFT, AC_SUBST(have_openal, yes) OPENAL_LIBS="-lopenal", [])
	)
fi
test $have_openal = yes &&
	AC_DEFINE_UNQUOTED(HAVE_OPENAL, 1, HAVE_OPENAL)


########################################################
# OpenOffice or LibreOffice
# --with-openoffice / --without-openoffice as argument to configure
########################################################
AC_LANG_PUSH([C++])
CPPFLAGS_SAVED="$CPPFLAGS"
have_openoffice=no
want_openoffice=check
path_openoffice_root=
path_openoffice_inccpp=
AC_SUBST(LIBOPENOFFICE_LIBADD, "")
AC_ARG_WITH(openoffice,
[  --with-openoffice=PATH             Build with OpenOffice/LibreOffice (Presentation sharing). PATH is optional.],
    if test "x$withval" = "xyes"; then
      want_openoffice=yes
    elif test "x$withval" = "xno"; then
      want_openoffice=no
    elif test "$withval"; then
      want_openoffice=yes
      path_openoffice_root=$withval
    fi,
)
test "x$path_openoffice_root" = "x" && path_openoffice_root=/opt/openoffice4
test "x$want_openoffice" != "xno" && LDFLAGS="-L$path_openoffice_root/sdk/lib $LDFLAGS" && CPPFLAGS="-I$path_openoffice_root/sdk/includecpp -I$path_openoffice_root/sdk/include $CPPFLAGS"
AC_SUBST(OPENOFFICE_ROOT, "$path_openoffice_root")
CPPFLAGS="-DUNX -DSAL_UNX -DCPPU_ENV=gcc3 $CPPFLAGS" #UNIX, GCC3+ (right values will be defined in the makefile using *.mk from the sdk)
# only if --without-openoffice not used
if test $want_openoffice != no; then
	# check for lib and headers
	AC_CHECK_HEADERS(com/sun/star/bridge/XUnoUrlResolver.hpp,
       	AC_CHECK_LIB(uno_sal, osl_getProcessWorkingDir, AC_DEFINE_UNQUOTED(HAVE_OPENOFFICE, 1, HAVE_OPENOFFICE) [have_openoffice=yes], 
		AC_DEFINE_UNQUOTED(HAVE_OPENOFFICE, 0, HAVE_OPENOFFICE) [have_openoffice=no]
	))	  
	# if openoffice not found and requested then, die.
  	test $have_openoffice:$want_openoffice = no:yes &&  
		AC_MSG_ERROR([You requested OpenOffice/LibreOffice(Presentation sharing) but not found...die])
fi
AM_CONDITIONAL([USE_OPENOFFICE], [test $have_openoffice = yes])
#test $have_openoffice = check && have_openoffice=no
AC_LANG_POP([C++])
CPPFLAGS="$CPPFLAGS_SAVED"

###########
# DOUBANGO
###########
AC_SUBST(have_tinysak, no)
AC_SUBST(have_tinynet, no)
AC_SUBST(have_tinyhttp, no)
AC_SUBST(have_tinysip, no)
AC_SUBST(have_tinydav, no)
AC_SUBST(have_tinysdp, no)
AC_SUBST(have_tinysigcomp, no)
AC_SUBST(have_tinymedia, no)
AC_SUBST(have_tinyxcap, no)
AC_SUBST(have_tinysms, no)
AC_SUBST(have_tinymsrp, no)
AC_SUBST(have_tinyrtp, no)
AC_SUBST(have_tinyipsec, no)
PKG_CHECK_MODULES(TINYSAK, tinySAK >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysak, yes), AC_MSG_WARN(Cannot find tinySAK >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYNET, tinyNET >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinynet, yes), AC_MSG_WARN(Cannot find tinyNET >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYHTTP, tinyHTTP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinyhttp, yes), AC_MSG_WARN(Cannot find tinyHTTP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYSIP, tinySIP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysip, yes), AC_MSG_WARN(Cannot find tinySIP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYDAV, tinyDAV >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinydav, yes), AC_MSG_WARN(Cannot find tinyDAV >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYSDP, tinySDP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysdp, yes), AC_MSG_WARN(Cannot find tinySDP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYSIGCOMP, tinySIGCOMP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysigcomp, yes), AC_MSG_WARN(Cannot find tinySIGCOMP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYMEDIA, tinyMEDIA >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinymedia, yes), AC_MSG_WARN(Cannot find tinyMEDIA >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYXCAP, tinyXCAP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinyxcap, yes), AC_MSG_WARN(Cannot find tinyxcap >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYSMS, tinySMS >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinysms, yes), AC_MSG_WARN(Cannot find tinySMS >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYMSRP, tinyMSRP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinymsrp, yes), AC_MSG_WARN(Cannot find tinyMSRP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYRTP, tinyRTP >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinyrtp, yes), AC_MSG_WARN(Cannot find tinyRTP >= ${DOUBANGO_MINVER} using pkg-config))
PKG_CHECK_MODULES(TINYIPSEC, tinyIPSec >= ${DOUBANGO_MINVER}, AC_SUBST(have_tinyipsec, yes), AC_MSG_WARN(Cannot find tinyIPSec >= ${DOUBANGO_MINVER} using pkg-config))

if test $have_tinysak:$have_tinynet:$have_tinyhttp:$have_tinysip:$have_tinydav:$have_tinysdp:$have_tinysigcomp:$have_tinymedia:$have_tinyxcap:$have_tinysms:$have_tinymsrp:$have_tinyrtp:$have_tinyipsec != yes:yes:yes:yes:yes:yes:yes:yes:yes:yes:yes:yes:yes ; then
	AC_MSG_WARN([
***
*** Failed to find all DOUBANGO libraries using pkg-config.
*** This most likely means the DOUBANGO pkg files (*.pc) are not 
*** in the default path where pkg-config expect them.
***
*** Please set your PKG_CONFIG_PATH environment variable to include the
*** directory in which the *.pc files are installed. Most likely
*** these files are installed in /usr/local/lib/pkgconfig; if that is the
*** case, try:
***
***     export PKG_CONFIG_PATH="\$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
***
*** then run this ./configure <your options here> again.
***
*** or, try:
***
***   PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./configure <your options here>
***
*** Anyways, we're trying another way to find DOUBANGO.
***
]);

AC_SUBST(DOUBANGO_INCLUDES_PATH,/usr/local/include)
AC_SUBST(DOUBANGO_LIBS_FALLBACK,-L/usr/local/lib)
AC_SUBST(TINYSAK_LIBS,-ltinySAK) AC_SUBST(TINYSAK_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysak)
AC_SUBST(TINYNET_LIBS,-ltinyNET) AC_SUBST(TINYNET_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinynet)
AC_SUBST(TINYHTTP_LIBS,-ltinyHTTP) AC_SUBST(TINYHTTP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinyhttp)
AC_SUBST(TINYSIP_LIBS,-ltinySIP) AC_SUBST(TINYSIP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysip)
AC_SUBST(TINYDAV_LIBS,-ltinyDAV) AC_SUBST(TINYDAV_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinydav)
AC_SUBST(TINYSDP_LIBS,-ltinySDP) AC_SUBST(TINYSDP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysdp)
AC_SUBST(TINYSIGCOMP_LIBS,-ltinySIGCOMP) AC_SUBST(TINYSIGCOMP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysigcomp)
AC_SUBST(TINYMEDIA_LIBS,-ltinyMEDIA) AC_SUBST(TINYMEDIA_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinymedia)
AC_SUBST(TINYXCAP_LIBS,-ltinyXCAP) AC_SUBST(TINYXCAP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinyxcap)
AC_SUBST(TINYSMS_LIBS,-ltinySMS) AC_SUBST(TINYSMS_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinysms)
AC_SUBST(TINYMSRP_LIBS,-ltinyMSRP) AC_SUBST(TINYMSRP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinymsrp)
AC_SUBST(TINYRTP_LIBS,-ltinyRTP) AC_SUBST(TINYRTP_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinyrtp)
AC_SUBST(TINYIPSEC_LIBS,-ltinyIPSec) AC_SUBST(TINYIPSEC_CFLAGS,-I${DOUBANGO_INCLUDES_PATH}/tinyipsec)
fi

if test $have_tinysak != yes; then
	AC_CHECK_HEADERS(tinysak/tinysak_config.h, 
		AC_CHECK_LIB(tinySAK, tsk_object_new, AC_SUBST(have_tinysak, yes), AC_MSG_ERROR(Failed to find libtinySAK), [${LIBDL}])
	)
fi
if test $have_tinynet != yes; then
	AC_CHECK_HEADERS(tinynet/tinynet_config.h, 
		AC_CHECK_LIB(tinyNET, tnet_startup, AC_SUBST(have_tinynet, yes), AC_MSG_ERROR(Failed to find libtinyNET), [-ltinySAK ${LIBDL}])
	)
fi
if test $have_tinyhttp != yes; then
	AC_CHECK_HEADERS(tinyhttp/tinyhttp_config.h, 
		AC_CHECK_LIB(tinyHTTP, thttp_stack_create, AC_SUBST(have_tinyhttp, yes), AC_MSG_ERROR(Failed to find libtinyHTTP), [-ltinySAK -ltinyNET ${LIBDL}])
	)
fi
if test $have_tinysip != yes; then
	AC_CHECK_HEADERS(tinysip/tinysip_config.h, 
		AC_CHECK_LIB(tinySIP, tsip_stack_create, AC_SUBST(have_tinysip, yes), AC_MSG_ERROR(Failed to find libtinySIP), [-ltinySAK -ltinyNET -ltinyHTTP -ltinyIPSec -ltinySDP -ltinyMEDIA -ltinySIGCOMP ${LIBDL} -lm])
	)
fi
if test $have_tinydav != yes; then
	AC_CHECK_HEADERS(tinydav/tinydav_config.h, 
		AC_CHECK_LIB(tinyDAV, tdav_init, AC_SUBST(have_tinydav, yes), AC_MSG_ERROR(Failed to find libtinyDAV), [-ltinySAK -ltinyNET -ltinySDP -ltinyRTP -ltinyMEDIA -ltinyMSRP ${LIBDL}])
	)
fi
if test $have_tinysdp != yes; then
	AC_CHECK_HEADERS(tinysdp/tinysdp_config.h, 
		AC_CHECK_LIB(tinySDP, tsdp_message_create, AC_SUBST(have_tinysdp, yes), AC_MSG_ERROR(Failed to find libtinySDP), [-ltinySAK ${LIBDL}])
	)
fi
if test $have_tinysigcomp != yes; then
	AC_CHECK_HEADERS(tinysigcomp/tinysigcomp_config.h, 
		AC_CHECK_LIB(tinySIGCOMP, tcomp_manager_create, AC_SUBST(have_tinysigcomp, yes), AC_MSG_ERROR(Failed to find libtinySIGCOMP), [-ltinySAK ${LIBDL} -lm])
	)
fi
if test $have_tinymedia != yes; then
	AC_CHECK_HEADERS(tinymedia/tinymedia_config.h, 
		AC_CHECK_LIB(tinyMEDIA, tmedia_session_mgr_create, AC_SUBST(have_tinymedia, yes), AC_MSG_ERROR(Failed to find libtinyMEDIA), [-ltinySAK -ltinyNET -ltinySDP ${LIBDL}])
	)
fi
if test $have_tinyxcap != yes; then
	AC_CHECK_HEADERS(tinyxcap/tinyxcap_config.h, 
		AC_CHECK_LIB(tinyXCAP, txcap_stack_create, AC_SUBST(have_tinyxcap, yes), AC_MSG_ERROR(Failed to find libtinyXCAP), [-ltinySAK -ltinyHTTP -ltinyNET ${LIBDL}])
	)
fi
if test $have_tinysms != yes; then
	AC_CHECK_HEADERS(tinysms/tinysms_config.h, 
		AC_CHECK_LIB(tinySMS, tsms_tpdu_message_init, AC_SUBST(have_tinysms, yes), AC_MSG_ERROR(Failed to find libtinySMS), [-ltinySAK ${LIBDL}])
	)
fi
if test $have_tinymsrp != yes; then
	AC_CHECK_HEADERS(tinymsrp/tinymsrp_config.h, 
		AC_CHECK_LIB(tinyMSRP, tmsrp_create_report, AC_SUBST(have_tinymsrp, yes), AC_MSG_ERROR(Failed to find libtinyMSRP), [-ltinySAK -ltinyNET -ltinySDP -ltinyHTTP ${LIBDL}])
	)
fi
if test $have_tinyrtp != yes; then
	AC_CHECK_HEADERS(tinyrtp/tinyrtp_config.h, 
		AC_CHECK_LIB(tinyRTP, trtp_manager_create, AC_SUBST(have_tinyrtp, yes), AC_MSG_ERROR(Failed to find libtinyRTP), [-ltinySAK -ltinyNET -ltinyMEDIA ${LIBDL}])
	)
fi
if test $have_tinyipsec != yes; then
	AC_CHECK_HEADERS(tinyipsec/tinyipsec_config.h, 
		AC_CHECK_LIB(tinyIPSec, tipsec_ctx_create, AC_SUBST(have_tinyipsec, yes) , AC_MSG_ERROR(Failed to find libtinyIPSec), [-ltinySAK ${LIBDL}])
	)
fi




AC_OUTPUT(
Makefile
)


AC_MSG_NOTICE([
 
**************************************************************************
 				CONGRATULATIONS
**************************************************************************
Target OS:                                     ${target_os}
Target CPU:                                    ${target_cpu}
Host setup:                                    ${host}
Install prefix:                                ${prefix}
Compiler:                                      ${CC}

OpenAL Soft (3D audio):                        ${have_openal}

OpenOffice/LibreOffice (Presentaion sharing):  ${have_openoffice}

DOUBANGO
     MIN_VER:                                  ${DOUBANGO_MINVER}
     TINYSAK:                                  ${have_tinysak}
     TINYNET:                                  ${have_tinynet}
     TINYHTTP:                                 ${have_tinyhttp}
     TINYSIP:                                  ${have_tinysip}
     TINYDAV:                                  ${have_tinydav}
     TINYSDP:                                  ${have_tinysdp}
     TINYSIGCOMP:                              ${have_tinysigcomp}
     TINYMEDIA:                                ${have_tinymedia}
     TINYXCAP:                                 ${have_tinyxcap}
     TINYSMS:                                  ${have_tinysms}
     TINYMSRP:                                 ${have_tinymsrp}
     TINYRTP:                                  ${have_tinyrtp}
     TINYIPSEC:                                ${have_tinyipsec}

FFMPEG

    AVCODEC:                                   ${have_libavcodec}
    AVFILTER                                   ${have_libavfilter}
    AVFORMAT                                   ${have_libavformat}
    AVUTIL                                     ${have_libavutil}
    SWRESAMPLE                                 ${have_libswresample}
    SWSCAL                                     ${have_libswscale}



Report issues at https://groups.google.com/group/opentelepresence

-------------------------------------------------------------------------
Next steps
  1) run 'make' to build the source
  2) run 'make install' to install binaries
  3) run 'make samples' to install sample files (required at least once)
-------------------------------------------------------------------------

])






