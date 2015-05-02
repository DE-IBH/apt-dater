/* apt-dater - terminal-based remote package update manager
 *
 * Authors:
 *   Thomas Liske <liske@ibh.de>
 *
 * Copyright Holder:
 *   2008-2015 (C) IBH IT-Service GmbH [https://www.ibh.de/apt-dater/]
 *
 * License:
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this package; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "apt-dater.h"
#include "glue.h"

#ifndef HAVE_GLIB_TIMEOUT_ADD_SECONDS
#warning Your glib2 does not support g_timeout_seconds (glib >= 2.13.5) - using glue code.
guint
g_timeout_add_seconds(guint interval, GSourceFunc function, gpointer data) {
    g_timeout_add(interval*1000, function, data);
}
#endif

#ifndef HAVE_GLIB_SPAWN_CHECK_EXIT_STATUS
#warning Your glib2 does not support g_spawn_check_exit_status (glib >= 2.33.4) - using glue code.
/* Implementation has been taken from:
 *
 * gspawn.[ch] - Process launching
 *
 *  Copyright 2000 Red Hat, Inc.
 *  g_execvpe implementation based on GNU libc execvp:
 *   Copyright 1991, 92, 95, 96, 97, 98, 99 Free Software Foundation, Inc.
 */
#include <sys/types.h>
#include <sys/wait.h>
gboolean
g_spawn_check_exit_status(gint exit_status, GError **error) {
  gboolean ret = FALSE;

  if (WIFEXITED (exit_status))
    {
      if (WEXITSTATUS (exit_status) != 0)
	{
	  g_set_error (error, 0, WEXITSTATUS (exit_status),
		       _("Child process exited with code %ld"),
		       (long) WEXITSTATUS (exit_status));
	  goto out;
	}
    }
  else if (WIFSIGNALED (exit_status))
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Child process killed by signal %ld"),
		   (long) WTERMSIG (exit_status));
      goto out;
    }
  else if (WIFSTOPPED (exit_status))
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Child process stopped by signal %ld"),
		   (long) WSTOPSIG (exit_status));
      goto out;
    }
  else
    {
      g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
		   _("Child process exited abnormally"));
      goto out;
    }

  ret = TRUE;
 out:
  return ret;
}
#endif
