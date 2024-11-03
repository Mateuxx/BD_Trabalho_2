#include "bplustree_prim.h"

// Construtor para BPlusNodePrim
BPlusNodePrim::BPlusNodePrim(bool leaf) : isLeaf(leaf) {}

void BPlusNodePrim::saveToDisk(std::fstream& file) {
    if (position == -1) {
        file.seekp(0, std::ios::end);
        position = file.tellp();
    } else {
        file.seekp(position);
    }
    file.write(reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));

    int keysSize = keys.size();
    file.write(reinterpret_cast<char*>(&keysSize), sizeof(keysSize));
    file.write(reinterpret_cast<char*>(keys.data()), keysSize * sizeof(int));

    if (isLeaf) {
        int recordsSize = records.size();
        file.write(reinterpret_cast<char*>(&recordsSize), sizeof(recordsSize));
        file.write(reinterpret_cast<char*>(records.data()), recordsSize * sizeof(IndexRecordPrim));
        file.write(reinterpret_cast<char*>(&nextLeaf), sizeof(nextLeaf));
    } else {
        int childrenSize = children.size();
        file.write(reinterpret_cast<char*>(&childrenSize), sizeof(childrenSize));
        file.write(reinterpret_cast<char*>(children.data()), childrenSize * sizeof(long));
    }
}

void BPlusNodePrim::loadFromDisk(std::fstream& file, long pos) {
    position = pos;
    file.seekg(position);

    file.read(reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));

    int keysSize;
    file.read(reinterpret_cast<char*>(&keysSize), sizeof(keysSize));
    keys.resize(keysSize);
    file.read(reinterpret_cast<char*>(keys.data()), keysSize * sizeof(int));

    if (isLeaf) {
        int recordsSize;
        file.read(reinterpret_cast<char*>(&recordsSize), sizeof(recordsSize));
        records.resize(recordsSize);
        file.read(reinterpret_cast<char*>(records.data()), recordsSize * sizeof(IndexRecordPrim));
        file.read(reinterpret_cast<char*>(&nextLeaf), sizeof(nextLeaf));
    } else {
        int childrenSize;
        file.read(reinterpret_cast<char*>(&childrenSize), sizeof(childrenSize));
        children.resize(childrenSize);
        file.read(reinterpret_cast<char*>(children.data()), childrenSize * sizeof(long));
    }
}

BPlusTreePrim::BPlusTreePrim(int ord, const std::string& filename) : order(ord) {
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }
    rootPosition = -1;
}

BPlusTreePrim::~BPlusTreePrim() {
    file.close();
}

void BPlusTreePrim::saveNode(BPlusNodePrim& node) {
    node.saveToDisk(file);
}

BPlusNodePrim BPlusTreePrim::loadNode(long position) {
    BPlusNodePrim node;
    node.loadFromDisk(file, position);
    return node;
}

// Dividindo um filho
void BPlusTreePrim::splitChild(BPlusNodePrim& parent, int index, BPlusNodePrim& child) {
    BPlusNodePrim newNode(child.isLeaf);
    int mid = order - 1;

    // Copia a metade superior das chaves do filho para o novo nó
    newNode.keys.assign(child.keys.begin() + mid + 1, child.keys.end());
    child.keys.resize(mid);

    if (child.isLeaf) {
        // Copia metade dos registros
        newNode.records.assign(child.records.begin() + mid, child.records.end());
        child.records.resize(mid);
        newNode.nextLeaf = child.nextLeaf;
        child.nextLeaf = newNode.position;
    } else {
        // Move metade das crianças para o novo nó
        newNode.children.assign(child.children.begin() + mid + 1, child.children.end());
        child.children.resize(mid + 1);
    }

    // Adiciona nova chave no nó pai
    parent.keys.insert(parent.keys.begin() + index, child.keys[mid]);
    parent.children.insert(parent.children.begin() + index + 1, newNode.position);

    saveNode(child);
    saveNode(newNode);
}

// Inserindo em nó não cheio
void BPlusTreePrim::insertNonFull(BPlusNodePrim& node, const IndexRecordPrim& record) {
    if (node.isLeaf) {
        // Inserção ordenada em nó folha
        auto it = node.records.begin();
        while (it != node.records.end() && it->id < record.id) ++it;
        node.records.insert(it, record);
        saveNode(node);
    } else {
        // Procura o filho correto para inserção
        int i = node.keys.size() - 1;
        while (i >= 0 && record.id < node.keys[i]) i--;
        i++;
        BPlusNodePrim childNode = loadNode(node.children[i]);
        if (childNode.records.size() == 2 * order - 1) {
            splitChild(node, i, childNode);
            if (record.id > node.keys[i]) i++;
        }
        insertNonFull(childNode, record);
    }
}

void BPlusTreePrim::insert(const IndexRecordPrim& record) {
    if (rootPosition == -1) {
        BPlusNodePrim rootNode(true);
        rootNode.records.push_back(record);
        saveNode(rootNode);
        rootPosition = rootNode.position;
    } else {
        BPlusNodePrim rootNode = loadNode(rootPosition);
        if (rootNode.records.size() == 2 * order - 1) {
            BPlusNodePrim newRoot(false);
            newRoot.children.push_back(rootNode.position);
            splitChild(newRoot, 0, rootNode);
            rootPosition = newRoot.position;
            insertNonFull(newRoot, record);
        } else {
            insertNonFull(rootNode, record);
        }
    }
}

IndexRecordPrim* BPlusTreePrim::search(int idKey, int& blocksRead) {
    if (rootPosition == -1) return nullptr;
    BPlusNodePrim node = loadNode(rootPosition);
    blocksRead = 1;

    while (!node.isLeaf) {
        int i = 0;
        while (i < node.keys.size() && idKey > node.keys[i]) i++;
        node = loadNode(node.children[i]);
        blocksRead++;
    }

    for (const auto& record : node.records) {
        if (record.id == idKey) return new IndexRecordPrim(record);
    }
    return nullptr;
}
