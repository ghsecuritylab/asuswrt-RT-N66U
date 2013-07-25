#
# include/Makefile
#
# Copyright 2005, Broadcom, Inc.
#
# $Id: Makefile 241376 2011-02-18 03:19:15Z stakita $
#

SRCBASE	= ..

TARGETS	= wpsvers.h


all release:
	bash wpsvers.sh
	mv wpsvers.h wps_version.h

clean:
	rm -rf ${TARGETS} *.prev


.PHONY: all release clean
