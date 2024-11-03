#include "hash.h"

HashTable::HashTable() {
    std::fstream hashFile("hash_table.dat", std::ios::in | std::ios::out | std::ios::binary);
    if (!hashFile) {
        hashFile.open("hash_table.dat", std::ios::out | std::ios::binary);
        Bucket emptyBucket;
        for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
            hashFile.write(reinterpret_cast<char*>(&emptyBucket), sizeof(Bucket));
        }
        hashFile.close();
    }
}

int HashTable::hashFunction(int id) {
    const double A = 0.6180339887;
    return static_cast<int>(HASH_TABLE_SIZE * (id * A - static_cast<int>(id * A)));
}

long HashTable::inserirArtigo(const Artigo& artigo) {
    int hashIndex = hashFunction(artigo.id);
    std::fstream hashFile("hash_table.dat", std::ios::in | std::ios::out | std::ios::binary);

    if (!hashFile) {
        std::cerr << "Erro ao abrir o arquivo da tabela hash." << std::endl;
        return -1;
    }

    hashFile.seekg(hashIndex * sizeof(Bucket));
    Bucket bucket;
    hashFile.read(reinterpret_cast<char*>(&bucket), sizeof(Bucket));

    for (int i = 0; i < BUCKET_SIZE; ++i) {
        if (bucket.blocos[i].count < BLOCK_SIZE) {
            bucket.blocos[i].artigos[bucket.blocos[i].count++] = artigo;
            bucket.count++;
            long address = hashIndex * sizeof(Bucket);
            hashFile.seekp(address);
            hashFile.write(reinterpret_cast<const char*>(&bucket), sizeof(Bucket));
            hashFile.close();
            return address;
        }
    }

    hashFile.close();
    return inserirNoOverflow(artigo, hashIndex, bucket);
}

long HashTable::inserirNoOverflow(const Artigo& artigo, int hashIndex, Bucket& bucket) {
    std::fstream overflowFile("overflow.dat", std::ios::in | std::ios::out | std::ios::app | std::ios::binary);

    if (!overflowFile) {
        std::cerr << "Erro ao abrir o arquivo de overflow." << std::endl;
        return -1;
    }

    overflowFile.seekp(0, std::ios::end);
    long pos = overflowFile.tellp();
    if (bucket.overflowStartPos == -1) {
        bucket.overflowStartPos = pos;

        std::fstream hashFile("hash_table.dat", std::ios::in | std::ios::out | std::ios::binary);
        hashFile.seekp(hashIndex * sizeof(Bucket));
        hashFile.write(reinterpret_cast<const char*>(&bucket), sizeof(Bucket));
        hashFile.close();
    }

    overflowFile.write(reinterpret_cast<const char*>(&artigo), sizeof(Artigo));
    overflowFile.close();
    return pos;
}

Artigo* HashTable::buscarArtigo(long address, int id) {
    std::fstream hashFile("hash_table.dat", std::ios::in | std::ios::binary);
    if (!hashFile) {
        std::cerr << "Erro ao abrir o arquivo da tabela hash." << std::endl;
        return nullptr;
    }

    hashFile.seekg(address);
    Bucket bucket;
    hashFile.read(reinterpret_cast<char*>(&bucket), sizeof(Bucket));

    for (int i = 0; i < BUCKET_SIZE; ++i) {
        Bloco& bloco = bucket.blocos[i];
        for (int j = 0; j < bloco.count; ++j) {
            if (bloco.artigos[j].id == id) {
                Artigo* foundArtigo = new Artigo(bloco.artigos[j]);
                hashFile.close();
                return foundArtigo;
            }
        }
    }
    hashFile.close();
    return nullptr;
}
