#include "hash.h"            // Inclui a implementação da HashTable
#include "bplustree_sec.h"   // Inclui a implementação da BPlusTreeSec (título)
#include "bplustree_prim.h"  // Inclui a implementação da BPlusTreePrim (ID)
#include <iostream>
#include <string>

constexpr int B_PLUS_TREE_ORDER = 3; // Define a ordem mínima da B+Tree

void findrec(HashTable& hashTable, int id) {
    int hashAddress = hashTable.hashFunction(id) * sizeof(Bucket);
    Artigo* artigo = hashTable.buscarArtigo(hashAddress, id);

    if (artigo) {
        std::cout << "Artigo encontrado diretamente pelo ID: " << id << std::endl;
        std::cout << "Título: " << artigo->titulo << ", Ano: " << artigo->ano << std::endl;
        delete artigo;
    } else {
        std::cout << "Artigo com ID " << id << " não encontrado diretamente." << std::endl;
    }
}

void seek1(BPlusTreePrim& idIndexTree, HashTable& hashTable, int id) {
    int blocksRead = 0;
    IndexRecordPrim* foundRecord = idIndexTree.search(id, blocksRead);

    if (foundRecord) {
        Artigo* foundArtigo = hashTable.buscarArtigo(foundRecord->address, id);
        if (foundArtigo) {
            std::cout << "Artigo encontrado pelo índice primário!" << std::endl;
            std::cout << "Título: " << foundArtigo->titulo << ", Ano: " << foundArtigo->ano << std::endl;
            delete foundArtigo;
        } else {
            std::cout << "Artigo com ID " << id << " não encontrado nos dados." << std::endl;
        }
        delete foundRecord;
    } else {
        std::cout << "Erro: Registro com ID " << id << " não encontrado no índice primário." << std::endl;
    }
}

void seek2(BPlusTreeSec& titleIndexTree, HashTable& hashTable, const std::string& title) {
    int blocksRead = 0;
    IndexRecordSec* record = titleIndexTree.search(title, blocksRead);

    if (record) {
        Artigo* artigo = hashTable.buscarArtigo(record->address, -1);
        if (artigo) {
            std::cout << "Artigo encontrado pelo índice secundário!" << std::endl;
            std::cout << "Título: " << artigo->titulo << ", ID: " << artigo->id << std::endl;
            delete artigo;
        } else {
            std::cout << "Erro: Artigo com título \"" << title << "\" não encontrado nos dados." << std::endl;
        }
        delete record;
    } else {
        std::cout << "Erro: Registro com título \"" << title << "\" não encontrado no índice secundário." << std::endl;
    }
}

int main() {
    // Inicialização da HashTable, da B+Tree para título e da B+Tree para ID
    HashTable hashTable;
    BPlusTreeSec titleIndexTree(B_PLUS_TREE_ORDER, "title_index_tree.dat");
    BPlusTreePrim idIndexTree(B_PLUS_TREE_ORDER, "id_index_tree.dat");

    // Exemplo de inserção
    Artigo artigo1 = {101, "Aprendizado de Máquina", 2021, "Autor Exemplo", 10, "2023-01-01", "Exemplo de artigo."};
    Artigo artigo2 = {102, "Redes Neurais", 2022, "Outro Autor", 20, "2023-02-01", "Outro exemplo."};

   long address1 = hashTable.inserirArtigo(artigo1);
if (address1 != -1) {
    std::cout << "Endereço de inserção para 'Aprendizado de Máquina' na HashTable: " << address1 << std::endl;
    titleIndexTree.insert(IndexRecordSec(artigo1.titulo, address1));
    idIndexTree.insert(IndexRecordPrim(artigo1.id, address1));
}

long address2 = hashTable.inserirArtigo(artigo2);
if (address2 != -1) {
    std::cout << "Endereço de inserção para 'Redes Neurais' na HashTable: " << address2 << std::endl;
    titleIndexTree.insert(IndexRecordSec(artigo2.titulo, address2));
    idIndexTree.insert(IndexRecordPrim(artigo2.id, address2));
}

    // Testando a busca direta pelo ID com `findrec`
    int buscaID = 101;
    std::cout << "\nfindrec <ID> Testando busca direta pelo ID: " << buscaID << std::endl;
    findrec(hashTable, buscaID);

    // Testando a busca pelo índice primário com `seek1`
    std::cout << "\nseek1 <ID> Testando busca pelo índice primário com ID: " << buscaID << std::endl;
    seek1(idIndexTree, hashTable, buscaID);

    // Testando a busca pelo índice secundário com `seek2`
    std::string buscaTitulo = "Redes Neurais";
    std::cout << "\nseek2 <Título> Testando busca pelo índice secundário com Título: " << buscaTitulo << std::endl;
    seek2(titleIndexTree, hashTable, buscaTitulo);

    return 0;
}
