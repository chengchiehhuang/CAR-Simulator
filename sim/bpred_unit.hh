/*
 * Copyright (c) 2011, 2014 ARM Limited
 * Copyright (c) 2016 Cheng-Chieh Huang
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
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
 *          Timothy M. Jones
 */

#ifndef __BP_PRED_UNIT_HH__
#define __BP_PRED_UNIT_HH__
#include "types.h"
#include <vector>
#include <stdio.h>
#include <unistd.h>

#include "sat_counter.hh"
typedef uint32_t Addr;

/**
 * Implements a local predictor that uses the PC to index into a table of
 * counters.  Note that any time a pointer to the bp_history is given, it
 * should be NULL using this predictor because it does not have any branch
 * predictor state that needs to be recorded or updated; the update can be
 * determined solely by the branch being taken or not taken.
 */

class BPredUnit
{
  public:
    /**
     * Default branch predictor constructor.
     */
    BPredUnit()
	{
		
	}
	virtual ~BPredUnit()
	{
		
	}
    /**
     * Looks up the given address in the branch predictor and returns
     * a true/false value as to whether it is taken.
     * @param branch_addr The address of the branch to look up.
     * @param bp_history Pointer to any bp history state.
     * @return Whether or not the branch is taken.
     */
    virtual bool lookup(Addr branch_addr, uint32_t *globalHistory)=0;

    /**
     * Updates the branch predictor with the actual result of a branch.
     * @param branch_addr The address of the branch to update.
     * @param taken Whether or not the branch was taken.
     */
    virtual void update(Addr branch_addr, bool taken, uint32_t *globalHistory, bool squash)=0;

    //virtual void reset();
};

class AlwaysTakenBP : public BPredUnit
{
  public:
    /**
     * Default branch predictor constructor.
     */
    AlwaysTakenBP()
	{
		
	}
	virtual ~AlwaysTakenBP()
	{
		
	}
	
    bool lookup(Addr branch_addr, uint32_t *globalHistory)
	{
		return true;
	}

    void update(Addr branch_addr, bool taken, uint32_t *globalHistory, bool squash)
	{
			//do nothing
	}
};

class AlwaysNotTakenBP : public BPredUnit
{
  public:
    /**
     * Default branch predictor constructor.
     */
    AlwaysNotTakenBP()
	{
		
	}
	virtual ~AlwaysNotTakenBP()
	{
		
	}
	
    bool lookup(Addr branch_addr, uint32_t *globalHistory)
	{
		return false;
	}

    void update(Addr branch_addr, bool taken, uint32_t *globalHistory, bool squash)
	{
			//do nothing
	}
};

#endif // __BP_PRED_UNIT_HH__
