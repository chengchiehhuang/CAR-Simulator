#ifndef _CACHE_IMPL_H_
#define _CACHE_IMPL_H_
//#include "instruction.h"
/*
 * Copyright (c) 2012-2013 ARM Limited
 * Copyright (c) 2016 Cheng-Chieh Huang
 * All rights reserved.
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
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
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
 * Authors: Erik Hallnor
 *          Cheng-Chieh Huang
 */
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int32_t INT32;

/** uint64_t constant */
#define ULL(N)          ((uint64_t)N##ULL)
/** int64_t constant */
#define LL(N)           ((int64_t)N##LL)
inline int
floorLog2(unsigned x)
{
    //assert(x > 0);

    int y = 0;

    if (x & 0xffff0000) {
        y += 16;
        x >>= 16;
    }
    if (x & 0x0000ff00) {
        y +=  8;
        x >>=  8;
    }
    if (x & 0x000000f0) {
        y +=  4;
        x >>=  4;
    }
    if (x & 0x0000000c) {
        y +=  2;
        x >>=  2;
    }
    if (x & 0x00000002) {
        y +=  1;
    }

    return y;
}

inline int
floorLog2(unsigned long x)
{
    //assert(x > 0);

    int y = 0;

#if defined(__LP64__)
    if (x & ULL(0xffffffff00000000)) {
        y += 32;
        x >>= 32;
    }
#endif
    if (x & 0xffff0000) {
        y += 16;
        x >>= 16;
    }
    if (x & 0x0000ff00) {
        y +=  8;
        x >>=  8;
    }
    if (x & 0x000000f0) {
        y +=  4;
        x >>=  4;
    }
    if (x & 0x0000000c) {
        y +=  2;
        x >>=  2;
    }
    if (x & 0x00000002) {
        y +=  1;
    }

    return y;
}

inline int
floorLog2(unsigned long long x)
{
    //assert(x > 0);

    int y = 0;

    if (x & ULL(0xffffffff00000000)) {
        y += 32;
        x >>= 32;
    }
    if (x & ULL(0x00000000ffff0000)) {
        y += 16;
        x >>= 16;
    }
    if (x & ULL(0x000000000000ff00)) {
        y +=  8;
        x >>=  8;
    }
    if (x & ULL(0x00000000000000f0)) {
        y +=  4;
        x >>=  4;
    }
    if (x & ULL(0x000000000000000c)) {
        y +=  2;
        x >>=  2;
    }
    if (x & ULL(0x0000000000000002)) {
        y +=  1;
    }

    return y;
}

inline int
floorLog2(int x)
{
    //assert(x > 0);
    return floorLog2((unsigned)x);
}

inline int
floorLog2(long x)
{
    //assert(x > 0);
    return floorLog2((unsigned long)x);
}

inline int
floorLog2(long long x)
{
    //assert(x > 0);
    return floorLog2((unsigned long long)x);
}

enum CacheBlkStatusBits {
    /** valid, readable */
    BlkValid =          0x01,
    /** write permission */
    BlkWritable =       0x02,
    /** read permission (yes, block can be valid but not readable) */
    BlkReadable =       0x04,
    /** dirty (modified) */
    BlkDirty =          0x08,
    /** block was referenced */
    BlkReferenced =     0x10,
    /** block was a hardware prefetch yet unaccessed*/
    BlkHWPrefetched =   0x20,
    /** block holds data from the secure memory space */
    BlkSecure =         0x40,
    /** can the block transition to E? (hasn't been shared with another cache)
      * used to close a timing gap when handling WriteInvalidate packets */
    BlkCanGoExclusive = 0x80
};

class CacheBlk
{
public:

    /** The address space ID of this block. */
    //int asid;
    /** Data block tag value. */
    uint32_t tag;

    /** block state: OR of CacheBlkStatusBit */
    typedef unsigned State;

    /** The current status of this block. @sa CacheBlockStatusBits */
    State status;
	
    uint32_t set;
public:

    CacheBlk()
        : tag(0), status(0), 
          set(-1)
    {
    }

    /**
     * Copy the state of the given block into this one.
     * @param rhs The block to copy.
     * @return a const reference to this block.
     */
    const CacheBlk& operator=(const CacheBlk& rhs)
    {
        tag = rhs.tag;
        status = rhs.status;
        set = rhs.set;

        return *this;
    }

    /**
     * Checks that a block is valid.
     * @return True if the block is valid.
     */
    bool isValid() const
    {
        return (status & BlkValid) != 0;
    }

    void invalidate()
    {
        status = 0;
    }

    bool isDirty() const
    {
        return (status & BlkDirty) != 0;
    }
    
    bool wasPrefetched() const
    {
        return (status & BlkHWPrefetched) != 0;
    }
    
    void markPrefetched() 
    {
        status |= BlkHWPrefetched;
    }
    
    void clearPrefetchFlag() 
    {
        status &= ~BlkHWPrefetched;
    }       
    
    void markDirty() 
    {
        status |= BlkDirty;
    }    

};

template <class Blktype>
class CacheSet
{
public:
    /** The associativity of this set. */
    int assoc;

    /** Cache blocks in this set, maintained in LRU order 0 = MRU. */
    Blktype **blks;

    /**
     * Find a block matching the tag in this set.
     * @param way_id The id of the way that matches the tag.
     * @param tag The Tag to find.
     * @param is_secure True if the target memory space is secure.
     * @return Pointer to the block if found. Set way_id to assoc if none found
     */
    Blktype* findBlk(Addr tag, bool is_secure, int& way_id) const ;
    Blktype* findBlk(Addr tag, bool is_secure) const ;

