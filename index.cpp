#pragma GCC optimize("O3")
#pragma GCC optimization("unroll-loops")
#include "index.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#define key first
#define value second
#define MIN_INT -2147483648
#define all(v) v.begin(), v.end()
using namespace std;
// An internal node has at most M children
const int M = 13;
const int CHUNK_SIZE = 4096;
inline void writeFile(const vector<int>& vec, FILE* f) {
    char buf[CHUNK_SIZE + 64];
    int cc = 0;
    int i = 0;
    const int len = vec.size();

    while (i < len) {
        cc += sprintf(&buf[cc], "%d\n", vec[i]);
        i++;

        if (cc >= CHUNK_SIZE) {
            fwrite(buf, CHUNK_SIZE, 1, f);
            cc -= CHUNK_SIZE;
            memcpy(buf, &buf[CHUNK_SIZE], cc);
        }
    }

    if (cc > 0) fwrite(buf, 1, cc, f);
}

// --------------- Bplus tree ----------------

/*
void BPlusTree::test(int s) const {
    unordered_set<int> ss;
    cout << "#####################\n";
    if (level == 0) return;
    vector<Internal*> nodes = {(Internal*)root};
    bool bad = 0;
    for (int i = 0; i < level; i++) {
        cout << '%' << endl;
        vector<Internal*> sw;
        for (Internal* node : nodes) {
            auto& keys = node->keys;
            if (i != level - 1) {
                for (int j = 0; j < keys.size(); j++) {
                    sw.push_back((Internal*)node->children[j]);
                    cout << node->keys[j] << ':';
                    for (auto& v : ((Internal*)node->children[j])->keys) {
                        cout << v << ',';
                        if (v >= node->keys[j]) bad = 1;
                    }
                    cout << endl;
                }
                cout << "*:";
                for (auto& v : ((Internal*)node->children.back())->keys) {
                    cout << v << ',';
                    if (v < node->keys.back()) bad = 1;
                }
                sw.push_back((Internal*)node->children.back());
                cout << endl;
            } else {
                for (int j = 0; j < keys.size(); j++) {
                    cout << node->keys[j] << ':';
                    for (auto& v : ((Leaf*)node->children[j])->entries) {
                        cout << v.key << ',';
                        if (v.key >= node->keys[j]) bad = 1;
                        ss.insert(v.key);
                    }
                    cout << endl;
                }
                cout << "*:";
                for (auto& v : ((Leaf*)node->children.back())->entries) {
                    cout << v.key << ',';
                    if (v.key < node->keys.back()) bad = 1;
                    ss.insert(v.key);
                }
                cout << endl;
            }
        }
        nodes.swap(sw);
    }
    // assert(ss.size() == s);
    // assert(!bad);
}
*/

BPlusTree::BPlusTree() {
    // empty
}

BPlusTree::~BPlusTree() {
    if (!root) return;
    vector<void*> p = {root};
    for (int i = 0; i < level; i++) {
        vector<void*> q;
        for (auto a : p) {
            Internal* inode = (Internal*)a;
            q.insert(q.end(), all(inode->children));
            delete inode;
        }
        p.swap(q);
    }
    for (auto a : p) {
        Leaf* lnode = (Leaf*)a;
        delete lnode;
    }
}

void BPlusTree::insert(int k, int v) {
    if (!root) {
        Leaf* l = new Leaf();
        l->prev = l->next = NULL;
        l->entries.emplace_back(k, v);
        l->x = v;
        root = l;
        return;
    }

    // Find leaf node
    vector<Internal*> layer;
    void* _n = root;
    for (int i = 0; i < level; i++) {
        Internal* inode = (Internal*)_n;
        layer.push_back(inode);
        _n = inode->findChildByKey(k);
    }

    // Insert entry into leaf node
    Leaf* lnode = (Leaf*)_n;
    if (lnode->entries.size() + 1 <= M) {
        lnode->insert(k, v);
        return;
    }

    // Check splitting to top
    int kick;
    void* split = lnode->insertAndSplit(k, v, kick);
    for (int i = level - 1; i >= 0; i--) {
        Internal* inode = layer[i];
        if (inode->keys.size() + 1 <= M - 1) {
            inode->insert(kick, split);
            return;
        }
        split = inode->insertAndSplit(kick, split, kick);
    }

    // Root is splitted
    if (split) {
        Internal* newRoot = new Internal();
        newRoot->keys.push_back(kick);
        newRoot->children.push_back(split);
        newRoot->children.push_back(root);
        root = newRoot;
        level++;
    }
}

int BPlusTree::query(const int k) const {
    if (!root) return -1;
    void* _n = root;
    for (int i = 0; i < level; i++) {
        _n = ((Internal*)_n)->findChildByKey(k);
    }
    return ((Leaf*)_n)->query(k);
}

int BPlusTree::query(const int l, const int r) const {
    if (!root) return -1;

    void* _n = root;
    void* _r = root;
    for (int i = 0; i < level; i++) {
        _n = ((Internal*)_n)->findChildByKey(l);
        _r = ((Internal*)_r)->findChildByKey(r);
    }

    int ret = MIN_INT;
    Leaf* lnode = (Leaf*)_n;
    auto lix = lower_bound(all(lnode->entries), l, [](const entry& e, const int& n) {
        return e.key < n;
    });
    for (; lix != lnode->entries.end() && lix->key <= r; lix++) {
        ret = max(ret, lix->value);
    }
    if (lnode == _r) return ret == MIN_INT ? -1 : ret;

    lnode = lnode->next;
    for (; lnode != _r; lnode = lnode->next) {
        /*auto lix = lnode->entries.begin();
        for (; lix != lnode->entries.end() && lix->key <= r; lix++) {
            ret = max(ret, lix->value);
        }*/
        ret = max(ret, lnode->x);
    }
    lix = lnode->entries.begin();
    for (; lix != lnode->entries.end() && lix->key <= r; lix++) {
        ret = max(ret, lix->value);
    }
    return ret == MIN_INT ? -1 : ret;
}

