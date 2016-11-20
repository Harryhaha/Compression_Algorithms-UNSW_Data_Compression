#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <ctype.h>
#include <queue>
#include <cctype>

//Windows
//#include<io.h>
//Linux
#include <unistd.h>
#include <dirent.h>

//typedef pair<string, float> PAIR;
using namespace std;

//typedef pair<string, float> PAIR;
//
//struct cmp {
//    bool operator() (const PAIR &P1, const PAIR &P2)
//    {
//        return P1.second > P2.second;
//    }
//};

map<int, int> **statShawn;
map<int, int> **statHarry;

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
    vector<string> files;//store all filenames

    DIR *dir;
    struct dirent *ptr;

    if ((dir=opendir(path.c_str())) == NULL) {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr=readdir(dir)) != NULL) {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) continue;  //current dir or parrent dir
        else if(ptr->d_type == 8)  files.push_back(ptr->d_name);   //file type
    }
    closedir(dir);

    //sorting from small to large
    sort(files.begin(), files.end());
    return files;
}

map<string, map<int, vector<int> > > preprocFiles( vector<string> files, string path ){
    map<string, map<int, vector<int> > > invertedIndex;
    // { "hello": { ID1: [15, 87, 198,...], ID2: [4, 68] }
    // { "harry": { ID1: [2,7,28,...], ID2: [49, 200, 463,...] }
    int ch;
    for( int fileIndex=0; fileIndex<files.size(); fileIndex++ ){
        int count = 0;
        int position = 0; bool alreadyStore = false;
        //ifstream in;

        // NOTE:
        //in.open( (path + "\\\\" + files[ fileIndex ]).c_str() ); //transfer string to char*
        //in.open( path + "\\" + files[fileIndex] );
        FILE *pFile = fopen( (path+"/"+files[fileIndex]).c_str(), "r" );

        string eachKeyword = "";
        ch = fgetc( pFile );
        while( ch != EOF ){
        //while( !in.eof() ){
            //in.read( &ch, 1 );
            //if( ch == ' ' )
            if( !((ch>=65&&ch<=90) || (ch>=97&&ch<=122)) ){  // ' ' or A-Z or a-z
                if( eachKeyword != "" ){
                    invertedIndex[ eachKeyword ][ fileIndex ].push_back( position );
                }
                alreadyStore = false; eachKeyword = ""; count += 1; ch = fgetc( pFile ); continue;
            }

            if( alreadyStore == false ) position = count;
            alreadyStore = true;
            eachKeyword += ch;
            count += 1;

            ch = fgetc( pFile );
        }
        if( eachKeyword != "" ){
            invertedIndex[ eachKeyword ][ fileIndex ].push_back( position );
        }

        //in.close();
        fclose( pFile );
    }

    return invertedIndex;
}

int getFileSize( FILE *pFile ) {
    fseek(pFile,0,SEEK_END);
    int size = ftell(pFile);
    return size;
}


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

vector<int> sunday( int strLen, FILE *pFile, string searchStr, int* next ) {
    vector<int> positions;
    //int strLen = getFileSize( (pathname+"/"+fileName).c_str() );  // ??????
    //int subLen = searchStr.length();

//    int maxSize=256;
//    int next[maxSize];
    int i,j,pos;
    int subLen = searchStr.length();
//    for(i=0;i<maxSize;i++) next[i] = subLen+1;
//    for(i=0;i<subLen;i++) next[ searchStr[i] ] = subLen-i;
    pos=0;
    char ch;
    while(pos<=(strLen-subLen)) {
        i = pos;
        for( j=0; j<subLen; j++,i++ ) {
            fseek( pFile, i, 0 ); ch = fgetc( pFile );
            if( ch != searchStr[j] ) {
                fseek( pFile, pos+subLen, 0 ); ch = fgetc( pFile );
//                if( ch < 0 ) pos += subLen+1;   // used for non-ASCII character
//                else pos += next[ ch ];         // move forward
                pos += next[ ch ];         // move forward
                break;
            }
        }
        if( j == subLen){
            //positions.push_back( pos );
//            if( (pos+subLen) < strLen ){
//                fseek( pFile, pos+subLen, 0 );
              ch = fgetc( pFile );
//                if( ch < 0 ) pos += subLen+1;   // used for non-ASCII character
//                else pos += next[ ch ];         // move forward
                pos += next[ ch ];         // move forward
//            }
//            else break;
        }
    }
    return positions;
}