    /**
     * Move the given block to the head of the list.
     * @param blk The block to move.
     */
    void moveToHead(Blktype *blk);

    /**
     * Move the given block to the tail of the list.
     * @param blk The block to move
     */
    void moveToTail(Blktype *blk);

};

template <class Blktype>
Blktype*
CacheSet<Blktype>::findBlk(Addr tag, bool is_secure, int& way_id) const
{
    /**
     * Way_id returns the id of the way that matches the block
     * If no block is found way_id is set to assoc.
     */
    way_id = assoc;
    for (int i = 0; i < assoc; ++i) {
        if (blks[i]->tag == tag && blks[i]->isValid()) {
            way_id = i;
            return blks[i];
        }
    }
    return NULL;
}

template <class Blktype>
Blktype*
CacheSet<Blktype>::findBlk(Addr tag, bool is_secure) const
{
    int ignored_way_id;
    return findBlk(tag, is_secure, ignored_way_id);
}

template <class Blktype>
void
CacheSet<Blktype>::moveToHead(Blktype *blk)
{
    // nothing to do if blk is already head
    if (blks[0] == blk)
        return;

    // write 'next' block into blks[i], moving up from MRU toward LRU
    // until we overwrite the block we moved to head.

    // start by setting up to write 'blk' into blks[0]
    int i = 0;
    Blktype *next = blk;

    do {
        //assert(i < assoc);
        // swap blks[i] and next
        Blktype *tmp = blks[i];
        blks[i] = next;
        next = tmp;
        ++i;
    } while (next != blk);
}

template <class Blktype>
void
CacheSet<Blktype>::moveToTail(Blktype *blk)
{
    // nothing to do if blk is already tail
    if (blks[assoc - 1] == blk)
        return;

    // write 'next' block into blks[i], moving from LRU to MRU
    // until we overwrite the block we moved to tail.

    // start by setting up to write 'blk' into tail
    int i = assoc - 1;
    Blktype *next = blk;

    do {
        //assert(i >= 0);
        // swap blks[i] and next
        Blktype *tmp = blks[i];
        blks[i] = next;
        next = tmp;
        --i;
    } while (next != blk);
}


class SimpleCache
{
protected:
    typedef CacheBlk BlkType;
    typedef CacheSet<BlkType> SetType;
    SetType *sets;
    BlkType *blks;
    unsigned int numSets;
    unsigned int assoc;
    unsigned int blkSize;
    unsigned int setMask;
    int setShift;
    int tagShift;

public:

    SimpleCache ()
    {
    }

    SimpleCache (uint64_t _size, uint32_t _assoc, int _blkSize)
    {
        initCache(_size,_assoc,_blkSize);
    }

    virtual ~SimpleCache ()
    {
    }

    void initCache(uint64_t _size, uint32_t _assoc, int _blkSize)
    {
        numSets = _size / (_blkSize * _assoc);
        assoc = _assoc;
        blkSize = _blkSize;
        setShift = floorLog2(blkSize);
        tagShift = setShift + floorLog2(numSets);
        setMask = numSets - 1;
        sets = new SetType[numSets];
        blks = new BlkType[numSets * assoc];

        unsigned blkIndex = 0;       // index into blks array
        for (unsigned i = 0; i < numSets; ++i) {
            sets[i].assoc = assoc;

            sets[i].blks = new BlkType*[assoc];

            // link in the data blocks
            for (unsigned j = 0; j < assoc; ++j) {
                // locate next cache block
                BlkType *blk = &blks[blkIndex];
                ++blkIndex;

                // invalidate new cache block
                blk->invalidate();

                blk->tag = j;
                sets[i].blks[j]=blk;
                blk->set = i;
            }
        }
    }

    uint64_t regenerateBlkAddr(uint64_t tag, unsigned set) const
    {
        return ((tag << tagShift) | ((Addr)set << setShift));
    }

    uint64_t regenerateBlkAddr(BlkType* blk) const
    {
        return ((blk->tag << tagShift) | (blk->set << setShift));
    }

    uint64_t extractTag(uint64_t fullIndex)
    {
        return (fullIndex >> tagShift);
    }
    uint32_t extractSet(Addr fullIndex)
    {
        return ((fullIndex >> setShift) & setMask);
    }
    
    bool findBlock(uint64_t addr)
    {
        Addr tag = extractTag(addr);
        unsigned set = extractSet(addr);
        BlkType *blk = sets[set].findBlk(tag,false);
        if (blk != NULL) {
            return true;
        }
        return false;
    }
    
    BlkType* access(uint64_t addr)
    {
        uint64_t tag = extractTag(addr);
        unsigned set = extractSet(addr);
        BlkType *blk = sets[set].findBlk(tag,false);
        if (blk != NULL) {
            sets[set].moveToHead(blk);
            return blk;
        }
        return NULL;
    }
    
    BlkType* findVictim(uint64_t addr)
    {
        unsigned set = extractSet(addr);
        return sets[set].blks[assoc-1];
    }
    
    BlkType *
    insertBlock(uint64_t addr,BlkType *victimBlk)
    {
        unsigned set = extractSet(addr);
        uint64_t tag = extractTag(addr);
        victimBlk->tag = tag;	//update tag
        victimBlk->status = BlkValid;
		
        sets[set].moveToHead(victimBlk);
        return victimBlk;
    }
    
    void
    invalidate(BlkType *blk)
    {
        // should be evicted before valid blocks
        unsigned set = blk->set;
        sets[set].moveToTail(blk);
        blk->invalidate();
    }
};

#endif /* _CACHE_IMPL_H_ */