// --------------- Leaf ----------------

Leaf::Leaf() {
    entries.reserve(M);
}

inline void Leaf::insert(int key, int value) {
    auto a = upper_bound(all(entries), key, [](const int& n, const entry& e) {
        return n < e.key;
    });
    entries.insert(a, entry(key, value));
    x = max(x, value);
}

inline Leaf* Leaf::insertAndSplit(int key, int value, int& kick) {
    const int reqSize = (M + 1) / 2;  // floor
    const int remSize = M + 1 - reqSize;

    Leaf* ret = new Leaf();
    int j = 0;
    bool put = 0;
    for (int i = 0; i < reqSize; i++) {
        if (put || entries[j].key < key) {
            ret->x = max(ret->x, entries[j].value);
            ret->entries.push_back(entries[j++]);
        } else {
            ret->x = max(ret->x, value);
            ret->entries.emplace_back(key, value);
            put = 1;
        }
    }

    x = MIN_INT;
    for (int i = 0; i < remSize; i++) {
        if (j == entries.size() || (!put && key < entries[j].key)) {
            entries[i].key = key;
            entries[i].value = value;
            x = max(x, value);
            put = 1;
        }  //
        else {
            x = max(x, entries[j].value);
            entries[i] = entries[j++];
        }
    }
    entries.resize(remSize);
    entries.reserve(M);

    ret->next = this;
    ret->prev = prev;
    if (prev) prev->next = ret;
    prev = ret;

    kick = entries[0].key;
    return ret;
}

inline int Leaf::query(int key) const {
    auto a = lower_bound(all(entries), key, [](const entry& e, int n) {
        return e.key < n;
    });
    if (a == entries.end()) return -1;
    return a->key == key ? a->value : -1;
}

// --------------- Leaf ----------------

Internal::Internal() {
    children.reserve(M);
    keys.reserve(M - 1);
}

inline void Internal::insert(int key, void* child) {
    auto a = upper_bound(all(keys), key);
    int index = a - keys.begin();
    keys.insert(a, key);
    children.insert(children.begin() + index, child);
}

inline Internal* Internal::insertAndSplit(int key, void* newChild, int& kick) {
    const int reqSize = M / 2;  // ceil;
    const int remSize = M - 1 - reqSize;

    Internal* ret = new Internal();
    int kp = 0, cp = 0;
    bool put = 0;
    for (int i = 0; i < reqSize; i++) {
        if (put || keys[kp] < key) {
            ret->keys.push_back(keys[kp++]);
            ret->children.push_back(children[cp++]);
        }  //
        else {
            ret->keys.push_back(key);
            ret->children.push_back(newChild);
            put = 1;
        }
    }

    if (put || keys[kp] < key) {
        kick = keys[kp++];
        ret->children.push_back(children[cp++]);
    } else {
        kick = key;
        ret->children.push_back(newChild);
        put = 1;
    }

    int j = 0;
    for (int i = 0; i < remSize; i++, j++) {
        if (kp == keys.size() || (!put && key < keys[kp])) {
            keys[i] = key;
            children[j++] = newChild;
            children[j] = children[cp++];
            put = 1;
        } else {
            keys[i] = keys[kp++];
            children[j] = children[cp++];
        }
    }
    if (cp < M) {
        children[j] = children[cp++];
    }

    keys.resize(remSize);
    keys.reserve(M - 1);
    children.resize(remSize + 1);
    children.reserve(M);
    return ret;
}

inline void* Internal::findChildByKey(int key) const {
    auto a = upper_bound(all(keys), key);
    int index = a - keys.begin();
    return children[index];
}

// --------------- Index ----------------

/**
 * Constructs a B+ tree index by inserting the key-value pairs into the B+ tree ​one by 
 * one​.
 */
Index::Index(int num_rows, const vector<int>& keys, const vector<int>& values) {
    for (int i = 0; i < keys.size(); i++) {
        // One by one
        b.insert(keys[i], values[i]);
    }
}

/**
 * Outputs a file key_query_out.txt, each row consists of an integer which is the value
 * corresponds to the keys in query_keys; or -1 if the key is not found.
 */
void Index::key_query(vector<int>& keys) {
    for (int i = 0; i < keys.size(); i++) {
        keys[i] = b.query(keys[i]);
    }
    FILE* f = fopen("key_query_out.txt", "w");
    writeFile(keys, f);
    fclose(f);
}

/**
 * Outputs a file range_query_out.txt, each row consists of an integer which is the
 * MAXIMUM​ value in the given query key range; or -1 if no key found in the range.
 */
void Index::range_query(const vector<pair<int, int>>& keys) {
    vector<int> ans(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        ans[i] = b.query(keys[i].first, keys[i].second);
    }
    FILE* f = fopen("range_query_out.txt", "w");
    writeFile(ans, f);
    fclose(f);
}

void Index::clear_index() {
    // The b+ tree will be freed automatically when this Index object is destructed
    // So leave empty
}
