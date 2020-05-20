#include "index.h"

#include <bits/stdc++.h>

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
#define normal
using namespace std;

// --------------- Bplus tree ----------------

// An internal node has at most M children
const int M = 127;

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
    /*if (!root) return;
    vector<void*> p = {root};
    vector<void*> q;
    for (int i = 0; i < level; i++) {
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
    }*/
}

void BPlusTree::insert(int k, int v) {
    // cout << '.' << flush;
    // Check empty tree
    if (!root) {
        Leaf* l = new Leaf();
        l->prev = l->next = NULL;
        l->entries.emplace_back(k, v);
        root = l;
        // cout << '*' << endl;
        return;
    }
    // cout << '#' << flush;

    // Find leaf node
    vector<Internal*> layer;
    void* _n = root;
    // cout << ':' << flush;
    for (int i = 0; i < level; i++) {
        Internal* inode = (Internal*)_n;
        // assert(inode->keys.size() >= 1);
        // cout << inode->keys[0] << ':' << flush;
        layer.push_back(inode);
        _n = inode->findChildByKey(k);
    }
    // cout << '#' << flush;

    // Insert entry into leaf node
    Leaf* lnode = (Leaf*)_n;
    // cout << '!' << flush;
    if (lnode->entries.size() + 1 <= M) {
        // cout << '?' << flush;
        lnode->insert(k, v);
        // cout << '*' << endl;
        return;
    }
    // cout << '#' << flush;

    // Check splitting to top
    int kick;
    void* split = lnode->insertAndSplit(k, v, kick);
    for (int i = level - 1; i >= 0; i--) {
        // assert(!layer.empty());
        // assert(i < layer.size());
        Internal* inode = layer[i];
        if (inode->keys.size() + 1 <= M - 1) {
            inode->insert(kick, split);
            // cout << '*' << endl;
            return;
        }
        split = inode->insertAndSplit(kick, split, kick);
        // assert(split);
    }
    // cout << '#' << flush;

    // Root is splitted
    if (split) {
        Internal* newRoot = new Internal();
        newRoot->keys.push_back(kick);
        newRoot->children.push_back(split);
        newRoot->children.push_back(root);
        root = newRoot;
        level++;
        // cout << '^' << flush;
    }

    // cout << '#' << flush;
    // cout << '*' << endl;
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
    for (int i = 0; i < level; i++) {
        _n = ((Internal*)_n)->findChildByKey(l);
    }

    int ret = MIN_INT;
    Leaf* lnode = (Leaf*)_n;
    auto lix = lower_bound(all(lnode->entries), l, [](const entry& e, const int& n) {
        return e.key < n;
    });
    for (; lix != lnode->entries.end(); lix++) {
        if (lix->key <= r)
            ret = max(ret, lix->value);
        else
            return ret == MIN_INT ? -1 : ret;
    }

    lnode = lnode->next;
    for (; lnode; lnode = lnode->next) {
        auto lix = lnode->entries.begin();
        for (; lix != lnode->entries.end() && lix->key <= r; lix++) {
            ret = max(ret, lix->key);
        }
    }
    return ret == MIN_INT ? -1 : ret;
}

// --------------- Leaf ----------------

Leaf::Leaf() {
    // entries.reserve(M);
}

inline void Leaf::insert(int key, int value) {
    // assert(entries.size() + 1 <= M);
    auto a = upper_bound(all(entries), key, [](const int& n, const entry& e) {
        return n < e.key;
    });
    entries.insert(a, entry(key, value));
    // assert(is_sorted(all(entries)));
    // assert(is_sorted(all(entries)));
}