//sunday algorithm combined with skip pointer
vector<int> sundayWithSkipPointer( int fileSeq, int strLen, FILE *pFile, string searchStr, int* next ) {
    vector<int> positions;

    string preStr = searchStr.substr(0, 2);
    int invertedIndex = prefixToIndex( preStr );
    map<int, int> skipPointer = statHarry[ invertedIndex ][ fileSeq ];
//    if( skipPointer.size() == 0 ) {
//        return positions;               // if has no any from->to, then just return empty. since if it has any from->to, it at lease has 0->end
//    }

    int i,j,pos;
    int subLen = searchStr.length();
    pos=0;
    char ch;
    bool prefixExist = false;
    int from, to = 0;
    while(pos<=(strLen-subLen)) {
        i = pos;

        from = (skipPointer.begin())->first;
        if( (from>=i ) && ( from<(i+subLen) ) ){
            prefixExist = true;
            to = (skipPointer.begin())->second;
        }

        for( j=0; j<subLen; j++,i++ ) {
            fseek( pFile, i, 0 ); ch = fgetc( pFile );
            if( ch != searchStr[j] ) {
                if( prefixExist == true ){      //should skip, instead of next[ch] or (subLen+1)(non-ASCII)
                    pos = to;                    //skip to next
                    prefixExist = false;        //reset the flag
                    skipPointer.erase( skipPointer.begin() ); //also erase the first item
                }else{
                    fseek( pFile, pos+subLen, 0 ); ch = fgetc( pFile );
                    if( ch < 0 ) pos += subLen+1;   // used for non-ASCII character
                    else pos += next[ ch ];         // move forward
                }
                break;   //break anyway
            }
        }

        if( j == subLen){                       //if matches
            positions.push_back( pos );
            if( (pos+subLen) < strLen ){
                if( prefixExist == true ){      //should skip, instead of (subLen+1)
                    pos = to;                    //skip to next
                    prefixExist = false;        //reset the flag
                    skipPointer.erase( skipPointer.begin() ); //also erase the first item
                }
                else{
                    fseek( pFile, pos+subLen, 0 ); ch = fgetc( pFile );
                    if( ch < 0 ) pos += subLen+1;   // used for non-ASCII character
                    else pos += next[ ch ];         // move forward
                }
            }
            else break;  //terminate whole loop !
        }
    }
    return positions;
}

void sundaySearch( vector<string> keywords, vector<string> files, string pathname ){
    int fileNum = files.size();
    int keywordNum = keywords.size();
    int **nextArray = new int *[keywordNum];
    int maxSize=256;
    string keyword = ""; int subLen = 0;
    for( int i=0; i<keywordNum; i++ ){
        keyword = keywords[i];
        subLen = keyword.length();
        nextArray[i] = new int[maxSize];  //initialize to subLen+1
        for( int j=0; j < maxSize; j++ ){
            nextArray[i][j] = subLen+1;
        }
        for( int j=0; j < subLen; j++){
            nextArray[i][ keyword[j] ] = subLen-j;
        }
    }

    FILE *pFile = NULL;
    for( int i=0; i<fileNum; i++ ){

        //combined with skip pointer
//        bool noSearchPatter = false;
//        for( int j=0; j<keywords.size(); j++ ){
//            string preStr = keywords[j].substr(0, 2);
//            int invertedIndex = prefixToIndex( preStr );
//            map<int, int> skipPointer = statHarry[ invertedIndex ][ i ];
//            if( skipPointer.size() == 0 ) {
//                noSearchPatter = true; break;              // if has no any from->to, then just return empty. since if it has any from->to, it at lease has 0->end
//            }
//        }
//        if( noSearchPatter == true ) continue;

        /*****************************************************************/
        //FILE *pFile = NULL;
        pFile = fopen( (pathname+"/"+files[i]).c_str(),"rb" );
        int fileSize = getFileSize( pFile );

        bool wordExist = true;
        //pFile = fopen( (pathname+"/"+files[i]).c_str(), "r" );
        float freq = 0.0;
        for( int j=0; j<keywords.size(); j++ ){

            vector<int> positions = sunday( fileSize, pFile, keywords[j], nextArray[j] );                  //pure sunday algorithm
//            //vector<int> positions = sundayWithSkipPointer( i, fileSize, pFile, keywords[j], nextArray[j] );  //sunday algorithm combined with skip pointer

//            int posSize = positions.size();
//            if( posSize == 0 ){
//                wordExist = false; break;
//            }
//            else freq += posSize;
            //freq += posSize;   //looks like now it must have matches
        }

//        if( wordExist == true ){
//            freq = freq / keywords.size();
//            //stat[ files[i] ] = freq;
//            //sortPos( files[i], freq, result ); currently unused
//        }

        //cout << "keyword:" << keywords[0] << "; filename: " << files[i] << "; freq: " << freq << endl;
        fclose(pFile);
    }

    cout << "DONE!" << endl;

    // Sort by value in map to output the result
//    vector<PAIR> freqVector( stat.begin(), stat.end() );
//    //转化为PAIR的vector
//    sort( freqVector.begin(), freqVector.end(), cmp() );  //需要指定cmp
//
//    for( int i=0; i<=freqVector.size(); i++ ){
//        cout << freqVector[i].first << " " << freqVector[i].second << endl;
//    }  //也要按照vector的形式输出

}

