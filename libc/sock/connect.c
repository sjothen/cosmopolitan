/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/calls/strace.internal.h"
#include "libc/dce.h"
#include "libc/intrin/asan.internal.h"
#include "libc/sock/internal.h"
#include "libc/sock/sockdebug.h"
#include "libc/sysv/errfuns.h"

/**
 * Connects socket to remote end.
 *
 * ProTip: Connectionless sockets, e.g. UDP, can be connected too. The
 * benefit is not needing to specify the remote address on each send. It
 * also means getsockname() can be called to retrieve routing details.
 *
 * @return 0 on success or -1 w/ errno
 * @asyncsignalsafe
 */
int connect(int fd, const void *addr, uint32_t addrsize) {
  int rc;
  if (addr && !(IsAsan() && !__asan_is_valid(addr, addrsize))) {
    _firewall(addr, addrsize);
    if (!IsWindows()) {
      rc = sys_connect(fd, addr, addrsize);
    } else if (__isfdkind(fd, kFdSocket)) {
      rc = sys_connect_nt(&g_fds.p[fd], addr, addrsize);
    } else {
      rc = ebadf();
    }
  } else {
    rc = efault();
  }
  STRACE("connect(%d, %s) -> %d% m", fd, __describe_sockaddr(addr, addrsize),
         rc);
  return rc;
}
