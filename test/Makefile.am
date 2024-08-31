
TESTS = geo_test

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)."\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GENERICGLM_CFLAGS) \
	$(GENERICIMG_CFLAGS) \
	$(GEODATA_CFLAGS) \
	-I../src

AM_CFLAGS= \
	-Wall -Wpedantic

AM_CXXFLAGS = \
	-Wall -Wpedantic

check_PROGRAMS = geo_test

geo_test_LDADD = \
	-L../src/.libs -lgeodata $(GEODATA_LIBS)

geo_test_SOURCES = \
    geo_test.cpp

