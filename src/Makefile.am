bin_PROGRAMS = iptux
iptux_SOURCES = AboutIptux.cpp Command.cpp Control.cpp CoreThread.cpp \
	DetectPal.cpp DialogGroup.cpp DialogPeer.cpp IptuxSetup.cpp MainWindow.cpp Pal.cpp \
	RecvFile.cpp RevisePal.cpp SendFile.cpp ShareFile.cpp StatusIcon.cpp Transport.cpp \
	UdpData.cpp baling.cpp iptux.cpp my_chooser.cpp my_entry.cpp my_file.cpp output.cpp \
	support.cpp utils.cpp

# set the include path found by configure
# set the include path found by configure
INCLUDES= $(all_includes) \
	@GTK_CFLAGS@ \
	@GTHREAD_CFLAGS@ \
	@GCONF_CFLAGS@ \
	-DGETTEXT_PACKAGE=\""@GETTEXT_PACKAGE@"\" \
	-D__LOCALE_DIR=\""$(datadir)/locale"\" \
	-D__LOGO_DIR=\""$(datadir)/pixmaps"\" \
	-D__BACK_DIR=\""$(datadir)/iptux/back"\" \
	-D__ICON_DIR=\""$(datadir)/iptux/icon"\" \
	-D__TIP_DIR=\""$(datadir)/iptux/tip"\" \
	-D__IP_TUX__ \
	-DWARNING \
	-DMESSAGE \
	-DDEBUG

# the library search path.
iptux_LDADD = @GTK_LIBS@ @GTHREAD_LIBS@ @GCONF_LIBS@ $(INTLLIBS)
iptux_LDFLAGS = $(all_libraries)
noinst_HEADERS = AboutIptux.h Command.h Control.h CoreThread.h DetectPal.h \
	DialogGroup.h DialogPeer.h IptuxSetup.h MainWindow.h Pal.h RecvFile.h RevisePal.h \
	SendFile.h ShareFile.h StatusIcon.h Transport.h UdpData.h baling.h common.h face.h \
	ipmsg.h my_chooser.h my_entry.h my_file.h net.h output.h support.h sys.h udt.h \
	utils.h
