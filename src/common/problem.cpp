#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <ctime>

#include "problem.h"
#include "log.h"

using namespace std;

bool Block::Compare(const Block *b1, const Block *b2)
{
    if (b1->total != b2->total)
        return b1->total > b2->total;
    else if (b1->volume != b2->volume)
        return b1->volume > b2->volume;
    else if (b1->ax != b2->ax)
        return b1->ax > b2->ax;
    else if (b1->ay != b2->ay)
        return b1->ay > b2->ay;
    else if (b1->lx != b2->lx)
        return b1->lx > b2->lx;
    else if (b1->ly != b2->ly)
        return b1->ly > b2->ly;
    else if (b1->lz != b2->lz)
        return b1->lz > b2->lz;
    else
    {
        for (int i = 0; i < MaxBox; ++i)
        {
            if (b1->require[i] != b2->require[i])
                return b1->require[i] > b2->require[i];
        }
    }

    return false;
}

bool Block::Equal(const Block *b1, const Block *b2)
{
    return !Compare(b1, b2) && !Compare(b2, b1);
}

bool Block::IsContain(const Block *block, const Box *box)
{
    if (block->type == SimpleBlock)
        return block->box == box;
    else
        return IsContain(block->childs[0], box) || IsContain(block->childs[1], box);
}

bool PackingProblem::Input(FILE *fin)
{
    int	a, b;
    scanf("%d %d", &a, &b);

    scanf("%d %d %d %d", &lx, &ly, &lz, &n);
    volume = lx * ly * lz;

    boxListLen = 0;
    int	type, lx, fx, ly, fy, lz, fz;
    for (int i = 0; i < n; ++i)
    {
        scanf("%d %d %d %d %d %d %d %d", &type, &lx, &fx, 
            &ly, &fy, &lz, &fz, &num[i]);
        --type;

        // Generate box structure using lx as height.
        if (fx)
        {
            Box &box = boxListAux[boxListLen++];
            box.lz = lx;
            box.lx = ly;
            box.ly = lz;
            box.type = type;
            box.volume = lx * ly * lz;
            //m++;

            if (ly != lz)
            {
                Box &box = boxListAux[boxListLen++];
                box.lz = lx;
                box.lx = lz;
                box.ly = ly;
                box.type = type;
                box.volume = lx * ly * lz;
                //m++;
            }
        }

        // Generate box structure using ly as height.
        if (fy && !(fx && lx == ly))
        {
            Box &box = boxListAux[boxListLen++];
            box.lz = ly;
            box.lx = lx;
            box.ly = lz;
            box.type = type;
            box.volume = lx * ly * lz;
            //m++;

            if (lz != lx)
            {
                Box &box = boxListAux[boxListLen++];
                box.lz = ly;
                box.lx = lz;
                box.ly = lx;
                box.type = type;
                box.volume = lx * ly * lz;
                //m++;
            }
        }

        // Generate box structure using lz as height.
        if (fz && !(fx && lx == lz) && !(fy && ly == lz))
        {
            Box &box = boxListAux[boxListLen++];
            box.lz = lz;
            box.lx = lx;
            box.ly = ly;
            box.type = type;
            box.volume = lx * ly * lz;
            //m++;

            if (lx != ly)
            {
                Box &box = boxListAux[boxListLen++];
                box.lz = lz;
                box.lx = ly;
                box.ly = lx;
                box.type = type;
                box.volume = lx * ly * lz;
                //m++;
            }
        }
    }

    for (int i = 0; i < boxListLen; ++i)
        boxList[i] = &boxListAux[i];

    GenBlocks();

    sort(boxList, boxList + boxListLen, Box::Compare);

    return true;
}

