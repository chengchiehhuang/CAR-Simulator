#ifndef _CACHE_H_
#define _CACHE_H_
#include "instruction.h"
//#include "stages.h"
//#include "cpu.h"
#include "cache_impl.hh"
#include "prefetchers.hh"
#include <deque>

extern uint32_t cache_size;
extern uint32_t cache_assoc;
extern int prefetcher_type;

//You do not need to parameterize these two.
extern uint32_t cacheline_size;

enum CacheRequestType
{
	CacheReadReq,
	CacheWriteReq,
	CacheWriteBackReq,
	CachePrefetchReq
};

struct CachePacket
{
	uint32_t pc;
	uint32_t addr;
	CacheRequestType reqType;		//0: read, 1: write, 2: prefetch
	int size;			//1: BYTE 4: DWORD 
	uint32_t data;
	uint32_t readyCycleCount;
	
	CachePacket(uint32_t curCycle, uint32_t _pc, uint32_t _addr, CacheRequestType _reqType, int _size, uint32_t _data=0)
	{
		pc = _pc;
		addr=_addr;
		reqType=_reqType;		
		size=_size;
		data=_data;
		
		readyCycleCount = curCycle;
	}
};


class CacheCtrl {
private:
	cpu_core* core;
	CachePacket* pendingMissReq;
	//We did not support multiple outstanding misses here. std::deque<CachePacket*> missQueue; 
	std::deque<CachePacket*> requestQueue;	
	std::deque<uint32_t> prefetchQueue;
	
	SimpleCache *cache;
	Prefetcher *prefetcher;
	
	uint32_t hits;
	uint32_t misses;
	
	void responseToCPU(CachePacket *pkt);
	void handleCacheFill(CachePacket *pkt);	
	
public:
	CacheCtrl(cpu_core *c)
	{
		core = c;
		pendingMissReq=NULL;
		cache = NULL;
		hits=0;
		misses=0;
		if (cache_size>0) {
			printf("Cache Config: Size = %d b, Assoc = %d, Line Size = %d b, Prefetcher = %s\n", cache_size, cache_assoc, cacheline_size, prefetcher_type > 0 ?"enabled":"disabled");
			cache = new SimpleCache(cache_size, cache_assoc, cacheline_size);
			if (prefetcher_type == 1) {
				prefetcher = new NextLinePrefetcher(cache, 4);
			} else if (prefetcher_type == 2) {
				prefetcher = new StridePrefetcher(cache, 1024, 1);
			} else {
				prefetcher=NULL;
			}
		}
	}
	~CacheCtrl()
	{
		if (cache)
			delete cache;
	}
	void cycle();
	void access(CachePacket *pkt);

	void printStat();
};

#endif /* _CACHE_H_ */
