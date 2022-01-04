#ifndef RBTREE_H
#define RBTREE_H
/**
  性质：
1,列表项结点是红色或黑色。
2, 根是黑色。
3, 所有叶子都是黑色（叶子是NIL结点）。
4, 每个红色结点必须有两个黑色的子结点。（从每个叶子到根的所有路径上不能有两个连续的红色结点。）
5, 从任一结点到其每个叶子的所有简单路径都包含相同数目的黑色结点。
为了便于处理红黑树中的边界情况，使用一个哨兵来代表所有的NIL结点，也就是说所有指向NIL的指针都指向哨兵T.nil。
*/

#define RED      0    // 红色节点
#define BLACK    1    // 黑色节点

typedef int Type;

// 红黑树的节点
typedef struct RBTreeNode{
    unsigned char color;        // 颜色(RED 或 BLACK)
    Type   key;                 // 关键字(键值)
    struct RBTreeNode *left;    // 左孩子
    struct RBTreeNode *right;   // 右孩子
    struct RBTreeNode *parent;  // 父结点
}Node, *RBTree;

// 红黑树的根
typedef struct RBRoot{
    Node *node;
}RBRoot;

// 创建红黑树，返回"红黑树的根"！
RBRoot* create_rbtree();

// 销毁红黑树
void destroy_rbtree(RBRoot *root);

// 将结点插入到红黑树中。插入成功，返回0；失败返回-1。
int insert_rbtree(RBRoot *root, Type key);

// 删除结点(key为节点的值)
void delete_rbtree(RBRoot *root, Type key);


// 前序遍历"红黑树"
void preorder_rbtree(RBRoot *root);
// 中序遍历"红黑树"
void inorder_rbtree(RBRoot *root);
// 后序遍历"红黑树"
void postorder_rbtree(RBRoot *root);

// (递归实现)查找"红黑树"中键值为key的节点。找到的话，返回0；否则，返回-1。
int rbtree_search(RBRoot *root, Type key);
// (非递归实现)查找"红黑树"中键值为key的节点。找到的话，返回0；否则，返回-1。
int iterative_rbtree_search(RBRoot *root, Type key);

// 返回最小结点的值(将值保存到val中)。找到的话，返回0；否则返回-1。
int rbtree_minimum(RBRoot *root, int *val);
// 返回最大结点的值(将值保存到val中)。找到的话，返回0；否则返回-1。
int rbtree_maximum(RBRoot *root, int *val);

// 打印红黑树
void print_rbtree(RBRoot *root);

#endif // RBTREE_H
