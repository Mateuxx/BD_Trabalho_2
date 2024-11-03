#ifndef HASH_H
#define HASH_H

#include <fstream>
#include <iostream>
#include <cstring>

constexpr int BUCKET_SIZE = 4;
constexpr int BLOCK_SIZE = 3;
constexpr int HASH_TABLE_SIZE = 1000;

struct Artigo {
    int id;
    char titulo[301];
    int ano;
    char autores[151];
    int citacoes;
    char atualizacao[21];
    char snippet[501];
};

struct Bloco {
    int count = 0;
    Artigo artigos[BLOCK_SIZE];
};

struct Bucket {
    int count = 0;
    Bloco blocos[BUCKET_SIZE];
    int overflowStartPos = -1;
};

class HashTable {
public:
    HashTable();
    long inserirArtigo(const Artigo& artigo);
    Artigo* buscarArtigo(long address, int id);

private:
    int hashFunction(int id);
    long inserirNoOverflow(const Artigo& artigo, int hashIndex, Bucket& bucket);
};

#endif // HASH_H
