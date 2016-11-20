//
// Created by Harry on 2016/3/20.
//
#include <iostream>
#include <queue>
#include <algorithm>
#include "ahtree.h"

using namespace std;

/***************** class methods definition used by encode as well as decode ****************/
bool AHTNode::isLeafNode( AHTNode* node ){
    if( (node -> lchild == NULL) && (node -> rchild == NULL) ) return true;
    else return false;
}

void AHTNode::freeNode( AHTNode* root ){
    if( root->lchild != NULL ) freeNode( root->lchild );
    if( root->rchild != NULL ) freeNode( root->rchild );
    delete root;
}

void AHTNode::swapWithLeaderNode( AHTNode *&node, AHTNode* root ){
    bool leafFlag = false;
    if( node->lchild == NULL && node->rchild == NULL ) leafFlag = true;
    AHTNode *leaderNode = root->getLeaderNodeWithinBlock( node, leafFlag, root );
    if( leaderNode != node ){  // if these two node are different, they should be swapped !
        AHTNode *currentParentNode = node -> parent;  // get the parent node of current one in advance

        // change the relationship of (current)node below:
        AHTNode *leaderParentNode =  leaderNode -> parent;
        if( leaderParentNode ){  //just make sure. actually it is unnecessary I think...
            node -> parent = leaderParentNode;
            if( leaderParentNode -> lchild == leaderNode ) leaderParentNode -> lchild = node;
            else leaderParentNode -> rchild = node;
        }

        // change the relationship of leaderNode below:
        leaderNode -> parent = currentParentNode;
        if( currentParentNode -> lchild == node ) currentParentNode -> lchild = leaderNode;
        else currentParentNode -> rchild = leaderNode;
        // swap finish
    }
}

void AHTNode::slideAheadOfBlock( AHTNode* node, vector<AHTNode*> followingBlock ){
    for( int i=0; i<followingBlock.size(); ++i ){
        if( node->parent->lchild == node ){  //left child
            node->parent->lchild = followingBlock[i];
            node->parent->rchild = node;
        }
        else{
            AHTNode *nodeParent = node->parent;
            AHTNode *swapNodeParent = followingBlock[i]->parent;

            node->parent = swapNodeParent;
            followingBlock[i]->parent = nodeParent;

            nodeParent->rchild = followingBlock[i];
            swapNodeParent->lchild = node;
        }
    }
}

void AHTNode::assignLeafAndItsParent( AHTNode *&leafToIncrement, AHTNode *&q, AHTNode *node ){
    AHTNode *parentNode = node->parent; AHTNode *siblingNode = NULL;
    if( parentNode -> lchild == node ) siblingNode = parentNode -> rchild;
    else siblingNode = parentNode -> lchild;

    //if q is the sibling of the O-node then (0-node include internal node and NYT node)
    if( siblingNode->data == 0 && siblingNode->lchild==NULL && siblingNode->rchild==NULL ){
        leafToIncrement = q;  q = q->parent;   //otherwise it is NULL which is initialized value
    }
}

void AHTNode::constructNewNode( AHTNode *&root, AHTNode *&NYT, AHTNode *&newNode, AHTNode *&internalNode, char c ){
    AHTNode *parentOfNYT = NYT->parent;
    newNode = new AHTNode( c );
    internalNode = new AHTNode( 0 );
    newNode -> parent = internalNode; NYT -> parent = internalNode; internalNode -> lchild = NYT; internalNode -> rchild = newNode;

    if( parentOfNYT ){
        internalNode -> parent = parentOfNYT;
        if( parentOfNYT->lchild == NYT ) parentOfNYT->lchild = internalNode;
        else parentOfNYT->rchild = internalNode;
    }
    else root = internalNode; //in and only in the first round must change the root from NYT to new internal node
    // q is internalNode, while leafToIncrement is newNode
}

AHTNode* AHTNode::findNYTNode(AHTNode* root){
    if( root->data == 0 && root->lchild == NULL && root->rchild == NULL ) return root;
    if( root->lchild ){
        AHTNode* resultLeft = findNYTNode( root->lchild );
        if( resultLeft ) return resultLeft;
    }
    if( root->rchild ){
        AHTNode* resultRight = findNYTNode( root->rchild );
        if( resultRight ) return resultRight;
    }
    return NULL;
}

AHTNode* AHTNode::getCorrespondingNodeBFS(char c, AHTNode* root){
    queue<AHTNode*> q;
    q.push(root);
    while( !q.empty() ){
        AHTNode *node = q.front();
        if( node->data == c ) return node;
        q.pop();
        if( node->lchild ) q.push(node->lchild);
        if( node->rchild ) q.push(node->rchild);
    }
}

