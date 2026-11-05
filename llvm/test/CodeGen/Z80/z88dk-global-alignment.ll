; The z88dk format cannot align globals, so a defined global with no explicit
; alignment gets an explicit align 1 before optimization: known bits must not
; treat the low bits of its address as zero and turn the add into an or.
; Other Z80 outputs keep LLVM's preferred alignment (16 for a global over 128
; bits) and still fold the add into an or.

; RUN: opt -mtriple=z80-unknown-none-z88dk -O2 -S < %s | FileCheck %s --check-prefix=IR
; RUN: opt -mtriple=z80-unknown-none-z88dk -passes='default<O0>' -S < %s | FileCheck %s --check-prefix=IR-PIN
; RUN: opt -mtriple=z80 -z80-asm-format=z88dk -O2 -S < %s | FileCheck %s --check-prefix=IR
; RUN: opt -mtriple=z80 -O2 -S < %s | FileCheck %s --check-prefix=IR-ELF
; RUN: opt -mtriple=z80-unknown-none-z88dk -O2 < %s | llc -O2 | FileCheck %s --check-prefix=ASM
; RUN: llc -mtriple=z80-unknown-none-z88dk -O2 < %s | FileCheck %s --check-prefix=ASM
; RUN: llc -mtriple=z80-unknown-none-z88dk -O0 < %s | FileCheck %s --check-prefix=ASM
; RUN: llc -mtriple=z80-unknown-none-z88dk -O2 -print-after=z80-align-globals -o /dev/null < %s 2>&1 | FileCheck %s --check-prefix=PASS
; RUN: llc -mtriple=z80 -O2 -print-after=z80-align-globals < %s 2>&1 | FileCheck %s --check-prefix=NOPASS
; RUN: opt -mtriple=z80 -O2 < %s | llc -O2 | FileCheck %s --check-prefix=ASM-ELF

; IR:     @buf = internal global [41 x i8] zeroinitializer, align 1
; IR:     @aligned = {{.*}}global [41 x i8] zeroinitializer, align 2
; IR:     @ext = external global [41 x i8]{{$}}
; IR-LABEL: define void @put(
; IR:     add i16 %{{.*}}, ptrtoint (ptr @buf to i16)
; IR-NOT: {{ or }}

; IR-PIN: @buf = internal global [41 x i8] zeroinitializer, align 1
; IR-PIN: @aligned = global [41 x i8] zeroinitializer, align 2
; IR-PIN: @ext = external global [41 x i8]{{$}}

; IR-ELF: @buf = internal global [41 x i8] zeroinitializer{{$}}
; IR-ELF-LABEL: define void @put(
; IR-ELF: or disjoint i16 %{{.*}}, ptrtoint (ptr @buf to i16)

; ASM-LABEL: _put:
; ASM-NOT: {{[[:space:]]or[[:space:]]}}
; ASM:     add hl,de
; ASM:     ret

; PASS: IR Dump After Z80 z88dk global alignment
; NOPASS-NOT: Z80 z88dk global alignment

; ASM-ELF-LABEL: put:
; ASM-ELF: or e
; ASM-ELF: ret

@buf = internal global [41 x i8] zeroinitializer
@aligned = global [41 x i8] zeroinitializer, align 2
@ext = external global [41 x i8]

define void @put(i16 %n, i8 %c) {
  %o = and i16 %n, 3
  %a = ptrtoint ptr @buf to i16
  %s = add i16 %a, %o
  %p = inttoptr i16 %s to ptr
  store i8 %c, ptr %p
  ret void
}

define ptr @use() {
  %x = load i8, ptr @ext
  store i8 %x, ptr @aligned
  ret ptr @ext
}
