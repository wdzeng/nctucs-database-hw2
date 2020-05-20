#include <utility>
#include <vector>
using namespace std;
typedef pair<int, int> entry;

/*
template <typename E>
class Array {
   private:
    E store[M];
    int size = 0;

   public:
    Array();
    Array(const E& e) { add(e); }
    inline void add(const E& e) {
        // assert(size < M);
        store[size++] = e;
    }
    inline E& operator[](int index) { return store[index]; }
    inline const E& operator[](int index) const { return [index]; }
    inline int length() const { return size; }
    inline Array addSplit(const E& e) {
        // assert(size == M);
        Array<E> ret;
        const int requireSplitSize = M / 2;
        int i = 0;
        for (int j = 0; j < requireSplitSize; j++) {
            if (e < store[i])
                ret.add(e);
            else
                ret.add(store[i++]);
        }
        int k = 0;
        while (i < M) {
            if (e < store[i])
                store[k++] = e;
            else
                store[k++] = store[i++];
        }
        size = M - requireSplitSize;
        return ret;
    }
};
*/
class Internal {
   public:
    Internal();
    vector<void*> children;
    vector<int> keys;
    inline void insert(int key, void* child);
    inline Internal* insertAndSplit(int key, void* newChild, int& kick);
    inline void* findChildByKey(int key) const;
};

class Leaf {
   public:
    vector<entry> entries;
    Leaf* prev = NULL;
    Leaf* next = NULL;
    Leaf();
    inline void insert(int key, int value);
    inline Leaf* insertAndSplit(int key, int value, int& kick);
    inline int query(int key) const;
};

class BPlusTree {
   private:
    void* root = NULL;
    int level = 0;

   public:
    BPlusTree();
    ~BPlusTree();
    void insert(int k, int v);
    int query(const int k) const;
    int query(const int from, const int to) const;
    void test(int s) const;
};

class Index {
   private:
    BPlusTree b;

   public:
    Index(int num_rows, const vector<int>& keys, const vector<int>& values);
    void key_query(const vector<int>& keys);
    void range_query(const vector<pair<int, int>>& keys);
    void clear_index();
};