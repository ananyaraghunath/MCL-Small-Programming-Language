
#include "parserInterp.h"
#include "val.h"
map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants 
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}


//PrintStmt:= PRINT (ExpreList) 
bool PrintStmt(istream& in, int& line) {
	LexItem t;
	ValQue = new queue<Value>;

	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
	bool ex = ExprList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing expression list after Print");
		while (!(*ValQue).empty())
		{
			ValQue->pop();
		}
		delete ValQue;
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		while (!(*ValQue).empty())
		{
			ValQue->pop();
		}
		delete ValQue;
		return false;
	}
	//Evaluate: print out the list of expressions' values
	while (!(*ValQue).empty())
	{
		Value nextVal = (*ValQue).front();
		cout << nextVal;
		ValQue->pop();
	}
	delete ValQue;
	cout << endl;
	return true;
}//End of PrintStmt


//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;
	Value retVal;
	
	status = Expr(in, line, retVal);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}
	ValQue->push(retVal);
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		
		status = ExprList(in, line);
		
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}//End of ExprList

bool IdentList(istream& in, int& line);
//Program is: Prog = PROGRAM IDENT CompStmt
bool Prog(istream& in, int& line){
    bool sl = false;
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() == PROGRAM) {
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() == IDENT) {
            sl = CompStmt(in, line);
            if( !sl ){
                ParseError(line, "Invalid Program");
            return false;
            }
        cout << "(DONE)" << endl;
        return true;
        }
        else{
            ParseError(line, "Missing Program name");
            return false;
        }
    }
    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        ParseError(line, "Missing Program keyword");
        return false;
    }
}//End of Prog

//StmtList ::= Stmt { Stmt }
bool StmtList(istream& in, int& line){
    bool status;
    LexItem tok;
    status = Stmt(in, line);
    tok = Parser::GetNextToken(in, line);
    while(status && tok != RBRACE){
        Parser::PushBackToken(tok);
        status = Stmt(in, line);
        tok = Parser::GetNextToken(in, line);
    }
    if(!status){
        ParseError(line, "Syntactic error in statement list.");
        return false;
    }
    Parser::PushBackToken(tok);
    return true;
}//End of StmtList

//DeclStmt ::= ( INT | FLOAT | BOOL | CHAR | STRING) VarList
bool DeclStmt(istream& in, int& line) {
    bool status = false;
    LexItem tok;
    LexItem t = Parser::GetNextToken(in, line);
    if(t == INT || t == FLOAT || t == CHAR || t == BOOL || t == STRING ) {
        status = VarList(in, line, t);
        if (!status)
        {
            ParseError(line, "Incorrect variable in Declaration Statement.");
            return status;
        }
        return true;
    }
    return true;
}//End of Decl

//Stmt ::= DeclStmt ; | ControlStmt ; | CompStmt
bool Stmt(istream& in, int& line) {
    bool status = true;
    LexItem t = Parser::GetNextToken(in, line);
    switch( t.GetToken() ) {
    case PRINT: case IF: case IDENT:
        Parser::PushBackToken(t);
        status = ControlStmt(in, line);
        if(!status){
            ParseError(line, "Invalid control statement.");
            return false;
        }
        break;
    case INT: case FLOAT: case CHAR: case BOOL: case STRING:
        Parser::PushBackToken(t);
        status = DeclStmt(in, line);
        if(!status){
            ParseError(line, "Invalid declaration statement.");
            return false;
        }
        t = Parser::GetNextToken(in, line);
        if(t == SEMICOL){
            return true;
        }
        else{
            ParseError(line, "Missing semicolon at end of Statement.");
            return false;
        }
        break;
    case LBRACE:
        Parser::PushBackToken(t);
        status = CompStmt(in, line);
        if(!status)
        {
        ParseError(line, "Invalid compound statement.");
        return false;
        }
        break;
    default:
        Parser::PushBackToken(t);
        return false;
    }
    return status;
}//End of Stmt