vector<string> split(const string &text, char sep) {
    vector<string> tokens;
    size_t start = 0, end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        std::string temp = text.substr(start, end - start);
        if (temp != "") tokens.push_back(temp);
        start = end + 1;
    }
    string temp = text.substr(start);
    if (temp != "") tokens.push_back(temp);
    return tokens;
}

void skipPointerBuildShawn( vector<string> files, string pathName, int gap ){

    //[ [{ from:to },{ from:to },{ from:to },...], [{ from:to },{ from:to },{ from:to },...], [{ from:to },{ from:to },{ from:to },...] ]
    char ch;
    int previousOffset[676] = { -1 };
    for( int fileIndex=0; fileIndex<files.size(); fileIndex++ ){
        int count = 0;
        //ifstream in;
        FILE *pFile = fopen( (pathName+"/"+files[fileIndex]).c_str(), "r" );

        bool twoChar = false; char previous; string prefixTwo;
        int prefixIndex, prevPrefexOffset = 0;
        ch = fgetc( pFile );
        while( ch != EOF ){
            prefixTwo = "";

            if( !((ch>=65&&ch<=90) || (ch>=97&&ch<=122)) ){  // ' ' or A-Z or a-z
                twoChar = false;
                count += 1;
                ch = fgetc( pFile );
                previous = -1;  //seems like useless
                continue;
            }

            ch = tolower( ch );
            if( twoChar == false ){
                twoChar = true;
                previous = ch;
                count += 1;
                ch = fgetc( pFile );
                continue;
            }

            prefixTwo += previous;
            prefixTwo += ch;
            prefixIndex = prefixToIndex( prefixTwo );
            prevPrefexOffset = previousOffset[prefixIndex];
            if( ((count-1) - prevPrefexOffset) >= gap ){    //If the gap between this prefix string is larger than expected, than store from->to
                statShawn[ fileIndex ][ prefixIndex ][ prevPrefexOffset ] = count - 1;
            }
            previousOffset[ prefixIndex ] = count - 1;

            previous = ch;
            count += 1;
            ch = fgetc( pFile );
        }

        for( int k=0; k<676; k++ ){
            prevPrefexOffset = previousOffset[ k ];
            if( prevPrefexOffset != -1 ){
                statShawn[ fileIndex ][ k ][ prevPrefexOffset ] = count - 1;  //keep track of skip pointer from previous to the end file offset
            }
        }

        fclose( pFile );
    }
//    return invertedIndex;
}

