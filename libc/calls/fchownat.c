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
#include "libc/bits/weaken.h"
#include "libc/calls/calls.h"
#include "libc/calls/internal.h"
#include "libc/calls/strace.internal.h"
#include "libc/dce.h"
#include "libc/intrin/asan.internal.h"
#include "libc/sysv/errfuns.h"
#include "libc/zipos/zipos.internal.h"

/**
 * Changes owner and/or group of path.
 *
 * @param dirfd is open()'d relative-to directory, or AT_FDCWD, etc.
 * @param uid is user id, or -1 to not change
 * @param gid is group id, or -1 to not change
 * @param flags can have AT_SYMLINK_NOFOLLOW, etc.
 * @return 0 on success, or -1 w/ errno
 * @see chown(), lchown() for shorthand notation
 * @see /etc/passwd for user ids
 * @see /etc/group for group ids
 * @asyncsignalsafe
 */
int fchownat(int dirfd, const char *path, uint32_t uid, uint32_t gid,
             int flags) {
  int rc;
  if (IsAsan() && !__asan_is_valid(path, 1)) {
    rc = efault();
  } else if (weaken(__zipos_notat) && (rc = __zipos_notat(dirfd, path)) == -1) {
    STRACE("zipos fchownat not supported yet");
  } else {
    rc = sys_fchownat(dirfd, path, uid, gid, flags);
  }
  STRACE("fchownat(%d, %#s, %d, %d, %#b) → %d% m", dirfd, path, uid, gid, flags,
         rc);
  return rc;
}
