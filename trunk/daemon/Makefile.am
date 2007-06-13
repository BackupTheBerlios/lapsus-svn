
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

# Lets change default KDE prefix. Lapsusd might be used by anything and
# does not need or use KDE - it uses only Qt.
# In the future it might be a separate daemon package, with various
# frontends (KDE, Gnome?) as also separate packages. So it should go
# to /usr/sbin rather then to $KDEDIR/sbin directory.
prefix = /usr

# the application source, library search path, and link libraries
lapsusd_SOURCES = acpi_event_parser.cpp lapsus_signal.cpp lapsus_daemon.cpp lapsus_dbus.cpp main.cpp \
lapsus_init.cpp lapsus_config.cpp lapsus_module.cpp modules_list.cpp lapsus_mixer.cpp sys_backend.cpp \
sys_asus.cpp sys_ibm.cpp synaptics.cpp \
../global/lapsus_validator.cpp


lapsusd_LDFLAGS = $(KDE_RPATH) $(all_libraries)

lapsusd_LDADD = $(LIB_QT) $(DBUS_LIBS) $(ALSA_LIBS) $(top_builddir)/dbus-qt4-qt3backport/libdbus-1-qt3.la
