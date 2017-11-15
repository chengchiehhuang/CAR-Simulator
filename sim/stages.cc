#include "cpu.h"
#include "stages.h"
#include <cassert>
#include <cstdio>

// the three operands are encoded inside a single uint16_t. This method cracks
// them open
static void inline decode_ops(uint16_t input, byte *dest, byte *src1,
                              byte *src2) {
  *dest = input & 0x001F;
  *src1 = (input & 0x03E0) >> 5;
  *src2 = (input & 0x7C00) >> 10;
}

static bool inline isBranch(IDl latch) { return latch.control()->branch; }

/**
   The execute implementations. These actually perform the action of the stage.
 **/

// Instructions are fetched from memory in this stage, and passed into the CPU's
// ID latch
void InstructionFetchStage::cycle() {
  if (OBF) {
    return;
  }
  IBF = false;

  right.opcode = core->mem->get<byte>(core->PC);
  uint16_t operands = core->mem->get<uint16_t>(core->PC + 1);
  decode_ops(operands, &right.Rdest, &right.Rsrc1, &right.Rsrc2);
  right.immediate = core->mem->get<uint32_t>(core->PC + 3);
  right.PC = core->PC;

  if (isBranch(right)) {
    right.predict_taken = core->bp->lookup(right.PC, &right.bpHistory);
  } else {
    right.predict_taken = false;
  }

  if (right.predict_taken) {
    right.recoveryPC = (core->PC + 8);
    core->PC = right.immediate;
  } else {
    right.recoveryPC = right.immediate;
    core->PC += 8;
  }

  OBF = true;
}

void InstructionDecodeStage::cycle() {
  if (!IBF || OBF) { // no input or output is not read by the next stage
    return;
  }
  IBF = false;

  core->registers[0].value =
      0; // wire register 0 to zero for all register reads
  right.Rsrc1Val = *(int32_t *)&core->registers[left.Rsrc1].value;
  right.Rsrc2Val = *(int32_t *)&core->registers[left.Rsrc2].value;
  right.immediate = left.immediate;
  right.Rsrc1 = left.Rsrc1;
  right.Rsrc2 = left.Rsrc2;
  right.Rdest = left.Rdest;
  right.opcode = left.opcode;
  right.predict_taken = left.predict_taken;
  right.PC = left.PC;
  right.recoveryPC = left.recoveryPC;
  right.bpHistory = left.bpHistory;

  if (right.Rsrc1 && core->registers[right.Rsrc1].lockRefCount) {
    // printf("Rsrc1 (%d) is locked, stall ?\n", right.Rsrc1);
    right.setRsrc1Ready(false);

  } else {
    right.setRsrc1Ready(true);
  }

  if (right.Rsrc2 && core->registers[right.Rsrc2].lockRefCount) {
    // printf("Rsrc2 (%d) is locked, stall ?\n", right.Rsrc2);
    right.setRsrc2Ready(false);
  } else {
    right.setRsrc2Ready(true);
  }

  // lock destination register
  if (right.Rdest && right.control()->register_write) {
    core->registers[right.Rdest].lockRefCount++;
  }

  OBF = true;
}

