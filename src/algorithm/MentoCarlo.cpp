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



void PackingUtility::Adapt(unordered_map<const Block *, double> &policy, PackingState &state) {
  for (int i = 0; i < state.plan.size(); ++i) {
    policy[state.plan[i].block] *= 1.5;
  }
}

void PackingUtility::Rollout(unordered_map<const Block *, double> &policy, PackingState &state)
{
    InitState(state);

    int k = 0;
    while (!state.spaceStack.empty())
    {
        Block *blockList[MaxBlockList];
        int blockListLen = 0;

        GenBlockList(state, 32, blockList, blockListLen);

        double total = 0;
        vector<double> weight(blockListLen);
        for (int i = 0; i < blockListLen; ++i) {
          PackingState tempState = state;
          Space space;
          UpdateState(tempState, blockList[i], space);
          CompleteSolution(tempState);
          //weight[i] = policy[blockList[i]] * blockList[i]->volume * blockList[i]->volume;
          weight[i] = policy[blockList[i]] * tempState.volume * tempState.volume;
          total += weight[i];
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
          while (selected < blockListLen-1 && sum + weight[selected] < p) {
            sum += weight[selected++];
          }

          UpdateState(state, blockList[selected], space);
          ++k;
        }
    }
}

void PackingUtility::MentoCarloSearch(
    int level, int iterations, unordered_map<const Block *, double> &policy, PackingState &best)
{
  InitState(best);
  Rollout(policy, best);
  
  if (level == 0) {
    Rollout(policy, best);
  } else {
    for (int i = 0; i < iterations; ++i) {
      PackingState state;
      unordered_map<const Block *, double> newPolicy = policy;
      MentoCarloSearch(level - 1, iterations, newPolicy, state);
      if (state.volume > best.volume) {
        best = state;
        Adapt(policy, best);
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

    unordered_map<const Block *, double> policy;
    for (int i = 0; i < blockTableLen; ++i)
      policy[blockTable[i]] = 1;
    MentoCarloSearch(level, iterations, policy, best);

    return best;
}
