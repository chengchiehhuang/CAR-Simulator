rsim: cpu.o syscall.o stages.o simulator.o memory.o 2bit_local.o gshare.o 2level.o cache.o
	g++ $(FLAGS) -m64 $^ -o rsim

cpu.o: sim/cpu.cc sim/memory.h sim/types.h sim/cpu.h sim/stages.h sim/instruction.h sim/syscall.h sim/cache.h sim/2bit_local.hh sim/sat_counter.hh sim/2level.hh sim/gshare.hh
	g++ $(FLAGS) -m64 -c sim/cpu.cc

stages.o: sim/stages.cc sim/cpu.h sim/memory.h sim/types.h sim/instruction.h sim/stages.h sim/cache.h 
	g++ $(FLAGS) -m64 -c sim/stages.cc

syscall.o: sim/syscall.cc sim/syscall.h sim/cpu.h sim/memory.h sim/types.h sim/instruction.h sim/stages.h
	g++ $(FLAGS) -m64 -c sim/syscall.cc

simulator.o: sim/simulator.cc sim/memory.h sim/types.h sim/cpu.h sim/instruction.h sim/stages.h
	g++ $(FLAGS) -m64 -c sim/simulator.cc

memory.o: sim/memory.cc sim/memory.h sim/types.h
	g++ $(FLAGS) -m64 -c sim/memory.cc
	
2bit_local.o: sim/2bit_local.cc sim/2bit_local.hh sim/sat_counter.hh
	g++ $(FLAGS) -m64 -c sim/2bit_local.cc	
	
2level.o: sim/2level.cc sim/2level.hh sim/sat_counter.hh
	g++ $(FLAGS) -m64 -c sim/2level.cc		

gshare.o: sim/gshare.cc sim/gshare.hh sim/sat_counter.hh
	g++ $(FLAGS) -m64 -c sim/gshare.cc	

cache.o: sim/cache.cc sim/cache.h sim/stages.h sim/stages.cc sim/cpu.h sim/memory.h sim/types.h sim/instruction.h 
	g++ $(FLAGS) -m64 -c sim/cache.cc   
