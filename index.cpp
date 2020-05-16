#include "index.h"

#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;
typedef pair<int, int> entry;

inline entry* newEntry(int k, int v) {
    return new entry(k, v);
}

inline entry* newEntry(int k) {
    return new entry(k, 0);
}

Node::Node(bool leaf) : leaf(leaf) {}

Node::Node(int k) : e1(newEntry(k)) {}

Node::Node(int k, int v) : e1(newEntry(k, v)), leaf(true) {}

Node::~Node() {
    if (leaf) return;
    if (l) delete l;
    if (m) delete m;
    if (r) delete r;
    if (e1) delete e1;
    if (e2) delete e2;
}

inline int Node::size() const {
    return e2 ? 2 : 1;
}

BPlusTree::BPlusTree() {
}

BPlusTree::~BPlusTree() {
    if (root) delete root;
}

void BPlusTree::insert(int k, int v) {
    if (!root) {
        root = new Node(k, v);
        return;
    }

    vector<Node*> vec;
    Node* n = root;
    for (int i = 0; i < level; i++) {
        vec.push_back(n);
        // n is not leaf
        if (k < n->e1->first)
            n = n->l;
        else if (n->e2 == NULL || k < n->e2->first)
            n = n->m;
        else
            n = n->r;
    }

    // n is leaf
    if (k == n->e1->first) {
        n->e1->second = v;
        return;
    }
    if (n->size() == 1) {
        entry* e = newEntry(k, v);
        if (k < n->e1->first) {
            n->e2 = n->e1;
            n->e1 = e;
        }  //
        else {
            n->e2 = e;
        }
        return;
    }
    if (k == n->e2->first) {
        n->e2->second = v;
        return;
    }

    Node* prev = n->l;
    Node* split = NULL;
    if (k < n->e1->first) {
        split = new Node(k, v);
        split->e1 = newEntry(k, v);
    }  //
    else if (k < n->e2->first) {
        split = new Node(true);
        split->e1 = n->e1;
        n->e1 = newEntry(k, v);
    }  //
    else {
        split = new Node(true);
        split->e1 = n->e1;
        n->e1 = n->e2;
        n->e2 = newEntry(k, v);
    }
    k = n->e1->first;
    // printf("     kick up %d\n", k);
    split->l = prev;
    split->r = n;
    n->l = split;
    if (prev) prev->r = split;

    while (!vec.empty()) {
        Node* n = vec.back();
        vec.pop_back();
        // n is not leaf

        if (n->size() == 1) {
            if (k < n->e1->first) {
                n->e2 = n->e1;
                n->e1 = newEntry(k);
                n->r = n->m;
                n->m = n->l;
                n->l = split;
            }  //
            else {
                n->e2 = newEntry(k);
                n->r = n->m;
                n->m = split;
            }
            return;
        }

        Node* newSplit = new Node(false);
        if (k < n->e1->first) {
            newSplit->e1 = newEntry(k);
            newSplit->l = split;
            newSplit->m = n->l;
            k = n->e1->first;
            n->e1 = n->e2;
            n->l = n->m;
            n->m = n->r;
            n->r = NULL;
            n->e2 = NULL;
        }  //
        else if (k < n->e2->first) {
            newSplit->e1 = n->e1;
            newSplit->l = n->l;
            newSplit->m = split;
            n->l = n->m;
            n->m = n->r;
            n->r = NULL;
            n->e1 = n->e2;
            n->e2 = NULL;
        }  //
        else {
            int tmp = n->e2->first;
            newSplit->e1 = n->e1;
            newSplit->l = n->l;
            newSplit->m = n->m;
            n->l = split;
            n->m = n->r;
            n->r = NULL;
            n->e1 = newEntry(k);
            n->e2 = NULL;
            k = tmp;
        }
        split = newSplit;
    }

    if (split) {
        Node* newRoot = new Node(k);
        newRoot->l = split;
        newRoot->m = root;
        level++;
        root = newRoot;
    }
}

int BPlusTree::query(const int k) const {
    if (root == NULL) return -1;

    Node* n = root;
    for (int i = 0; i < level; i++) {
        if (k < n->e1->first) {
            n = n->l;
        } else if (n->e2 == NULL || k < n->e2->first) {
            n = n->m;
        } else {
            n = n->r;
        }
    }

    if (k == n->e1->first) return n->e1->second;
    if (n->e2 && k == n->e2->first) return n->e2->second;
    return -1;
}

int BPlusTree::query(const int from, const int to) const {
    if (root == NULL) return -1;

    Node* n = root;
    for (int i = 0; i < level; i++) {
        if (from < n->e1->first)
            n = n->l;
        else if (n->e2 == NULL || from < n->e2->first)
            n = n->m;
        else
            n = n->r;
    }

    int ret = -(1 << 30);
    while (n && n->e1->first <= to) {
        if (from <= n->e1->first && n->e1->first <= to) ret = max(ret, n->e1->second);
        if (n->e2 && from <= n->e2->first && n->e2->first <= to) ret = max(ret, n->e2->second);
        n = n->r;
    }
    return (ret == -(1 << 30)) ? -1 : ret;
}

/**
 * Constructs a B+ tree index by inserting the key-value pairs into the B+ tree ​one by 
 * one​.
 */
Index::Index(int num_rows, const vector<int>& keys, const vector<int>& values) {
    for (int i = 0; i < num_rows; i++) b.insert(keys[i], values[i]);
}

/**
 * Outputs a file key_query_out.txt, each row consists of an integer which is the value
 * corresponds to the keys in query_keys; or -1 if the key is not found.
 */
void Index::key_query(const vector<int>& keys) {
    // ios::sync_with_stdio(false);
    ofstream o;
    o.open("key_query_out.txt");
    for (int i = 0; i < keys.size(); i++) o << b.query(keys[i]) << '\n';
    o.close();
}

/**
 * Outputs a file range_query_out.txt, each row consists of an integer which is the
 * MAXIMUM​ value in the given query key range; or -1 if no key found in the range.
 */
void Index::range_query(const vector<pair<int, int>>& keys) {
    // ios::sync_with_stdio(false);
    ofstream o;
    o.open("range_query_out.txt");
    for (int i = 0; i < keys.size(); i++) o << b.query(keys[i].first, keys[i].second) << '\n';
    o.close();
}

void Index::clear_index() {
    // Leave empty
    // The b+ tree is freed automatically
}
