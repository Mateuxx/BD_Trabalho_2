#ifndef BPLUSTREE_PRIM_H
#define BPLUSTREE_PRIM_H

#include <fstream>
#include <vector>
#include <string>

struct IndexRecordPrim {
    int id;
    long address;

    IndexRecordPrim(int i = -1, long addr = -1) : id(i), address(addr) {}
};

class BPlusNodePrim {
public:
    bool isLeaf;
    std::vector<int> keys;
    std::vector<long> children;
    std::vector<IndexRecordPrim> records;
    long nextLeaf = -1;
    long position = -1;

    BPlusNodePrim(bool leaf = false);
    void saveToDisk(std::fstream& file);
    void loadFromDisk(std::fstream& file, long pos);
};

class BPlusTreePrim {
private:
    std::fstream file;
    long rootPosition;
    int order;

    void saveNode(BPlusNodePrim& node);
    BPlusNodePrim loadNode(long position);
    void splitChild(BPlusNodePrim& parent, int index, BPlusNodePrim& child);
    void insertNonFull(BPlusNodePrim& node, const IndexRecordPrim& record);

public:
    BPlusTreePrim(int ord, const std::string& filename);
    ~BPlusTreePrim();

    void insert(const IndexRecordPrim& record);
    IndexRecordPrim* search(int idKey, int& blocksRead);
};

#endif // BPLUSTREE_PRIM_H
