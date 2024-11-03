#include "bplustree_sec.h"
#include <iostream>

// Construtor de BPlusNodeSec
BPlusNodeSec::BPlusNodeSec(bool leaf) : isLeaf(leaf) {}

// Salvando o nó no disco
void BPlusNodeSec::saveToDisk(std::fstream& file) {
    if (position == -1) {
        file.seekp(0, std::ios::end);
        position = file.tellp();
    } else {
        file.seekp(position);
    }
    file.write(reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));

    int keysSize = titleKeys.size();
    file.write(reinterpret_cast<char*>(&keysSize), sizeof(keysSize));
    for (const auto& key : titleKeys) {
        int keyLength = key.size();
        file.write(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
        file.write(key.c_str(), keyLength);
    }

    if (isLeaf) {
        int recordsSize = records.size();
        file.write(reinterpret_cast<char*>(&recordsSize), sizeof(recordsSize));
        for (const auto& record : records) {
            int keyLength = record.titleKey.size();
            file.write(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
            file.write(record.titleKey.c_str(), keyLength);
            file.write(reinterpret_cast<const char*>(&record.address), sizeof(record.address));
        }
        file.write(reinterpret_cast<char*>(&nextLeaf), sizeof(nextLeaf));
    } else {
        int childrenSize = children.size();
        file.write(reinterpret_cast<char*>(&childrenSize), sizeof(childrenSize));
        for (long childPosition : children) {
            file.write(reinterpret_cast<char*>(&childPosition), sizeof(childPosition));
        }
    }
}

// Carregando o nó do disco
void BPlusNodeSec::loadFromDisk(std::fstream& file, long pos) {
    position = pos;
    file.seekg(position);

    file.read(reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));

    int keysSize;
    file.read(reinterpret_cast<char*>(&keysSize), sizeof(keysSize));
    titleKeys.clear();
    for (int i = 0; i < keysSize; ++i) {
        int keyLength;
        file.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
        std::string key(keyLength, ' ');
        file.read(&key[0], keyLength);
        titleKeys.push_back(key);
    }

    if (isLeaf) {
        int recordsSize;
        file.read(reinterpret_cast<char*>(&recordsSize), sizeof(recordsSize));
        records.clear();
        for (int i = 0; i < recordsSize; ++i) {
            int keyLength;
            file.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
            std::string key(keyLength, ' ');
            file.read(&key[0], keyLength);

            long address;
            file.read(reinterpret_cast<char*>(&address), sizeof(address));
            records.emplace_back(key, address);
        }
        file.read(reinterpret_cast<char*>(&nextLeaf), sizeof(nextLeaf));
    } else {
        int childrenSize;
        file.read(reinterpret_cast<char*>(&childrenSize), sizeof(childrenSize));
        children.clear();
        for (int i = 0; i < childrenSize; ++i) {
            long childPosition;
            file.read(reinterpret_cast<char*>(&childPosition), sizeof(childPosition));
            children.push_back(childPosition);
        }
    }
}

// Construtor de BPlusTreeSec
BPlusTreeSec::BPlusTreeSec(int ord, const std::string& filename) : order(ord) {
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }
    rootPosition = -1;
}

// Destrutor de BPlusTreeSec
BPlusTreeSec::~BPlusTreeSec() {
    file.close();
}

// Salvando um nó
void BPlusTreeSec::saveNode(BPlusNodeSec& node) {
    node.saveToDisk(file);
}

// Carregando um nó
BPlusNodeSec BPlusTreeSec::loadNode(long position) {
    BPlusNodeSec node;
    node.loadFromDisk(file, position);
    return node;
}

// Dividindo um filho
void BPlusTreeSec::splitChild(BPlusNodeSec& parent, int index, BPlusNodeSec& child) {
    BPlusNodeSec newNode(child.isLeaf);
    int mid = order - 1;

    newNode.titleKeys.assign(child.titleKeys.begin() + mid + 1, child.titleKeys.end());
    child.titleKeys.resize(mid);

    if (child.isLeaf) {
        newNode.records.assign(child.records.begin() + mid, child.records.end());
        child.records.resize(mid);
        newNode.nextLeaf = child.nextLeaf;
        child.nextLeaf = newNode.position;
    } else {
        newNode.children.assign(child.children.begin() + mid + 1, child.children.end());
        child.children.resize(mid + 1);
    }

    parent.titleKeys.insert(parent.titleKeys.begin() + index, child.titleKeys[mid]);
    parent.children.insert(parent.children.begin() + index + 1, newNode.position);

    saveNode(child);
    saveNode(newNode);
}

// Inserindo em nó não cheio
void BPlusTreeSec::insertNonFull(BPlusNodeSec& node, const IndexRecordSec& record) {
    if (node.isLeaf) {
        auto it = node.records.begin();
        while (it != node.records.end() && it->titleKey < record.titleKey) ++it;
        node.records.insert(it, record);
        saveNode(node);
    } else {
        int i = node.titleKeys.size() - 1;
        while (i >= 0 && record.titleKey < node.titleKeys[i]) i--;
        i++;
        BPlusNodeSec childNode = loadNode(node.children[i]);
        if (childNode.records.size() == 2 * order - 1) {
            splitChild(node, i, childNode);
            if (record.titleKey > node.titleKeys[i]) i++;
        }
        insertNonFull(childNode, record);
    }
}

// Inserção no BPlusTreeSec
void BPlusTreeSec::insert(const IndexRecordSec& record) {
    if (rootPosition == -1) {
        BPlusNodeSec rootNode(true);
        rootNode.records.push_back(record);
        saveNode(rootNode);
        rootPosition = rootNode.position;
    } else {
        BPlusNodeSec rootNode = loadNode(rootPosition);
        if (rootNode.records.size() == 2 * order - 1) {
            BPlusNodeSec newRoot(false);
            newRoot.children.push_back(rootNode.position);
            splitChild(newRoot, 0, rootNode);
            rootPosition = newRoot.position;
            insertNonFull(newRoot, record);
        } else {
            insertNonFull(rootNode, record);
        }
    }
}

// Busca no BPlusTreeSec
IndexRecordSec* BPlusTreeSec::search(const std::string& titleKey, int& blocksRead) {
    if (rootPosition == -1) return nullptr;
    BPlusNodeSec node = loadNode(rootPosition);
    blocksRead = 1;

    while (!node.isLeaf) {
        int i = 0;
        while (i < node.titleKeys.size() && titleKey > node.titleKeys[i]) i++;
        node = loadNode(node.children[i]);
        blocksRead++;
    }

    for (const auto& record : node.records) {
        if (record.titleKey == titleKey) return new IndexRecordSec(record);
    }
    return nullptr;
}
