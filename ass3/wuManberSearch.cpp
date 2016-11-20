#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <map>
#include <ctype.h>
#include <queue>
#include <cctype>
#include <sys/stat.h>
#include <unordered_map>

//Windows
//#include<io.h>
//Linux
#include <unistd.h>
#include <dirent.h>

using namespace std;

// the struct is used as element to build the prefix table during preprocessing before Wu Manber Search
typedef struct node {
    unsigned short prefixHash;  // prefix hash value
    string pattern;               // pattern string
    node *pNodeNext;              // next node pointer
} PNode, *PrefixNode;

int block;                        // refer to Wu Manber algorithm, it is always 2 or 3 bytes, here choose 2
int shortestLen;                  // the shortest length of search pattern(s)
unsigned char *shiftArray;
PrefixNode *prefixArray;           // used to match prefix first when the last two chars matched some pattern(s)
int fileBlock = 6 * 1024 * 1024;  // segment the file to read to meet the memory requirement

struct sortFreqDescendingAndFilenameAscending {
    bool operator()( pair<float,string> &leftPair, pair<float,string> &rightPair) {
        if( leftPair.first != rightPair.first )
            return leftPair.first > rightPair.first;
        else
            return leftPair.second < rightPair.second;
    }
};

//Windows
//vector<string> getAllFilesWin( string path ) {
//    vector<string> files;//store all filenames
//
//    long hFile   =   0; //file handler
//    struct _finddata_t fileinfo; //file info
//    string p;
//    if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1) {
//        do {
//            // if it is directory, iterate it; if not, add it to vector
//            if((fileinfo.attrib &  _A_SUBDIR)) continue;
//            else files.push_back( fileinfo.name );
//
//        } while(_findnext(hFile, &fileinfo)  == 0);
//        _findclose(hFile);
//    }
//
//    return files;
//}
//Linux
vector<string> getAllFilesLinux( string path ) {
    vector<string> files; //store all filenames
    DIR *directory;
    struct dirent *dirPtr;
    directory=opendir( path.c_str() );
    while ((dirPtr=readdir(directory)) != NULL) {
        if(strcmp(dirPtr->d_name,".")==0 || strcmp(dirPtr->d_name,"..")==0) continue;  //current dir or parrent dir
        else if(dirPtr->d_type == 8)  files.push_back(dirPtr->d_name);   //file type
    }
    closedir(directory);

    //sorting from small to large alphabetically
    sort(files.begin(), files.end());
    return files;
}

int getFileSizeWindows( FILE *pFile ) {
    fseek(pFile,0,SEEK_END);
    int size = ftell(pFile);
    rewind(pFile);
    return size;
}
int getFileSizeLinux( const char *filename ) {
    struct stat statbuf;
    stat( filename, &statbuf );
    return statbuf.st_size;
}

/******************************************** unused now below *************************************************/
int prefixToIndex( string prefixTwo ){
    char secondBit = prefixTwo[0] - 97;
    char firstBit = prefixTwo[1] - 97;
    return 26*secondBit + firstBit;
}
string indexToPrefix( int index ){
    string prefixTwo = "";
    int secondIndex = index / 26;
    int firstIndex = index % 26;
    char secondChar = secondIndex + 97;
    char firstChar = firstIndex + 97;
    prefixTwo += secondChar;
    prefixTwo += firstChar;
    return prefixTwo;
}
/******************************************** unused now above *************************************************/


void freeResources() {
    delete[] shiftArray;
    delete[] prefixArray;
}

void InsertNode( unsigned short wordHash, string pattern, unsigned short prefixHash ) {
    PrefixNode pNode = new PNode;
    pNode->pattern = pattern;                   // whole pattern string
    pNode->pNodeNext = prefixArray[wordHash];
    pNode->prefixHash = prefixHash;             // hash value of the prefix
    prefixArray[wordHash] = pNode;
}

void BuildShiftTable( vector<string> patterns ) {
    shiftArray = new unsigned char[65536];  // use array to store, element is unsigned char type
    prefixArray = new PrefixNode[65536];      // element is node struct pointer
    // Shift array
    //cout << shortestLen << endl;
    for (int i = 0; i < 65536; i++) {
        shiftArray[i] = shortestLen - 1;
    }
    // Hash table: initialize the shiftArray and prefixArray
    memset( prefixArray, 0, 65536 * sizeof(prefixArray[0]) ); //prefixArray[0] is acutally the size of each PHONE struct !
    for (unsigned int i = 0; i < patterns.size(); i++) {
        for (int j = 0; j < shortestLen - block + 1; j++) {
            // get every two consecutive chars before shortestLen for each search pattern and
            // calculate their (shortest, if multiple exist) distance to the last char of shortestLen
            unsigned short wordHash = *(unsigned short *) ( &patterns[i][shortestLen - 1 - j - 1] );
            if (shiftArray[wordHash] > j) {
                shiftArray[wordHash] = j;
            }
            // for each search pattern corresponding to last two chars before shortestLen,
            // put it and its first 2 chars prefix into the prefixArray and form
            // the linklist by its referencce to point to the next possible one if it exist in the array already
            if ( j == 0 ) {
                InsertNode(wordHash, patterns[i], *(unsigned short *) (&patterns[i][0]));
            }
        }
    }
}



