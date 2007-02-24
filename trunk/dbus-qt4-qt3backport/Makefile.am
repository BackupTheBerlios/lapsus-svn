INCLUDES=-I$(srcdir) $(QT_INCLUDES) $(DBUS_CFLAGS) \
	     -DDBUS_COMPILATION

noinst_LTLIBRARIES=libdbus-1-qt3.la

libdbus_1_qt3_la_SOURCES =               \
	$(srcdir)/dbus/qdbusproxy.h      \
	$(srcdir)/dbus/qdbusserver.h     \
	$(srcdir)/qdbusconnection_p.h    \
	$(srcdir)/qdbusconnection.cpp    \
	$(srcdir)/qdbuserror.cpp         \
	$(srcdir)/qdbusintegrator.cpp    \
	$(srcdir)/qdbusmarshall.cpp      \
	$(srcdir)/qdbusmessage.cpp       \
	$(srcdir)/qdbusserver.cpp        \
	$(srcdir)/qdbusproxy.cpp         \
	$(srcdir)/qdbusdata.cpp          \
	$(srcdir)/qdbusdatalist.cpp

$(srcdir)/qdbusserver.cpp: qdbusserver.moc
$(srcdir)/qdbusconnection.cpp: qdbusconnection.moc
$(srcdir)/qdbusproxy.cpp: qdbusproxy.moc

CLEANFILES = qdbusserver.moc qdbusconnection.moc qdbusproxy.moc

# files that are an exception
qdbusserver.moc: dbus/qdbusserver.h
	$(MOC) -o qdbusserver.moc $(srcdir)/dbus/qdbusserver.h

qdbusconnection.moc: qdbusconnection_p.h
	$(MOC) -o qdbusconnection.moc $(srcdir)/qdbusconnection_p.h

qdbusproxy.moc: dbus/qdbusproxy.h
	$(MOC) -o qdbusproxy.moc $(srcdir)/dbus/qdbusproxy.h
