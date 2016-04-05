#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <unordered_map>

#include "common/packing.h"
#include "common/log.h"

using namespace std;


void PackingUtility::Adapt(unordered_map<const Block *, double> &policy, PackingState &state, PackingState &best) {
  for (int i = state.plan.size(); i < best.plan.size(); ++i) {
    policy[best.plan[i].block] *= 1.5;
  }
}

void PackingUtility::Rollout(unordered_map<const Block *, double> &policy, PackingState &state)
{
    int k = 0;
    while (!state.spaceStack.empty())
    {
        Block *blockList[MaxBlockList];
        int blockListLen = 0;

        GenBlockList(state, 32, blockList, blockListLen);

        double total = 0;
        vector<double> weight(blockListLen);
        for (int i = 0; i < blockListLen; ++i) {
          //PackingState tempState = state;
          //Space space;
          //UpdateState(tempState, blockList[i], space);
          //CompleteSolution(tempState);
          weight[i] = policy[blockList[i]] * blockList[i]->volume * blockList[i]->volume;
          //weight[i] = policy[blockList[i]] * tempState.volume * tempState.volume;
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

PackingState PackingUtility::MentoCarloSearch(
    int level, int iterations, unordered_map<const Block *, double> &policy, PackingState &state)
{
  PackingState best = state;
  Rollout(policy, best);
  
  if (level > 0) {
    for (int i = 0; i < iterations; ++i) {
      unordered_map<const Block *, double> newPolicy = policy;
      PackingState newState = MentoCarloSearch(level - 1, iterations, newPolicy, state);
      if (newState.volume > best.volume) {
        best = newState;
        Adapt(policy, state, best);
      }
    }
  }
  return best;
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

    //PackingState state;
    //InitState(state);
    //best = MentoCarloSearch(level, iterations, policy, state);

    PackingState state;
    InitState(state);

    while (!state.spaceStack.empty()) {
//      const int *avail = current.avail;
//      int len = 0;
//      for (int i = 0; i < blockTableLen; ++i)
//      {
//          Block *block = blockTable[i];
//          if ((stage == 1 || block->type == SimpleBlock))
//          {
//              const int *require = block->require;
//              const int *boxIndex = block->boxIndex;
//              int boxLen = block->boxLen;
//
//              bool flag = true;
//              for (int j = 0; j < boxLen; ++j)
//              {
//                  const int &k = boxIndex[j];
//                  if (require[k] > avail[k])
//                  {
//                      flag = false;
//                      break;
//                  }
//              }
//
//              if (flag)
//                  blockTable[len++] = block;
//          }
//      }
//      blockTableLen = len;
//
//      fill_n(&dict[0][0], MaxLength*MaxLength, MaxNum);
//
//      len = 0;
//      for (int i = 0; i < boxListLen; ++i)
//      {
//          Box *box = boxList[i];
//          if (avail[box->type])
//          {
//              boxList[len++] = box;
//              if (box->lx < dict[box->ly][box->lz])
//                  dict[box->ly][box->lz] = box->lx;
//          }
//      }
//      boxListLen = len;
//
//      for (int i = 0; i < MaxLength; ++i)
//      {
//          for (int j = 0; j < MaxLength; ++j)
//          {
//              int &x = dict[i][j];
//              if (i > 0 && dict[i-1][j] < x)
//                  x = dict[i-1][j];
//              if (j > 0 && dict[i][j-1] < x)
//                  x = dict[i][j-1];
//          }
//      }
//
      unordered_map<const Block *, double> policy;
      for (int i = 0; i < blockTableLen; ++i)
        policy[blockTable[i]] = 1;

      PackingState next = MentoCarloSearch(level, iterations, policy, state);
      //cerr << next.plan.size() << " " << state.plan.size() << endl;
      Placement placement = next.plan[state.plan.size()];

      Space space;
      UpdateState(state, placement.block, space);
    }
    best = state;

    return best;
}
