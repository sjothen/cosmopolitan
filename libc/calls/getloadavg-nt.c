/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2022 Justine Alexandra Roberts Tunney                              │
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
#include "libc/calls/calls.h"
#include "libc/calls/syscall_support-nt.internal.h"
#include "libc/dce.h"
#include "libc/fmt/conv.h"
#include "libc/intrin/spinlock.h"
#include "libc/macros.internal.h"
#include "libc/nt/accounting.h"
#include "libc/runtime/sysconf.h"

#define FT(x) (x.dwLowDateTime | (uint64_t)x.dwHighDateTime << 32)

static int cpus;
static double load;
_Alignas(64) static int lock;
static struct NtFileTime idle1, kern1, user1;

textwindows int sys_getloadavg_nt(double *a, int n) {
  int i, rc;
  uint64_t elapsed, used;
  struct NtFileTime idle, kern, user;
  _spinlock(&lock);
  if (GetSystemTimes(&idle, &kern, &user)) {
    elapsed = (FT(kern) - FT(kern1)) + (FT(user) - FT(user1));
    if (elapsed) {
      used = elapsed - (FT(idle) - FT(idle1));
      load = (double)used / elapsed * cpus;
      load = MIN(MAX(load, 0), cpus * 2);
      idle1 = idle, kern1 = kern, user1 = user;
    }
    for (i = 0; i < n; ++i) {
      a[i] = load;
    }
    rc = n;
  } else {
    rc = __winerr();
  }
  _spunlock(&lock);
  return rc;
}

static textstartup void sys_getloadavg_nt_init(void) {
  double a[3];
  if (IsWindows()) {
    load = 1;
    cpus = GetCpuCount() / 2;
    cpus = MAX(1, cpus);
    GetSystemTimes(&idle1, &kern1, &user1);
  }
}

const void *const sys_getloadavg_nt_ctor[] initarray = {
    sys_getloadavg_nt_init,
};
