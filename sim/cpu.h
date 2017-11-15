#ifndef _CPU_H_
#define _CPU_H_
#include "gshare.hh"
#include "2bit_local.hh"
#include "2level.hh"
#include "memory.h"
#include "instruction.h"
#include "stages.h"
#include "cache.h"

struct RegisterStruct {
  int lockRefCount;
  uint32_t value;
  RegisterStruct() {
    lockRefCount = 0;
    value = 0;
  }
};

// Register machine core state.
class cpu_core {
public:
  cpu_core()
      : dcache_ctl(this), ifs(this), ids(this), exs(this), mys(this),
        wbs(this) {
    bp = NULL;
  }

  uint32_t PC;
  uint32_t cycles;
  int bp_type;
  BPredUnit *bp;
  uint32_t BPHits;
  uint32_t BPMisses;
  bool usermode, verbose;
  memory *mem;
  // uint32_t registers[32];
  RegisterStruct registers[32];
  CacheCtrl dcache_ctl;
  InstructionFetchStage ifs;
  InstructionDecodeStage ids;
  ExecuteStage exs;
  MemoryStage mys;
  WriteBackStage wbs;
};

void run_cpu(memory *m, const bool verbose, const int bp_type);

#endif /* _CPU_H_ */
