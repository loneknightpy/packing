#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>

#include "packing.h"
#include "log.h"

using namespace std;

void PackingUtility::GenBlockList(PackingState &state,
                                  int noBranch, Block *blockList[], int &blockListLen)
{
    blockListLen = 0;
    Space &space = state.spaceStack.back();
    int lx = space.lx;
    int ly = space.ly;
    int lz = space.lz;

    if (lx < dict[ly][lz])
        return;

    const int *avail = state.avail;
    int volume = lx * ly * lz;

    bool flag = false;
    for (Box **p = boxList + boxListLen-1; p >= boxList; --p)
    {
        const Box *box = *p;

        if (box->volume > volume)
            break;

        if (avail[box->type] > 0 && box->lx <= lx && box->ly <= ly && box->z <= lz)
        {
            flag = true;
            break;
        }
    }

    if (!flag)
        return;

    int low = 0;
    int up = blockTableLen;
    while (low < up)
    {
        int mid = (low + up) >> 1;
        if (blockTable[mid]->total > volume)
            low = mid + 1;
        else
            up = mid;
    }

    blockTable[blockTableLen] = NULL;
    for (Block **p = blockTable + low; *p != NULL; ++p)
    {
        Block *block = *p;

        if (lx < block->lx || ly < block->ly || lz < block->lz)
            continue;

        const int *require = block->require;
        const int *boxIndex = block->boxIndex;
        int boxLen = block->boxLen;

        bool flag = true;
        for (int j = 0; j < boxLen; ++j)
        {
            const int &k = boxIndex[j];
            if (require[k] > avail[k])
            {
                flag = false;
                break;
            }
        }

        if (flag)
        {
            blockList[blockListLen++] = block;
            if (blockListLen == noBranch)
                break;
        }
    }
}

void PackingUtility::InitState(PackingState &state)
{
    state.volume = 0;
    state.volumeCompelete = 0;
    state.code = 0;
    copy(problem->num, problem->num + problem->n, state.avail);

    state.plan.resize(0);
    state.plan.reserve(MaxPlan);
    state.spaceStack.resize(0);
    state.spaceStack.reserve(MaxPlan);

    Space space;
    space.x = space.y = space.z = 0;
    space.lx = problem->lx;
    space.ly = problem->ly;
    space.lz = problem->lz;
    space.px = space.lx;
    space.py = space.ly;
    space.pz = space.lz;
    space.size = 0;
    space.direction = DirectionX;
    //space.isMax = true;
    state.spaceStack.push_back(space);
}

void PackingUtility::UpdateState(PackingState &state,
                                 const Block *block, Space &space)
{
    vector<Placement> &plan = state.plan;
    vector<Space> &spaceStack = state.spaceStack;

    space = spaceStack.back();
    spaceStack.pop_back();

        Placement place;
        place.block = block;
        place.space = space;
        plan.push_back(place);
    if (block != NULL)
    {
        state.volume += block->volume;

        int *avail = state.avail;
        const int *require = block->require;
        const int *boxIndex = block->boxIndex;
        int boxLen = block->boxLen;

        for (int j = 0; j < boxLen; ++j)
        {
            const int &k = boxIndex[j];
            avail[k] -= require[k];
        }

        GenResidue(state, block, space);
    }
    else if (spaceStack.size() != 0)
    {
        for (int i = 1; i <= space.size; ++i)
            spaceStack[spaceStack.size()-i].Swap(space.direction);
        

//         Space &top = spaceStack.back();

        //if (space.isMax)
        //{
        //    if (top.x + top.lx == space.x)
        //        top.lx += space.lx;
        //    else
        //        top.ly += space.ly;
        //}

//         if (top.x + top.lx == space.x
//             && top.y > space.y && top.y + top.ly == space.y + space.ly
//             && top.z == space.z && top.lz == space.lz)
//         {
//             top.lx += space.lx;
//         }
//         else if (top.x > space.x && top.x + top.lx == space.x + space.lx
//             && top.y + top.ly == space.y
//             && top.z == space.z && top.lz == space.lz)
//         {
//             top.ly += space.ly;
//         }
    }
}

