#ifndef _LATCH_H_
#define _LATCH_H_
#include "instruction.h"
#include "cache.h"

enum StageState {
  STAGE_BUSY = 0,
  STAGE_IDLE
};

class latch {
public:
  byte opcode;
  byte Rdest, Rsrc1, Rsrc2;
  latch() {
    opcode = 0; // initial default op to nop
  }

  inline const instruction *control() { return &instructions[opcode]; }

  virtual bool isReady() { return true; }

  virtual void reset() {
    Rdest = 0;
    Rsrc1 = 0;
    Rsrc2 = 0;
    opcode = 0;
  }
};

class IDl : public latch {
public:
  uint32_t immediate;
  bool predict_taken;
  uint32_t PC;
  uint32_t recoveryPC;
  uint32_t bpHistory;
};

class DEl : public latch {
public:
  uint32_t immediate;
  int32_t Rsrc1Val, Rsrc2Val;
  bool predict_taken;
  bool ready; // indicate whether the data in the latch is ready or not
  uint32_t PC;
  uint32_t recoveryPC;
  uint32_t bpHistory;

  void setRsrc1Ready(bool _ready) {
    Rsrc1Ready = _ready;
    ready = (Rsrc1Ready && Rsrc2Ready);
  }

  void setRsrc2Ready(bool _ready) {
    Rsrc2Ready = _ready;
    ready = (Rsrc1Ready && Rsrc2Ready);
  }

  virtual bool isReady() { return ready; }

  virtual void reset() {
    Rdest = 0;
    Rsrc1 = 0;
    Rsrc2 = 0;
    Rsrc1Val = 0;
    Rsrc2Val = 0;
    opcode = 0;
    ready = false;
  }

private:
  bool Rsrc1Ready, Rsrc2Ready;
};

class EMl : public latch {
public:
  uint32_t PC;
  uint32_t aluresult;
  int32_t Rsrc1Val, Rsrc2Val;
  virtual void reset() {
    aluresult = 0;
    Rsrc1Val = 0;
    Rsrc2Val = 0;
    opcode = 0;
    PC = 0;
  }
};

class MWl : public latch {
public:
  uint32_t PC;
  uint32_t aluresult, mem_data;
  int32_t Rsrc2Val, Rsrc1Val;
  virtual void reset() {
    PC = 0;
    aluresult = 0;
    mem_data = 0;
    Rsrc1Val = 0;
    Rsrc2Val = 0;
    opcode = 0;
  }
};

class PipelineStage {
public:
  cpu_core *core;
  bool OBF; // output buffer full flag
  bool IBF; // input buffer full flag
  PipelineStage() {
    OBF = false;
    IBF = false;
  }
  virtual void cycle() = 0;
  virtual void make_nop() = 0;
  virtual void shift() {}
  virtual void doForwarding() {}
};

class InstructionFetchStage : public PipelineStage {
public:
  cpu_core *core;
  IDl right;
  InstructionFetchStage(cpu_core *c) {
    core = c;
    make_nop();
  }

  void cycle();
  void make_nop() { right.reset(); }
};

class InstructionDecodeStage : public PipelineStage {
public:
  cpu_core *core;
  IDl left;
  DEl right;
  InstructionDecodeStage(cpu_core *c) {
    core = c;
    make_nop();
  }

  void cycle();
  void shift();
  void make_nop();
};

class ExecuteStage : public PipelineStage {
public:
  cpu_core *core;
  DEl left;
  EMl right;
  int busyCycles;

  ExecuteStage(cpu_core *c) {
    core = c;
    make_nop();
    busyCycles = 0;
  }

  void cycle();
  void shift();
  void doForwarding();
  void make_nop() {
    busyCycles = 0;
    right.reset();
    left.reset();
  }
};

class MemoryStage : public PipelineStage {
public:
  cpu_core *core;
  EMl left;
  MWl right;
  StageState state;

  MemoryStage(cpu_core *c) {
    core = c;
    make_nop();
    setIdle();
  }

  bool isIdle() { return state == STAGE_IDLE; }
  void setBusy() { state = STAGE_BUSY; }
  void setIdle() { state = STAGE_IDLE; }
  void cycle();
  void shift();
  void doForwarding();
  void make_nop() {
    right.reset();
    left.reset();
  }
  void response(CachePacket *pkt);
};

class WriteBackStage : public PipelineStage {
public:
  cpu_core *core;
  MWl left;
  WriteBackStage(cpu_core *c) {
    core = c;
    make_nop();
  }

  void cycle();
  void shift();
  void make_nop() { left.reset(); }
};

#endif /* _LATCH_H_ */