bool CompStmt(istream& in, int& line){
    bool status=false;
    LexItem t = Parser::GetNextToken(in, line);
    if(t == LBRACE){
        status = StmtList(in, line);
        if(!status){
            ParseError(line, "Incorrect statement list");
            return false;
        }
        LexItem t = Parser::GetNextToken(in, line);
            if(t == RBRACE){
            return true;
        }
        else if(t == DONE){
            ParseError(line, "Missing end of program right brace.");
            return false;
        }
        else{
            ParseError(line, "Missing right brace.");
            return false;
        }
    }
    return status;
}//End of CompStmt
bool ControlStmt(istream& in, int& line) {
    bool status;
    LexItem t = Parser::GetNextToken(in, line);
    switch( t.GetToken() ) {
    case PRINT:
        status = PrintStmt(in, line);
        if(!status)
        {
        ParseError(line, "Incorrect Print Statement");
        return false;
        }
        t = Parser::GetNextToken(in, line);
        if(t == SEMICOL)
        {
        return true;
        }
        else
        {
        ParseError(line, "Missing semicolon at end of Statement.");
        return false;
        }
        break;
    case IDENT:
        Parser::PushBackToken(t);
        status = AssignStmt(in, line);
        if(!status)
        {
        ParseError(line, "Incorrect Assignment Statement");
        return false;
        }
        t = Parser::GetNextToken(in, line);
        if(t == SEMICOL)
        {
        return true;
        }
        else
        {
        ParseError(line, "Missing semicolon at end of Statement.");
        return false;
        }
        break;
    case IF:
        status = IfStmt(in, line);
        if(!status)
        {
        ParseError(line, "Incorrect IF Statement");
        return false;
        }
        break;
    default:
        Parser::PushBackToken(t);
        return true;
    }
    return status;
}//End of ControlStmt

//VarList ::= Var [= Expr] {, Var [= Expr]}
bool VarList(istream& in, int& line, LexItem & idtok) {
	bool status = false, exprstatus = false;
	string identstr;
	Value retVal;
    idtok = Parser::GetNextToken(in, line);
	
	//idtok = Parser::GetNextToken(in, line);
	if(idtok == IDENT)
	{
	
		identstr = idtok.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			defVar[identstr] = true;
		}	
		else
		{
			ParseError(line, "Variable Redefinition");
			return false;
		}
		
	}
	else
	{
	
		ParseError(line, "Missing Variable Name");
		return false;
	}
		
	idtok = Parser::GetNextToken(in, line);
	if(idtok == ASSOP)
	{
	    cout << "before" << retVal<< endl;
		exprstatus = Expr(in, line, retVal);
        cout << "aft" << retVal;
		if(!exprstatus)
		{
			ParseError(line, "Incorrect initialization for a variable.");
			return false;
		}
		
		cout<< "Initialization of the variable " << identstr <<" with value " << retVal << "in the declaration statement at line " << line << endl;
        TempsResults[identstr] = retVal;
        
		idtok = Parser::GetNextToken(in, line);
		
		if (idtok == COMMA) {
			
			status = VarList(in, line, idtok);
		
		}
		else
		{
			Parser::PushBackToken(idtok);
			return true;
		}
	}
	else if (idtok == COMMA) {
		
		status = VarList(in, line, idtok);
	}
	else if(idtok == ERR)
	{
		ParseError(line, "Unrecognized Input Pattern");
		
		return false;
	}
	else {
	
		Parser::PushBackToken(idtok);
		return true;
	}
	
	return status;
	
}//End of VarList
	



