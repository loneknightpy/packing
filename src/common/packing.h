#ifndef __PACKING_H_

#define __PACKING_H_

#include <set>
#include <unordered_map>
#include <vector>

#include "problem.h"

const int MaxThread = 10;

const int MaxLength = 256;
const int MaxPackingStatePoolSize = 20000;
const int MaxNum = 1000000000;
const int MaxDepth = 10;
const int MaxBlockList = 256;
const int MaxPlan = 200;

const int TimeLimitStage0 = 500;
const int TimeLimitStage1 = 500;

const int MaxPhaseDepth = 2;
const int MaxCandidate = 256;
const int MinCandidate = 16;

const int MinBranch = 3;
const int EffortFact = 3;
const int MaxEffort = 243;
const int HeapSize = 6;

extern int reachEffort;

struct Placement
{
    const Block *block;
    Space space;
};

struct PackingState 
{
    std::vector<Placement> plan;
    std::vector<Space> spaceStack;
    int avail[MaxBox];
    int volume;
    int volumeCompelete;
    unsigned long long code;

    static bool Compare(const PackingState *s1, const PackingState *s2)
    {
        if (s1->volumeCompelete != s2->volumeCompelete)
            return s1->volumeCompelete > s2->volumeCompelete;
        else
            s1->code > s2->code;
    }
};

struct PartialPackingState
{
    Block *blocks[MaxDepth];
    int len;
    int volumeCompelete;
    unsigned long long code;
};

struct PackingSequence
{
    int index[MaxPlan];
    int len;
    int volume;
};

struct PackingSolution
{
    int len;
    Box positions[BufferSize];
};

struct PhaseUnit
{
    std::vector<std::vector<PackingState *> > heaps;
    const Block *block;
    int effort;
    int bestVolume;
    int depth;

    static bool Compare(const PhaseUnit *p1, const PhaseUnit *p2)
    {
        return p1->bestVolume > p2->bestVolume;
    }
};

struct PackingUtility 
{
    const PackingProblem *problem;

    int totalAdd;
    int searchEffort;
    int branches[MaxPhaseDepth+1][MaxEffort+1];
    int dict[MaxLength][MaxLength];

    Block *blockTable[2*MaxBlockTable];
    int blockTableLen;

    Box *boxList[BufferSize];
    int boxListLen;

    PackingState packingStatePool[MaxPackingStatePoolSize];
    PackingState *packingStateStack[MaxPackingStatePoolSize];
    int packingStateStackLen;

    std::vector<std::vector<PackingState *> > phaseHeaps;

    PhaseUnit phasesAux[BufferSize];
    PhaseUnit *phases[BufferSize];
    int phasesLen;

    int index;
    PackingState current;
    PackingState best;
    //PackingState tempBest;
    PackingState tempComplete;

    PackingSolution solution;
    double costTime;

    PackingUtility(const PackingProblem *problem = NULL) 
    { 
        SetPackingProblem(problem); 
        phaseHeaps.resize(MaxDepth);

        packingStateStackLen = 0;
        for (int i = 0; i < MaxPackingStatePoolSize; ++i)
            packingStateStack[packingStateStackLen++] = &packingStatePool[i];

        for (int i = 0; i <= MaxPhaseDepth; ++i)
        {
            int branch = 0;
            for (int j = 0; j <= MaxEffort; ++j)
            {
                int product = 1;
                for (int k = 0; k < i; ++k)
                    product *= branch+1;
                if (product <= j)
                    ++branch;
                branches[i][j] = branch;
                if (branches[i][j] < MinBranch)
                    branches[i][j] = MinBranch;
            }
        }
    }

    void SetPackingProblem(const PackingProblem *problem) { this->problem = problem; }
    const PackingProblem *GetPackingProblem() const { return this->problem; }

    void GenBlockList(PackingState &state, int noBranch, 
        Block *block[], int &blockListLen);

    void InitState(PackingState &state);
    void UpdateState(PackingState &state, const Block *block, Space &space);
    void RestoreState(PackingState &state, const Block *block, const Space &space);
    void GenResidue(PackingState &state, const Block *block, const Space &space);

    void ExtendSolution(PackingState &state, int maxAdd, int add, int noBranch);
    void CompleteSolution(PackingState &state);

    void SplitPartialState(PackingState &state, int index, PartialPackingState &partial);
    void MergePartialState(PackingState &state, PartialPackingState &partial);

    void GenSolution(PackingState &state, PackingSolution &solution);
    void ApplyBlock(int x, int y, int z, const Block *block, 
        PackingSolution *solution);

    PackingState *NewPackingState();
    void FreePackingState(PackingState *state);
    void PushPackingState(int depth, PackingState &state);

    void InitPhaseUnit(PhaseUnit &phase, PackingState &state, Block *block, int effort);
    void PhaseSearch(PhaseUnit &phase);
    int MultiPhaseSearch(PackingState &state, Block *block, int adds, int effort);

    const Block *FindNextBlock(PackingState &state);

    void UpdateBlockTable();
    void UpdateBoxList();

    PackingState TreeSearch(int stage);

    void Heuristic(PackingSequence &ps, PackingState &state);
    PackingState SolveSA(double ts, double tf, double dt, int length, bool isLinear, int stage);

    void Rollout(std::unordered_map<const Block *, double> &policy, PackingState &state);
    void Adapt(std::unordered_map<const Block *, double> &policy, PackingState &state);
    void MentoCarloSearch(int level, int iterations, std::unordered_map<const Block *, double> &policy, PackingState &state);
    PackingState MentoCarlo(int level, int iterations, int stage);

    // Check if two boxes are consistent.
    bool Check(const Box &b1, const Box &b2);
    // Check if a solution is consistent.
    bool Check(const Box *p, int n);
    // Check if a space is available for the specific box.
    bool Check(const Box &b, const Space &s);
    // Check if a space is available for some box.
	bool Check(const Space &s) {return true;}
    // Check if two spaceListAux are consistent.
    bool Check(const Space &s1, const Space &s2);
    // Check if a few spaceListAux are consistent.
    bool Check(Space *spaceList[BufferSize], int len);
};

#endif
