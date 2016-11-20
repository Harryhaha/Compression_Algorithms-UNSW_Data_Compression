//
// Created by Harry on 2016/3/31.
//
#include <iostream>
#include <string.h>
#include <queue>
#include <vector>
#include <algorithm>

using namespace std;

/****************** class declaration used by encode as well as decode ******************/
class AHTNode{
public:
    bool isLeafNode( AHTNode* node );

    void freeNode( AHTNode* root );

    void swapWithLeaderNode( AHTNode *&node, AHTNode *root );  //added

    void updateTree( char c, AHTNode *&root, bool existedData[] ); //added. used only by decoder

    void slideAheadOfBlock( AHTNode* node, vector<AHTNode*> followingBlock );

    void assignLeafAndItsParent( AHTNode *&leafToIncrement, AHTNode *&q, AHTNode *node ); //added

    void constructNewNode( AHTNode *&root, AHTNode *&NYT, AHTNode *&newNode, AHTNode *&internalNode, char c ); //added

    AHTNode* findNode(AHTNode *node, char ch);  //added. used only by decoder

    AHTNode* getCorrespondingNodeBFS(char c, AHTNode* root);

    AHTNode* findNYTNode(AHTNode* root);

    AHTNode* slideAndIncrement(AHTNode* node, AHTNode* root);

    AHTNode* getLeaderNodeWithinBlock( AHTNode* subNode, bool leafFlag, AHTNode* root );

    vector<AHTNode*> findFollowingBlock( AHTNode* targetNode, AHTNode* root, int type );

    vector<AHTNode*> getFollowingBlock( AHTNode* node, AHTNode* root );


    unsigned int weight;
    char data;
    struct AHTNode *parent, *lchild, *rchild;
    AHTNode( char data ){
        this->parent = NULL;
        this->lchild = NULL;
        this->rchild = NULL;
        this->weight = 0;
        this->data = data;
    }
};
/****************** class declaration used by encode as well as decode ******************/