//IfStmt ::= IF (Expr) Stmt [ ELSE Stmt ]
bool IfStmt(istream& in, int& line) {
    bool ex=false, status ;
    static int nestlevel = 0;
    Value retVal;
    LexItem t;
    t = Parser::GetNextToken(in, line);
    if( t != LPAREN ) {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    ex = Expr(in, line, retVal);
    if( !ex ) {
        ParseError(line, "Missing if statement condition");
        return false;
    }
    if (!retVal.IsBool()) {
		ParseError(line, "Illegal logic operation in if.");
		return false;
	}
    t = Parser::GetNextToken(in, line);
    if(t != RPAREN ) {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    nestlevel +=1;
    cout << "in IfStmt then-clause at nesting level: " << nestlevel << endl;
    status = Stmt(in, line);
    if(!status){
        ParseError(line, "If-Stmt Syntax Error");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if( t == ELSE ) {
    cout << "in IfStmt else-clause at nesting level: " << nestlevel <<
    endl;
    status = Stmt(in, line);
    if(!status)
    {
    ParseError(line, "Missing Statement for If-Stmt Else-Part");
    return false;
    }
    nestlevel--;
    return true;
    }
    nestlevel--;
    Parser::PushBackToken(t);
    return true;
}//End of IfStmt function

//Var:= ident
bool Var(istream& in, int& line, LexItem& idtok) {
	string identstr;
	LexItem tok = Parser::GetNextToken(in, line);
	if (tok == IDENT) {
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second)) {
			ParseError(line, "Undeclared Variable");
			return false;
		}
		Parser::PushBackToken(tok);
		return true;
	} else if (tok.GetToken() == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}
//End of Var
//AssgnStmt ::= Var ( = | += | -= | *= | /= | %= ) Expr
bool AssignStmt(istream& in, int& line) {
	bool varstatus = false, status = false;
	LexItem t;
	Value val1, val2;
	string identstr;
	varstatus = Var(in, line,t);
	t = Parser::GetNextToken(in, line);
	if (varstatus) {
        
		identstr = t.GetLexeme();
        cout << SymTable[identstr] ;
		if (SymTable[identstr] == BCONST) {
        	val1.SetType(VBOOL);
        }
		else if (SymTable[identstr] == ICONST)
			val1.SetType(VINT);
		else if (SymTable[identstr] == RCONST) {
            val1.SetType(VREAL);
            cout << "I RAN";
        }
			
		else if (SymTable[identstr] == SCONST){
            val1.SetType(VSTRING);
        }
        
			
        
		t = Parser::GetNextToken(in, line);
		if (t == ASSOP) {
			status = Expr(in, line, val2);//--------------------------
			if (!status) {
				ParseError(line, "Missing Expression in Assignment Statement");
				return status;
			}
		}
		else if (t.GetToken() == ERR) {
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		} else {
			ParseError(line, "Missing Assignment Operator");
			return false;
		}
	} else {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	} if (val1.GetType() == VREAL) {
		if (val2.GetType() == VINT) {
			val1.SetReal((float)(val2.GetInt()));
			TempsResults[identstr] = val1;
			return status;
		}
		if (val2.GetType() == VSTRING) {
			ParseError(line, "Illegal mixed-mode assignment operation");
			return false;
		}
        if (val2.GetType() == VBOOL) {
			ParseError(line, "Illegal mixed-mode assignment operation");
			return false;
		}
		TempsResults[identstr] = val2;
	}
	if (val1.GetType() == VINT) {
		if (val2.GetType() == VREAL) {
			val1.SetInt(int(val2.GetReal()));
			TempsResults[identstr] = val1;
			return status;
		}
		if (val2.GetType() == VSTRING) {
			ParseError(line, "String dif in int. Wrong assignment");
			return false;
		}
		TempsResults[identstr] = val2;
	}
	if (val1.GetType() == VSTRING) {
		if (val2.GetType() == VINT || val2.GetType() == VREAL) {
			ParseError(line, "Illegal mixed-mode assignment operation");
			return false;
		}
		TempsResults[identstr] = val2;
	}

	if (val1.GetType() == VBOOL) {
        cout << "I RAN BRO";
		if (val2.GetType() == VINT || val2.GetType() == VREAL || val2.GetType() == VSTRING) {

			ParseError(line, "Illegal mixed-mode assignment operation");
			return false;
		}
		TempsResults[identstr] = val2;
	}
	return status;
}


//Expr ::= LogANDExpr { || LogANDRxpr }
bool Expr(istream& in, int& line, Value& retVal) {
	Value val1, val2;
	LexItem tok;
	bool t1 = LogANDExpr(in, line,val1);
	if (!t1) {
        
		return false;
	}
	retVal = val1;
	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while (tok == OR) {
		t1 = LogANDExpr(in, line,val2);
		if (!t1){
            retVal = val2;
			ParseError(line, "Missing operand after operator");
			return false;
		}
		retVal = retVal || val2;
		if (retVal.IsErr()){
			cout << "Error occurred during logical AND operation." << endl;
		}
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == ERR) {
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}

	}
	Parser::PushBackToken(tok);
	return true;
}
//LogANDExpr ::= EqualExpr { && EqualExpr }
bool LogANDExpr(istream& in, int& line, Value & retVal) {
    Value val1, val2;
    LexItem tok;
    bool t1 = EqualExpr(in, line, val1);
    if( !t1 ) {
        return false;
    }
    retVal = val1;
    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    while ( tok == AND ){
        t1 = EqualExpr(in, line, val2);
        if( !t1 ){
            retVal = val2;
            ParseError(line, "Missing operand after operator");
            return false;
        }
        retVal = retVal && val2;
		if (retVal.IsErr()){
			cout << "Error occurred during logical AND operation." << endl;
		}
        tok = Parser::GetNextToken(in, line);
        if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
return true;
}//End of LogANDExpr
//EqualExpr ::= RelExpr [ (== | !=) RelExpr ]
bool EqualExpr(istream& in, int& line, Value & retVal) {
    Value val1, val2;
    LexItem tok;
    bool t1 = RelExpr(in, line, val1);
    if( !t1 ) {
        return false;
    }
    retVal = val1;
    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    if ( tok == EQ || tok == NEQ )
    {
        t1 = RelExpr(in, line, val2);
        if( !t1 )
        {
            retVal = val2;
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if (tok == EQ){
            retVal = retVal == val2;
            if (retVal.IsErr()){
                cout << "Error occurred during logical EQ operation." << endl;
            }
        }
        if (tok == NEQ){
            retVal = retVal != val2;
            if (retVal.IsErr()){
                cout << "Error occurred during logical NEQ operation." << endl;
            }
        }
        tok = Parser::GetNextToken(in, line);
        if(tok == EQ || tok == NEQ)
        {
            ParseError(line, "Illegal Equality Expression.");
            return false;
        }
        else if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
    return true;
}//End of EqualExpr
           
//RelExpr ::= AddExpr [ ( < | > ) AddExpr ]
bool RelExpr(istream& in, int& line, Value & retVal) {
    Value val1, val2;
    LexItem tok;
    bool t1 = AddExpr(in, line, val1);
    if( !t1 ) {
        return false;
    }
    retVal = val1;
    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    if ( tok == LTHAN || tok == GTHAN)
    {
        t1 = AddExpr(in, line, val2);
        if( !t1 ){
            retVal = val2;
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if (tok  == LTHAN){
            retVal = retVal < val2;
            if (retVal.IsErr()){
                cout << "Error occurred during logical < operation." << endl;
            }
        }
        if (tok  == GTHAN){
            retVal = retVal > val2;
            if (retVal.IsErr()){
                cout << "Error occurred during logical > operation." << endl;
            }
        }
        tok = Parser::GetNextToken(in, line);
        if(tok == LTHAN || tok == GTHAN)
        {
            ParseError(line, "Illegal Relational Expression.");
            return false;
        }
        else if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
    Parser::PushBackToken(tok);
return true;
}//End of RelExpr
//AddExpr :: MultExpr { ( + | - ) MultExpr }
   bool AddExpr(istream& in, int& line, Value & retVal) {
   Value val1;
   bool t1 = MultExpr(in, line, retVal);
   LexItem tok;
   if( !t1 ) {
       return false;
   }
   retVal = val1;
   tok = Parser::GetNextToken(in, line);
   if(tok.GetToken() == ERR){
       ParseError(line, "Unrecognized Input Pattern");
       cout << "(" << tok.GetLexeme() << ")" << endl;
       return false;
    }
   while ( tok == PLUS || tok == MINUS ) {
       t1 = MultExpr(in, line, retVal);
       if (tok == PLUS){
           retVal = retVal + val1;
       }
       else if (tok == MINUS){
           retVal = retVal - val1;
       }
       if( !t1 )
       {
           ParseError(line, "Missing operand after operator");
           return false;
       }
       if (retVal.GetType() == VERR) {
			ParseError(line, "Illegal operand types for an arithmetic operator");
			return false;
		}
       tok = Parser::GetNextToken(in, line);
       if(tok.GetToken() == ERR){
           ParseError(line, "Unrecognized Input Pattern");
           cout << "(" << tok.GetLexeme() << ")" << endl;
           return false;
       }
   }
   Parser::PushBackToken(tok);
   return true;
}//End of AddExpr
//MultExpr ::= UnaryExpr { ( * | / | %) UnaryExpr }
bool MultExpr(istream& in, int& line, Value & retVal) {
    bool t1 = UnaryExpr(in, line, retVal);
    Value val1;
    LexItem tok;
    if( !t1 ) {
        return false;
    }
    retVal = val1;
    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    while ( tok == MULT || tok == DIV || tok == REM){
        t1 = UnaryExpr(in, line, retVal);
        if (tok == MULT)
			retVal = retVal * val1;
		else if (tok == REM) {
			retVal = retVal % val1;
		} else {
			if (val1.GetInt() == 0) {
				ParseError(line, "Run-Time Error-Illegal division by Zero");
				return false;
			} else {
				retVal = retVal / val1;
			}
		}
        if( !t1 ) {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        if (retVal.GetType() == VERR) {
			ParseError(line, "Illegal operand types for an arithmetic operator");
			return false;
		}
        tok = Parser::GetNextToken(in, line);
        if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
    }
Parser::PushBackToken(tok);
return true;
}//End of MultExpr
//UnaryExpr ::= ( - | + | ! ) PrimaryExpr | PrimaryExpr
bool UnaryExpr(istream& in, int& line, Value & retVal){
    LexItem t = Parser::GetNextToken(in, line);
    bool status;
    int sign = 0;
    if(t == MINUS ){
        sign = -1;
        
    }
    else if(t == PLUS){
        sign = 1;
        
    }
    else if (t == NOT){
        sign = 2;
        
    }
    else
        Parser::PushBackToken(t);
    status = PrimaryExpr(in, line, sign, retVal);
return status;
    
}//End of UnaryExpr
           
//PrimaryExpr ::= IDENT | ICONST | RCONST | SCONST | BCONST | CCONST | ( Expr )
bool PrimaryExpr(istream& in, int& line, int sign, Value & retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    cout << retVal<< "bRAH" << endl;
	if (tok == IDENT) {
		string lexeme = tok.GetLexeme();
		if (!(defVar.find(lexeme)->second)) 
        {
            retVal = Value();
			ParseError(line, "Using Undefined Variable");
			return false;
		}
		//retVal = TempsResults[lexeme];
		if (TempsResults.find(lexeme) == TempsResults.end()) {
            // If lexeme is not found in TempsResults, handle the error
            cout << lexeme;
            ParseError(line, "Using uninitialized Variable");
            return false;
        } else if (TempsResults[lexeme].GetType() == VERR) {
            // If the lexeme is found but its type is VERR (error type)
            cout << lexeme;
            ParseError(line, "Using uninitialized Variable");
            return false;
        }
        if (sign == -1  && (retVal.GetType() == VSTRING || retVal.GetType() == VBOOL)) {
			ParseError(line, "Illegal Operand Type for Sign/NOT Operator");
			return false;
		}
		if (sign == 1  && (retVal.GetType() == VSTRING || retVal.GetType() == VBOOL)) {
			ParseError(line, "Illegal Operand Type for Sign/NOT Operator");
			return false;
		}
		if (sign == 2 && (retVal.GetType() == VINT || retVal.GetType() == VREAL)) {
			ParseError(line, "Illegal Operand Type for Sign/NOT Operator");
			return false;
		}
		if (retVal.GetType() == VINT)
			retVal.SetInt(sign * retVal.GetInt());
		if (retVal.GetType() == VREAL)
			retVal.SetReal(sign * retVal.GetReal());
		return true;
	} else if (tok == ICONST) {
		string lexeme = tok.GetLexeme();
		double temp = stoi(lexeme);
		retVal.SetType(VINT);
		retVal.SetInt(sign * temp);
		return true;
	} else if (tok == SCONST) {
		retVal.SetType(VSTRING);
		retVal.SetString( tok.GetLexeme());
		return true;
	} else if (tok == RCONST) {
		retVal.SetType(VREAL);
		retVal.SetReal(sign * stof(tok.GetLexeme()));
		return true;
	} else if (tok == BCONST) {
		bool temp =false;
		if (tok.GetLexeme() == "true")
			temp = true;
		retVal.SetType(VBOOL);

		if (sign == -2) {
			retVal.SetBool(!temp );
		} else {
			retVal.SetBool(temp);
		}
		return true;
	} else if (tok == CCONST){
        retVal.SetType(VCHAR);
        char c = tok.GetLexeme()[0];
		retVal.SetChar(c);
        return true;
    }
    else if (tok == LPAREN) {
		bool ex = Expr(in, line,retVal);
		if (!ex) {
			ParseError(line, "Missing expression after (");
			return false;
		}
		tok = Parser::GetNextToken(in, line);
		if (tok == RPAREN) {
			return ex;
		} else {
			Parser::PushBackToken(tok);
			ParseError(line, "Missing ) after expression");
			return false;
		}
	} else if (tok.GetToken() == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	Parser::PushBackToken(tok);
	return false;
   
}//End of PrimaryExpr
