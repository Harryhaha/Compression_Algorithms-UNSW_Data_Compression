//
// Created by Harry on 2016/3/19.
//
#include <iostream>
#include <queue>
#include <algorithm>
#include <stack>
#include <string.h>
#include "ahtree.h"

using namespace std;

//void printTree(AHTNode *node, int level){
//    if (!node)             //null node, return to previous level
//    {
//        return;
//    }
//    printTree(node->rchild,level+1);    //print right child tree, level+1
//    for (int i=0;i<level;i++)    //recursively print spaces
//    {
//        printf("   ");
//    }
//
//    if (!node->isLeafNode(node)){
//        cout<<":"<<to_string(node->weight)<<endl;
//    }else{
//        cout<<node->data<<":"<<to_string(node->weight)<<endl;
//    }
//
//    printTree(node->lchild,level+1);    //print left child tree, level+1
//}

string binary(char x){
    std::string s;
    for(int i=0; i<8; i++){
        s.push_back('0' + (x & 1));
        x >>= 1;
    }
    std::reverse(s.begin(), s.end());
    return s;
}
string get_code(AHTNode *node, char c, bool existOrNot){  
    string subCode = "";
    stack<char> stack;
    while(node->parent!=NULL)
    {
        if(node->parent->lchild==node) stack.push('0');
        else if(node->parent->rchild==node) stack.push('1');
        node=node->parent;
    }
    while(!stack.empty())
    {
        if(stack.top()=='0') subCode+="0";
        else if(stack.top()=='1') subCode+="1";
        stack.pop();
    }

    if( existOrNot == false ) subCode = subCode + binary( c );
    return subCode;
}

int main( int argc, char *argv[] ) {
    bool format = ( argc == 2 && string(argv[1]).compare("-s")==0 ) ? true : false;
    AHTNode *root;
    bool existedData[256];
    string code;

    string line;
    char c;

    while( getline(cin,line) ){
        code = ""; //reset the result encode for each line
        AHTNode *newNode; AHTNode *internalNode;
        root = new AHTNode( 0 ); //reset

        for( int i=0; i<line.length(); ++i ){
            c = line[i];
            AHTNode *leafToIncrement = NULL; AHTNode *q = NULL; // Initialization

            if( existedData[c+128] == false ){    //new char occurs
                existedData[c+128] = true;

                // Replace NYT node by an internal O-node with two leaf O-node children, such that the right child corresponds to new node,,
                AHTNode *NYT = root->findNYTNode( root );

                //code generation here:
                code += get_code( NYT, c, false );
                if( format == true &&  i!=(line.length()-1) ) code += " ";

                root->constructNewNode( root, NYT, newNode, internalNode, c );
                // q is internalNode, while leafToIncrement is newNode
                leafToIncrement = newNode; q = internalNode;
            }else{                             //existed char
                AHTNode *node = root->getCorrespondingNodeBFS( c, root );

                //code generation here:
                code += get_code( node, 0, true );
                if( format == true &&  i!=(line.length()-1) ) code += " ";

                root->swapWithLeaderNode( node, root );
                q = node;  // IMPORTANT!
                // now q is refer to 'node' now
                root->assignLeafAndItsParent( leafToIncrement, q, node );
            }

            while( q->parent != NULL ){
                q = root->slideAndIncrement( q, root );
            }
            q->weight = ++(q->weight);   // added

            if( leafToIncrement != NULL ) root->slideAndIncrement( leafToIncrement, root );
        }
        cout << code <<endl;                 // print out the code for this round(line)
        memset( existedData, false, 256 );  //reset the existedData

        root->freeNode( root );
        //printTree(root, 0);
    }
}
