#ifndef __PROBLEM_H_

#define __PROBLEM_H_

#include <vector>
#include <algorithm>

#define STABLE				0

const int BufferSize = 10000;
const int MaxBox = 100;
const int MaxBlockTable = 10000;
const int MaxRound = 5;

const double MinFillRate = 0.98;
const double MinAreaRate = 0.96;

enum DirectionType { DirectionX = 1, DirectionY = 2, DirectionZ = 4, };
enum BlockType { SimpleBlock, CombineBlockX, CombineBlockY, CombineBlockZ};

// Box structure, one box type may have at most six box structure, one for each direction.
struct Box
{
    int x, y, z;
    int lx, ly, lz;
    int type;
    int volume;

    static bool Compare(const Box *b1, const Box *b2)
    {
        if (b1->volume != b2->volume)
            return b1->volume > b2->volume;
        return b1->type < b2->type;
    }
};

// Space remain for packing.
struct Space
{
    int x, y, z;
    int lx, ly, lz;
    //bool isMax;
    //int volume;
    DirectionType direction;
    int size;
    int px, py, pz;

    void Swap(DirectionType type)
    {
        if (type == DirectionX)
            std::swap(lx, px);
        else if (type == DirectionY)
            std::swap(ly, py);
        else if (type == DirectionZ)
            std::swap(lz, pz);
    }

    int volume() const {
        return lx * ly * lz;
    }
};

// A packing block contain the same type box.
struct Block
{
    int type;
    int ax, ay;
    int lx, ly, lz;
    int volume;
    int total;

    int level;
    int require[MaxBox];
    int boxIndex[MaxBox];
    int boxLen;

    const Box *box;
    int nx, ny, nz;
    int use;

    const Block *childs[2];

    // Compare function for blocks.
    static bool Compare(const Block *b1, const Block *b2);
    static bool Equal(const Block *b1, const Block *b2);
    static bool IsContain(const Block *block, const Box *box);
};

struct PackingProblem
{
    // The number of boxes.
    int n;
    // The number of boxes, considers different direction as different types.
    //int m;
    // The three dimensions of the container.
    int lx, ly, lz;
    int volume;

    Box *boxList[BufferSize];
    Box boxListAux[BufferSize];
    int boxListLen;
    // Boxes for packing.
    //Box boxes[BufferSize];
    // Number of per box type.
    int num[MaxBox];

    int simpleBlockLen;
    std::vector<Block *> blockTable;
    std::vector<Block> blockTableAux;

    ~PackingProblem() { ClearBlocks(); }

    bool Input(FILE *fin);
    void GenBlocks();
    void ClearBlocks();
};

#endif
