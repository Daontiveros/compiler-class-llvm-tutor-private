#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

using namespace llvm;

struct MemorySSADemoPass : PassInfoMixin<MemorySSADemoPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    auto &MSSAResult = AM.getResult<MemorySSAAnalysis>(F);
    auto &MSSA = MSSAResult.getMSSA();

    errs() << "Analyzing function: " << F.getName() << "\n";

    // Create DOT file
    SmallString<128> Filename;
    Filename = F.getName();
    Filename += "_MemorySSA.dot";

    std::error_code EC;
    raw_fd_ostream DotFile(Filename, EC, sys::fs::OF_Text);
    if (EC) {
      errs() << "Error opening file: " << EC.message() << "\n";
      return PreservedAnalyses::all();
    }

    DotFile << "digraph \"" << F.getName() << "_MemorySSA\" {\n";
    DotFile << "  node [shape=box];\n";

    auto printNodeName = [](MemoryAccess *MA) {
      std::string S;
      raw_string_ostream OS(S);
      MA->print(OS);
      for (char &c : S)
        if (c == '%' || c == ' ' || c == '*' || c == '\n')
          c = '_';
      return S;
    };

    // Iterate over basic blocks
    for (auto &BB : F) {
      for (auto &I : BB) {
        MemoryAccess *MA = MSSA.getMemoryAccess(&I);
        if (!MA) continue;

        std::string NodeName = printNodeName(MA);
        DotFile << "  \"" << NodeName << "\";\n";

        // MemoryUseOrDef: connect to defining access
        if (auto *MUOD = dyn_cast<MemoryUseOrDef>(MA)) {
          if (MemoryAccess *Def = MUOD->getDefiningAccess()) {
            std::string DefName = printNodeName(Def);
            DotFile << "  \"" << DefName << "\" -> \"" << NodeName << "\";\n";
          }
        }
        // MemoryPhi: connect to incoming values
        else if (auto *MPhi = dyn_cast<MemoryPhi>(MA)) {
          for (unsigned i = 0; i < MPhi->getNumIncomingValues(); ++i) {
            MemoryAccess *Inc = MPhi->getIncomingValue(i);
            std::string IncName = printNodeName(Inc);
            DotFile << "  \"" << IncName << "\" -> \"" << NodeName << "\";\n";
          }
        }
      }
    }

    DotFile << "}\n";
    errs() << "MemorySSA graph written to: " << Filename << "\n";

    return PreservedAnalyses::all();
  }
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MemorySSADemoPass", "v1.0",
          [](PassBuilder &PB) {
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                  FAM.registerPass([] { return MemorySSAAnalysis(); });
                });

            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "memssa-demo") {
                    FPM.addPass(MemorySSADemoPass());
                    return true;
                  }
                  return false;
                });
          }};
}



