//
// Created by Ananya Raghunath on 10/10/24.
//
#include <cctype>
#include "lex.h"
#include "lex.h"
#include <cctype>
#include <map>
#include <unordered_map>
#include <algorithm> // for std::transform

using std::map;

enum TokState {
    START,
    INID,
    ININT,
    INREAL,
    INSTRING,
    INCHAR,
    INERR
};


ostream& operator<<(ostream& out, const LexItem& tok) {
    switch (tok.GetToken()) {
        case ICONST:
            out << "ICONST: (" << tok.GetLexeme() << ")" << endl;
            break;
        case RCONST:
            out << "RCONST: (" << tok.GetLexeme() << ")" << endl;
            break;
        case BCONST:
            out << "BCONST: (" << tok.GetLexeme() << ")" << endl;
            break;
        case IDENT:
            out << "IDENT: <" << tok.GetLexeme() << ">" << endl;
            break;
        case SCONST:
            out << "SCONST: " << tok.GetLexeme() << endl;
            break;
        case CCONST:
            out << "CCONST: " << tok.GetLexeme() << endl;
            break;
        case ERR:
            out << "ERR: Unrecognized Lexeme {" << tok.GetLexeme() << "} in line " << tok.GetLinenum() << endl;
            break;
        case DONE:
            out << "DONE" << endl;
            break;
        // Operator cases
        case PLUS:
            out << "PLUS" << endl;
            break;
        case MINUS:
            out << "MINUS" << endl;
            break;
        case MULT:
            out << "MULT" << endl;
            break;
        case DIV:
            out << "DIV" << endl;
            break;
        case ASSOP:
            out << "ASSOP" << endl;
            break;
        case EQ:
            out << "EQ" << endl;
            break;
        case NEQ:
            out << "NEQ" << endl;
            break;
        case LTHAN:
            out << "LTHAN" << endl;
            break;
        case GTHAN:
            out << "GTHAN" << endl;
            break;
        case REM:
            out << "REM" << endl;
            break;
        case AND:
            out << "AND" << endl;
            break;
        case OR:
            out << "OR" << endl;
            break;
        case NOT:
            out << "NOT" << endl;
            break;
        case ADDASSOP:
            out << "ADDASSOP" << endl;
            break;
        case SUBASSOP:
            out << "SUBASSOP" << endl;
            break;
        case MULASSOP:
            out << "MULASSOP" << endl;
            break;
        case DIVASSOP:
            out << "DIVASSOP" << endl;
            break;
        case REMASSOP:
            out << "REMASSOP" << endl;
            break;
        case COMMA:
            out << "COMMA" << endl;
            break;
        case SEMICOL:
            out << "SEMICOL" << endl;
            break;
        case LPAREN:
            out << "LPAREN" << endl;
            break;
        case RPAREN:
            out << "RPAREN" << endl;
            break;
        case LBRACE:
            out << "LBRACE" << endl;
            break;
        case DOT:
            out << "DOT" << endl;
            break;
        case RBRACE:
            out << "RBRACE" << endl;
            break;
        // Handle any other tokens as necessary
        default:
            out << tok.GetLexeme() << endl;
            break;
    }
    return out;
}


LexItem id_or_kw(const std::string& lexeme, int linenum) {
    // Mapping of keywords to their corresponding tokens
    static const std::unordered_map<std::string, Token> keywordMap = {
        {"program", PROGRAM},
        {"string", STRING},
        {"else", ELSE},
        {"if", IF},
        {"int", INT},
        {"float", FLOAT},
        {"char", CHAR},
        {"print", PRINT},
        {"bool", BOOL},
        {"true", TRUE},
        {"false", FALSE}
        // Add other keywords as needed
    };

    std::string lowerLexeme = lexeme;
    std::transform(lowerLexeme.begin(), lowerLexeme.end(), lowerLexeme.begin(), ::tolower);

    // Check if the lexeme is a keyword
    auto it = keywordMap.find(lowerLexeme);
    if (it != keywordMap.end()) {
        Token keywordToken = it->second;

        // Handle special case for TRUE and FALSE
        if (keywordToken == TRUE || keywordToken == FALSE) {
            return LexItem(BCONST, lexeme, linenum);
        }

        // Return the corresponding LexItem for the keyword
        return LexItem(keywordToken, lexeme, linenum);
    }

    // If not a keyword, return as an identifier
    return LexItem(IDENT, lexeme, linenum);
}

