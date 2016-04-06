#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <unordered_map>

#include "common/packing.h"
#include "common/log.h"

using namespace std;


void PackingUtility::Adapt(vector<unordered_map<const Block *, double>> &policy, PackingState &state, PackingState &best) {
  for (int i = state.plan.size(); i < best.plan.size(); ++i) {
    if (policy[i][best.plan[i].block] < 100)
      policy[i][best.plan[i].block] *= 1.05;
  }
}

void PackingUtility::Rollout(vector<unordered_map<const Block *, double>> &policy, PackingState &state)
{
    int numBlocks = 256;
    int k = 0;
    while (!state.spaceStack.empty())
    {
        Block *blockList[MaxBlockList];
        int blockListLen = 0;

        GenBlockList(state, numBlocks, blockList, blockListLen);

        double total = 0;
        vector<double> weight(blockListLen);
        unordered_map<const Block *, double> &current_policy = policy[state.plan.size()];
        for (int i = 0; i < blockListLen; ++i) {
          //PackingState tempState = state;
          //Space space;
          //UpdateState(tempState, blockList[i], space);
          //CompleteSolution(tempState);
          if (current_policy.find(blockList[i]) == current_policy.end()) {
            current_policy[blockList[i]] = 1;
          }
          weight[i] = current_policy[blockList[i]] * blockList[i]->volume * blockList[i]->volume;
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
        numBlocks = max(numBlocks/2, 2);
    }
}

PackingState PackingUtility::MentoCarloSearch(
    int level, int iterations, vector<unordered_map<const Block *, double>> &policy, PackingState &state)
{
  PackingState best = state;
  Rollout(policy, best);
  
  int count = 0;
  if (level > 0) {
    for (int i = 0; i < iterations; ++i) {
      //unordered_map<const Block *, double> newPolicy = policy;
      PackingState newState = MentoCarloSearch(level - 1, iterations, policy, state);
      if (newState.volume > best.volume) {
        iterations = max(iterations, i + 1000);
        ++count;
        best = newState;
        Adapt(policy, state, best);
      }
    }

    //if (count > 0)
      //cerr << "update " << state.plan.size() << " " << count << endl;
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

    //unordered_map<const Block *, double> policy;
    //for (int i = 0; i < blockTableLen; ++i)
    //  policy[blockTable[i]] = 1;

    //PackingState state;
    //InitState(state);
    //best = MentoCarloSearch(level, iterations, policy, state);
    int numAttempt = 1;
    int numRun = 25;
    InitState(best);
    for (int attempt = 0; attempt < numAttempt; ++attempt) { 
      PackingState state;
      InitState(state);

       vector<unordered_map<const Block *, double>> policy(blockTableLen);
        for (int i = 0; i < blockTableLen; ++i) {
          for (int j = 0; j < blockTableLen; ++j) {
              policy[i][blockTable[j]] = 1;
          }      
        }

      while (!state.spaceStack.empty()) {
        Block *blockList[MaxBlockList];
        int blockListLen = 0;

        GenBlockList(state, 64, blockList, blockListLen);

        if (blockListLen > 0) {
          PackingState bestNext;
          InitState(bestNext);
          for (int run = 0; run < numRun; ++run) {
            for (int i = 0; i < blockTableLen; ++i) {
              policy[i].clear();
            }
            PackingState next = MentoCarloSearch(level, iterations, policy, state);
            if (next.volume > bestNext.volume) {
              bestNext = next;
            }
          }
          //cerr << next.plan.size() << " " << state.plan.size() << endl;
          Placement placement = bestNext.plan[state.plan.size()];

          Space space;
          UpdateState(state, placement.block, space);
        }
        else {
          Space space;
          UpdateState(state, NULL, space);
        }
      }

      if (state.volume > best.volume) 
        best = state;
    }

    GenSolution(best, solution);

    return best;
}
