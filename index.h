#include <utility>
#include <vector>
using namespace std;
typedef pair<int, int> entry;

class Node {
   public:
    Node *l = NULL, *m = NULL, *r = NULL;
    entry *e1 = NULL, *e2 = NULL;
    bool leaf = false;
    Node(bool leaf);
    Node(int k);
    Node(int k, int v);
    ~Node();
    inline int size() const;
};

class BPlusTree {
   private:
    Node* root = NULL;
    int level = 0;

   public:
    BPlusTree();
    ~BPlusTree();
    void insert(int k, int v);
    int query(const int k) const;
    int query(const int from, const int to) const;
};

class Index {
   private:
    BPlusTree b;
    int level = 0;

   public:
    Index(int num_rows, const vector<int>& keys, const vector<int>& values);
    void key_query(const vector<int>& keys);
    void range_query(const vector<pair<int, int>>& keys);
    void clear_index();
};