#include "cpu.h"
#include "stages.h"
#include <cstdio>
#include <cassert>

const uint32_t CACHE_MISS_NUMBER_OF_STALLS = 4;			// 4 stall cycles for cache miss (5 cycles)


void CacheCtrl::cycle()
{
	if (!requestQueue.empty())
	{
		assert(requestQueue.size()==1);			//in this simple pipeline simulator, we can only have one request from MEM stage.
		
		CachePacket* pkt = requestQueue.front();
		
		CacheBlk *blk=cache->access(pkt->addr);
		
		if (blk && blk->isValid()) {
			
			if (prefetcher && blk->wasPrefetched()) {		//update prefetcher
				blk->clearPrefetchFlag();
				prefetcher->updatePrefetchQueue(pkt->pc, pkt->addr);
			}
		
			if (pkt->reqType == CacheWriteReq)
				blk->markDirty();
			
			responseToCPU(pkt);
			hits++;
			requestQueue.pop_front();
			
		} else {
			//If there is a pendingMissReq, we should retry it in the next cycle.
			//This will not happen before you implement the prefetcher.			
			if (pendingMissReq == NULL) {		//only support one outstanding miss request
				//handle cache miss	
				if (prefetcher) {				//update prefetcher
					prefetcher->updatePrefetchQueue(pkt->pc, pkt->addr);
				}						
				pendingMissReq=pkt;
				pendingMissReq->readyCycleCount += CACHE_MISS_NUMBER_OF_STALLS;
				
				requestQueue.pop_front();
				misses++;
			}
		}
	} else if (cache && !pendingMissReq && prefetcher) {			//no other miss req, let's do a prefetch
		//printf("prefetch addr:%X\n", pendingPrefetchReq->addr);
		
		while(!(prefetcher->prefetchQueue.empty())) {
			uint32_t prefetch_addr = prefetcher->prefetchQueue.front();
			prefetcher->prefetchQueue.pop_front();
			if (!cache->findBlock(prefetch_addr)) {
				pendingMissReq = new CachePacket( core->cycles ,
											0,					//not used
											prefetch_addr, 
											CachePrefetchReq,
											cacheline_size		//not used
											);
				pendingMissReq->readyCycleCount += CACHE_MISS_NUMBER_OF_STALLS;
				break;
			}
		}
	}

	if (pendingMissReq && (core->cycles >= pendingMissReq->readyCycleCount) ) {	
		
		handleCacheFill(pendingMissReq);
		
		if (pendingMissReq->reqType != CachePrefetchReq) {
			responseToCPU(pendingMissReq);
		} else {
			delete pendingMissReq;
		}
		
		pendingMissReq=NULL;
	}
}

void CacheCtrl::access(CachePacket *pkt)
{	
	if (cache) {
		requestQueue.push_back(pkt);
	} else {					//no cache, always miss.
		pendingMissReq=pkt;
		pendingMissReq->readyCycleCount += CACHE_MISS_NUMBER_OF_STALLS;
		misses++;
	}
}

void CacheCtrl::handleCacheFill(CachePacket *pkt)
{
	if (!cache)
		return;
	//handle cache fill
	
	CacheBlk *victimBlk=cache->findVictim(pkt->addr);
	
	if (victimBlk->isValid() && victimBlk->isDirty()) {
		//Theoreically, cache will issue a writeback request to the memory, 
		//but you can assume ignore this writeback here.
		//issueWriteBack();
	}
	
	victimBlk=cache->insertBlock(pkt->addr,victimBlk);
	
	if (pkt->reqType == CacheWriteReq)
		victimBlk->markDirty();	
	
	if (pkt->reqType == CachePrefetchReq)
		victimBlk->markPrefetched();
}

void CacheCtrl::responseToCPU(CachePacket *pkt)
{
	//NOTE: You do not need to simulate the data in the cache.
	//This is a hack for you to get the data from memory.
	if (pkt->reqType == CacheWriteReq) {
		if (pkt->size == 1) {
			core->mem->set<byte>(pkt->addr, pkt->data);
		} else if (pkt->size == 4) {
			core->mem->set<uint32_t>(pkt->addr, pkt->data);
		} else {
			assert(1);
		}
	} else 	if (pkt->reqType == CacheReadReq) {
		if (pkt->size == 1) {
			pkt->data = core->mem->get<byte>(pkt->addr);
		} else if (pkt->size == 4) {
			pkt->data = core->mem->get<uint32_t>(pkt->addr);
		} else {
			assert(1);
		}
	} else {
		assert(1);
	}
	
	core->mys.response(pkt);
}

void CacheCtrl::printStat()
{
	//print the statistic here
	printf("stat.dcache_hits: %d\n", hits);
	printf("stat.dcache_misses: %d\n", misses);
	printf("stat.dcache_miss_rate: %.2f %%\n", ((double)misses/(double)(hits+misses)) * 100);	
}