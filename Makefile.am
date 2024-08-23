## Process this file with automake to produce Makefile.in

SUBDIRS = src test

dist_doc_DATA = \
	README.md \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS

ACLOCAL_AMFLAGS=-I m4

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)
