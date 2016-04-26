#ifndef _PREFETCHERS_H_
#define _PREFETCHERS_H_
#include "instruction.h"
#include "cache_impl.hh"
#include <deque>

extern uint32_t cache_size;
extern uint32_t cache_assoc;
extern int prefetcher_type;
extern uint32_t cacheline_size;
const uint32_t MAX_PREFETCH_QUEUE_SIZE = 32;			//PREFETCH QUEUE SIZE

enum PrefetcherState
{
	Initial,
	Transient,
	Steady,
	NoPred
};

struct StrideEntry
{
	uint32_t tag;
	uint32_t last_addr;
	int stride;
	PrefetcherState state;
};

class Prefetcher {
protected:
	int	degree;	
	SimpleCache *cache;
public:
	std::deque<uint32_t> prefetchQueue;	
	Prefetcher()
	{
		cache=NULL;
	}
	~Prefetcher()
	{
	}
	virtual bool updatePrefetchQueue(uint32_t pc, uint32_t miss_addr)=0;
};

class StridePrefetcher : public Prefetcher {
private:
	uint32_t size;
	StrideEntry *RPT;
public:
	StridePrefetcher(SimpleCache *_cache, uint32_t _size, uint32_t prefetch_degree=1)
	{
		cache=_cache;
		size = _size;
		degree=prefetch_degree;
		
		RPT = new StrideEntry[size];
		
		for (uint32_t i=0;i < size;i++)
		{
			RPT[i].tag=0;
			RPT[i].stride=0;
			RPT[i].last_addr=0;
			RPT[i].state=Initial;
		}
	}
	
	~StridePrefetcher()
	{
		if (RPT)
			delete[] RPT;
	}
	
	bool updatePrefetchQueue(uint32_t pc, uint32_t miss_addr)
	{
		updateTable(pc, miss_addr);
		
		uint32_t index = pc & (size - 1);
		//prefetch next line when a miss is happen
		
		if (RPT[index].tag!=pc)
			return NULL;
		
		if (RPT[index].state != Steady)
			return NULL;
			
		for (int i=0 ;i < degree; i++)
		{
			if (prefetchQueue.size() > MAX_PREFETCH_QUEUE_SIZE)
				prefetchQueue.pop_front();
			uint32_t prefetch_addr = RPT[index].last_addr + RPT[index].stride * (i+1);
			prefetchQueue.push_back(prefetch_addr);	
		}
		
		return true;
	}
	void updateTable(uint32_t pc, uint32_t miss_addr)
	{
		uint32_t index = pc & (size - 1);
		if (RPT[index].tag != pc) {
			RPT[index].tag = pc;
			RPT[index].state = Initial;
			RPT[index].last_addr = miss_addr;
			RPT[index].stride = 0;
		} else {
			bool incorrect = (miss_addr != (RPT[index].last_addr + RPT[index].stride));
			
			if (incorrect && RPT[index].state == Initial) {
				
				RPT[index].stride = (miss_addr - RPT[index].last_addr);	
				RPT[index].state = Initial;	
				
			} else if (incorrect && RPT[index].state == Steady){
				
				RPT[index].state = Transient;
				
			} else if (incorrect && RPT[index].state == (Transient || RPT[index].state == NoPred)){
				
				RPT[index].state = NoPred;
				RPT[index].stride = (miss_addr - RPT[index].last_addr);	
				
			} else if (!incorrect && RPT[index].state == NoPred) {
				
				RPT[index].state = Transient;
				
			} else if (!incorrect && (RPT[index].state == Initial || RPT[index].state == Transient || RPT[index].state == Steady)) {
				
				RPT[index].state=Steady;
				
			} 
			RPT[index].last_addr=miss_addr;
		}
	}
};


class NextLinePrefetcher : public Prefetcher {
public:
	NextLinePrefetcher(SimpleCache *_cache, uint32_t prefetch_degree=1)
	{
		cache=_cache;
		degree = prefetch_degree;
	}
	~NextLinePrefetcher()
	{
		
	}
	
	bool updatePrefetchQueue(uint32_t pc, uint32_t miss_addr)
	{
		//prefetch next line when a miss is happen
		for (int i=0; i < 8; i++)
		{
			if (prefetchQueue.size() > MAX_PREFETCH_QUEUE_SIZE)
				prefetchQueue.pop_front();
			uint32_t prefetch_addr = ((miss_addr / cacheline_size) + (i+1) ) * cacheline_size;
			prefetchQueue.push_back(prefetch_addr);
			
		}
		return NULL;
	}
};

#endif /* _PREFETCHERS_H_ */
