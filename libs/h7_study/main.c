#include <stdio.h>
#include <stdlib.h>
#include "lang/avl_tree.h"
#include "lang/rbtree.h"

#define LENGTH(a) ((sizeof(a))/(sizeof(a[0])))

/**
红黑树和 平衡2叉树: 区别
1，平衡2叉树： 严格的平衡树。 深度差不超过1. (适合查询多的)
2, 红黑树： 弱平衡。（适合删除，添加操作多的。）
*/
static void avl_tree_test(){
    avltree tree=NULL;
    int a[]={3,2,1,4,5,6,7,16,15,14,13,12,11,10,8,9};
    //int a[]={7,4,13,12,15,11};
    int length=LENGTH(a);
    for(int i=0;i<length;i++)
    {
        tree=avltree_insertNode(tree,a[i]);
    }


    int max_height=getNode_height(tree);
    printf("tree height is: %d\n",max_height);
    printf("travel pre_order:");
    pre_order_avltree(tree);

    printf("\n");
    printf("travel mid_order:");
    in_order_avltree(tree);
    printf("\n");
    printf("travel post_order:");
    post_order_avltree(tree);
    printf("\n");

    printf("min=%d\n",minimun_node(tree)->key);
    printf("max=%d\n",maximun_node(tree)->key);
    print_avltree(tree,tree->key,0);


    printf("delete node\n");
    tree=avltree_deleNode(tree,6);
    print_avltree(tree,tree->key,0);
}

#define CHECK_INSERT 0  // 1 means open
#define CHECK_DELETE 0
static void rbtree_test(){
    int a[] = {10, 40, 30, 60, 90, 70, 20, 50, 80};
    int i, ilen=LENGTH(a);
    RBRoot *root=NULL;

    root = create_rbtree();
    printf("== raw data: ");
    for(i=0; i<ilen; i++)
        printf("%d ", a[i]);
    printf("\n");

    for(i=0; i<ilen; i++)
    {
        insert_rbtree(root, a[i]);
#if CHECK_INSERT
        printf("== add node: %d\n", a[i]);
        printf("== tree detail: \n");
        print_rbtree(root);
        printf("\n");
#endif
    }

    printf("== travel pre-order: ");
    preorder_rbtree(root);

    printf("\n== travel mid-order: ");
    inorder_rbtree(root);

    printf("\n== travel post-order: ");
    postorder_rbtree(root);
    printf("\n");

    if (rbtree_minimum(root, &i)==0)
        printf("== min: %d\n", i);
    if (rbtree_maximum(root, &i)==0)
        printf("== max: %d\n", i);
    printf("== tree detail : \n");
    print_rbtree(root);
    printf("\n");

#if CHECK_DELETE
    for(i=0; i<ilen; i++)
    {
        delete_rbtree(root, a[i]);

        printf("== delete node: %d\n", a[i]);
        if (root)
        {
            printf("== tree detail: \n");
            print_rbtree(root);
            printf("\n");
        }
    }
#endif

    destroy_rbtree(root);
}

int main()
{
    avl_tree_test();
    rbtree_test();
}
