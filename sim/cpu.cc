#include <stdio.h>
#include "cpu.h"
#include "memory.h"
#include <unistd.h>

// Some minimal state display. If I had the time, I'd do a quick gui app, which is more natural
// understanding what is going on under the covers.
static void print_stages(cpu_core *core)
{
	printf("0x%08x ", core->PC - 8);
	printf("(if '%s', $%-2d {$%-2d $%-2d} %08x) "
	      , core->ifs.right.control()->name
	      , core->ifs.right.Rdest
	      , core->ifs.right.Rsrc1
	      , core->ifs.right.Rsrc2
	      , core->ifs.right.immediate);

	printf("(id '%s' $%-2d {$%-2d $%-2d}={%08x, %08x} %08x) "
	      , core->ids.left.control()->name
	      , core->ids.right.Rdest
	      , core->ids.right.Rsrc1
	      , core->ids.right.Rsrc2
	      , core->ids.right.Rsrc1Val
	      , core->ids.right.Rsrc2Val
	      , core->ids.right.immediate);

	if (core->exs.busyCycles>0)
	{
		printf("(ex '%s' \033[31m busy\033[0m) "
			, core->exs.left.control()->name);
	} else {
		printf("(ex '%s' $%-2d <= $%-2d%c($%-2d or %08x)) "
			, core->exs.left.control()->name
			, core->exs.right.Rdest
			, core->exs.right.Rsrc1
			, (core->exs.left.opcode == 1 || 
				(core->exs.left.opcode == 9) ? '+' : (core->exs.left.opcode == 8 ? '-' : ' '))
			, core->exs.right.Rsrc2
			, core->exs.right.aluresult);
	}
	
	if (!core->mys.isIdle())
	{
		printf(" (ms '%s' %s: 0x%0X \033[31mstall\033[0m) "
		      , core->mys.left.control()->name
			  , ((core->mys.left.control()->mem_read) ? " read_mem":"write_mem")
		      , core->mys.left.aluresult);
	}
	else if (core->mys.left.control()->mem_read) {
		printf(" (ms '%s'  read_mem: 0x%0X val: %08x) "
		      , core->mys.left.control()->name
		      , core->mys.left.aluresult, core->mys.right.mem_data);
	}
	else if (core->mys.left.control()->mem_write) {
		printf(" (ms '%s' write_mem: 0x%0X val: %08x) "
		      , core->mys.left.control()->name
		      , core->mys.left.aluresult, core->mys.right.mem_data);
	}
	else if (core->mys.left.control()->branch) {
		printf(" (ms done) ");
	}
	else {
		printf(" (ms '%s') ", core->mys.left.control()->name);
	}

	if (core->wbs.left.control()->register_write) {
		if (core->wbs.left.control()->mem_to_register) {
			printf(" (wb '%-s' $%-2d<=%08x)"
			      , core->wbs.left.control()->name
			      , core->wbs.left.Rdest
			      , core->wbs.left.mem_data);
		}
		else {
			printf(" (wb '%-s' $%-2d<=%08x )"
			      , core->wbs.left.control()->name
			      , core->wbs.left.Rdest
			      , core->wbs.left.aluresult);
		}
	}
	else {
		printf(" (wb '%-s' nowriteback)", core->wbs.left.control()->name);
	}

	printf("\n");
}


// Actual execution of whatever is in the CPU will occur here.
void run_cpu(memory *mem, const bool verbose_cpu, const int bp_type)
{
	cpu_core core;

	core.cycles=0;
	core.BPHits=0; 
	core.BPMisses=0; 
	core.PC = text_segment;
	core.usermode = true;
	core.mem = mem;
	core.verbose = verbose_cpu;
	core.bp_type= bp_type;

	if (core.bp_type==0) {
		core.bp = new AlwaysNotTakenBP();
	} else if (core.bp_type==1) {
		core.bp = new AlwaysTakenBP();
	} else if (core.bp_type==2) {
		core.bp = new LocalBP(2048,2,3);
	} else if (core.bp_type==3) {
		core.bp = new TwoLevelBP(2048,2,10);		
	} else if (core.bp_type==4) {
		core.bp = new GShareBP(2048,2,3,10);
	}		
	
	// initialize registers
	//for (int32_t x = 0; x < 32; x++) core.registers[x] = 0;

	// start the cpu loop
	try {
		while (core.usermode) {
			// Execute the stages
			//core.mys.Execute();

			core.wbs.cycle(); // First so that writes happen before reads
			core.mys.cycle();
			core.dcache_ctl.cycle();
			core.ifs.cycle();
			core.ids.cycle();
			core.exs.cycle();

			// Do Forwarding
			core.exs.doForwarding();
			core.mys.doForwarding();

			// Print the intermediate stages onto the screen for debugging.
			if (core.verbose) {
				print_stages(&core);
			}

			// shift the latches
			//core.ids.Shift();
			//core.exs.Shift();
			//core.mys.Shift();
			core.wbs.shift();
			core.mys.shift();
			core.exs.shift();
			core.ids.shift();
			core.cycles++;
#if 0
			// Occasionally this stuff is useful
			if (core.verbose) {
				for (int32_t x = 0; x < 8; x++) {
					printf("$%d:\t0x%08x\t\t$%d:\t0x%08x\t\t$%d:\t0x%08x\t\t$%d:\t0x%08x\n"
					      , x, core.registers[x]
					      , x + 8, core.registers[x + 8]
					      , x + 16, core.registers[x + 16]
					      , x + 24, core.registers[x + 24]);
				}
				//getchar(); // slows things down!
			}
#endif
		}
		printf("stat.processor_cycles: %d\n", core.cycles);
		printf("stat.bp_hits: %d\n", core.BPHits);
		printf("stat.bp_misses: %d\n", core.BPMisses);
		core.dcache_ctl.printStat();
		//printf("BP miss rate: %f\n",(double)core.BPMisses/ (double)(core.BPHits+core.BPMisses));
	} catch (const char *e) {
		printf("CPU fault: %s\n", e);
	}

	if (core.bp)
		delete core.bp;
	
}
