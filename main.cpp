//
//
//  Project 4
//
//  Created by Lorenzo Bolls on 6/10/24.
//

#include <iostream>
#include <sstream>
#include <list>
#include <string>
#include <cctype>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <cassert>


using namespace std;

//GET RID OF THIS JUST FOR TESTING
//THIS IS SO THAT YOU CAN ACCESS WHERE THE FILE IS LOCATED LIKE WHICH FOLDER


class HashTable
{
public:
    HashTable(size_t size);
    void insert(const string& key, size_t offset);
    list<size_t> find(const string& key) const;

private:
    vector<list<pair<string, size_t>>> table;
    hash<string> hashFunction;
};

HashTable::HashTable(size_t size) : table(size) {}

void HashTable::insert(const string& key, size_t offset)
{
    size_t index = hashFunction(key) % table.size();
    table[index].emplace_back(key, offset);
}

list<size_t> HashTable::find(const string& key) const
{
    size_t index = hashFunction(key) % table.size();
    list<size_t> offsets;
    for (const auto& pair : table[index])
    {
        if (pair.first == key) 
        {
            offsets.push_back(pair.second);
        }
    }
    return offsets;
}

void populateHashTable(const string& fileContents, HashTable& hashTable, size_t N)
{
    for (size_t i = 0; i <= fileContents.size() - N; i++)
    {
        string substring = fileContents.substr(i, N);
        hashTable.insert(substring, i);
    }
}

string readFile(istream& file)
{
    string fileInfo;
    char ch;
    while (file.get(ch)) {
        fileInfo += ch;
    }
    return fileInfo;
}

void createRevision(istream& fold, istream& fnew, ostream& frevision) {
    const size_t N = 8; // Length of sequences to hash
    string oldContents = readFile(fold);
    string newContents = readFile(fnew);

    HashTable hashTable(1024); // Adjust size as needed
    populateHashTable(oldContents, hashTable, N);

    size_t i = 0;
    string addSeq;
    char delim = 33; // Initial delimiter

    while (i < newContents.size()) {
        string substring = newContents.substr(i, N);
        auto offsets = hashTable.find(substring);

        if (!offsets.empty()) {
            size_t longestMatchLength = 0;
            size_t bestOffset = 0;

            for (size_t offset : offsets) {
                size_t matchLength = N;
                while (i + matchLength < newContents.size() && offset + matchLength < oldContents.size() && newContents[i + matchLength] == oldContents[offset + matchLength]) {
                    ++matchLength;
                }

                if (matchLength > longestMatchLength) {
                    longestMatchLength = matchLength;
                    bestOffset = offset;
                }
            }

            if (!addSeq.empty()) {
                int i = 0;
                while (i < 1) {
                    if (addSeq.find(delim) == string::npos) {
                        i++;
                    } else {
                        delim++;
                    }
                }

                // Output the accumulated addition string with delimiters
                frevision << "+" << delim << addSeq << delim;
                addSeq.clear();
            }

            frevision << "#" << bestOffset << "," << longestMatchLength;
            i += longestMatchLength;
        } else {
            addSeq += newContents[i];
            i++;
        }
    }

    if (!addSeq.empty()) {
        
        // Continue to find a new delimiter until it's not in the string
        int i = 0;
        while (i < 1) {
            if (addSeq.find(delim) == string::npos) {
                i++;
            } else {
                delim++;
            }
        }

        // Output any remaining accumulated addition string with delimiters
        frevision << "+" << delim << addSeq << delim;
    }
}



//
//    if (!finalContents.empty())
//        {
//            frevision << "+" << delim << finalContents << delim;
//        }


bool getInt(istream& inf, int& n)
{
    char ch;
    if (!inf.get(ch)  ||  !isascii(ch)  ||  !isdigit(ch))
        return false;
    inf.unget();
    inf >> n;
    return true;
}

bool getCommand(istream& inf, char& cmd, char& delim, int& length, int& offset)
{
    if (!inf.get(cmd))
    {
        cmd = 'x';  // signals end of file
        return true;
    }
    switch (cmd)
    {
      case '+':
        return inf.get(delim).good();
      case '#':
        {
            char ch;
            return getInt(inf, offset) && inf.get(ch) && ch == ',' && getInt(inf, length);
        }
      case '\r':
      case '\n':
        return true;
    }
    return false;
}



