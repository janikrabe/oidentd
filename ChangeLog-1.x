Fri Feb  9 05:11:08 EST 2001    Ryan McCabe <ryan@numb.org>

    * Increased the allowed length of spoofed ident replies to 512
      characters.

    * Simply drop connections that send a malformed request instead of
      replying with INVALID-PORT.

Fri Jan 12 23:40:56 EST 2001    Ryan McCabe <ryan@numb.org>

    * Applied patches from Arkadiusz Miskiewicz <misiek@pld.ORG.PL>
      to fix some IPv6 problems 

Sat Dec 09 17:47:15 EST 2000    Ryan McCabe <ryan@numb.org>

    * Fixes for IPv6 support from Daniel Brafford.

Thu Dec 07 00:03:58 EST 2000    Ryan McCabe <ryan@numb.org>

    * Applied patch from Daniel Brafford <pod@charter.net> for
      IPv6 support on Linux.

Sun Oct 22 14:54:06 EDT 2000    Ryan McCabe    <ryan@numb.org>

    * Released as version 1.7.1

Thu Oct 19 17:02:22 EDT 2000    Ryan McCabe <ryan@numb.org>

    * Applied a patch from Sean 'Shaleh' Perry <shaleh@debian.org> to
      fix a bug that prevented oidentd from running in stand-alone mode.

Tue Oct 03 02:12:39 EDT 2000    Ryan McCabe <ryan@numb.org>

    * Ported to FreeBSD 4.*

Mon Aug 21 03:58:52 EDT 2000    Ryan McCabe <ryan@numb.org>

    * Applied patch from Slawomir Piotrowski <slawek@telsatgp.com.pl>
      adding IP masq support for OpenBSD.

    * Code cleanup.

Wed May 31 14:58:18 EDT 2000    Ryan McCabe <ryan@numb.org>

    * Applied patch from Bjarni R. Einarsson <bre at mmedia.is> that
      adds libudb support.

Sun Jan 16 12:32:35 EST 2000    Ryan McCabe    <ryan@numb.org>

    * Fix a typo.

Sat Jan 15 13:28:58 EST 2000    Ryan McCabe    <ryan@numb.org>

    * Added 'F' and 'O' options.  Original patch was from
      Jeroen Massar <fuzzel@unfix.org>

Fri Jan 14 23:55:24 EST 2000    Ryan McCabe    <ryan@numb.org>

    * Integraded patch from Guus Sliepen <sliepen@phys.uu.nl> that adds
      netfilter support to oidentd.

Sun Jan 02 22:43:13 EST 2000    Ryan McCabe    <ryan@numb.org>

    * Added the ability to specify an identd response for users in the
      /etc/identd.spoof file.  For example, an entry of "nobody:UNKNOWN"
      will cause the string "UNKNOWN" to be returned upon every successful
      lookup of a connection owned by user nobody.

    * Code cleanups.

    * Documentation updates.

Sun Dec 12 20:06:21 EST 1999    Ryan McCabe    <ryan@numb.org>

    * Fix a bug that caused an incorrect port to be returned upon a
      successful forward of an ident request.
        - Reported by Pawel Mojski <pawcio@free.net.pl>

Sun Aug 22 21:17:15 EDT 1999    Ryan McCabe    <ryan@numb.org>

    * Release as version 1.6.3.

Sun Jul 25 15:06:00 EDT 1999    Ryan McCabe    <ryan@numb.org>

    * Merged in Solaris 2.x support from ultr0s <ultros@ojnk.net>

    * Added a -q option (quiet) to suppress logging messages via syslog.

Mon May 31 01:29:24 EDT 1999    Ryan McCabe    <ryan@numb.org>

    * Fixed a bug that caused specifying a forward port other than 113
      to fail.

    * Fixed a typo in oidentd.c.
        - Thanks to Stuart Adamson <stuart.adamson@compsoc.net> for
          pointing both out.

Fri May 28 19:22:13 EDT 1999    Ryan McCabe    <ryan@numb.org>

    * Released version 1.6.2

Thu May 27 02:56:19 EDT 1999    Ryan McCabe    <ryan@numb.org>

    * Documentation updates.

    * Fixed a problem with wait mode.

    * Add new flag -x <username>.. If a lookup fails and this flag is
      set, the username specified will be returned to the client as if
      the query had succeeded.

Tue Apr  6 19:31:55 EDT 1999    Ryan McCabe    <ryan@numb.org>

    * Cleaned up the forward timeout stuff.

    * Allow service names when specifying ports.

    * Other small cleanups.

Mon Mar 29 02:29:59 EST 1999    Ryan McCabe    <ryan@numb.org>

    * Changed the default maximum length of spoofed identd responses
      to 24 (was 10).

Sat Mar  6 20:33:19 EST 1999    Ryan McCabe    <ryan@numb.org>

    * If our connection times out to a host we're forwarding to,
      fallback to /etc/oidentd.users instead of exiting.
      (Bug reported by Lourdes Jones <lourdes@ljones.com>)

    * Changed timeout interval to 15 seconds when forwarding a
      connection to a masqd host (was 30).

Fri Feb 26 03:30:31 EST 1999    Ryan McCabe    <ryan@numb.org>

    * dmess0r noticed that when running in standalone mode, -r was
      broken (it would use the same username over and over again).
      This is fixed now.

Fri Feb 19 17:58:45 EST 1999    Ryan McCabe    <ryan@numb.org>

    * Allow masks in /etc/oidentd.users (eg, 192.168.1.0/24)

    * Allow hostnames in /etc/oidentd.users

Mon Feb  1 01:15:17 EST 1999    Ryan McCabe    <ryan@numb.org>

    * All users are allowed to spoof with -S even if /etc/identd.spoof
      doesn't exist.

    * Install the manual page.

Sat Jan 23 21:22:29 EST 1999    Ryan McCabe    <ryan@numb.org>

    * Bumped version number up to 1.6.0.

    * Rewrote the INSTALL file.

    * Added the '-P' option, which enables oidentd to allow for a
      proxy to successfully issue a query for a masqueraded
      connection.

    * Fixed (hopefully) the forwarding stuff.

Tue Jan 12 22:43:07 EST 1999    Ryan McCabe    <ryan@numb.org>

    * Bumped version number up to 1.5.1.

    * Regenerate scripts using autoconf 2.13.

    * More minor code and documentation updates.

Sat Jan  9 20:04:53 EST 1999    Ryan McCabe    <ryan@numb.org>

    * Various code and documentation cleanups.

    * Removed CHARSET flag.

Mon Jan  4 18:22:51 EST 1999    Ryan McCabe    <ryan@numb.org>

    * Released version 1.5

    * Added new option '-S' that does the same thing as '-s' only all
      users are allowed to spoof their identd replies except those
      users listed in the oidentd users file (by default
      /etc/identd.spoof)
