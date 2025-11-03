#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/CFG.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

struct SimpleLICM : public PassInfoMixin<SimpleLICM> {
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR,
                        LPMUpdater &) {

    errs() << "SimpleLICM pass is running!\n";
    DominatorTree &DT = AR.DT;

    BasicBlock *Preheader = L.getLoopPreheader();
    if (!Preheader) {
      errs() << "No preheader, skipping loop\n";
      return PreservedAnalyses::all();
    }

    SmallPtrSet<Instruction *, 8> InvariantSet;
    bool Changed = true;

    while (Changed) {
      Changed = false;
      for (BasicBlock *BB : L.blocks()) {
        for (Instruction &I : *BB) {
          if (isa<PHINode>(&I)) continue;
          if (!I.isBinaryOp() && !isa<CmpInst>(&I) && !isa<CastInst>(&I))
            continue;

          bool AllOperandsInvariant = true;
          for (Use &U : I.operands()) {
            Value *OpV = U.get();
            if (isa<Constant>(OpV)) continue; // ✅ Fix: constants are invariant
            if (Instruction *OpI = dyn_cast<Instruction>(OpV)) {
              if (!L.contains(OpI) || !InvariantSet.count(OpI)) {
                AllOperandsInvariant = false;
                break;
              }
            }
          }

          if (AllOperandsInvariant && !InvariantSet.count(&I)) {
            InvariantSet.insert(&I);
            Changed = true;
          }
        }
      }
    }

    for (Instruction *I : InvariantSet) {
      if (isSafeToSpeculativelyExecute(I)) {
        errs() << "Hoisting: " << *I << "\n";
        I->moveBefore(Preheader->getTerminator());
      }
    }

    return PreservedAnalyses::none();
  }
};

llvm::PassPluginLibraryInfo getSimpleLICMPluginInfo() {
  errs() << "SimpleLICM plugin: getSimpleLICMPluginInfo() called\n";
  return {LLVM_PLUGIN_API_VERSION, "simple-licm", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, LoopPassManager &LPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "simple-licm") {
                    LPM.addPass(SimpleLICM());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  errs() << "SimpleLICM plugin: llvmGetPassPluginInfo() called\n";
  return getSimpleLICMPluginInfo();
}

