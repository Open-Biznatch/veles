// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The Veles developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "chain.h"
#include "chainparams.h"
#include "main.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

#include <math.h>

// Crypostle Difficulty Retargeting 1.1 - CalculateNextWorkRequired
unsigned int CalculateNextWorkRequiredCrypostle(const CBlockIndex* pindexLast, int64_t nFirstBlockTime)
{
    //if (params.fPowNoRetargeting)
    //    return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    //LogPrintf("  nActualTimespan = %d  before bounds\n", nActualTimespan);
    int64_t nTargetTimespan = Params().TargetSpacing()/80;  //changed for quark algo
    if (nActualTimespan < nTargetTimespan)
        nActualTimespan = nTargetTimespan;
    if (nActualTimespan > nTargetTimespan)
        nActualTimespan = nTargetTimespan;

    // Retarget
    const uint256 bnPowLimit = uint256(Params().ProofOfWorkLimit());
    uint256 bnNew;
    uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    bnNew *= nActualTimespan;
    bnNew /= Params().TargetSpacing();

    // Limit
    if (bnNew > bnPowLimit)
    {
        bnNew = bnPowLimit;
    }

    /// debug print
    /*
    LogPrintf("GetNextWorkRequiredCrypostle RETARGET\n");
    LogPrintf("params.nPowTargetTimespan = %d    nActualTimespan = %d\n", params.nPowTargetTimespan, nActualTimespan);
    LogPrintf("Before: %08x  %s\n", pindexLast->nBits, bnOld.ToString());
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());
    */

    return bnNew.GetCompact();
}

// Crypostle Difficulty Retargeting 1.1 - GetNextWorkRequired
unsigned int GetNextWorkRequiredCrypostle(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
    unsigned int nProofOfWorkLimit = uint256(Params().ProofOfWorkLimit()).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
    {
        return nProofOfWorkLimit;
    }

    if (Params().AllowMinDifficultyBlocks())
    {
        // Special difficulty rule for testnet:
        // If the new block's timestamp is more than 2* 2.5 minutes
        // then allow mining of a min-difficulty block.
        if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + Params().TargetSpacing() * 2)
        {
            return nProofOfWorkLimit;
        }
        else
        {
            // Return the last non-special-min-difficulty-rules-block
            const CBlockIndex* pindex = pindexLast;
            while (pindex->pprev && pindex->nHeight % 1 != 0 && pindex->nBits == nProofOfWorkLimit)
            {
                pindex = pindex->pprev;
                return pindex->nBits;
            }

        }
    }

    // Go back by what we want to be 1 blocks
    int nHeightFirst = pindexLast->nHeight;
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

   return CalculateNextWorkRequiredCrypostle(pindexLast, pindexFirst->GetBlockTime());
}
	
unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock)
{
	return GetNextWorkRequiredCrypostle(pindexLast, pblock);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    bool fNegative;
    bool fOverflow;
    uint256 bnTarget;

    if (Params().SkipProofOfWorkCheck())
        return true;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > Params().ProofOfWorkLimit())
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    // if (hash > bnTarget)
    //     return error("CheckProofOfWork() : hash doesn't match nBits");

    return true;
}

uint256 GetBlockProof(const CBlockIndex& block)
{
    uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}