inline Leaf* Leaf::insertAndSplit(int key, int value, int& kick) {
    // cout << 's' << flush;
    // assert(entries.size() == M);
    const int reqSize = (M + 1) / 2;  // floor
    const int remSize = M + 1 - reqSize;
    // assert(reqSize + remSize == M + 1);

    Leaf* ret = new Leaf();
    int j = 0;
    bool put = 0;
    for (int i = 0; i < reqSize; i++) {
        // assert(j != entries.size());
        if (put || entries[j].key < key) {
            // assert(j < entries.size());
            ret->entries.push_back(entries[j++]);
        } else {
            ret->entries.emplace_back(key, value);
            put = 1;
        }
    }
    // assert(ret->entries.size() == reqSize);

    for (int i = 0; i < remSize; i++) {
        // assert(j <= entries.size());
        if (j == entries.size() || (!put && key < entries[j].key)) {
            // assert(!put);
            entries[i].key = key;
            entries[i].value = value;
            put = 1;
        }  //
        else {
            // assert(j < entries.size());
            // assert(i < entries.size());
            entries[i] = entries[j++];
        }
    }
    // assert(remSize <= entries.size());
    entries.resize(remSize);

    ret->next = this;
    ret->prev = prev;
    if (prev) prev->next = ret;
    prev = ret;

    // assert(entries.size() >= 1);
    // assert(ret->entries.size() >= 1);
    // assert(entries.size() + ret->entries.size() == M + 1);
    kick = entries[0].key;
    // cout << 'k' << kick << flush;

    /*  cout << endl;
    for (auto& e : entries) cout << e.key << ' ';
    cout << endl;
    for (auto& e : ret->entries) cout << e.key << ' ';
    cout << endl;*/

    // assert(is_sorted(all(entries)));
    // assert(is_sorted(all(ret->entries)));
    for (auto& e : ret->entries) // assert(e.key < kick);
    for (auto& e : entries) // assert(e.key >= kick);
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
    // children.reserve(M);
    // keys.reserve(M - 1);
}

inline void Internal::insert(int key, void* child) {
    // assert(keys.size() + 1 <= M - 1);
    auto a = upper_bound(all(keys), key);
    int index = a - keys.begin();
    keys.insert(a, key);
    // cout << 'i' << index << 'i' << flush;
    children.insert(children.begin() + index, child);
    // cout << '@' << flush;
    // assert(children.size() == keys.size() + 1);
    // assert(is_sorted(all(keys)));
}

inline Internal* Internal::insertAndSplit(int key, void* newChild, int& kick) {
    // cout << "Get " << key << endl;
    // if (key == 602842) cout << "New " << ((Internal*)newChild)->keys[0] << endl;
    // cout << 'S' << flush;
    // assert(keys.size() + 1 == M);
    // assert(children.size() == M);
    const int reqSize = M / 2;  // ceil;
    const int remSize = M - 1 - reqSize;
    // assert(reqSize + remSize == M - 1);

    Internal* ret = new Internal();
    int kp = 0, cp = 0;
    bool put = 0;
    for (int i = 0; i < reqSize; i++) {
        // assert(kp < keys.size());
        if (put || keys[kp] < key) {
            // assert(kp < keys.size());
            ret->keys.push_back(keys[kp++]);
            // assert(cp < children.size());
            ret->children.push_back(children[cp++]);
        }  //
        else {
            ret->keys.push_back(key);
            ret->children.push_back(newChild);
            put = 1;
        }
    }

    // assert(kp < keys.size());
    if (put || keys[kp] < key) {
        kick = keys[kp++];
        ret->children.push_back(children[cp++]);
    } else {
        kick = key;
        ret->children.push_back(newChild);
        put = 1;
    }

    // assert(ret->keys.size() == reqSize);
    // assert(ret->children.size() == ret->keys.size() + 1);
    // cout << "Kick " << kick << endl;

    int j = 0;
    for (int i = 0; i < remSize; i++, j++) {
        // assert(kp <= keys.size());
        if (kp == keys.size() || (!put && key < keys[kp])) {
            // assert(i < keys.size());
            keys[i] = key;
            // assert(j < children.size());
            children[j++] = newChild;
            // assert(j < children.size());
            // assert(cp < children.size());
            children[j] = children[cp++];
            put = 1;
        } else {
            // assert(kp < keys.size());
            // assert(i < keys.size());
            keys[i] = keys[kp++];
            // assert(cp < children.size());
            // assert(j < children.size());
            children[j] = children[cp++];
        }
    }
    if (cp < M) {
        children[j] = children[cp++];
    }
    // cout << cp << ' ' << M << endl;
    // assert(cp == M);
    // assert(remSize <= keys.size());
    keys.resize(remSize);
    // assert(remSize + 1 <= children.size());
    children.resize(remSize + 1);

    // assert(children.size() == keys.size() + 1);
    // assert(keys.size() + ret->keys.size() == M - 1);
    // assert(children.size() + ret->children.size() == M + 1);
    // for (auto k : keys) cout << k << ' ';
    // cout << endl;
    // assert(is_sorted(all(keys)));
    // assert(is_sorted(all(ret->keys)));
    return ret;
}