void ExecuteStage::cycle() {
  if (!IBF || OBF) { // no input or output is not read by the next stage
    return;
  }

  if (busyCycles == 0) {
    // copy forward from previous latch
    right.opcode = left.opcode;
    right.Rsrc1 = left.Rsrc1;
    right.Rsrc2 = left.Rsrc2;
    right.Rsrc1Val = left.Rsrc1Val;
    right.Rsrc2Val = left.Rsrc2Val;
    right.Rdest = left.Rdest;
    right.PC = left.PC;
    // load execution cycles
    busyCycles = (left.control()->exe_cycles - 1);
  } else {
    busyCycles--;
  }

  // execution stage start
  if (busyCycles) { // return if we need to wait for more cycles
    // printf("Exe Stage Stall: Wait %d cycles\n",busyCycles);
    return;
  }

  // set IBF to 0;
  IBF = false;

  uint32_t param;

  switch (left.control()->alu_source) {
  case 0: // source from register
    param = left.Rsrc2Val;
    break;

  case 1: // immediate add/sub
    param = left.immediate;
    break;

  case 2: // address calculation
    param = left.immediate + *(uint32_t *)&left.Rsrc1Val;
    break;
  }
  // for those operands which require signed arithmatic.
  int32_t svalue = left.Rsrc1Val;
  int32_t sparam = *(int32_t *)&param;
  int32_t result = sparam;

  if (left.control()->branch) {
    bool taken = (left.opcode == 2 && left.Rsrc1Val == 0) ||
                 (left.opcode == 3 && left.Rsrc1Val >= left.Rsrc2Val) ||
                 (left.opcode == 4 && left.Rsrc1Val != left.Rsrc2Val);

    // update BP history
    core->bp->update(left.PC, taken, &left.bpHistory,
                     left.predict_taken != taken);

    // if mispredict, nop out IF and ID. (mispredict == prediction and taken
    // differ)
    if (core->verbose)
      printf(taken ? "taken %d  %d\n" : "nottaken %d  %d\n", left.Rsrc1Val,
             left.Rsrc2Val);
    if (left.predict_taken != taken) {
      if (core->verbose)
        printf("\033[32m*** MISPREDICT!\033[0m\n");
      core->ifs.make_nop();
      core->ids.make_nop();
      core->BPMisses++;
      core->PC = left.recoveryPC;
    } else {
      core->BPHits++;
    }
  }

  switch (left.control()->alu_operation) {
  case 0:
    // do nothing, this operation does not require an alu op (copy forward)
    break;

  case 1:
    // do a signed add of reg1 to param
    result = svalue + sparam;
    break;

  case 2:
    // do a signed subtract of param from reg1
    result = svalue - sparam;
    break;
  case 3:
    // do a signed multiply of param from reg1
    result = svalue * sparam;

    break;
  }
  right.aluresult = *(uint32_t *)&result;
  OBF = true;
}

void MemoryStage::cycle() {

  if (!IBF || OBF) { // no input or output is not read by the next stage
    return;
  }
  if (this->isIdle()) {
    // copy forward from previous latch
    right.opcode = left.opcode;
    right.Rsrc1Val = left.Rsrc1Val;
    right.Rsrc2Val = left.Rsrc2Val;
    right.Rdest = left.Rdest;
    // load execution cycles
    right.mem_data = 0;
    right.aluresult = left.aluresult;
    if (left.control()->mem_read || left.control()->mem_write) {
      this->setBusy();

      CachePacket *pkt = new CachePacket(
          core->cycles, left.PC, left.aluresult,
          left.control()->mem_write > 0 ? CacheWriteReq : CacheReadReq,
          left.control()->mem_read > 0 ? left.control()->mem_read
                                       : left.control()->mem_write,
          left.control()->mem_write > 0 ? left.Rsrc2Val : 0);
      core->dcache_ctl.access(pkt);
    } else {
      IBF = false;
      OBF = true;
    }
  }
}

void MemoryStage::response(CachePacket *pkt) {
  if (left.control()->mem_read > 0) { // is a read?
    right.mem_data = pkt->data;
  }

  this->setIdle();
  IBF = false;
  OBF = true;

  delete pkt; // delete the packet
}

void WriteBackStage::cycle() {
  if (!IBF) { // no input
    return;
  }
  IBF = false;

  const instruction *control = left.control();

  if (control->special_case != NULL) {
    control->special_case(core);
  }
  if (control->register_write) {
    if (control->mem_to_register) {
      // write the mem_data into register Rdest
      core->registers[left.Rdest].value = left.mem_data;
    } else {
      // write the alu result into register Rdest
      core->registers[left.Rdest].value = left.aluresult;
    }
    core->registers[0].value = 0; // wire back to zero
    assert(core->registers[left.Rdest].lockRefCount > 0);
    core->registers[left.Rdest].lockRefCount--;
  }
}

void InstructionDecodeStage::shift() {
  if (IBF || OBF || !right.isReady()) // if OBF is set or data is not ready in
                                      // the right latch, we cannot fetch from
                                      // the previous stage
    return;

  if (core->ifs.OBF && core->ifs.right.isReady()) {
    left = core->ifs.right;
    core->ifs.OBF = false;
  } else {
    // create a nop if the previous stage is not ready
    left.reset();
  }
  IBF = true;
}

