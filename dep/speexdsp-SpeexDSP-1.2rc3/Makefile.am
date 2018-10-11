## Process this file with automake to produce Makefile.in. -*-Makefile-*-

# To disable automatic dependency tracking if using other tools than
# gcc and gmake, add the option 'no-dependencies'
AUTOMAKE_OPTIONS = 1.8
ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = speexdsp.pc

EXTRA_DIST = SpeexDSP.spec SpeexDSP.spec.in SpeexDSP.kdevelop speexdsp.pc.in README.blackfin

#Fools KDevelop into including all files
SUBDIRS = libspeexdsp include doc win32 symbian ti

DIST_SUBDIRS = libspeexdsp include doc win32 symbian ti

rpm: dist
	rpmbuild -ta ${PACKAGE}-${VERSION}.tar.gz
