/***************************************************************************
 *   Copyright (C) 2007 by Jakub Schmidtke                                 *
 *   sjakub@users.berlios.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include <qapplication.h>

#include "lapsus.h"
#include "lapsus_daemon.h"
#include "acpi_event_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>


#define ACPI_SOCK		"/var/run/acpid.socket"
#define ACPI_EVENTS		"/proc/acpi/event"

#define DAEMON_NAME		"lapsusd"
#define LOCK_FILE		("/var/lock/" DAEMON_NAME )

#define DAEMON_START_CHILD	2
#define DAEMON_OK		1
#define DAEMON_ERR		0

#define qPrintable(str)         (str.ascii())


static bool doDaemonize = true;
static int child_stat = 0;
static pid_t my_parent = 0;

int run_child(int acpi_fd, int argc, char *argv[]);

int lock()
{
	char lock_buffer[21];
	int fd, pid, n;

	/* While, so we can re-create this file if stale lock is removed */
	while ((fd = open(LOCK_FILE, O_EXCL | O_CREAT | O_RDWR, 0644)) < 0)
	{
		if (errno == EEXIST && (fd = open(LOCK_FILE, O_RDONLY)) >= 0)
		{
			/* Read the lock file to find out who is the owner */

			n = read(fd, lock_buffer, 20);
			close(fd);

			if (n <= 0)
			{
				fprintf(stderr, "Can't read pid from lock file %s, code %d (%s)\n",
					LOCK_FILE, errno, strerror(errno) );
				return 0;
			}
			else
			{
				lock_buffer[n] = 0;
				pid = atoi(lock_buffer);

				if (kill(pid, 0) == -1 && errno == ESRCH)
				{
					/* pid no longer exists - remove the lock file */
					if (unlink(LOCK_FILE) != 0)
					{
						fprintf(stderr, "Couldn't remove stale lock %s\n", LOCK_FILE);
						return 0;
					}

					fprintf(stderr, "Removed stale lock %s (pid %d)\n", LOCK_FILE, pid);
				}
				else if (pid == getpid())
				{
					if (unlink(LOCK_FILE) != 0)
					{
						fprintf(stderr, "Couldn't remove stale lock %s (pid %d - ME?!?)\n",
							LOCK_FILE, pid);
						return 0;
					}

					fprintf(stderr, "Removed stale lock %s (pid %d - ME?!?)\n", LOCK_FILE, pid);
				}
				else
				{
					fprintf(stderr, "%s is locked by pid %d\n", LOCK_FILE, pid);
					return 0;
				}
			}
		}
		else
		{
			fprintf(stderr, "Can't create lock file %s, code %d (%s)\n", LOCK_FILE, errno, strerror(errno) );
			return 0;
		}
	}

	snprintf(lock_buffer, 20, "%d\n", getpid());
	write(fd, lock_buffer, 20);
	close(fd);

	return 1;
}

void unlock()
{
	unlink(LOCK_FILE);
}

static void child_handler(int signum)
{
	if (signum == SIGUSR1)
		child_stat = 1;
	else if (signum == SIGCHLD)
		child_stat = -1;
}