void PackingUtility::RestoreState(PackingState &state, const Block *block, const Space &space)
{
    vector<Placement> &plan = state.plan;
    vector<Space> &spaceStack = state.spaceStack;

    if (block != NULL)
    {
        plan.pop_back();
        state.volume -= block->volume;

        int *avail = state.avail;
        const int *require = block->require;
        const int *boxArray = block->boxIndex;
        int boxLen = block->boxLen;

        for (int j = 0; j < boxLen; ++j)
        {
            const int &k = boxArray[j];
            avail[k] += require[k];
        }

        for (int i = 0; i < 3; ++i)
            spaceStack.pop_back();
    }
    else if (spaceStack.size() != 0)
    {
        for (int i = 1; i <= space.size; ++i)
            spaceStack[spaceStack.size()-i].Swap(space.direction);
//         Space &top = spaceStack.back();
//
//         //if (space.isMax)
//         if (top.x + top.lx == space.x + space.lx
//             && top.y + top.ly == space.y + space.ly
//             && top.z == space.z && top.lz == space.lz)
//         {
//             if (top.x > space.x)
//                 top.ly -= space.ly;
//             else if (top.y > space.y)
//                 top.lx -= space.lx;
//         }
    }

    state.spaceStack.push_back(space);
}

void PackingUtility::GenResidue(PackingState &state, const Block *block, const Space &space)
{
    vector<Space> &spaceStack = state.spaceStack;

    // Initialize x1, y1, z1 to the start position of block and space.
    int	x1 = space.x;
    int	y1 = space.y;
    int	z1 = space.z;

    // Initialize x2, y2, z2 to the end position of block.
    int	x2 = space.x + block->lx;
    int	y2 = space.y + block->ly;
    int	z2 = space.z + block->lz;

    // Initialize x3, y3, z3 to the end position of space.
    int	x3 = space.x + space.lx;
    int	y3 = space.y + space.ly;
    int z3 = space.z + space.lz;

    int ax = block->ax;
    int ay = block->ay;

	int mx = x3 - x2;
	int my = y3 - y2;
	int mz = z3 - z2;

#if !STABLE
	if (mz <= mx && mz <= my)
	{
#endif
		if (mx <= my)
		{
			Space space;
			space.x = x1;
			space.y = y1;
			space.z = z2;
			space.lx = ax;
			space.ly = ay;
			space.lz = z3 - z2;

            space.direction = DirectionZ;
            space.size = 0;
            space.px = x3 - x1;
            space.py = y3 - y1;

			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x2;
			space.y = y1;
			space.z = z1;
			space.lx = x3 - x2;
			space.ly = y2 - y1;
			space.lz = z3 - z1;

            space.direction = DirectionX;
#if STABLE
            space.size = 0;
#else
            space.size = 1;
#endif
            space.py = y3 - y1;
			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x1;
			space.y = y2;
			space.z = z1;
			space.lx = x3 - x1;
			space.ly = y3 - y2;
			space.lz = z3 - z1;

            space.direction = DirectionY;
#if STABLE
            space.size = 1;
#else
            space.size = 2;
#endif

			//space.isMax = true;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);
		}
		else
		{
			Space space;
			space.x = x1;
			space.y = y1;
			space.z = z2;
			space.lx = ax;
			space.ly = ay;
			space.lz = z3 - z2;

            space.direction = DirectionZ;
            space.size = 0;
            space.px = x3 - x1;
            space.py = y3 - y1;

			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x1;
			space.y = y2;
			space.z = z1;
			space.lx = x2 - x1;
			space.ly = y3 - y2;
			space.lz = z3 - z1;

            space.direction = DirectionY;
#if STABLE
            space.size = 0;
#else
            space.size = 1;
#endif
            space.px = x3 - x1;
			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x2;
			space.y = y1;
			space.z = z1;
			space.lx = x3 - x2;
			space.ly = y3 - y1;
			space.lz = z3 - z1;

            space.direction = DirectionX;
#if STABLE
            space.size = 1;
#else
            space.size = 2;
#endif
			//space.isMax = true;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);
		}
#if !STABLE
	}
	else if (mx <= my && mx <= mz)
	{
		if (my <= mz)
		{
			Space space;
			space.x = x2;
			space.y = y1;
			space.z = z1;
			space.lx = x3 - x2;
			space.ly = y2 - y1;
			space.lz = z2 - z1;

            space.direction = DirectionX;
            space.size = 0;
            space.py = y3 - y1;
            space.pz = z3 - z1;

			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x1;
			space.y = y2;
			space.z = z1;
			space.lx = x3 - x1;
			space.ly = y3 - y2;
			space.lz = z2 - z1;

            space.direction = DirectionY;
            space.size = 1;
            space.pz = z3 - z1;
			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x1;
			space.y = y1;
			space.z = z2;
			space.lx = x3 - x1;
			space.ly = y3 - y1;
			space.lz = z3 - z2;

            space.direction = DirectionZ;
            space.size = 2;
			//space.isMax = true;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);
		}
		else
		{
			Space space;
			space.x = x2;
			space.y = y1;
			space.z = z1;
			space.lx = x3 - x2;
			space.ly = y2 - y1;
			space.lz = z2 - z1;

            space.direction = DirectionX;
            space.size = 0;
            space.py = y3 - y1;
            space.pz = z3 - z1;

			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x1;
			space.y = y1;
			space.z = z2;
			space.lx = x3 - x1;
			space.ly = y2 - y1;
			space.lz = z3 - z2;

            space.direction = DirectionZ;
            space.size = 1;
            space.py = y3 - y1;
			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x1;
			space.y = y2;
			space.z = z1;
			space.lx = x3 - x1;
			space.ly = y3 - y2;
			space.lz = z3 - z1;

            space.direction = DirectionY;
            space.size = 2;

			//space.isMax = true;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);
		}
	}
	else
	{
		if (mx <= mz)
		{
			Space space;
			space.x = x1;
			space.y = y2;
			space.z = z1;
			space.lx = x2 - x1;
			space.ly = y3 - y2;
			space.lz = z2 - z1;

            space.direction = DirectionY;
            space.size = 0;
            space.px = x3 - x1;
            space.pz = z3 - z1;

			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x2;
			space.y = y1;
			space.z = z1;
			space.lx = x3 - x2;
			space.ly = y3 - y1;
			space.lz = z2 - z1;

            space.direction = DirectionX;
            space.size = 1;
            space.pz = z3 - z1;

			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x1;
			space.y = y1;
			space.z = z2;
			space.lx = x3 - x1;
			space.ly = y3 - y1;
			space.lz = z3 - z2;

            space.direction = DirectionZ;
            space.size = 2;
			//space.isMax = true;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);
		}
		else
		{
			Space space;
			space.x = x1;
			space.y = y2;
			space.z = z1;
			space.lx = x2 - x1;
			space.ly = y3 - y2;
			space.lz = z2 - z1;

            space.direction = DirectionY;
            space.size = 0;
            space.px = x3 - x1;
            space.pz = z3 - z1;

			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x1;
			space.y = y1;
			space.z = z2;
			space.lx = x2 - x1;
			space.ly = y3 - y1;
			space.lz = z3 - z2;

            space.direction = DirectionZ;
            space.size = 1;
            space.px = x3 - x1;
			//space.isMax = false;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);

			space.x = x2;
			space.y = y1;
			space.z = z1;
			space.lx = x3 - x2;
			space.ly = y3 - y1;
			space.lz = z3 - z1;

            space.direction = DirectionX;
            space.size = 2;
			//space.isMax = true;
			//space.volume = space.lx * space.ly * space.lz;
			spaceStack.push_back(space);
		}
	}
#endif
}