LexItem getNextToken(istream& in, int& linenum) {
    TokState lexstate = TokState::START;
    string lexeme;
    string testLexeme;
    char ch;
    string line;
    char peek;
    while(in.get(ch)) {
        switch(lexstate) {
        case START:
            lexeme = ch;
            if (ch == '\n' || ch == '\r') {
                linenum++;
                break;
            }
            else if (ch == '+' || ch == '-') { // Allow signs at the start
                peek = in.peek();
                if (isdigit(peek)) {
                    in.get(); // Consume the digit
                    lexstate = ININT; // Move to INREAL directly
                    lexeme += peek; // Add the digit to lexeme
                } else {
                    if (ch == '+') {
                        peek = in.peek();
                        if (peek == '=') {
                            in.get();  // Consume '='
                            return LexItem(ADDASSOP, "+=", linenum);
                        }
                        if (isdigit(peek)){
                            in.get();
                            lexstate = TokState::ININT;
                            break;
                        }
                        return LexItem(PLUS, "+", linenum);
                    }
            else if (ch == '-') {
                        peek = in.peek();
                        if (peek == '=') {
                            in.get();  // Consume '='
                            return LexItem(SUBASSOP, "-=", linenum);
                        }
                        if (isdigit(peek)){

                            lexstate = TokState::ININT;
                            break;
                        }
                        else if (peek == '.'){

                            lexstate = TokState::ININT;
                            in.get();
                            break;
                        }
                        return LexItem(MINUS, "-", linenum);
                    }// Error if not followed by digit
                }

            }

            else if (ch == '/') {
                    peek = in.peek();
                    if (peek == '/') {
                        // Skip the rest of the line
                        std::string temp;
                        std::getline(in, temp); // Read until end of line
                        linenum++; // Increment line number
                        // Return to START state to check the next character in the next iteration
                        return getNextToken(in, linenum);
                    } else if (peek == '*'){
                        while (in.get(ch)) {
                            if (ch == '*') {
                                if (in.peek() == '/') {
                                    in.get();
                                    linenum++;// Consume the '/'
                                    break; // Exit the comment
                                }
                            }
                            if (ch == '\n') {
                                linenum++; // Increment line number for each new line
                            }
                        }
                         // Skip to the next iteration// Skip to the next iteration
                     } else if (peek == '=') {
                         in.get();  // Consume '='
                         return LexItem(DIVASSOP, "/=", linenum);
                     }

                   return LexItem(DIV, "/", linenum);
                }
            else if (ch == ' ' || ch == '\t') {
                break;
            }
            else if (ch == '@' || ch == '$' || isalpha(ch) || ch == '_') {
                lexstate = INID;
                break;
            }
            else if (isdigit(ch)) {

                lexstate = ININT;

                break;
            }
            else if (ch == '\"') {
                lexstate = INSTRING;
                break;
            }
            else if (ch == '\'') {
                lexstate = INCHAR;
                break;
            }
            else if (ch == '+') {
                        peek = in.peek();
                        if (peek == '=') {
                            in.get();  // Consume '='
                            return LexItem(ADDASSOP, "+=", linenum);
                        }
                        if (isdigit(peek)){
                            in.get();
                            lexstate = TokState::ININT;
                            break;
                        }
                        return LexItem(PLUS, "+", linenum);
                    }
            else if (ch == '-') {
                        peek = in.peek();
                        if (peek == '=') {
                            in.get();  // Consume '='
                            return LexItem(SUBASSOP, "-=", linenum);
                        }
                        if (isdigit(peek)){

                            lexstate = TokState::ININT;
                            break;
                        }
                        else if (peek == '.'){

                            lexstate = TokState::ININT;
                            in.get();
                            break;
                        }
                        return LexItem(MINUS, "-", linenum);
                    }
            else if (ch == '*') {
                        peek = in.peek();
                        if (peek == '=') {
                            in.get();  // Consume '='
                            return LexItem(MULASSOP, "*=", linenum);
                        }
                        return LexItem(MULT, "*", linenum);
                    }
            else if (ch == '/') {
                return LexItem(DIV, lexeme, linenum);
            }
            else if (ch == '=') {
                        peek = in.peek();
                        if (peek == '=') {
                            in.get();  // Consume second '='
                            return LexItem(EQ, "==", linenum);
                        }
                        return LexItem(ASSOP, "=", linenum);
                    }
            else if (ch == '(') {
                return LexItem(LPAREN, lexeme, linenum);
            }
            else if (ch == ')') {
                return LexItem(RPAREN, lexeme, linenum);
            }
            else if (ch == '{') {
                return LexItem(LBRACE, lexeme, linenum);
            }
            else if (ch == '}') {
                return LexItem(RBRACE, lexeme, linenum);
            }
            else if (ch == '>') {
                return LexItem(GTHAN, lexeme, linenum);
            }
            else if (ch == '<') {
                return LexItem(LTHAN, lexeme, linenum);
            }
            else if (ch == '.') {
                peek = in.peek();
                        if (isdigit(peek)) {
                            //in.get();  // Consume second '='
                            lexstate = INREAL;
                            break;
                        }
                return LexItem(DOT, lexeme, linenum);
            }
            else if (ch == ',') {
                return LexItem(COMMA, lexeme, linenum);
            }
            else if (ch == ';') {
                return LexItem(SEMICOL, lexeme, linenum);
            }
            else if (ch == '&') {
                        peek = in.peek();
                        if (peek == '&') {
                            in.get();  // Consume second '&'
                            return LexItem(AND, "&&", linenum);
                        }
                        return LexItem(ERR, "&", linenum);  // Handle unrecognized '&'
                    } else if (ch == '|') {
                        peek = in.peek();
                        if (peek == '|') {
                            in.get();  // Consume second '|'
                            return LexItem(OR, "||", linenum);
                        }
                        return LexItem(ERR, "|", linenum);  // Handle unrecognized '|'
                    } else if (ch == '%') {
                        peek = in.peek();
                        if (peek == '=') {
                            in.get();  // Consume '='
                            return LexItem(REMASSOP, "%=", linenum);
                        }
                        return LexItem(REM, "%", linenum);

                     }
            else if (ch == '!') {
                        peek = in.peek();
                        if (peek == '=') {
                            in.get();  // Consume '='
                            return LexItem(NEQ, "!=", linenum);
                        }
                        return LexItem(NOT, "!", linenum);  // Handle unrecognized '!'
                    }
            else {
               lexstate = INERR;
                break;
            }
            //break;
            return LexItem(ERR, lexeme, linenum);
        case ININT:
            if (isdigit(ch)) {
                lexeme += ch; // Continue building integer
            }
            else if (ch == '.') {
                lexeme += ch; // Add the dot for real numbers
                lexstate = INREAL; // Transition to real number state
            }
            else {
                in.putback(ch); // Put back the character
                return LexItem(ICONST, lexeme, linenum); // Return the integer constant
            }
            break;

        case INREAL:
            if (isdigit(ch)) {
                lexeme += ch; // Continue building the real number
            }
            else {
                in.putback(ch); // Put back the character for further processing
                return LexItem(RCONST, lexeme, linenum); // Return the real constant
            }
            break;

        case INID:
            if (isalnum(ch) || ch == '_') {
                lexeme += ch;
            }
            else {
                in.putback(ch);
                return id_or_kw(lexeme, linenum);
            }
            while(in.get(ch)) {
                if (isalnum(ch) || ch == '_') {
                    lexeme += ch;
                }
                else {
                    in.putback(ch);
                    return id_or_kw(lexeme, linenum);
                }
            }
            break;
        case INSTRING:
            if (ch != '\n') {
                lexeme += ch;
            }
            if (ch == '\"') {
                return LexItem(SCONST, lexeme, linenum);
            }
            else if (ch == '\n') {
                return LexItem(ERR, lexeme, linenum);
            }
            while(in.get(ch)) {
                if (ch != '\n') {
                    lexeme += ch;
                }
                if (ch == '\"') {

                    return LexItem(SCONST, lexeme, linenum);
                }
                else if (ch == '\n') {
                    return LexItem(ERR, lexeme, linenum);
                }
            }
            break;
        case TokState::INCHAR:
            if (ch != '\n') {
                    lexeme += ch;
            }
            if (ch == '\'') {
                return LexItem(SCONST, lexeme, linenum);
            }
            else if (ch == '\n') {
                return LexItem(ERR, lexeme, linenum);
            }
            while(in.get(ch)) {
                if (ch != '\n') {
                    lexeme += ch;
                }
                if (ch == '\'') {

                    return LexItem(SCONST, lexeme, linenum);
                }
                else if (ch == '\n') {
                    return LexItem(ERR, lexeme, linenum);
                }
            }
            break;

        case TokState::INERR:
                        // Demo: Read the rest of the chars until whitespace or EOF.
                   ch = in.get();
                   while(!isspace(ch) && ch != EOF) {
                        lexeme += ch;
                        ch = in.get();
                    }
                    in.putback(ch);
                    break;
            default:
                return LexItem(ERR, lexeme, linenum);
        }

    }
    return LexItem(DONE, lexeme, linenum);
}