bool revise(istream& fold, istream& frevision, ostream& fnew) {
    // Read the entire old file into a string
    
    string oldFileContent;
    char ch;
    while (fold.get(ch)) {
        oldFileContent += ch;
    }

    char cmd;
    char delim;
    int length, offset;

    
    /*
     you have a while loop that checks every character where if this character is in the substring, chosoe another one until you find a character that is not in the substring
     
     if the last character is not a number, then it is an add operator
     
     check if it is a digit, that means the previous operator ia copy operator. If its not a digit, then it is an add operator
     
     */
    
    //frevision
    // Parse the revision file
    while (getCommand(frevision, cmd, delim, length, offset)) {
        //end of file
        if (cmd == 'x')
        {
            break;
        }
        //add
        if (cmd == '+')
        {
            string addText;
            if (!getline(frevision, addText, delim))
                return false;
            
            fnew << addText;
        }
        //copy command
        else if (cmd == '#')
        {
            
//            MIGHT UNCOMMENT
//            fold.clear();
//            fold.seekg(offset);
            
            //checks if you can't copy
            if (offset < 0 || offset + length > oldFileContent.size())
                return false;
            
            //fold.seekg(offset)
            //fold.get(add, length)
            
            fnew << oldFileContent.substr(offset, length);
        }
        //invalid command
        else if (cmd != '\r' && cmd != '\n')
        {
            return false;
        }
    }
    return true;
}

/*
 if youre trying to add a string to an already fine string, you need to change the delimiter if the stirng you're adding alreay is a delimeter
 */


bool runtest(string oldName, string newName, string revisionName, string newName2)
{
    if (revisionName == oldName  ||  revisionName == newName  ||
        newName2 == oldName  ||  newName2 == revisionName  ||
            newName2 == newName)
    {
        cerr << "Files used for output must have names distinct from other files" << endl;
        return false;
    }
    ifstream oldFile(oldName, ios::binary);
    if (!oldFile)
    {
        cerr << "Cannot open " << oldName << endl;
        return false;
    }
    ifstream newFile(newName, ios::binary);
    if (!newFile)
    {
        cerr << "Cannot open " << newName << endl;
        return false;
    }
    ofstream revisionFile(revisionName, ios::binary);
    if (!revisionFile)
    {
        cerr << "Cannot create " << revisionName << endl;
        return false;
    }
    createRevision(oldFile, newFile, revisionFile);
    revisionFile.close();

    oldFile.clear();   // clear the end of file condition
    oldFile.seekg(0);  // reset back to beginning of the file
    ifstream revisionFile2(revisionName, ios::binary);
    if (!revisionFile2)
    {
        cerr << "Cannot read the " << revisionName << " that was just created!" << endl;
        return false;
    }
    ofstream newFile2(newName2, ios::binary);
    if (!newFile2)
    {
        cerr << "Cannot create " << newName2 << endl;
        return false;
    }
    assert(revise(oldFile, revisionFile2, newFile2));
    newFile2.close();

    newFile.clear();
    newFile.seekg(0);
    ifstream newFile3(newName2, ios::binary);
    if (!newFile)
    {
        cerr << "Cannot open " << newName2 << endl;
        return false;
    }
    if ( ! equal(istreambuf_iterator<char>(newFile), istreambuf_iterator<char>(),
                 istreambuf_iterator<char>(newFile3), istreambuf_iterator<char>()))
    {
        cerr << newName2 << " is not identical to " << newName
                 << "; test FAILED" << endl;
        return false;
    }
    return true;
}

int main()
{
    //maybe make two different folders
    
    string pathStem = "/Users/lorenzobolls/Desktop/CS 32/project 4/project 4/";
    string path2 = "/Users/lorenzobolls/Desktop/TEST CASES/";
    
    string pathG32 = "/w/home.25/home/lorenzobolls/cs32/project4/";
    
    assert(runtest(pathG32+"strange1.txt", pathG32+"strange2.txt", pathG32+"strangeREVISIONSSSSS.txt", pathG32+"strangeNEWFILESSS.txt"));
    cerr << "Test PASSED" << endl;
}



/*
 For the delimiter,
 
 maybe make an array of ASCII values with all the characters and the delimitier is in the stirng, delete it from this list and then pick randomly from this list
 
 */

//FOR THE HASH TABLE, IT SHOULD GET THE SIZEOF THE STRING (8 bytes)
/*
 How to make hash table:
 
 CAN'T USE UNORDERED MAP
 
 Make a hash table class
 just make insert and
 
 
 a list of pair
 
 a vector of lists and within the lists are a string and an int
 */
//
//
//void runtest(string oldtext, string newtext)
//{
//    istringstream oldFile(oldtext);
//    istringstream newFile(newtext);
//    ostringstream revisionFile;
//    createRevision(oldFile, newFile, revisionFile);
//    string result = revisionFile.str();
//    cout << "The revision file length is " << result.size()
//         << " and its text is " << endl;
//    cout << result << endl;
//
//    oldFile.clear();   // clear the end of file condition
//    oldFile.seekg(0);  // reset back to beginning of the stream
//    istringstream revisionFile2(result);
//    ostringstream newFile2;
//    assert(revise(oldFile, revisionFile2, newFile2));
//    assert(newtext == newFile2.str());
//}
//
//int main()
//{
//    runtest("There's a bathroom on the right.",
//            "There's a bad moon on the rise.");
//    runtest("ABCDEFGHIJBLAHPQRSTUVPQRSTUV",
//            "XYABCDEFGHIJBLETCHPQRSTUVPQRSTQQ/OK");
//    cout << "All tests passed" << endl;
//}
