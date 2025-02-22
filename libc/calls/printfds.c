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
#include "libc/calls/internal.h"
#include "libc/calls/state.internal.h"
#include "libc/intrin/kprintf.h"
#include "libc/intrin/spinlock.h"

static const char *__fdkind2str(int x) {
  switch (x) {
    case kFdEmpty:
      return "kFdEmpty";
    case kFdFile:
      return "kFdFile";
    case kFdSocket:
      return "kFdSocket";
    case kFdProcess:
      return "kFdProcess";
    case kFdConsole:
      return "kFdConsole";
    case kFdSerial:
      return "kFdSerial";
    case kFdZip:
      return "kFdZip";
    case kFdEpoll:
      return "kFdEpoll";
    default:
      return "kFdWut";
  }
}

void __printfds(void) {
  int i;
  _spinlock(&__fds_lock);
  for (i = 0; i < g_fds.n; ++i) {
    if (!g_fds.p[i].kind) continue;
    kprintf("%3d %s", i, __fdkind2str(g_fds.p[i].kind));
    if (g_fds.p[i].zombie) kprintf(" zombie");
    if (g_fds.p[i].flags) kprintf(" flags=%#x", g_fds.p[i].flags);
    if (g_fds.p[i].mode) kprintf(" mode=%#o", g_fds.p[i].mode);
    if (g_fds.p[i].handle) kprintf(" handle=%ld", g_fds.p[i].handle);
    if (g_fds.p[i].extra) kprintf(" extra=%ld", g_fds.p[i].extra);
    kprintf("\n");
  }
  _spunlock(&__fds_lock);
}
