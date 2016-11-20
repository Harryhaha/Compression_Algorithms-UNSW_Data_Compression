#include <iostream>
#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <list>
#include <vector>

using namespace std;

FILE * pFile;
int totalCharNo;  // the no. of char in source BWT file

char transferCode( char c ){                 // compressed to the range 0-97
    if( c >= 32 && c <= 126 ) return c-29;   // 32-126 transferred to be 3-97
    else if( c == 9 ) return 0;             // 9 transferred to be 0
    else if( c == 10 ) return 1;            // 10 transferred to be 1
    else return 2;                          // 13 transferred to be 2
}

int getOcc(char ch, int index, int** wholeArray, int segmentSize ){
    if( index == -1 ) return 0;

    int c;
    int occurrence = 0, tmpSubOccurrence = 0, offset = 0;

    int block = index / segmentSize;
    if( block > 0 ) occurrence = wholeArray[block-1][ch]; // get the occurrence before the last block

    bool reverse = false;
    if( (index+1) < totalCharNo ){
        if( (index-block*segmentSize) >= segmentSize/2 ){
            offset = segmentSize - index + block * segmentSize - 1;
            fseek(pFile, index+1, 0);
            reverse = true;
        }else{
            offset = index - block * segmentSize + 1;
            fseek(pFile, block*segmentSize, 0);
        }

        // iterate the remaining block to count the occurrence
        for(int i=0; i<offset; i++){
            c = fgetc( pFile );
            if( c == EOF ) break;
            c = transferCode(c);
            if( c == ch ){
                tmpSubOccurrence += 1;
            }
        }
        if( reverse == true ){
            if( block != 0 ) tmpSubOccurrence = wholeArray[block][ch] - wholeArray[block-1][ch] - tmpSubOccurrence;
            else tmpSubOccurrence = wholeArray[block][ch] - tmpSubOccurrence;
        }
    }
    else{
        tmpSubOccurrence = wholeArray[block][ch] - wholeArray[block-1][ch];
    }

    occurrence += tmpSubOccurrence;
    return occurrence;
}

int getBucketNumber(float MegaByte, int bucketSize){
    return int( MegaByte*1024*1024 / bucketSize );
}

//int getFileSize( const char* fileName ){
//    ifstream mySource;
//    mySource.open(fileName, ios_base::binary);
//    mySource.seekg(0,ios_base::end);
//    int size = mySource.tellg();
//    mySource.close();
//    return size;
//}

int getSegmentSize( int fileSize, int bucketNumber ){
    int segmentSize = fileSize * 1024 * 1024 / bucketNumber;
    int remaining = fileSize % bucketNumber;
    if( remaining != 0 ) segmentSize += 1;
    return segmentSize;
}

int** getSegmentArray( int* bucketNumber, int segmentSize ){
    // 2 dimensional array Initialization, this will satisfy the runtime memory requirement
    int **segmentsArray = new int *[*bucketNumber];
    for (int i = 0; i < *bucketNumber; i++) {
        segmentsArray[i] = new int[98]();
//        for (int j = 0; j < 98; j++) {
//            segmentsArray[i][j] = 0;
//        }
    }

//    pFile = fopen (fileName,"r");
    int c;
    int segmentOffset = 0; int segmentNo = 0;
    c = fgetc( pFile );
    while(c != EOF){
        totalCharNo += 1;                   // to get total number of char
        c = transferCode(c);
        segmentsArray[segmentNo][c] += 1;   // update the count of char for each block
        segmentOffset += 1;
        if( segmentOffset == segmentSize ){
            segmentOffset = 0;
            segmentNo += 1;                 // update the block to be the next
        }
        c = fgetc( pFile );
    }

    // prune the unused buckets so that does not need to iterate them later
    if( totalCharNo % segmentSize == 0 ) *bucketNumber =  segmentNo;
    else *bucketNumber =  segmentNo + 1;

    return segmentsArray;
}

int** getAccumSegs(int** segmentsArray, int bucketNumber){
    for( int i=1; i<bucketNumber; i++){
        for( int j=0; j<98; j++){
            segmentsArray[i][j] += segmentsArray[i-1][j];
        }
    }
    return segmentsArray;
}

