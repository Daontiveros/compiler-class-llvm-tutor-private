#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class InductionVariableElimination : public PassInfoMixin<InductionVariableElimination> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

private:
    void eliminateInLoop(Loop *L, ScalarEvolution &SE, DominatorTree &DT);
};

PreservedAnalyses InductionVariableElimination::run(Function &F,
                                                    FunctionAnalysisManager &AM) {
    auto &LI = AM.getResult<LoopAnalysis>(F);
    auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    auto &DT = AM.getResult<DominatorTreeAnalysis>(F);

    errs() << "=== Running Induction Variable Elimination on: " << F.getName() << " ===\n";

    for (Loop *L : LI) {
        eliminateInLoop(L, SE, DT);
    }

    // Preserve analyses that are still valid
    PreservedAnalyses PA;
    PA.preserve<LoopAnalysis>();
    PA.preserve<DominatorTreeAnalysis>();
    PA.preserve<ScalarEvolutionAnalysis>();
    return PA;
}

void InductionVariableElimination::eliminateInLoop(Loop *L,
                                                   ScalarEvolution &SE,
                                                   DominatorTree &DT) {
    // Get the canonical IV
    PHINode *CanonicalIV = L->getCanonicalInductionVariable();
    if (!CanonicalIV) {
        errs() << "No canonical IV in loop at depth " << L->getLoopDepth() << "\n";
        return;
    }

    errs() << "Loop depth " << L->getLoopDepth()
           << " canonical IV: " << CanonicalIV->getName() << "\n";

    BasicBlock *Header = L->getHeader();
    Instruction *InsertPoint = &*Header->getFirstInsertionPt(); // safe insertion point
    SCEVExpander Expander(SE, Header->getModule()->getDataLayout(), "ive");

    SmallVector<PHINode *, 8> DerivedIVs;

    // Collect affine derived IVs
    for (PHINode &PN : Header->phis()) {
        if (&PN == CanonicalIV)
            continue;

        const SCEV *S = SE.getSCEV(&PN);
        if (auto *AR = dyn_cast<SCEVAddRecExpr>(S)) {
            if (AR->getLoop() == L && AR->isAffine() && PN.getType()->isIntegerTy()) {
                DerivedIVs.push_back(&PN);
                errs() << "  Derived IV to eliminate: " << PN.getName()
                       << " -> " << *AR << "\n";
            }
        }
    }

    for (PHINode *PN : DerivedIVs) {
        const SCEV *S = SE.getSCEV(PN);
        auto *AR = dyn_cast<SCEVAddRecExpr>(S);
        if (!AR) continue;

        auto *ARStepC = dyn_cast<SCEVConstant>(AR->getStepRecurrence(SE));
        auto *CanonicalAR = dyn_cast<SCEVAddRecExpr>(SE.getSCEV(CanonicalIV));
        auto *CanonicalStepC = dyn_cast<SCEVConstant>(CanonicalAR->getStepRecurrence(SE));
        if (!ARStepC || !CanonicalStepC) continue;

        int64_t Multiplier = ARStepC->getAPInt().getSExtValue() /
                            CanonicalStepC->getAPInt().getSExtValue();

        const SCEV *ReplacementSCEV =
            SE.getMulExpr(SE.getConstant(PN->getType(), Multiplier),
                        SE.getSCEV(CanonicalIV));

        Value *Replacement = Expander.expandCodeFor(ReplacementSCEV, PN->getType(), InsertPoint);

        std::string PNName = PN->getName().str(); // <-- copy before erasing
        PN->replaceAllUsesWith(Replacement);
        PN->eraseFromParent();

        errs() << "  Removed derived IV: " << PNName << "\n";
    }


    // Recursively process subloops
    for (Loop *SubL : L->getSubLoops())
        eliminateInLoop(SubL, SE, DT);
}

} // namespace

//===----------------------------------------------------------------------===//
// Register the pass with the new PassManager
//===----------------------------------------------------------------------===//

llvm::PassPluginLibraryInfo getInductionVariableEliminationPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "InductionVariableElimination",
            LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "induction-var-elim") {
                            FPM.addPass(InductionVariableElimination());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getInductionVariableEliminationPluginInfo();
}

