AHTNode* AHTNode::slideAndIncrement(AHTNode* node, AHTNode* root){
    AHTNode *formerParentNode = node -> parent;
    // block following p's block in the linked list;
    AHTNode *tempNode = node;
    vector<AHTNode*> followingBlock = getFollowingBlock( node, root );
    // now already get the block following block, but need to check !!!

    if( followingBlock.size() != 0 ){
        // slide p in the tree ahead of the nodes in followingBlock;
        slideAheadOfBlock( node, followingBlock );

        node->weight = ++(node->weight);
        if( isLeafNode(node) ) node = node->parent;
        else node = formerParentNode;
    }
    else{
        node->weight = ++(node->weight);
        node = node->parent;
    }
    return node;
}

AHTNode* AHTNode::getLeaderNodeWithinBlock( AHTNode* subNode, bool leafFlag, AHTNode* root ){
    queue<AHTNode*> q;
    q.push(root);
    while( !q.empty() ){
        AHTNode *node = q.front();
        if( node->weight == subNode->weight ){
            if( leafFlag == true ) {  // the block should only contain the leaf node with same data
                if( (node->lchild == NULL) && (node->rchild == NULL) ) return node;
            }
            else return node;
        }
        q.pop();
        if( node->rchild ) q.push( node->rchild );
        if( node->lchild ) q.push( node->lchild );
    }
    //It is possible to get the same node passed in.
}

vector<AHTNode*> AHTNode::getFollowingBlock( AHTNode* node, AHTNode* root ){
    vector<AHTNode*> block; //result
    if( isLeafNode(node) ) block = findFollowingBlock(node, root, 1);
    else block = findFollowingBlock(node, root, 0);
    reverse( block.begin(), block.end() );
    return block;
}

vector<AHTNode*> AHTNode::findFollowingBlock( AHTNode* targetNode, AHTNode* root, int type ){
    vector<AHTNode*> block; // result
    queue<AHTNode*> queue;
    queue.push(root);
    while( !queue.empty() ){
        AHTNode *node = queue.front();
        if( node == targetNode ) return block;
        if( type == 1 ){        //node is a leaf node
            if( !isLeafNode(node) && (node->weight==targetNode->weight) ) block.push_back( node );
        }
        else{                   //node is a leaf node
            if( isLeafNode(node) && (node->weight==(targetNode->weight+1)) ) block.push_back( node );
        }

        queue.pop();
        if( node->rchild ) queue.push(node->rchild);
        if( node->lchild ) queue.push(node->lchild);
    }
}

/***************** class methods definition used by encode as well as decode ****************/


/************************ class method definition used by decode only ***********************/
AHTNode* AHTNode::findNode(AHTNode *node, char ch){
    if( ch == '0' ) return node->lchild;
    else return node->rchild;
}

void AHTNode::updateTree( char c, AHTNode *&root, bool existedData[] ){
    AHTNode *newNode; AHTNode *internalNode;
    AHTNode *leafToIncrement = NULL; AHTNode *q = NULL; // Initialization

    if( existedData[c+128] == false ){    //new char occurs
        existedData[c+128] = true;

        // Replace NYT node by an internal O-node with two leaf O-node children, such that the right child corresponds to new node,,
        AHTNode *NYT = root->findNYTNode( root );

        root->constructNewNode( root, NYT, newNode, internalNode, c );

//        AHTNode *parentOfNYT = NYT->parent;
//        AHTNode *newNode = new AHTNode( c );
//        AHTNode *internalNode = new AHTNode( '0' );
//        newNode -> parent = internalNode; NYT -> parent = internalNode; internalNode -> lchild = NYT; internalNode -> rchild = newNode;
//
//        if( parentOfNYT ){
//            internalNode -> parent = parentOfNYT;
//            if( parentOfNYT->lchild == NYT ) parentOfNYT->lchild = internalNode;
//            else parentOfNYT->rchild = internalNode;
//        }
//        else root = internalNode; //in and only in the first round must change the root from NYT to new internal node
        // q is internalNode, while leafToIncrement is newNode
        leafToIncrement = newNode; q = internalNode;

    }else{                             //existed char
        AHTNode *node = root->getCorrespondingNodeBFS( c, root );

        root->swapWithLeaderNode( node, root );
        q = node;  

        // now q is refer to 'node' now
        root->assignLeafAndItsParent( leafToIncrement, q, node );
    }

    while( q->parent != NULL ){
        q = root->slideAndIncrement( q, root );
    }
    q->weight = ++(q->weight);   // added

    if( leafToIncrement != NULL ) root->slideAndIncrement( leafToIncrement, root );
}
/************************ class method definition used by decode only ***********************/