void ExecuteStage::shift() {
  // if (busyCycles>0)
  //	return;

  if (IBF || OBF || !right.isReady()) // if OBF is set or data is not ready in
                                      // the right latch, we cannot fetch from
                                      // the previous stage
    return;

  if (core->ids.OBF && core->ids.right.isReady()) {
    left = core->ids.right;
    core->ids.OBF = false;
  } else {
    // create a nop if the previous stage is not ready
    left.reset();
  }
  IBF = true;
}

void MemoryStage::shift() {
  if (IBF || OBF || !right.isReady()) // if OBF is set or data is not ready in
                                      // the right latch, we cannot fetch from
                                      // the previous stage
    return;

  if (core->exs.OBF && core->exs.right.isReady()) {
    left = core->exs.right;
    core->exs.OBF = false;
  } else {
    // create a nop if the previous stage is not ready
    left.reset();
  }
  IBF = true;
}

void WriteBackStage::shift() {
  if (IBF)
    return;

  if (core->mys.OBF && core->mys.right.isReady()) {
    left = core->mys.right;
    core->mys.OBF = false;
    IBF = true;
  } else {
    // create a nop if the previous stage is not ready
    left.reset();
  }
  IBF = true;
}

void ExecuteStage::doForwarding() {
  if (busyCycles > 0)
    return;

  // If the previous instruction is attempting a READ of the same register the
  // instruction
  // in this stage is supposed to WRITE, then here, update the previous stage's
  // right latch
  // with the value coming out of the ALU. (also, not reading zero reg)
  if (!core->exs.right.control()->mem_read && core->exs.right.Rdest != 0 &&
      core->exs.right.control()->register_write) {
    if (core->exs.right.Rdest == core->ids.right.Rsrc1) {
      core->ids.right.Rsrc1Val = core->exs.right.aluresult;
      core->ids.right.setRsrc1Ready(true);
      if (core->verbose)
        printf(
            "\033[34m*** FORWARDex1\033[0m:  %08x going to ID/EX's Rsrc1Val \n",
            core->exs.right.aluresult);
    }
    if (core->exs.right.Rdest == core->ids.right.Rsrc2) {
      core->ids.right.Rsrc2Val = core->exs.right.aluresult;
      core->ids.right.setRsrc2Ready(true);
      if (core->verbose)
        printf(
            "\033[34m*** FORWARDex2\033[0m:  %08x going to ID/EX's Rsrc2Val \n",
            core->exs.right.aluresult);
    }
  }
}

void MemoryStage::doForwarding() {
  if (!this->isIdle())
    return;
  // If the next to previous instruction (IDS) is attempting a READ of the same
  // register the instruction
  // in this stage is supposed to WRITE, then here, update the next-to-previous
  // stage's right latch
  // with the value coming out of the this stage. (also, the read is not on
  // zero)
  if (core->mys.right.Rdest != 0 && core->mys.right.control()->register_write) {
    if (core->exs.right.Rdest != core->ids.right.Rsrc1 &&
        core->mys.right.Rdest == core->ids.right.Rsrc1) {
      core->ids.right.Rsrc1Val = core->mys.right.control()->mem_read
                                     ? core->mys.right.mem_data
                                     : core->mys.right.aluresult;
      core->ids.right.setRsrc1Ready(true);

      if (core->verbose)
        printf(
            "\033[34m*** FORWARDmem1\033[0m: %08x going to ID/EX's Rsrc1Val \n",
            core->ids.right.Rsrc1Val);
    }
    if (core->exs.right.Rdest != core->ids.right.Rsrc2 &&
        core->mys.right.Rdest == core->ids.right.Rsrc2) {
      core->ids.right.Rsrc2Val = core->mys.right.control()->mem_read
                                     ? core->mys.right.mem_data
                                     : core->mys.right.aluresult;
      core->ids.right.setRsrc2Ready(true);

      if (core->verbose)
        printf(
            "\033[34m*** FORWARDmem2\033[0m: %08x going to ID/EX's Rsrc2Val \n",
            core->ids.right.Rsrc2Val);
    }
  }
}

void InstructionDecodeStage::make_nop() {
  if (right.Rdest && right.control()->register_write) {
    core->registers[right.Rdest].lockRefCount--;
    // printf("Lock dest register:R%d
    // ref:%d",right.Rdest,core->registers[right.Rdest].lockRefCount)
  }

  left.reset();
  right.reset();
  right.setRsrc1Ready(true);
  right.setRsrc2Ready(true);
}