void PackingUtility::ExtendSolution(PackingState &state, int maxAdd, int add, int noBranch)
{
    if (state.spaceStack.empty() || maxAdd == add)
    {
        tempComplete = state;
        CompleteSolution(tempComplete);
        state.volumeCompelete = tempComplete.volume;

        //if (tempComplete.volume > tempBest.volume)
        //    tempBest = tempComplete;

        if (tempComplete.volume > best.volume)
            best = tempComplete;

        PushPackingState(maxAdd, state);
        return;
    }

    Block *blockList[MaxBlockList];
    int blockListLen = 0;
    GenBlockList(state, noBranch, blockList, blockListLen);

    if (blockListLen == 0)
    {
        Space space;
        UpdateState(state, NULL, space);
        ExtendSolution(state, maxAdd, add, noBranch);
        RestoreState(state, NULL, space);
    }
    else
    {
        for (int i = 0; i < noBranch && i < blockListLen; ++i)
        {
            Space space;
            UpdateState(state, blockList[i], space);
            state.code = (state.code << 8) | i;
            ExtendSolution(state, maxAdd, add + 1, noBranch);
            state.code >>= 8;
            RestoreState(state, blockList[i], space);
        }
    }
}

void PackingUtility::CompleteSolution(PackingState &state)
{
    vector<Space> &spaceStack = state.spaceStack;
    Space space;
    Block *blockList[MaxBlockList];
    int blockListLen = 0;
    while (spaceStack.size() != 0)
    {
        GenBlockList(state, 1, blockList, blockListLen);
        if (blockListLen != 0)
            UpdateState(state, blockList[0], space);
        else
            UpdateState(state, NULL, space);
    }
}

void PackingUtility::SplitPartialState(PackingState &state, int index, PartialPackingState &partial)
{
    //vector<Placement> &plan = state.plan;
    //int size = plan.size();
    //Block **blocks = partial.blocks;
    //int &len = partial.len;

    //len = 0;
    //for (int i = index; i < size; ++i)
    //{
    //    blocks[i] = const_cast<Block *>(plan[i].block);
    //    if (blocks[i] != NULL)
    //        ++len;
    //}
}