void wuManberSearch( vector<string> patterns, vector<string> files, string pathname, int patternNum ){
    vector<pair<float, string> > items;
    for( int i=0; i < files.size(); i++ ){
        FILE *pFile = fopen( (pathname+"/"+files[i]).c_str(), "r" );
        //int fileSize = getFileSizeWindows( pFile );
        int fileSize = getFileSizeLinux( (pathname+"/"+files[i]).c_str() );

        float frequency = 0.0;
        int offset = 0;
        unordered_map<string, int> patternsMap;
        int beginIndex = 0;

        char ch;
        while( offset <= fileSize ){
            char *buffer = (char*) malloc( sizeof(char)*(fileBlock+256) ); // used within wuManber
            // segment the file and add 256 len bytes as offset to do the search for each segement
            fread( buffer, 1, fileBlock+256, pFile );
//          frequency = WuManber( fileSize, buffer, patterns, patternNum );

            // Do the Wu Manber search below
            while ( beginIndex + (shortestLen - 1) < fileSize && beginIndex<offset+fileBlock ) {
                //If the char is not whitespace or letters, than move M
                //        ch = wholeBuffer[nBeginIdx+(shortestLen-1)];   //get the last char of tailing matching 2-letter characters
                //        if( !((ch>=97 && ch<=122) || (ch>=65 && ch<=90) || ch==32) ){
                //            nBeginIdx += shortestLen;  continue;
                //        }
                buffer[beginIndex+shortestLen-2 -offset] = tolower( buffer[beginIndex+shortestLen-2  -offset] );
                buffer[beginIndex+shortestLen-1 -offset] = tolower( buffer[beginIndex+shortestLen-1  -offset] );
                unsigned short wordHash = *(unsigned short *) &buffer[beginIndex+(shortestLen-1)-1  -offset];
                if ( shiftArray[wordHash] != 0 ) {
                    beginIndex += shiftArray[wordHash];  // try to move pattern(s) forward to shortest distance to match the text
                }
                else {
                    PrefixNode pNode = prefixArray[wordHash];
                    while (pNode) {    // iterate the linklist to try to totally match the pattern(s) by checking their prefix first
                        buffer[beginIndex -offset] = tolower( buffer[beginIndex -offset] );
                        buffer[beginIndex+1 -offset] = tolower( buffer[beginIndex+1 -offset] );
                        //if any prefix matchs, then try to match the pattern from the 3rd char untill the end
                        if ( pNode->prefixHash == *(unsigned short *) &buffer[beginIndex -offset] ) {
                            int i = 2;
                            ch = buffer[ beginIndex+i  -offset];
                            while ( pNode->pattern[i] && beginIndex + i < fileSize && tolower(pNode->pattern[i]) == tolower(ch) ) {
                                i++;
                                ch = buffer[ beginIndex+i  -offset];
                            }
                            if (pNode->pattern[i] == 0) { //find a match here
//                                printMatchInfo(nBeginIdx -offset, pNode->pattern);
                                frequency += 1;
                                patternsMap[ pNode->pattern ] = 1;
                            }
                        }
                        pNode = pNode->pNodeNext;
                    }
                    beginIndex++;  //No matter find the pattern or not, both just increment the index by 1
                }
            }
            free( buffer );
            // Do the Wu Manber search above

            offset += fileBlock;
            beginIndex = offset;        // update the offset and nBeginIdx to search next segment content
            fseek( pFile,offset,0 );
        }

        //if find any pattern that has no match at all within the file, then no match
        if( patternsMap.size() != patternNum ){
            frequency = 0.0;
        }else{
            frequency = frequency / patternNum;
        }

        if( frequency == 0.0 ){
            fclose(pFile);
            continue;
        }

        items.push_back( make_pair(frequency, files[i]) );
        fclose(pFile);
    }

    // sort the files first according to their frequency; if frequency same, then sort them according to their filename alphabetically
    sort( items.begin(), items.end(), sortFreqDescendingAndFilenameAscending() );
    for (vector<pair<float,string> >::const_iterator iter = items.begin(); iter!= items.end(); iter++) {
        //cout << iter->second << " : " << iter->first << endl;
        cout << iter->second << endl;
    }
//    cout << "DONE!" << endl;
}

int main( int argc, char *argv[] ) {
    block = 2;

    string pathName( argv[1] );
    vector<string> files;

    //files = getAllFilesWin( pathName );          //Windows
    files = getAllFilesLinux( pathName );          //Linux

    vector<string> patterns;
    shortestLen = 256;

    int begin;
    string arg( argv[3] );
    if( arg == "-s" ) begin = 5;
    else begin = 3;
    for(int i=begin; i<argc; i++){
        string pattern = argv[i];
        int patLen = pattern.length();
        transform( pattern.begin(), pattern.end(), pattern.begin(), ::tolower);  // transfer to lowercase for each pattern
        patterns.push_back( pattern );
        if( patLen < shortestLen ) shortestLen = patLen; // get the shortest length of the patters for the preprocessing and searching
    }
    int patternNum = patterns.size();

    /******* Wu Manber search algorithm ********/
    BuildShiftTable( patterns );                                // preprocessing
    wuManberSearch( patterns, files, pathName, patternNum );    // searching
    freeResources();
    /******* Wu Manber search algorithm ********/

    return 0;
}