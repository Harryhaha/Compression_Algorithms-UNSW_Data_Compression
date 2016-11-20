//
// Created by Harry on 2016/3/19.
//
#include <iostream>
#include <algorithm>
#include <string.h>
#include <sstream>
#include <bitset>
#include "ahtree.h"

using namespace std;

char transferBitToChar(string str){
    stringstream stream(str);
    bitset<8> bits;
    stream >> bits;
    char c = char(bits.to_ulong());
    return c;
}

int main( int argc, char *argv[] ) {
    AHTNode *root;
    bool existedData[256];
    string code;

    string line;
    while ( getline(cin, line) ) {
        code = "";               //reset the result encode for each line
        root = new AHTNode( 0 ); //reset
        string binaryString = ""; char ch = 0;  //initialization about internal variables for each line process

        string firstBinaryString = line.substr(0, 8);
        char firstChar = transferBitToChar( firstBinaryString );
        root->updateTree( firstChar, root, existedData );
        code += firstChar;
        if( line.length() == 8 ){
            cout << code << endl;
            memset( existedData, false, 256 );   //reset the existedData
            continue;                           // the line only has 8 bits which form only one char
        }

        AHTNode *beginNode = root;
        for( int i = 8; i < line.length(); ++i ){
            if( line[i] == ' ' ) continue;                    // if it is a whitespace, just ignore it

            AHTNode *node = root->findNode( beginNode, line[i] );
            if( node->data == 0 && node->lchild == NULL && node->rchild == NULL ){  // means NYT
                binaryString = line.substr(i+1, i+8);
                ch = transferBitToChar( binaryString );
                i = i+ 8;                                       // update the index
                root->updateTree( ch, root, existedData );
                code += ch;
                beginNode = root;                               // reset the node to be the root
            }
            else if( node->data == 0 ) beginNode = node;       // means internal node
            else{                                               // means data node
                ch = node->data;
                root->updateTree( ch, root, existedData );
                code += ch;
                beginNode = root;                                // reset the node to be the root
            }
        }

        cout << code << endl;
        memset( existedData, false, 256 );  //reset the existedData

        root->freeNode( root );
    }
}