inline void* Internal::findChildByKey(int key) const {
    // assert(keys.size() >= 1);
    // cout << 'f' << flush;
    auto a = upper_bound(all(keys), key);
    int index = a - keys.begin();
    // cout << index << 'f' << flush;
    // assert(index < children.size());
    return children[index];
}

// --------------- Index ----------------

/**
 * Constructs a B+ tree index by inserting the key-value pairs into the B+ tree ​one by 
 * one​.
 */
Index::Index(int num_rows, const vector<int>& keys, const vector<int>& values) {
    // One by one
    for (int i = 0; i < keys.size() /*1000*/; i++) {
        b.insert(keys[i], values[i]);
       // cout << i + 1 << ' ';
       // b.test(i + 1);
    }
}

/**
 * Outputs a file key_query_out.txt, each row consists of an integer which is the value
 * corresponds to the keys in query_keys; or -1 if the key is not found.
 */
void Index::key_query(const vector<int>& keys) {
#ifdef normal
    ofstream o;
    o.open("key_query_out.txt");
    for (int i = 0; i < keys.size(); i++) o << b.query(keys[i]) << '\n';
    o.close();
#endif
#ifdef fast
    /*FILE* f = fopen("key_query_out.txt", "w");

    char file_buffer[CHUNK_SIZE + 64];
    int buffer_count = 0;
    int i = 0;
    const int cnt = keys.size();

    while (i < cnt) {
        buffer_count += sprintf(&file_buffer[buffer_count], "%d\n", b.query(keys[i]));
        i++;
        if (buffer_count >= CHUNK_SIZE) {
            fwrite(file_buffer, CHUNK_SIZE, 1, f);
            buffer_count -= CHUNK_SIZE;
            memcpy(file_buffer, &file_buffer[CHUNK_SIZE], buffer_count);
        }
    }

    if (buffer_count > 0) {
        fwrite(file_buffer, 1, buffer_count, f);
    }

    fclose(f);*/
    vector<int> ans;
    for (int i = 0; i < keys.size(); i++) ans.push_back(b.query(keys[i]));
    FILE* f = fopen("key_query_out.txt", "w");
    writeFile(ans, f);
    fclose(f);
#endif
#ifdef fastarray
    int arr[keys.size()];
    for (int i = 0; i < keys.size(); i++) arr[i] = b.query(keys[i]);
    FILE* f = fopen("key_query_out.txt", "w");
    writeFile(arr, keys.size(), f);
    fclose(f);
#endif
}

/**
 * Outputs a file range_query_out.txt, each row consists of an integer which is the
 * MAXIMUM​ value in the given query key range; or -1 if no key found in the range.
 */
void Index::range_query(const vector<pair<int, int>>& keys) {
#ifdef normal
    ofstream o;
    o.open("range_query_out.txt");
    for (int i = 0; i < keys.size(); i++) o << b.query(keys[i].first, keys[i].second) << '\n';
    o.close();
#endif
#ifdef fast
    vector<int> ans;
    for (int i = 0; i < keys.size(); i++) ans.push_back(b.query(keys[i].first, keys[i].second));
    FILE* f = fopen("range_query_out.txt", "w");
    writeFile(ans, f);
    fclose(f);
#endif
#ifdef fastarray
    int arr[keys.size()];
    for (int i = 0; i < keys.size(); i++) arr[i] = b.query(keys[i].first, keys[i].second);
    FILE* f = fopen("range_query_out.txt", "w");
    writeFile(arr, keys.size(), f);
    fclose(f);
#endif
}

void Index::clear_index() {
    // The b+ tree will be freed automatically when this Index object is destructed
    // So leave empty
}