void skipPointerBuildHarry( vector<string> files, string pathName, int gap ){
    // [
    //    aa: [ file1:{ from1:to1, from2:to2, from3:to3,... }, file2: { from1:to1, from2:to2, from3:to3,... } ]
    //    ab: [ file1:{ from1:to1, from2:to2, from3:to3,... }, file2: { from1:to1, from2:to2, from3:to3,... } ]
    //    ac: [ file1:{ from1:to1, from2:to2, from3:to3,... }, file2: { from1:to1, from2:to2, from3:to3,... } ]
    // ]
    char ch;
    for( int fileIndex=0; fileIndex<files.size(); fileIndex++ ){
        int previousOffset[ 676 ] = { -1 };
        int count = 0;
        FILE *pFile = fopen( (pathName+"/"+files[fileIndex]).c_str(), "r" );

        bool twoChar = false; char previous; string prefixTwo;
        int prefixIndex, prevPrefexOffset = 0;
        ch = fgetc( pFile );
        while( ch != EOF ){
            prefixTwo = "";

            if( !((ch>=65&&ch<=90) || (ch>=97&&ch<=122)) ){  // ' ' or A-Z or a-z
                twoChar = false;
                count += 1;
                ch = fgetc( pFile );
                previous = -1;  //seems like useless
                continue;
            }

            ch = tolower( ch );    //case insensitive
            if( twoChar == false ){
                twoChar = true;
                previous = ch;
                count += 1;
                ch = fgetc( pFile );
                continue;
            }

            prefixTwo += previous;
            prefixTwo += ch;
            prefixIndex = prefixToIndex( prefixTwo );
            prevPrefexOffset = previousOffset[prefixIndex];
            if( ((count-1) - prevPrefexOffset) >= gap ){    //If the gap between this prefix string is larger than expected, than store from->to
                statHarry[ prefixIndex ][ fileIndex ][ prevPrefexOffset ] = count - 1;
            }
            previousOffset[ prefixIndex ] = count - 1;

            previous = ch;
            count += 1;
            ch = fgetc( pFile );
        }

        for( int k=0; k<676; k++ ){
            prevPrefexOffset = previousOffset[ k ];
            if( prevPrefexOffset != -1 ){
                statHarry[ k ][ fileIndex ][ prevPrefexOffset ] = count - 1;  //keep track of skip pointer from previous to the end file offset
            }
        }

        fclose( pFile );
    }
//    return invertedIndex;
}

//int *getAllFileSize( vector<string> files, string pathName ){
//    int *result = new int[files.size()]();
//    for( int i=0; i<files.size(); i++ ){
//        result[i] = getFileSize( (pathName+"/"+files[i]).c_str() );
//    }
//    return result;
//}

void skipPointerBuildModified( vector<string> files, string pathName, int gap ){
    // [
    //    aa: [ file1:{ from1:to1, from2:to2, from3:to3,... }, file2: { from1:to1, from2:to2, from3:to3,... } ]
    //    ab: [ file1:{ from1:to1, from2:to2, from3:to3,... }, file2: { from1:to1, from2:to2, from3:to3,... } ]
    //    ac: [ file1:{ from1:to1, from2:to2, from3:to3,... }, file2: { from1:to1, from2:to2, from3:to3,... } ]
    // ]

    // from:0000 0111  to:1202 3238
    //int *fileSizeArray = getAllFileSize( files, pathName );


    char ch;
    for( int fileIndex=0; fileIndex<files.size(); fileIndex++ ){
        int previousOffset[ 676 ] = { -1 };
        int count = 0;
        FILE *pFile = fopen( (pathName+"/"+files[fileIndex]).c_str(), "r" );

        bool twoChar = false; char previous; string prefixTwo;
        int prefixIndex, prevPrefexOffset = 0;
        ch = fgetc( pFile );
        while( ch != EOF ){
            prefixTwo = "";

            if( !((ch>=65&&ch<=90) || (ch>=97&&ch<=122)) ){  // ' ' or A-Z or a-z
                twoChar = false;
                count += 1;
                ch = fgetc( pFile );
                previous = -1;  //seems like useless
                continue;
            }

            ch = tolower( ch );    //case insensitive
            if( twoChar == false ){
                twoChar = true;
                previous = ch;
                count += 1;
                ch = fgetc( pFile );
                continue;
            }

            prefixTwo += previous;
            prefixTwo += ch;
            prefixIndex = prefixToIndex( prefixTwo );
            prevPrefexOffset = previousOffset[prefixIndex];
            if( ((count-1) - prevPrefexOffset) >= gap ){    //If the gap between this prefix string is larger than expected, than store from->to
                statHarry[ prefixIndex ][ fileIndex ][ prevPrefexOffset ] = count - 1;
            }
            previousOffset[ prefixIndex ] = count - 1;

            previous = ch;
            count += 1;
            ch = fgetc( pFile );
        }

        for( int k=0; k<676; k++ ){
            prevPrefexOffset = previousOffset[ k ];
            if( prevPrefexOffset != -1 ){
                statHarry[ k ][ fileIndex ][ prevPrefexOffset ] = count - 1;  //keep track of skip pointer from previous to the end file offset
            }
        }

        fclose( pFile );
    }
//    return invertedIndex;
}



