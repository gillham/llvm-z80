//===-- Z80AlignGlobals.cpp - Pin z88dk global alignment ------------------===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// The z88dk object format cannot align a global: z80asm's ALIGN works only
// within one object file's section, so after linking a global may sit at any
// address. LLVM, though, assumes a defined global with no explicit alignment
// has DataLayout::getPreferredAlign, which raises any initialized global over
// 128 bits to 16 bytes. Known bits then treats the low address bits as zero
// and rewrites `add ptr, 3` as `or ptr, 3`, a miscompile at run time.
//
// This pass gives each such global its ABI alignment (1 on Z80) explicitly,
// before any optimization looks at it. A pass that raises the alignment later
// makes it explicit and above 1, and Z80AsmPrinter warns about it.
//
// The target machine adds the pass only for the z88dk environment.
//
//===----------------------------------------------------------------------===//

#include "Z80AlignGlobals.h"

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#define DEBUG_TYPE "z80-align-globals"

using namespace llvm;

bool llvm::pinZ80GlobalAlignment(Module &M) {
  const DataLayout &DL = M.getDataLayout();
  bool Changed = false;
  for (GlobalVariable &GV : M.globals()) {
    // Declarations get the ABI alignment from getPointerAlignment already;
    // an explicit alignment is never lowered.
    if (GV.isDeclaration() || GV.getAlign() || !GV.getValueType()->isSized())
      continue;
    GV.setAlignment(DL.getABITypeAlign(GV.getValueType()));
    Changed = true;
  }
  return Changed;
}

PreservedAnalyses Z80AlignGlobalsPass::run(Module &M,
                                           ModuleAnalysisManager &) {
  // Only an attribute of the globals changes; no analysis caches it.
  pinZ80GlobalAlignment(M);
  return PreservedAnalyses::all();
}

namespace {

class Z80AlignGlobals : public ModulePass {
public:
  static char ID;
  Z80AlignGlobals() : ModulePass(ID) {}

  StringRef getPassName() const override {
    return "Z80 z88dk global alignment";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override { return pinZ80GlobalAlignment(M); }
};

} // namespace

char Z80AlignGlobals::ID = 0;

INITIALIZE_PASS(Z80AlignGlobals, DEBUG_TYPE, "Z80 z88dk global alignment",
                false, false)

ModulePass *llvm::createZ80AlignGlobalsPass() { return new Z80AlignGlobals(); }
