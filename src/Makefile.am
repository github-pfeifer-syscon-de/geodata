## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir). "\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GENERICGLM_CFLAGS) \
	$(GENERICIMG_CFLAGS) \
	$(GTKMM_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(LIBSOUP3_CFLAGS) \
	$(JSONGLIB1_CFLAGS)

libgeodata_la_CFLAGS = -Wall

lib_LTLIBRARIES = libgeodata.la

libgeodata_la_CXXFLAGS = \
	-Wall -Wpedantic

libgeodata_la_LDFLAGS = \
	@EXTRA_LDFLAGS@ \
	-version-info @version_info@

libgeodata_la_SOURCES = \
	Spoon.cpp \
	Weather.cpp \
	RealEarth.cpp \
	WebMapService.cpp \
	GeoCoordinate.cpp \
	MapProjection.cpp \
	GeoJsonSimplifyHandler.cpp \
	GeoJson.cpp

libgeodata_la_LIBADD =  \
	$(GENERICGLM_LIBS) \
	$(GENERICIMG_LIBS) \
	$(GTKMM_LIBS) \
	$(GLIBMM_LIBS) \
	$(LIBSOUP3_LIBS) \
	$(JSONGLIB1_LIBS)


## using pkginclude_HEADERS will use include dir with packagename
##   (include_HEADERS  will use #install#/include)
pkginclude_HEADERS = \
	Spoon.hpp \
	Weather.hpp \
	RealEarth.hpp \
	WebMapService.hpp \
	GeoCoordinate.hpp \
	MapProjection.hpp \
	GeoJsonSimplifyHandler.hpp \
	GeoJson.hpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = geodata.pc

EXTRA_DIST = \
	geodata.pc.in
