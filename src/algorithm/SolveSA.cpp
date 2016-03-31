#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>

#include "common/packing.h"
#include "common/log.h"

using namespace std;

void PackingUtility::Heuristic(PackingSequence &ps, PackingState &state)
{
    InitState(state);

    int k = 0;
    while (!state.spaceStack.empty())
    {
        if (k == MaxPlan)
        {
            Log(LogError, "index error\n");
            exit(1);
        }

        if (k == ps.len)
            ps.index[ps.len++] = 0;

        Block *blockList[MaxBlockList];
        int blockListLen = 0;

        GenBlockList(state, ps.index[k] + 1, blockList, blockListLen);

        Space space;
        if (blockListLen == 0)
        {
            UpdateState(state, NULL, space);
        }
        else
        {
            if (ps.index[k] >= blockListLen)
                ps.index[k] = blockListLen-1;
            UpdateState(state, blockList[ps.index[k]], space);
            ++k;
        }
    }

    ps.len = k;
    ps.volume = state.volume;
}

PackingState PackingUtility::SolveSA(double ts, double tf, double dt, 
                                     int length, bool isLinear, int stage)
{
    copy(problem->blockTable.begin(), problem->blockTable.end(), blockTable);
    blockTableLen = problem->blockTable.size();

    copy(problem->boxList, problem->boxList + problem->boxListLen, boxList);
    boxListLen = problem->boxListLen;

    int len = 0;
    for (int i = 0; i < blockTableLen; ++i)
    {
        Block *block = blockTable[i];
        if ((stage == 1 || block->type == SimpleBlock))
                blockTable[len++] = block;
    }
    blockTableLen = len;

    fill_n(&dict[0][0], MaxLength*MaxLength, MaxNum);
    for (int i = 0; i < boxListLen; ++i)
    {
        Box *box = boxList[i];
        if (box->lx < dict[box->ly][box->lz])
            dict[box->ly][box->lz] = box->lx;
    }

    for (int i = 0; i < MaxLength; ++i)
    {
        for (int j = 0; j < MaxLength; ++j)
        {
            int &x = dict[i][j];
            if (i > 0 && dict[i-1][j] < x)
                x = dict[i-1][j];
            if (j > 0 && dict[i][j-1] < x)
                x = dict[i][j-1];
        }
    }

    PackingSequence temp;
    PackingSequence curr;
    PackingState state;

    best.volume = 0;
    temp.len = 0;

    Heuristic(temp, state);
    curr = temp;
    best = state;

    {
        temp.len = MaxPlan;
        for (int i = 0; i < MaxPlan; ++i)
            temp.index[i] = rand() % 80;

        Heuristic(temp, state);
        if (temp.volume > best.volume)
        {
            best = state;
            curr = temp;
        }
    }

    // Start temperature
    double	t = ts;

    while (t > tf)
    {
        for (int i = 0; i < length; ++i)
        {
            temp = curr;

            //cerr << temp.len << endl;
            // Generate a new packing selection for the buffer.
            int	k = rand() % temp.len;
            temp.index[k] = rand() % 80;

            // Calculate the heuristic result and use the metropolis principle to accept.
            Heuristic(temp, state);

            if (temp.volume > curr.volume)
                curr = temp;
            else if (rand() < exp((temp.volume - curr.volume)/t) * RAND_MAX)
                curr = temp;

            if (temp.volume > best.volume)
                best = state;
        }

        if (isLinear)
            t *= dt;
        else
            t = (1 - t*dt)*t;
    }

    return best;
}
