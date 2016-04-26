#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include "memory.h"
#include "cpu.h"

extern char   *optarg;
extern int32_t optind;
extern int32_t optopt;
extern int32_t opterr;
extern int32_t optreset;

using namespace std;

// Usage of the program
static void usage(char *name)
{
	cout << name << " usage:\n" <<
	        "\t-t text_stream_file: load .text with the contents of file\n" <<
	        "\t-d data_stream_file: [optional] load .data with contents of file\n" << endl;
}

int      bp_type = 4;
uint32_t cache_size = 0;
uint32_t cache_assoc = 2;
bool     enable_prefetcher = false;

uint32_t cacheline_size = 32;
int		prefetcher_type = 0;
	
// *****************************
//           entry point
// *****************************
int32_t main(int32_t argc, char **argv)
{
	memory   mem;
	bool     verb = false;
	bool     text_loaded = false;
	int32_t  ch;
	uint32_t text_ptr = text_segment;
	uint32_t data_ptr = data_segment;

	while ((ch = getopt(argc, argv, "t:d:vb:s:a:l:ox:")) != -1) {
		switch (ch) {
		case 't': {
			ifstream input(optarg, ios::binary);
			if (!(input.good() && input.is_open())) {
				cout << *argv << ": " << optarg << " does not exist" << endl;
				exit(20);
			}
			byte  c;
			char *pc = (char *)&c;
			while (!input.eof()) {
				input.read(pc, sizeof(byte));
				mem.set<byte>(text_ptr++, c);
			}
			input.close();
			text_loaded = true;
		}
		break;
		case 'd': {
			ifstream input(optarg, ios::binary);
			if (!(input.good() && input.is_open())) {
				cout << *argv << ": " << optarg << " does not exist" << endl;
				exit(20);
			}
			byte c;
			while (!input.eof()) {
				input.read((char *)&c, sizeof(byte));
				mem.set<byte>(data_ptr++, c);
			}
			input.close();
		}
		break;

		case 'b': 
			bp_type=atoi(optarg);
			if (bp_type > 4)
			{
				cout << "bp_type = "<<bp_type<< " is not supported" << endl;
				exit(20);				
			}
		break;
		
		case 's': 
		{
			int size_in_kb=atoi(optarg);
			
			if (size_in_kb != 0 && size_in_kb != 1 && size_in_kb != 2 && size_in_kb != 4 && size_in_kb != 8)
			{
				cout << "cache size = "<< size_in_kb <<" KB is not supported" << endl;
				exit(20);				
			} else {
				cache_size = size_in_kb*1024;
			}
		}
		break;
		
		case 'a':
			cache_assoc=atoi(optarg);
			if (cache_assoc != 1 && cache_assoc != 2 && cache_assoc != 4 && cache_assoc != 8)
			{
				cout << "cache_assoc = "<< cache_assoc <<" is not supported" << endl;
				exit(20);				
			}
		break;		

		
		case 'l':
			cacheline_size=atoi(optarg);
			if (cacheline_size != 8 && cacheline_size != 16 && cacheline_size != 32 && cacheline_size != 64)
			{
				cout << "cache line size = "<< cacheline_size <<" is not supported" << endl;
				exit(20);				
			}
		break;	
		
		case 'o':
			enable_prefetcher=true;
			if (enable_prefetcher) {
				cout << "Enable Hardware Prefetcher" << endl;
			}
		break;	
		
		case 'x':
			prefetcher_type=atoi(optarg);
			if (prefetcher_type != 0 && prefetcher_type != 1 && prefetcher_type != 2)
			{
				cout << "prefetcher_type = "<< prefetcher_type <<" is not supported" << endl;
				exit(20);				
			}
		break;			
		
		case 'v':
			verb = true;
			break;

		default:
			usage(*argv);
			exit(10);
			break;
		}
	}

	if (!text_loaded) {
		usage(*argv);
		exit(10);
	}
	cout << *argv << ": Starting CPU..." << endl;
	run_cpu(&mem, verb, bp_type);
	cout << *argv << ": CPU Finished" << endl;

	if (mem.is_collecting()) mem.display_memory_stats();
}