int* getC( int*C, int** segmentsArray, int bucketNumber ){
    for(int j=0; j<98; j++) {
        C[j] = segmentsArray[bucketNumber-1][j];
    }

    int count = 0;
    int value = 0, copyCount = 0;
    for(int j=0; j<98; j++){
        if( C[j] == 0 ) continue;
        copyCount =  C[j];
        C[j] = count+value;
        value = C[j];
        count = copyCount;
    }
    return C;
};

vector<int> findIdentifiers( int first, int last, int** segmentsArray,  int segmentSize, int* C ){
    vector<int> IDVector;
    //list<int> IDList;

    int ch = 0;
    int backwardChIndex = 0;
    bool skipFlag = false;
    for(int j=first; j<last+1; j++){
        skipFlag = false;
        int ID = 0, digit  = 1;
        backwardChIndex = j;
        bool IDFlag = false;
        fseek(pFile, j, 0); ch = fgetc( pFile );    //get the first corresponding char within bwt to trace back
        while( ch != '[' ){
            if( ch == ']' ) IDFlag = true;

            if( IDFlag == true && ch != '[' && ch != ']'){
                ID = digit*(ch - '0') + ID;
                digit *= 10;
            }

            ch = transferCode(ch);
            backwardChIndex = C[ch] + getOcc(ch, backwardChIndex, segmentsArray, segmentSize ) - 1; // get the index of previous char for original text
            fseek(pFile, backwardChIndex, 0);
            ch = fgetc( pFile );    //get the previous char during tracing back process

            if( (backwardChIndex>=first) && (backwardChIndex<=last) ){
                skipFlag = true; break;  // skip the current one since there is a matching occurred before, which share the same offset
            }
        }

        if( skipFlag == true ) continue;

        //IDList.push_back(i);
        IDVector.push_back(ID);
    }
    //return IDList;
    return IDVector;
}


int main( int argc, char *argv[] ) {
    string p(argv[4]);
    for(int i=0; i<p.length(); i++){
        p[i] = transferCode(p[i]);
    }
    pFile = fopen ( argv[2], "r" );

    //performance below:
    int bucketNumber = getBucketNumber(2.7, 98);
    //int segmentSize = getSegmentSize( 160, bucketNumber);
    int segmentSize = 6000;  // fixed
    int **segmentsArray = getSegmentArray( &bucketNumber, segmentSize ); // get each block info with the sub occurrence for each char
    segmentsArray = getAccumSegs(segmentsArray, bucketNumber);                   // update the occurrence to accumulate all before occurrence for each char

    int* C = new int[98]();
    C = getC(C, segmentsArray, bucketNumber);                                    // get the C
    //performance above:

    int index = p.length()-1;
    char ch = p[index];
    int first = C[ch];
    int last = 0, nextCharCount = 0;
    // if current char is the final one, then last would be (no. of char-1)
    for(int i=ch+1; i<98; i++){
        if( C[i] != 0 ){
            nextCharCount = C[i]; break;
        }
    }
    if( nextCharCount != 0 ) last = nextCharCount - 1;
    else last = totalCharNo - 1;

    while( (first <= last) && (index >= 1) ){
        ch = p[index-1];
        first = C[ch] + getOcc(ch, first-1, segmentsArray, segmentSize );
        last = C[ch] + getOcc(ch, last, segmentsArray, segmentSize) - 1;
        index = index-1;
    }

    if(last < first){                           // No match
        cout << "no match" << endl;  return 0;
    }

    if( string(argv[1]).compare("-n") == 0 ) {  // -n to count all matches include duplicity
        fclose(pFile);
        cout << (last-first+1) << endl;  return 0;
    }

    //list<int> IDList = findIdentifiers( first, last, segmentsArray, segmentSize, C, argv[2]);
    vector<int> IDVector = findIdentifiers( first, last, segmentsArray, segmentSize, C );
    fclose(pFile);

    if( string(argv[1]).compare("-r") == 0 ){  // -r to count unique matches for one record
        cout << IDVector.size() << endl;  return 0;
    }

    if( string(argv[1]).compare("-a") == 0 ) {  // -a to print belonging identifier
        sort( IDVector.begin(),IDVector.end() );
        //IDVector.sort();

        for( vector<int>::iterator iter=IDVector.begin(); iter != IDVector.end(); ++iter){
            cout << "[" << *iter << "]" << endl;
        }
        return 0;
    }
}