static int daemonize()
{
	pid_t pid, sid;

	/* already a daemon */
	if ( getppid() == 1 ) return DAEMON_ERR;

	/* Trap signals that we expect to recieve */
	signal(SIGCHLD, child_handler);
	signal(SIGUSR1, child_handler);

	/* Fork off the parent process */
	pid = fork();

	if (pid < 0)
	{
		fprintf(stderr, "Unable to fork daemon, code = %d (%s)\n",
			errno, strerror(errno) );
		return DAEMON_ERR;
	}

	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0)
	{
		/* Wee need to wait for the info about our child (or timeout) */
		if (!child_stat)
		{
			sleep(10);
		}

		if (child_stat != 1)
		{
			if (!child_stat)
				fprintf(stderr, "Timed-out waiting for child process status.\n");

			return DAEMON_ERR;
		}

		return DAEMON_OK;
	}

	if (!lock())
	{
		return DAEMON_ERR;
	}

	/* At this point we are executing as the child process */
	my_parent = getppid();

	/* Cancel certain signals */
	signal(SIGCHLD,SIG_DFL); /* A child process dies */
	signal(SIGTSTP,SIG_IGN); /* Various TTY signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */
	signal(SIGTERM,SIG_DFL); /* Die on SIGTERM */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();

	if (sid < 0)
	{
		fprintf(stderr, "Unable to create a new session, code = %d (%s)\n",
			errno, strerror(errno) );
		return 0;
	}

	/* Change the current working directory.  This prevents the current
	directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0)
	{
		fprintf(stderr, "Unable to change directory to /, code = %d (%s)\n",
			errno, strerror(errno) );
		return DAEMON_ERR;
	}

	return DAEMON_START_CHILD;
}

int get_acpi_fd()
{
	int acpi_fd;
	size_t len;
	struct sockaddr_un addr;

	len = sizeof(addr.sun_path)-1;
	bzero((void*) &addr,sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, ACPI_SOCK,len);

	addr.sun_path[len] = '\0';
	len = sizeof(addr.sun_family) + strlen(addr.sun_path);

	acpi_fd = socket(PF_UNIX,SOCK_STREAM,0);

	if (acpi_fd < 0)
	{
		fprintf(stderr, "Error opening " ACPI_SOCK ": %s. Is acpid running?\n", strerror(errno));
	}
	else
	{
		if (connect(acpi_fd, (struct sockaddr *) &addr, len) < 0)
		{
			fprintf(stderr, "Error connecting to " ACPI_SOCK ": %s. Is acpid running?\n", strerror(errno));
			acpi_fd = -1;
		}
	}

	if (acpi_fd < 0)
	{
 		fprintf(stderr, "Trying to use " ACPI_EVENTS " file. It won't be available for other programs!\n");

		acpi_fd = open(ACPI_EVENTS, O_RDONLY);

		if (acpi_fd < 0)
		{
			fprintf(stderr, "Error opening " ACPI_EVENTS " for reading: %s.\n"
				"Could not get access to ACPI events - terminating.\n", strerror(errno));
			return -1;
		}
	}

	fcntl(acpi_fd, F_SETFL, fcntl(acpi_fd, F_GETFL, 0) | O_NONBLOCK);

	return acpi_fd;
}


int main( int argc, char *argv[] )
{
	int d_stat;
	int acpi_fd;

	if (argc > 1)
	{
		if (!strcmp("--help", argv[1]))
		{
			printf("\nDaemon for 'Lapsus' utility.\n");
			printf("\nIt does not take any parameters.\n");
			printf("\nVisit http://lapsus.berlios.de for details.\n\n\n");
			printf("Copyright (c) 2007 Jakub Schmidtke\n\n");
			printf("This program is distributed under the terms of the GPL v2.\n");
			printf("Visit http://www.gnu.org/licenses/gpl.html for details.\n\n");

			return EXIT_SUCCESS;
		}

		fprintf(stderr, "Error: See lapsusd --help\n");
		return EXIT_FAILURE;
	}

	if ( (acpi_fd = get_acpi_fd()) < 0)
		return EXIT_FAILURE;

	if (doDaemonize)
		d_stat = daemonize();

	if (!doDaemonize)
	{
		int c_stat = run_child(acpi_fd, argc, argv);

		close(acpi_fd);
		unlock();
		return c_stat;
		
	}
	else if (d_stat == DAEMON_START_CHILD)
	{
		int c_stat = 0;

		/* Initialize the logging interface */
		openlog( DAEMON_NAME, LOG_PID, LOG_LOCAL5 );
		syslog( LOG_INFO, "starting" );

		c_stat = run_child(acpi_fd, argc, argv);

		close(acpi_fd);
		unlock();
		syslog( LOG_NOTICE, "terminated" );
		return c_stat;
	}
	else if (d_stat == DAEMON_ERR)
	{
		close(acpi_fd);
		fprintf(stderr, "Error runing " DAEMON_NAME " daemon!\n");
		return EXIT_FAILURE;
	}

	close(acpi_fd);
	return EXIT_SUCCESS;
}

int run_child(int acpi_fd, int argc, char *argv[])
{
	QApplication app(argc, argv, FALSE);
	LapsusDaemon lapDaemon;

	if (!lapDaemon.isValid())
	{
		fprintf(stderr, "No supported hardware found. Is asus-laptop kernel module loaded?\n");
		return EXIT_FAILURE;
	}

	QDBusConnection connection = QDBusConnection::addConnection(QDBusConnection::SystemBus);

	if (!connection.isConnected())
	{
		fprintf(stderr, "Cannot connect to the D-BUS system bus!\n");
		return EXIT_FAILURE;
	}
	
	if (!connection.requestName(LAPSUS_SERVICE_NAME))
	{
		fprintf(stderr, "Error registering D-BUS Lapsus Service Name '%s': %s\n",
			LAPSUS_SERVICE_NAME, qPrintable(connection.lastError().message()));
		return EXIT_FAILURE;
	}

	ACPIEventParser acpiEventParser(acpi_fd);

	lapDaemon.addACPIParser(&acpiEventParser);

	if (!lapDaemon.registerObject(&connection))
	{
		fprintf(stderr, "Error registering D-BUS Lapsus Object Path '%s': %s\n",
			LAPSUS_OBJECT_PATH, qPrintable(connection.lastError().message()));
		return EXIT_FAILURE;
	}

	if (doDaemonize)
	{
		/* Redirect standard files to /dev/null */
		freopen( "/dev/null", "r", stdin);
		freopen( "/dev/null", "w", stdout);
		freopen( "/dev/null", "w", stderr);

		/* Tell parent that everything went fine */
		kill(my_parent, SIGUSR1);
	}
	
	return app.exec();
}
