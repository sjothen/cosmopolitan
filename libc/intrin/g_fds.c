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
#include "libc/bits/pushpop.h"
#include "libc/calls/internal.h"
#include "libc/calls/strace.internal.h"
#include "libc/nt/runtime.h"
#include "libc/sysv/consts/o.h"

STATIC_YOINK("_init_g_fds");

struct Fds g_fds;
_Alignas(64) int __fds_lock;

textstartup void InitializeFileDescriptors(void) {
  struct Fds *fds;
  fds = VEIL("r", &g_fds);
  pushmov(&fds->n, ARRAYLEN(fds->__init_p));
  fds->p = fds->__init_p;
  if (IsMetal()) {
    pushmov(&fds->f, 3ull);
    fds->__init_p[0].kind = pushpop(kFdSerial);
    fds->__init_p[1].kind = pushpop(kFdSerial);
    fds->__init_p[2].kind = pushpop(kFdSerial);
    fds->__init_p[0].handle = VEIL("r", 0x3F8ull);
    fds->__init_p[1].handle = VEIL("r", 0x3F8ull);
    fds->__init_p[2].handle = VEIL("r", 0x3F8ull);
  } else if (IsWindows()) {
    pushmov(&fds->f, 3ull);
    fds->__init_p[0].kind = pushpop(kFdFile);
    fds->__init_p[1].kind = pushpop(kFdFile);
    fds->__init_p[2].kind = pushpop(kFdFile);
    fds->__init_p[0].handle = GetStdHandle(pushpop(kNtStdInputHandle));
    fds->__init_p[1].handle = GetStdHandle(pushpop(kNtStdOutputHandle));
    fds->__init_p[2].handle = GetStdHandle(pushpop(kNtStdErrorHandle));
  }
  fds->__init_p[0].flags = O_RDONLY;
  fds->__init_p[1].flags = O_WRONLY | O_APPEND;
  fds->__init_p[2].flags = O_WRONLY | O_APPEND;
}
