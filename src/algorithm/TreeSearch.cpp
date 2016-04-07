#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>

#include "common/packing.h"
#include "common/log.h"

using namespace std;

int reachEffort;

PackingState *PackingUtility::NewPackingState() {
    if (packingStateStackLen == 0) {
        Log(LogError, "packing state stack overflow\n");
        exit(1);
    }

    return packingStateStack[--packingStateStackLen];
}

void PackingUtility::FreePackingState(PackingState *state) {
    packingStateStack[packingStateStackLen++] = state;
}

void PackingUtility::PushPackingState(int depth, PackingState &state) {
    vector<PackingState *> &heap = phaseHeaps[depth];
    if (heap.size() < HeapSize
        || state.volumeCompelete > heap[0]->volumeCompelete) {
        for (int i = 0; i < heap.size(); ++i) {
            if (state.code == heap[i]->code)
                return;
        }

        PackingState *temp = NewPackingState();
        *temp = state;

        if (heap.size() < HeapSize) {
            heap.push_back(NULL);
        }
        else {
            pop_heap(heap.begin(), heap.end(), PackingState::Compare);
            FreePackingState(heap.back());
            heap.back() = NULL;
        }

        heap.back() = temp;
        push_heap(heap.begin(), heap.end(), PackingState::Compare);
    }
}

void PackingUtility::InitPhaseUnit(PhaseUnit &phase, PackingState &state, Block *block, int effort) {
    phase.block = block;
    phase.depth = 0;
    phase.effort = effort;
    phase.bestVolume = 0;

    Space space;
    UpdateState(state, block, space);
    state.code = 0;
    phase.heaps.swap(phaseHeaps);

    for (int i = 0; i < phaseHeaps.size(); ++i) {
        for (int j = 0; j < phaseHeaps[i].size(); ++j) {
            FreePackingState(phaseHeaps[i][j]);
        }
        phaseHeaps[i].resize(0);
    }
    phaseHeaps.resize(MaxDepth);
    ExtendSolution(state, 0, 0, 0);

    if (phaseHeaps[0][0]->volumeCompelete > phase.bestVolume)
        phase.bestVolume = phaseHeaps[0][0]->volumeCompelete;

    phase.heaps.swap(phaseHeaps);
    RestoreState(state, block, space);
}

void PackingUtility::PhaseSearch(PhaseUnit &phase) {
    phase.heaps.swap(phaseHeaps);

    int depth = phase.depth;
    int effort = phase.effort;
    for (int i = 0; i < phaseHeaps[depth].size(); ++i) {
        PackingState &state = *phaseHeaps[depth][i];
        for (int d = 1; d <= MaxPhaseDepth; ++d) {
            ExtendSolution(state, depth + d, depth, branches[d][effort]);
        }
    }

    for (int i = depth + 1; i <= depth + MaxPhaseDepth; ++i) {
        for (int j = 0; j < phaseHeaps[i].size(); ++j) {
            if (phaseHeaps[i][j]->volumeCompelete > phase.bestVolume)
                phase.bestVolume = phaseHeaps[i][j]->volumeCompelete;
        }
    }

    phase.depth++;
    phase.heaps.swap(phaseHeaps);
}

const Block *PackingUtility::FindNextBlock(PackingState &state) {
    Block *blockList[MaxBlockList];
    int blockListLen = 0;
    GenBlockList(state, MaxCandidate, blockList, blockListLen);

    if (blockListLen == 0)
        return NULL;
    else {
//         return blockList[blockListLen-1]; // test

        state.volumeCompelete = state.volume;
//         return blockList[0];

        phasesLen = blockListLen;
        for (int i = 0; i < phasesLen; ++i) {
            phases[i] = &phasesAux[i];
            InitPhaseUnit(*phases[i], state, blockList[i], searchEffort);
        }
        sort(phases, phases + phasesLen, PhaseUnit::Compare);
        //return phases[0]->block;

        int size = MaxCandidate;
        for (int i = 1; i <= totalAdd; ++i) {
            size >>= 1;
            if (size < MinCandidate)
                size = MinCandidate;

            if (phasesLen >= size)
                phasesLen = size;

            for (int j = 0; j < phasesLen; ++j)
                PhaseSearch(*phases[j]);

            sort(phases, phases + phasesLen, PhaseUnit::Compare);
        }

        return phases[0]->block;
    }
}

PackingState PackingUtility::TreeSearch(int stage) {
    best.volume = 0;
    {
        totalAdd = 2;
        searchEffort = 1;

        int timeLimit;
        if (stage == 0)
            timeLimit = TimeLimitStage0;
        else
            timeLimit = TimeLimitStage1;

        double start = clock();
        while (true) {
            totalAdd += 1;
            searchEffort *= EffortFact;

            if (searchEffort > reachEffort)
                reachEffort = searchEffort;

            copy(problem->blockTable.begin(), problem->blockTable.end(), blockTable);
            blockTableLen = problem->blockTable.size();

            copy(problem->boxList, problem->boxList + problem->boxListLen, boxList);
            boxListLen = problem->boxListLen;
            index = 0;
            InitState(current);

            while (!current.spaceStack.empty()) {
                const int *avail = current.avail;
                int len = 0;
                for (int i = 0; i < blockTableLen; ++i) {
                    Block *block = blockTable[i];
                    if ((stage == 1 || block->type == SimpleBlock)) {
                        const int *require = block->require;
                        const int *boxIndex = block->boxIndex;
                        int boxLen = block->boxLen;

                        bool flag = true;
                        for (int j = 0; j < boxLen; ++j) {
                            const int &k = boxIndex[j];
                            if (require[k] > avail[k]) {
                                flag = false;
                                break;
                            }
                        }

                        if (flag)
                            blockTable[len++] = block;
                    }
                }
                blockTableLen = len;

                fill_n(&dict[0][0], MaxLength * MaxLength, MaxNum);

                len = 0;
                for (int i = 0; i < boxListLen; ++i) {
                    Box *box = boxList[i];
                    if (avail[box->type]) {
                        boxList[len++] = box;
                        if (box->lx < dict[box->ly][box->lz])
                            dict[box->ly][box->lz] = box->lx;
                    }
                }
                boxListLen = len;

                for (int i = 0; i < MaxLength; ++i) {
                    for (int j = 0; j < MaxLength; ++j) {
                        int &x = dict[i][j];
                        if (i > 0 && dict[i - 1][j] < x)
                            x = dict[i - 1][j];
                        if (j > 0 && dict[i][j - 1] < x)
                            x = dict[i][j - 1];
                    }
                }

                const Block *block = FindNextBlock(current);
                Space space;
                UpdateState(current, block, space);
                ++index;
            }

            if (current.volume > best.volume)
                best = current;

            double end = clock();
            costTime = (end - start);
            if (costTime < 0)
                costTime += (unsigned(~0));
            costTime /= CLOCKS_PER_SEC;
            Log(LogInfo, "stage%d effort %d rate: %.4f%% time: %.2fs\n", stage,
                searchEffort, best.volume * 100.0 / problem->volume, costTime);

            if (searchEffort >= MaxEffort
                || (costTime > timeLimit && searchEffort >= reachEffort))
                break;
        }
    }

    GenSolution(best, solution);

    return best;
}
