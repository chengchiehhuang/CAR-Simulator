/*
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
 * Copyright (c) 2016 Cheng-Chieh Huang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Kevin Lim
 */
#include <cassert>
#include <math.h>
#include "2bit_local.hh"

LocalBP::LocalBP(const unsigned _localPredictorSize,
                 const unsigned _localCtrBits, const unsigned _instShiftAmt)
    : localPredictorSize(_localPredictorSize), localCtrBits(_localCtrBits),
      instShiftAmt(_instShiftAmt) {
  /*
  if (!isPowerOf2(localPredictorSize)) {
      //fatal("Invalid local predictor size!\n");
      assert(1);
  }
  */

  localPredictorSets = localPredictorSize / localCtrBits;
  /*
      if (!isPowerOf2(localPredictorSets)) {
          //fatal("Invalid number of local predictor sets! Check
     localCtrBits.\n");
          assert(1);
      }
  */
  // Setup the index mask.
  indexMask = localPredictorSets - 1;

  // printf("index mask: %#x\n", indexMask);

  // Setup the array of counters for the local predictor.
  localCtrs.resize(localPredictorSets);

  for (unsigned i = 0; i < localPredictorSets; ++i)
    localCtrs[i].setBits(localCtrBits);

  // printf("local predictor size: %i\n", localPredictorSize);

  // printf("local counter bits: %i\n", localCtrBits);

  // printf("instruction shift amount: %i\n", instShiftAmt);
  reset();
}

void LocalBP::reset() {
  for (unsigned i = 0; i < localPredictorSets; ++i) {
    localCtrs[i].reset();
  }
}

bool LocalBP::lookup(Addr branch_addr, uint32_t *bpHistory) {
  bool taken;
  uint8_t counter_val;
  unsigned local_predictor_idx = getLocalIndex(branch_addr);

  // printf("Looking up index %#x\n", local_predictor_idx);

  counter_val = localCtrs[local_predictor_idx].read();

  // printf("prediction is %i.\n", (int)counter_val);

  taken = getPrediction(counter_val);

  return taken;
}

void LocalBP::update(Addr branch_addr, bool taken, uint32_t *bpHistory,
                     bool squash) {
  unsigned local_predictor_idx;

  // Update the local predictor.
  local_predictor_idx = getLocalIndex(branch_addr);

  // printf("Looking up index %#x\n", local_predictor_idx);

  if (taken) {
    // printf("Branch updated as taken.\n");
    localCtrs[local_predictor_idx].increment();
  } else {
    // printf("Branch updated as not taken.\n");
    localCtrs[local_predictor_idx].decrement();
  }
}

inline bool LocalBP::getPrediction(uint8_t &count) {
  // Get the MSB of the count
  return (count >> (localCtrBits - 1));
}

inline unsigned LocalBP::getLocalIndex(Addr &branch_addr) {
  return (branch_addr >> instShiftAmt) & indexMask;
}