void PackingProblem::GenBlocks()
{
    ClearBlocks();

    for (int i = 0; i < boxListLen; ++i)
    {
        const Box *box = boxList[i];
        int count = num[box->type];
        for (int nx = 1; box->lx * nx <= lx && nx <= count; ++nx)
        {
            for (int ny = 1; box->ly * ny <= ly && nx * ny <= count; ++ny)
            {
                for (int nz = 1; box->lz * nz <= lz && nx * ny * nz <= count; ++nz)
                {
                    blockTableAux.resize(blockTableAux.size()+1);
                    Block *block = &blockTableAux.back();
                    fill_n(block->require, MaxBox, 0);
                    blockTable.push_back(block);

                    block->level = 0;
                    block->type = SimpleBlock;
                    block->box = box;
                    block->nx = nx;
                    block->ny = ny;
                    block->nz = nz;
                    block->use = nx * ny * nz;

                    block->lx = nx * box->lx;
                    block->ly = ny * box->ly;
                    block->lz = nz * box->lz;
                    block->ax = block->lx;
                    block->ay = block->ly;
                    block->volume = block->use * box->volume;
                    block->total = block->volume;

                    block->require[box->type] = block->use;
                }
            }
        }
    }

    sort(blockTable.begin(), blockTable.end(), Block::Compare);
    blockTable.erase(unique(blockTable.begin(), blockTable.end(), 
                    Block::Equal), blockTable.end());
    //Log(LogInfo, "initial blocks: %d\n", blockTable.size());

    simpleBlockLen = blockTable.size();

    for (int round = 0; round < MaxRound; ++round) 
    {
        int size = blockTable.size();
        for (int i = 0; i < size; ++i)
        {
            for (int j = i+1; j < size; ++j)
            {
                const Block *block1 = blockTable[i];
                const Block *block2 = blockTable[j];

                if (Block::Equal(block1, block2))
                    continue;

                if (block1->type == SimpleBlock && Block::IsContain(block2, block1->box)
                    || block2->type == SimpleBlock && Block::IsContain(block1, block2->box))
                    continue;

                if (max(block1->level, block2->level) < round)
                    continue;

                bool flag = true;
                for (int k = 0; k < n; ++k)
                {
                    if (block1->require[k] + block2->require[k] > num[k])
                    {
                        flag = false;
                        break;
                    }
                }

                if (!flag)
                    continue;

                {
                    if (block1->ly * block1->lz < block2->ly * block2->lz)
                    {
                        swap(block1, block2);
                    }
                    int lx = block1->lx + block2->lx;
                    int ly = max(block1->ly, block2->ly);
                    int lz = max(block1->lz, block2->lz);
                    int ax = block1->ax + block2->ax;
                    int ay = min(block1->ay, block2->ay);

                    if (block1->volume + block2->volume > MinFillRate * lx * ly * lz
#if STABLE
                        && ax * ay > MinAreaRate * lx * ly
                        && block1->ax == block1->lx && block2->ax == block2->lx 
                        && block1->lz == block2->lz
#endif
                        && lx <= this->lx && ly <= this->ly && lz <= this->lz)
                    {
                        blockTableAux.resize(blockTableAux.size()+1);
                        Block *block = &blockTableAux.back();
                        fill_n(block->require, MaxBox, 0);
                        blockTable.push_back(block);

                        block->level = max(block1->level, block2->level) + 1;
                        block->type = CombineBlockX;
                        block->childs[0] = block1;
                        block->childs[1] = block2;
                        block->lx = lx;
                        block->ly = ly;
                        block->lz = lz;

#if STABLE
                        block->ax = ax;
                        block->ay = ay;
#else
						block->lx = lx;
						block->ly = ly;
#endif

                        block->volume = block1->volume + block2->volume;
                        block->total = lx * ly * lz;
                        for (int k = 0; k < n; ++k)
                        {
                            block->require[k] = block1->require[k] + block2->require[k];
                            if (block->require[k] > num[k])
                            {
                                Log(LogError, "GenBlocks Error\n");
                                exit(1);
                            }
                        }
                    }
                }

                {
                    if (block1->lx * block1->lz < block2->lx * block2->lz)
                    {
                        swap(block1, block2);
                    }

                    int lx = max(block1->lx, block2->lx);
                    int ly = block1->ly + block2->ly;
                    int lz = max(block1->lz, block2->lz);
                    int ax = min(block1->ax, block2->ax);
                    int ay = block1->ay + block2->ay;

                    if (block1->volume + block2->volume > MinFillRate * lx * ly * lz
#if STABLE
                        && ax * ay > MinAreaRate * lx * ly
                        && block1->ay == block1->ly && block2->ay == block2->ly 
                        && block1->lz == block2->lz
#endif
                        && lx <= this->lx && ly <= this->ly && lz <= this->lz)
                    {
                        blockTableAux.resize(blockTableAux.size()+1);
                        Block *block = &blockTableAux.back();
                        fill_n(block->require, MaxBox, 0);
                        blockTable.push_back(block);

                        block->level = max(block1->level, block2->level) + 1;
                        block->type = CombineBlockY;
                        block->childs[0] = block1;
                        block->childs[1] = block2;
                        block->lx = lx;
                        block->ly = ly;
                        block->lz = lz;

#if STABLE
                        block->ax = ax;
                        block->ay = ay;
#else
						block->lx = lx;
						block->ly = ly;
#endif

                        block->volume = block1->volume + block2->volume;
                        block->total = lx * ly * lz;
                        for (int k = 0; k < n; ++k)
                        {
                            block->require[k] = block1->require[k] + block2->require[k];
                            if (block->require[k] > num[k])
                            {
                                Log(LogError, "GenBlocks Error\n");
                                exit(1);
                            }
                        }
                    }
                }

                {
                    if (block1->lx * block1->ly < block2->lx * block2->ly)
                    {
                        swap(block1, block2);
                    }

                    int lx = max(block1->lx, block2->lx);
                    int ly = max(block1->ly, block2->ly);
                    int lz = block1->lz + block2->lz;
                    int ax = block2->ax;
                    int ay = block2->ay;

                    if (block1->volume + block2->volume > MinFillRate * lx * ly * lz
#if STABLE
                        && ax * ay > MinAreaRate * lx * ly
                        && block1->ax >= block2->lx && block1->ay >= block2->ly
#endif
                        && lx <= this->lx && ly <= this->ly && lz <= this->lz)
                    {
                        blockTableAux.resize(blockTableAux.size()+1);
                        Block *block = &blockTableAux.back();
                        fill_n(block->require, MaxBox, 0);
                        blockTable.push_back(block);

                        block->level = max(block1->level, block2->level) + 1;
                        block->type = CombineBlockZ;
                        block->childs[0] = block1;
                        block->childs[1] = block2;
                        block->lx = lx;
                        block->ly = ly;
                        block->lz = lz;

#if STABLE
                        block->ax = ax;
                        block->ay = ay;
#else
						block->lx = lx;
						block->ly = ly;
#endif

                        block->volume = block1->volume + block2->volume;
                        block->total = lx * ly * lz;
                        for (int k = 0; k < n; ++k)
                        {
                            block->require[k] = block1->require[k] + block2->require[k];
                            if (block->require[k] > num[k])
                            {
                                Log(LogError, "GenBlocks Error\n");
                                exit(1);
                            }
                        }
                    }
                }
            }

            if (blockTable.size() > MaxBlockTable)
                break;
        }

        sort(blockTable.begin(), blockTable.end(), Block::Compare);
        blockTable.erase(unique(blockTable.begin(), blockTable.end(), 
            Block::Equal), blockTable.end());
        
        //Log(LogInfo, "round %d blocks: %d\n", round, blockTable.size());
    }

    //Log(LogInfo, "final blocks: %d\n", blockTable.size());

    for (int i = 0; i < blockTable.size(); ++i)
    {
        Block *block = blockTable[i];

        block->boxLen = 0;
        for (int k = 0; k < n; ++k)
        {
            if (block->require[k] > 0)
            {
                block->boxIndex[block->boxLen++] = k;
            }
        }
    }
}

void PackingProblem::ClearBlocks()
{
    blockTable.resize(0);
    blockTable.reserve(MaxBlockTable);
    blockTableAux.resize(0);
    blockTableAux.reserve(MaxBlockTable*10);
}
