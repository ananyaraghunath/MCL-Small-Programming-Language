#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "lex.h"

using namespace std;

int main(int argc, char** argv) {


    bool allFlag = false;
    bool nconstflag = false; //int or real
    bool cconstflag = false; //char
    bool sconstflag = false; //string
    bool idflag = false; //identifiers
    bool kwflag = false; //keywords
    bool bconstflag = false; //bools

    int tokensCounter = 0;
    int fileNameCounter = 0;
    map<string, Token> idMap;
    //map<in, Token> fconstMap;
    map<float, Token> nconstMap;
    map<string, Token> sconstMap;
    map<string, Token> cconstMap;
    map<Token, string> kwMap;
    map<string, Token> bconstMap;
    map<string, Token> operatorsMap;
    //map<string, Tokens> tokensA;

    string fileName,line;
    int linecount = 0;

    if(argc == 1) {
        cout << "No specified input file." << endl;
        return -1;
    }
    else {
        for (int i = 1; i < argc; i++) {
            string argument = argv[i];
            if (argument[0] == '-') {
                if (argument == "-all") {
                    allFlag = true;
                }
                else if (argument == "-num") {
                    nconstflag = true;
                }
                else if (argument == "-str") {
                    sconstflag = true;
                }
                else if (argument == "-id") {
                    idflag = true;
                }
                else if (argument == "-char") {
                    nconstflag = true;
                }
                else if (argument == "-kw") {
                    kwflag = true;
                }
                else if (argument == "-bool") {
                    bconstflag = true;
                }
                else {
                    cerr << "UNRECOGNIZED FLAG {" << argument << "}" << endl;
                    return 1;
                }
            }
            else {
                fileName = string(argv[1]);
                fileNameCounter++;
                if (fileNameCounter > 1) {
                    cerr << "ONLY ONE FILE NAME IS ALLOWED." << endl;
                    return 1;
                }
            }
        }
    }

    // Read file
    ifstream innFile;
    innFile.open(argv[1]);
    if(innFile.fail()) {
        cout << "CANNOT OPEN THE FILE " << argv[1] << endl;
        return -1;
    }
    while (getline(innFile, line)) {
        // Count non-blank lines
        if (!line.empty() || (line.empty() && !innFile.eof())) {
            linecount++;
        }
    }

    ifstream inputFile;
    inputFile.open(argv[1]);
    bool isEmpty = true;
    char ch;
    while (inputFile.get(ch)) {
        isEmpty = false;
        break; // No need to read further
    }
    if (isEmpty) {
        cout << "Empty file." << endl;
        exit(0);
    }

    // Tokenize
    LexItem currentToken;
    int currentLineNum = 0;
    int idcount = 0;
    int kwcount = 0;
    int numcount = 0;
    int boolcount = 0;
    int stringcount = 0;
    int charcount = 0;
    ifstream inFile;
    inFile.open(argv[1]);
    while((currentToken = getNextToken(inFile, currentLineNum)) != DONE) {
        tokensCounter++;
        if (currentToken == IDENT) {
            idMap.insert({currentToken.GetLexeme(), currentToken.GetToken()});


        }
        else if(currentToken == IF ||currentToken ==  ELSE|| currentToken == PRINT|| currentToken == INT|| currentToken == FLOAT||
currentToken ==  CHAR|| currentToken == STRING|| currentToken == BOOL || currentToken == PROGRAM || currentToken == TRUE|| currentToken == FALSE) {
            kwMap.insert({ currentToken.GetToken(), currentToken.GetLexeme()});
            tokensCounter++;


        }
        else if (currentToken == ICONST || currentToken == RCONST) {
            nconstMap.insert({stof(currentToken.GetLexeme()), currentToken.GetToken()});
            tokensCounter++;


        }
        else if (currentToken == BCONST) {
            bconstMap.insert({(currentToken.GetLexeme()), currentToken.GetToken()});
            tokensCounter++;

        }
        else if (currentToken == SCONST) {
            sconstMap.insert({currentToken.GetLexeme(), currentToken.GetToken()});
            tokensCounter++;

        }
        else if (currentToken == CCONST) {
            cconstMap.insert({currentToken.GetLexeme(), currentToken.GetToken()});
            tokensCounter++;

        }
        std::vector<Token> operatorTokens = {PLUS, MINUS, MULT, DIV, ASSOP, EQ, NEQ,
                                     LTHAN, GTHAN, REM, AND, OR, NOT,
                                     ADDASSOP, SUBASSOP, MULASSOP,
                                     DIVASSOP, REMASSOP};

        for (const auto& opToken : operatorTokens) {
            if (currentToken == opToken) {
                operatorsMap.insert({currentToken.GetLexeme(), currentToken.GetToken()});

                break; // Exit loop after inserting
            }
        }

        if (allFlag) {
            operator<<(cout, currentToken);
        }
    }


    idcount = idMap.size();
    kwcount = kwMap.size();
    numcount = nconstMap.size();
    boolcount = bconstMap.size();
    stringcount = sconstMap.size();
    charcount = cconstMap.size();

    //int total = idcount + kwcount+ numcount+ boolcount+stringcount +charcount;

    // Print results
    cout << "\nLines: " << linecount << endl;
    cout << "Total Tokens: " << tokensCounter << endl;
    cout << "Identifiers and Keywords: " << idcount + kwcount << endl;
    cout << "Numerics: " << numcount << endl;
    cout << "Booleans: " << boolcount << endl;
    cout << "Strings and Characters: " << stringcount + charcount << endl;


    // Display results based on flags



    if (nconstflag && numcount > 0) {
        cout << "NUMERIC CONSTANTS:" << endl;
        int count = 0;
        for (const auto& pair : nconstMap) {
            cout << pair.first;
            count++;
            // Print a comma if this is not the last element
            if (count < nconstMap.size()) {
                cout << ", ";
            }
        }
        cout << endl;
    }

    if (bconstflag && boolcount > 0) {
        cout << "BOOLEAN CONSTANTS:" << endl;
        for (const auto& pair : bconstMap) {
            cout << pair.first << ",";
        }
        cout << endl;
    }
    if ((cconstflag || sconstflag) && charcount > 0) {
        cout << "CHARACTER CONSTANTS:" << endl;
        int count = 0;
        for (const auto& pair : cconstMap) {
            cout << pair.first;
            count++;
            // Print a comma if this is not the last element
            if (count < cconstMap.size()) {
                cout << ", ";
            }
        }
        cout << endl;
    }
    if (sconstflag && stringcount > 0) {
        cout << "STRINGS:" << endl;
        int count = 0;
        for (const auto& pair : sconstMap) {
            cout << pair.first;
            count++;
            // Print a comma if this is not the last element
            if (count < sconstMap.size()) {
                cout << ", ";
            }
        }
        cout << endl;
    }
    if (idflag && idcount > 0) {
        cout << "IDENTIFIERS:" << endl;
        int count = 0;
        for (const auto& pair : idMap) {
            cout << pair.first;
            count++;
            // Print a comma if this is not the last element
            if (count < idMap.size()) {
                cout << ", ";
            }
        }
        cout << endl;
    }
    if (kwflag && kwcount > 0) {
        cout << "KEYWORDS:" << endl;
        int count = 0;
        for (const auto& pair : kwMap) {
            cout << pair.second;
            count++;
            // Print a comma if this is not the last element
            if (count < kwMap.size()) {
                cout << ", ";
            }
        }
        cout << endl;
    }

    inFile.close();

    return 0;
}

