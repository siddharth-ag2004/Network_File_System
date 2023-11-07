#ifndef TRIE_H
typedef struct TrieNode
{
    char directory[100];
    struct TrieNode *firstChild;
    struct TrieNode *sibling;
    int isFile;
}TrieNode;

TrieNode *createNode(char *directory);

void InsertTrie(char *directory,TrieNode* root);

TrieNode* SearchTrie(char *directory,TrieNode* root);

void DeleteTrie(char *directory,TrieNode* root);

void PrintTrie(struct TrieNode *root);

void TrieToString(struct TrieNode *root, char *str);

TrieNode* StringToTrie(char *str);

#endif