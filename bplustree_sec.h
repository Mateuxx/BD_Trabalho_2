#ifndef BPLUSTREE_SEC_H
#define BPLUSTREE_SEC_H

#include <fstream>
#include <vector>
#include <string>

struct IndexRecordSec {
    std::string titleKey;
    long address;

    IndexRecordSec(std::string k = "", long addr = -1) : titleKey(k), address(addr) {}
};

class BPlusNodeSec {
public:
    bool isLeaf;
    std::vector<std::string> titleKeys;
    std::vector<long> children;
    std::vector<IndexRecordSec> records;
    long nextLeaf = -1;
    long position = -1;

    BPlusNodeSec(bool leaf = false);

    void saveToDisk(std::fstream& file);
    void loadFromDisk(std::fstream& file, long pos);
};

class BPlusTreeSec {
private:
    std::fstream file;
    long rootPosition;
    int order;

    void insertNonFull(BPlusNodeSec& node, const IndexRecordSec& record);
    void splitChild(BPlusNodeSec& parent, int index, BPlusNodeSec& child);

public:
    BPlusTreeSec(int ord, const std::string& filename);
    ~BPlusTreeSec();

    void saveNode(BPlusNodeSec& node);
    BPlusNodeSec loadNode(long position);
    void insert(const IndexRecordSec& record);
    IndexRecordSec* search(const std::string& titleKey, int& blocksRead);
};

#endif