int main( int argc, char *argv[] ) {
    string pathName( argv[1] );
    vector<string> files;

    //files = getAllFilesWin( pathName );          //Windows
    files = getAllFilesLinux( pathName );          //Linux

    vector<string> keywords;
    for(int i=2; i<argc; i++){
        string keyword = argv[i];
//        transform( keyword.begin(), keyword.end(), keyword.begin(), ::tolower);  // transfer to lowercase for each keyword/phrase/substring
        keywords.push_back( keyword );
    }
    //keywords.push_back( argv[2] );
    /******* Split the phrase ********/
//    for( int i=2; i<argc; i++){
//        vector<string> subArgs = split( argv[i], ' ' );
//        keywords.insert( keywords.end(), subArgs.begin(), subArgs.end() );
//    }
    /******* Split the phrase ********/

    /******* Inverted index ********/
    //map<string, map<int, vector<int> > > invertedIndex = preprocFiles( files, pathName );
    //cout << invertedIndex.size() << endl;
    /******* Inverted Index ********/

    /******* Skip pointer *********/
    // Shawn
//    statShawn = new map<int, int> *[files.size()];
//    for (int i = 0; i < files.size(); i++) {
//        statShawn[i] = new map<int, int>[676]();
//    }
//    skipPointerBuildShawn( files, pathName, 10000 );
//    for(int j=0; j<files.size(); j++){
//        for(int k=0; k<676; k++){
//            string prefixTwo = indexToPrefix( k );
//            cout << prefixTwo << " : ";
//            map<int, int> test = stat[j][k];
//            map<int, int>::iterator iter;
//            for( iter=test.begin(); iter != test.end(); iter++ ){
//                cout << iter->first << " TO " << iter->second << " ; ";
//            }
//            cout << endl;
//        }
//    }

    //Harry
//    statHarry = new map<int, int> *[676];
//    for (int i = 0; i < 676; i++) {
//        statHarry[i] = new map<int, int>[files.size()];
//    }
//    skipPointerBuildHarry( files, pathName, 10000 );
//    for(int j=0; j<676; j++){
//        string prefixTwo = indexToPrefix( j );
//        //cout << prefixTwo << " : " << endl;
//        for(int k=0; k<files.size(); k++){
//            //cout << "filename " << files[k] << " : ";
//            map<int, int> test = statHarry[j][k];
//            if( test.size() == 0 ){
//                //cout << "No such prefix word " << prefixTwo;
//            }else{
//                map<int, int>::iterator iter;
//                for( iter=test.begin(); iter != test.end(); iter++ ){
//                    cout << iter->first  << iter->second << " ; ";
//                }
//            }
//            //cout << endl;
//        }
//    }

    /******* Skip pointer *********/

    /******* Sunday search ********/
    sundaySearch( keywords, files, pathName );
    /******* Sunday search ********/

    return 0;
}


// print two dimensional array which contains map
//    for(int j=0; j<files.size(); j++){
//        for(int k=0; k<676; k++){
//            map<int, int> test = stat[j][k];
//            map<int, int>::iterator iter;
//            for( iter=test.begin(); iter != test.end(); iter++ ){
//                cout << iter->first << " " << iter->second << endl;
//            }
//        }
//    }