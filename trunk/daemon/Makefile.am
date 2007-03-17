
# set the include path for X, qt and KDE
INCLUDES = $(all_includes) -I$(top_srcdir)/dbus-qt4-qt3backport -I$(top_srcdir)/global $(DBUS_CFLAGS) $(ALSA_CFLAGS)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables

sbin_PROGRAMS = lapsusd

# the application source, library search path, and link libraries
lapsusd_SOURCES = acpi_event_parser.cpp lapsus_daemon.cpp lapsus_dbus.cpp main.cpp \
sys_backend.cpp sys_asus.cpp sys_ibm.cpp


lapsusd_LDFLAGS = $(KDE_RPATH) $(all_libraries)

lapsusd_LDADD = $(LIB_QT) $(DBUS_LIBS) $(ALSA_LIBS) $(top_builddir)/dbus-qt4-qt3backport/libdbus-1-qt3.la