/*
    while((currentToken = getNextToken(inFile, currentLineNum)).GetToken() != DONE) {

        tokensCounter++;
        if (currentToken == IDENT) {
            idMap.insert({currentToken.GetLexeme(), currentToken.GetToken()});
            idorkw++;
        }
        else if(currentToken == IF ||currentToken ==  ELSE|| currentToken == PRINT|| currentToken == INT|| currentToken == FLOAT||
currentToken ==  CHAR|| currentToken == STRING|| currentToken == BOOL || currentToken == PROGRAM || currentToken == TRUE|| currentToken == FALSE) {
            kwMap.insert({currentToken.GetLexeme(), currentToken.GetToken()});
            idorkw++;

        }
        else if (currentToken == ICONST || currentToken == RCONST) {
            nconstMap.insert({stof(currentToken.GetLexeme()), currentToken.GetToken()});
            numcount++;

        }
        else if (currentToken == BCONST) {
            bconstMap.insert({(currentToken.GetLexeme()), currentToken.GetToken()});
            boolcount++;
        }
        else if (currentToken == SCONST) {
            sconstMap.insert({currentToken.GetLexeme(), currentToken.GetToken()});
            stringcount++;
        }
        else if (currentToken == CCONST) {
            cconstMap.insert({currentToken.GetLexeme(), currentToken.GetToken()});
            charcount++;
        }
        if (allFlag) {
            operator<<(cout, currentToken);
        }
    }

*/
