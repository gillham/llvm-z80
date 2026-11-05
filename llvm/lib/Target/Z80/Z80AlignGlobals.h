//===-- Z80AlignGlobals.h - Pin z88dk global alignment ----------*- C++ -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Z80_Z80ALIGNGLOBALS_H
#define LLVM_LIB_TARGET_Z80_Z80ALIGNGLOBALS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;
class ModulePass;
class PassRegistry;

/// Give every defined global variable without an explicit alignment its ABI
/// alignment as an explicit one. Returns true if anything changed.
bool pinZ80GlobalAlignment(Module &M);

/// New pass manager wrapper. Required, so it runs at -O0 and for optnone.
struct Z80AlignGlobalsPass : RequiredPassInfoMixin<Z80AlignGlobalsPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};

ModulePass *createZ80AlignGlobalsPass();
void initializeZ80AlignGlobalsPass(PassRegistry &);

} // namespace llvm

#endif // LLVM_LIB_TARGET_Z80_Z80ALIGNGLOBALS_H
