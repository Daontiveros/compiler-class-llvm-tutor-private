#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Check if there is an intervening store or load between Prev and Curr in MemorySSA
bool hasInterveningUse(Instruction *Prev, Instruction *Curr,
                       MemorySSA &MSSA, AAResults &AA) {
    MemoryAccess *PrevMA = MSSA.getMemoryAccess(Prev);
    MemoryAccess *CurrMA = MSSA.getMemoryAccess(Curr);

    if (!PrevMA || !CurrMA)
        return true; // conservative: assume there is an intervening use

    MemorySSAWalker *Walker = MSSA.getWalker();
    if (!Walker)
        return true; // safety check

    MemoryAccess *Clobber = CurrMA;
    SmallPtrSet<MemoryAccess *, 8> Visited; // detect cycles

    while (Clobber && Clobber != PrevMA) {
        if (!Visited.insert(Clobber).second)
            break; // cycle detected
        Clobber = Walker->getClobberingMemoryAccess(Clobber);
    }

    return Clobber != PrevMA; // true if there was an intervening use
}




//===----------------------------------------------------------------------===//
// Dead Store Elimination using MemorySSA
//===----------------------------------------------------------------------===//
struct MemorySSADemoDSEPass : PassInfoMixin<MemorySSADemoDSEPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {

        auto &MSSAResult = AM.getResult<MemorySSAAnalysis>(F);
        MemorySSA &MSSA = MSSAResult.getMSSA(); // reference, not pointer

        auto &AA = AM.getResult<AAManager>(F);
        auto &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);

        errs() << "Analyzing function: " << F.getName() << "\n";

        SmallVector<StoreInst*, 8> DeadStores;

        for (BasicBlock &BB : F) {
            for (auto InstIt = BB.rbegin(); InstIt != BB.rend(); ++InstIt) {
                Instruction &I = *InstIt;

                // Only consider instructions tracked by MemorySSA
                if (MemoryAccess *MA = MSSA.getMemoryAccess(&I)) {
                    if (auto *MD = dyn_cast<MemoryDef>(MA)) { // MemoryDef = store
                        StoreInst *SI = dyn_cast<StoreInst>(MD->getMemoryInst());
                        if (!SI) continue;

                        // Get previous clobbering MemoryAccess
                        MemoryAccess *Prev = MSSA.getWalker()->getClobberingMemoryAccess(MD);

                        // Only handle previous MemoryDefs (stores)
                        if (auto *PrevDef = dyn_cast<MemoryDef>(Prev)) {
                            Instruction *PrevInst = PrevDef->getMemoryInst();
                            if (!PrevInst) continue;

                            MemoryLocation Loc1 = MemoryLocation::get(SI);
                            MemoryLocation Loc2 = MemoryLocation::get(PrevInst);

                            AliasResult AR = AA.alias(Loc1, Loc2);
                            bool Intervening = hasInterveningUse(PrevInst, SI, MSSA, AA);
                            bool Dominates = PDT.dominates(&I, PrevInst);

                            if (AR == AliasResult::MustAlias &&
                                !Intervening &&
                                PDT.dominates(SI, PrevInst)) {

                                errs() << "Dead store detected at: " << *PrevInst << "\n";
                                PrevInst->eraseFromParent();
                                // Optional: erase the previous store
                                // PrevInst->eraseFromParent();
                            }
                        }
                    }
                }
            }
        }

        // Erase dead stores after iteration
        /*for (StoreInst *DS : DeadStores) {
            if (DS && !DS->isVolatile())
                DS->eraseFromParent();
        }*/

       

        return PreservedAnalyses::all();
    }
};

//===----------------------------------------------------------------------===//
// Pass registration
//===----------------------------------------------------------------------===//
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "MemorySSADemoDSEPass", "v1.0",
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "dead-store") {
                        FPM.addPass(MemorySSADemoDSEPass());
                        return true;
                    }
                    return false;
                });
        }};
}