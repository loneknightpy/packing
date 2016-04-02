#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <unordered_map>

#include "common/packing.h"
#include "common/log.h"

using namespace std;

unordered_map<int, int> dict;

void PackingUtility::Rollout(unordered_map<Block *, double> &policy, PackingState &state)
{
    InitState(state);

    int k = 0;
    while (!state.spaceStack.empty())
    {
        Block *blockList[MaxBlockList];
        int blockListLen = 0;

        GenBlockList(state, blockListLen, blockList, blockListLen);

        double total = 0;
        for (int i = 0; i < blockListLen; ++i) {
          total += policy[blockList[i]];
        }

        Space space;
        if (blockListLen == 0)
        {
            UpdateState(state, NULL, space);
        }
        else
        {
          double p = 1.0 * rand() / RAND_MAX * total;
          int selected = 0;
          double sum = 0;
          while (selected < blockListLen-1 && sum + policy[blockList[selected]] < p)
            ++selected;

          UpdateState(state, blockList[selected], space);
          ++k;
        }
    }
}

void PackingUtility::MentoCarloSearch(
    int level, int iterations, unordered_map<Block *, double> &policy, PackingState &best)
{
  InitState(best);
  Rollout(policy, best);
  
  if (level == 0) {
    Rollout(policy, best);
  } else {
    for (int i = 0; i < iterations; ++i) {
      PackingState state;
      MentoCarloSearch(level - 1, iterations, policy, state);
      if (state.volume > best.volume) {
        best = state;
        //adpt();
      }
    }
  }
}


PackingState PackingUtility::MentoCarlo(int level, int iterations, int stage)
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

    PackingState best;
    unordered_map<Block *, double> policy;
    for (int i = 0; i < blockTableLen; ++i)
      policy[blockTable[i]] = 1;
    MentoCarloSearch(level, iterations, policy, best);
    cerr << best.volume << " " << problem->volume << endl;

    return best;
}
