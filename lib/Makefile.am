lib_LTLIBRARIES = libjkiss.la
include_HEADERS = jkiss.h
libjkiss_la_SOURCES = jkiss.c jkiss.h
libjkiss_la_LDFLAGS = -version-info $(VERSION)

AM_CFLAGS = -DDEV_RANDOM=\"${random_device}\"

if JKISS_LIBBSD
  AM_CFLAGS += -lbsd
endif