void PackingUtility::MergePartialState(PackingState &state, PartialPackingState &partial)
{
    //Block **blocks = partial.blocks;
    //int &len = partial.len;
    //for (int i = 0; i < len; ++i)
    //{
    //    Block *block = blocks[i];
    //    Space space = state.spaceStack.back();
    //
    //    while (block->lx > space.lx || block->ly > space.ly || block->lz > space.lz)
    //        UpdateState(state, NULL, space);
    //    UpdateState(state, block, space);
    //}
}

void PackingUtility::GenSolution(PackingState &state, PackingSolution &solution)
{
    solution.len = 0;
    for (int i = 0; i < state.plan.size(); ++i)
    {
        Space &space = state.plan[i].space;
        const Block *block = state.plan[i].block;

        if (block != NULL)
            ApplyBlock(space.x, space.y, space.z, block, &solution);
    }

    int sum = 0;
    for (int i = 0; i < solution.len; ++i)
        sum += solution.positions[i].volume;

    if (sum != state.volume)
    {
        Log(LogError, "volume error\n");
        exit(1);
    }

    if (!Check(solution.positions, solution.len))
    {
        Log(LogError, "position error\n");
        exit(1);
    }
}

void PackingUtility::ApplyBlock(int _x, int _y, int _z, const Block *block, PackingSolution *solution)
{
    if (solution != NULL)
    {
        if (block->type == SimpleBlock)
        {
            for (int x = 0; x < block->nx; ++x)
            {
                for (int y = 0; y < block->ny; ++y)
                {
                    for (int z = 0; z < block->nz; ++z)
                    {
                        Box &box = solution->positions[solution->len++];
                        box = *block->box;

                        box.x = _x + box.lx * x;
                        box.y = _y + box.ly * y;
                        box.z = _z + box.lz * z;
                    }
                }
            }
        }
        else if (block->type == CombineBlockX)
        {
            ApplyBlock(_x, _y, _z, block->childs[0], solution);
            ApplyBlock(_x + block->childs[0]->lx, _y, _z, block->childs[1], solution);
        }
        else if (block->type == CombineBlockY)
        {
            ApplyBlock(_x, _y, _z, block->childs[0], solution);
            ApplyBlock(_x, _y + block->childs[0]->ly, _z, block->childs[1], solution);
        }
        else if (block->type == CombineBlockZ)
        {
            ApplyBlock(_x, _y, _z, block->childs[0], solution);
            ApplyBlock(_x, _y, _z + block->childs[0]->lz, block->childs[1], solution);
        }
    }
}

bool PackingUtility::Check(const Box &b1, const Box &b2)
{
    return (b1.x + b1.lx <= b2.x) || (b2.x + b2.lx <= b1.x)
           || (b1.y + b1.ly <= b2.y) || (b2.y + b2.ly <= b1.y)
           || (b1.z + b1.lz <= b2.z) || (b2.z + b2.lz <= b1.z);
}

bool PackingUtility::Check(const Box *p, int len)
{
    int avail[MaxBox];
    copy(problem->num, problem->num + problem->n, avail);

    for (int i = 0; i < len; ++i)
    {
        for (int j = i+1; j < len; ++j)
        {
            if (!Check(p[i], p[j]))
            {
                Log(LogError, "box overlap\n");
				return false;
            }
        }

        avail[p[i].type]--;
        if (avail[p[i].type] < 0)
        {
            Log(LogError, "box overuse\n");
			return false;
        }

        const Box &box = p[i];
        if (!(box.x >= 0 && box.y >= 0 && box.z >= 0
            && box.x + box.lx <= problem->lx
            && box.y + box.ly <= problem->ly
            && box.z + box.lz <= problem->lz))
        {
            Log(LogError, "box out of range\n");
			return false;
        }
    }

    return true;
}

bool PackingUtility::Check(const Space &s1, const Space &s2)
{
    return (s1.x + s1.lx <= s2.x) || (s2.x + s2.lx <= s1.x)
           || (s1.y + s1.ly <= s2.y) || (s2.y + s2.ly <= s1.y)
           || (s1.z + s1.lz <= s2.z) || (s2.z + s2.lz <= s1.z);
}

bool PackingUtility::Check(Space *spaceList[BufferSize], int len)
{
    for (int i = 0; i < len; ++i)
    {
        for (int j = i+1; j < len; ++j)
        {
            if (!Check(*spaceList[i], *spaceList[j]))
                return false;
        }
    }

    return true;
}
