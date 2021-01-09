#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using namespace std;

map<string, pair<vector<string>, map<string, string>>> tables;

int count = 0;

class Tree {
	public:
	string value;
	string rule;
	string type = "";
	vector<string> tokens;
	vector<Tree *> children;
};

class ParsingError {
	public:
	string errorMessage;
};

bool allCap (string s) {
	for (int i = 0; i < s.length(); i++) {
		if ((65 <= s[i]) & (s[i] <= 90)) {
			continue;
		} else {
			return false;
		}
	}
	return true;
}

Tree * makeTree() {
	Tree * parent = new Tree();
	string line;
	getline(cin, line);
	parent->rule = line;
	stringstream s(line);
	string head;
	s >> head;
	parent->value = head;
	string body;
	if (allCap(head)) {
		s >> body;
		parent->tokens.emplace_back(body);
	} else {
		while (s >> body) {
			parent->tokens.emplace_back(body);
		}
		int number = parent->tokens.size();
		for (int i = 0; i < number; i++) {
			parent->children.emplace_back(makeTree());
		}
	}
	return parent;
}

void deletingTree (Tree * parent) {
	if (!allCap(parent->value)) {
		int number = parent->children.size();
		for (int i = 0; i < number; i++) {
			deletingTree(parent->children[i]);
		}
	}
	delete parent;
}

void stHelper (Tree * parent, map<string, string> & table) {
	if (parent->value == "dcl") {
		if (parent->children[0]->children.size() == 1) {
			if (table.count(parent->children[1]->tokens[0]) == 0) {
				table[parent->children[1]->tokens[0]] = "int";
			} else {
				string message = "ERROR: duplicate variables with name " + parent->children[1]->tokens[0] + " found";
                        	ParsingError p = ParsingError();
                        	p.errorMessage = message;
                        	throw p;
			}
		} else {
			if (table.count(parent->children[1]->tokens[0]) == 0) {
                                table[parent->children[1]->tokens[0]] = "int*";
                        } else {
                                string message = "ERROR: duplicate variables with name " + parent->children[1]->tokens[0] + " found";
                                ParsingError p = ParsingError();
                                p.errorMessage = message;
                                throw p;
                        }
		}
	}
	int number = parent->children.size();
	for (int i = 0; i < number; i++) {
		stHelper(parent->children[i], table);
	}
}


void sgHelper (Tree * parent, vector<string> & table1) {
        if (parent->value == "dcl") {
                if (parent->children[0]->children.size() == 1) {
                        table1.emplace_back("int");
                } else {
                        table1.emplace_back("int*");
                }
        }
        int number = parent->children.size();
        for (int i = 0; i < number; i++) {
		sgHelper(parent->children[i], table1);
	}
}


bool varHelper (Tree * parent, map<string, string> & table) {
	if ((parent->rule == "factor ID") || (parent->rule == "lvalue ID")) {
		if (table.count(parent->children[0]->tokens[0]) == 0) {
			string message = "ERROR: variable " + parent->children[0]->tokens[0] + " declared before use";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
	} else if ((parent->rule == "factor ID LPAREN RPAREN") || (parent->rule == "factor ID LPAREN arglist RPAREN")) {
		if (table.count(parent->children[0]->tokens[0]) == 1) {
			string message = "ERROR: procedure " + parent->children[0]->tokens[0] + " with overlapping variable name";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
		if (tables.count(parent->children[0]->tokens[0]) == 0) {
			string message = "ERROR: procedrue " + parent->children[0]->tokens[0] + " used before declared";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
	}
	int number = parent->children.size();
	for (int i = 0; i < number; i++) {
		varHelper(parent->children[i], table);
	}
}


void typeOf (Tree * parent, map<string, string> & table);


void argHelper (Tree * parent, map<string, string> & table, vector<string> & temp) {
        if (parent->value == "expr") {
                typeOf(parent, table);
                temp.emplace_back(parent->type);
        } else {
		for (int i = 0; i < parent->children.size(); i++) {
                	argHelper(parent->children[i], table, temp);
		}
	}
}

bool sgEqual (Tree * parent, vector<string> & temp) {
	if (temp.size() != tables[parent->children[0]->tokens[0]].first.size()) {
		return false;
	}
	for (int i = 0; i < temp.size(); i++) {
		if (temp[i] != tables[parent->children[0]->tokens[0]].first[i]) {
			return false;
		}
	}
	return true;
}

void typeOf (Tree * parent, map<string, string> & table) {
	if (parent->type != "") {
		return;
	} else if (parent->value == "NUM") {
		parent->type = "int";
	} else if (parent->value == "NULL") {
		parent->type = "int*";
	} else if (parent->rule == "factor NUM") {
		parent->type = "int";
	} else if (parent->rule == "factor NULL") {
		parent->type = "int*";
	} else if (parent->rule == "factor ID") {
		parent->children[0]->type = table[parent->children[0]->tokens[0]];
		parent->type = parent->children[0]->type;
	} else if (parent->rule == "lvalue ID") {
                parent->children[0]->type = table[parent->children[0]->tokens[0]];
                parent->type = parent->children[0]->type;
	} else if (parent->rule == "factor LPAREN expr RPAREN") {
		typeOf(parent->children[1], table);
		parent->type = parent->children[1]->type;
	} else if (parent->rule == "lvalue LPAREN lvalue RPAREN") {
                typeOf(parent->children[1], table);
                parent->type = parent->children[1]->type;
	} else if (parent->rule == "factor AMP lvalue") {
		parent->type = "int*";
		typeOf(parent->children[1], table);
		if (parent->children[1]->type != "int") {
			string message = "ERROR: the type of lvalue in factor AMP lvalue must be int";
			ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
	} else if ((parent->rule == "factor STAR factor") || (parent->rule == "lvalue STAR factor")) {
		parent->type = "int";
		typeOf(parent->children[1], table);
		if (parent->children[1]->type != "int*") {
                        string message = "ERROR: the type of factor in factor STAR factor or lvalue STAR factor must be int*";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
                }
	} else if (parent->rule == "factor NEW INT LBRACK expr RBRACK") {
		parent->type = "int*";
		typeOf(parent->children[3], table);
		if (parent->children[3]->type != "int") {
			string message = "ERROR: the type of expr in factor NEW INT LBRACK expr RBRACK must be int";
			ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
	} else if (parent->rule == "factor ID LPAREN RPAREN") {
		parent->type = "int";
		if (tables[parent->children[0]->tokens[0]].first.size() != 0) {
		       string message = "ERROR: the procedure " + parent->children[0]->tokens[0] + " must have an empty signature";
		       ParsingError p = ParsingError();
                       p.errorMessage = message;
                       throw p;
		}
	} else if (parent->rule == "factor ID LPAREN arglist RPAREN") {
		parent->type = "int";
		vector<string> temp;
		argHelper(parent->children[2], table, temp);
		if (!sgEqual(parent, temp)) {
			string message = "ERROR: procedrue " + parent->children[0]->tokens[0] + " called with incorrect signature";
		       ParsingError p = ParsingError();
                       p.errorMessage = message;
                       throw p;
		}
	} else if (parent->rule == "term factor") {
		typeOf(parent->children[0], table);
		parent->type = parent->children[0]->type;
	} else if (parent->value == "term") {
		parent->type = "int";
		typeOf(parent->children[0], table);
		typeOf(parent->children[2], table);
		if ((parent->children[0]->type != "int") || (parent->children[2]->type != "int")) {
			string message = "ERROR: The term and factor directly derived from a term must have type int";
			ParsingError p = ParsingError();
                       	p.errorMessage = message;
                       	throw p;
		}
	} else if (parent->rule == "expr term") {
		typeOf(parent->children[0], table);
		parent->type = parent->children[0]->type;
	} else if (parent->rule == "expr expr PLUS term") {
		typeOf(parent->children[0], table);
		typeOf(parent->children[2], table);
		if ((parent->children[0]->type == "int") && (parent->children[2]->type == "int")) {
			parent->type = "int";
		} else if ((parent->children[0]->type == "int*") && (parent->children[2]->type == "int")) {
                        parent->type = "int*";
		} else if ((parent->children[0]->type == "int") && (parent->children[2]->type == "int*")) {
                        parent->type = "int*";
		} else {
			string message = "ERROR: ivalid opperends for PLUS";
			ParsingError p = ParsingError();
                       	p.errorMessage = message;
                       	throw p;
		}
	} else if (parent->rule == "expr expr MINUS term") {
		typeOf(parent->children[0], table);
                typeOf(parent->children[2], table);
                if ((parent->children[0]->type == "int") && (parent->children[2]->type == "int")) {
                        parent->type = "int";
		} else if ((parent->children[0]->type == "int*") && (parent->children[2]->type == "int")) {
                        parent->type = "int*";
		} else if ((parent->children[0]->type == "int*") && (parent->children[2]->type == "int*")) {
                        parent->type = "int";
		} else {
			string message = "ERROR: ivalid opperends for MINUS";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
	}
	for (int i = 0; i < parent->children.size(); i++) {
		typeOf(parent->children[i], table);
	}
}


void wellType (Tree * parent) {
	if (parent->rule == "statement lvalue BECOMES expr SEMI") {
		if (parent->children[0]->type != parent->children[2]->type) {
			string message = "ERROR: invalid assignment";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
	} else if (parent->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		if (parent->children[2]->type != "int") {
			string message = "ERROR: invalid PRINTLN";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
	} else if (parent->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
		if (parent->children[3]->type != "int*") {
                        string message = "ERROR: invalid DELETE";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
                }
	} else if (parent->value == "test") {
		if (parent->children[0]->type != parent->children[2]->type) {
			string message = "ERROR: invalid TEST";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
                }
	} else if (parent->rule == "dcls dcls dcl BECOMES NUM SEMI") {
		if (parent->children[1]->children[0]->children.size() != 1) {
			string message = "ERROR: invalid NUM initialization";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
		}
	} else if (parent->rule == "dcls dcls dcl BECOMES NULL SEMI") {
                if (parent->children[1]->children[0]->children.size() != 2) {
                        string message = "ERROR: invalid POINTER initialization";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
                }
	}
	for (int i = 0; i < parent->children.size(); i++) {
                wellType(parent->children[i]);
        }
}




void symbolTable (Tree * parent) {
        if (parent->value == "procedure") {
                if (tables.count(parent->children[1]->tokens[0]) == 0) {
                        map<string, string> parameters;
                        vector<string> signitures;
                        tables[parent->children[1]->tokens[0]].first = signitures;
                        tables[parent->children[1]->tokens[0]].second = parameters;
                        sgHelper(parent->children[3], signitures);
                        stHelper(parent->children[3], parameters);
                        stHelper(parent->children[6], parameters);
                        varHelper(parent->children[7], parameters);
                        varHelper(parent->children[9], parameters);
                        tables[parent->children[1]->tokens[0]].first = signitures;
                        tables[parent->children[1]->tokens[0]].second = parameters;
                        typeOf(parent->children[7], parameters);
                        typeOf(parent->children[9], parameters);
			wellType(parent->children[6]);
			wellType(parent->children[7]);
			if (parent->children[9]->type != "int") {
                                string message = "ERROR: procedure msut return INT";
                                ParsingError p = ParsingError();
                                p.errorMessage = message;
                                throw p;
                        }
                } else {
                        string message = "ERROR: duplicate procedures with name " + parent->children[1]->tokens[0] + " found";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;

                }
        } else if (parent->value == "main") {
                if (tables.count("wain") == 0) {
                        map<string, string> parameters;
                        vector<string> signitures;
                        tables["wain"].first = signitures;
                        tables["wain"].second = parameters;
                        sgHelper(parent->children[3], signitures);
                        sgHelper(parent->children[5], signitures);
                        stHelper(parent->children[3], parameters);
                        stHelper(parent->children[5], parameters);
                        stHelper(parent->children[8], parameters);
                        varHelper(parent->children[9], parameters);
                        varHelper(parent->children[11], parameters);
			if (parent->children[5]->children[0]->children.size() != 1) {
                                string message = "ERROR: second parameter of wain must be INT";
                                ParsingError p = ParsingError();
                                p.errorMessage = message;
                                throw p;
                        }
                        tables["wain"].first = signitures;
                        tables["wain"].second = parameters;
                        typeOf(parent->children[9], parameters);
                        typeOf(parent->children[11], parameters);
                	wellType(parent->children[8]);
			wellType(parent->children[9]);
			if (parent->children[11]->type != "int") {
				string message = "ERROR: wain msut return INT";
                        	ParsingError p = ParsingError();
                        	p.errorMessage = message;
                        	throw p;
			}
		} else {
                        string message = "ERROR: duplicate wain found";
                        ParsingError p = ParsingError();
                        p.errorMessage = message;
                        throw p;
                }
        }
        for (int i = 0; i < parent->children.size(); i++) {
                symbolTable(parent->children[i]);
        }
}


void offsetTableGeneration (Tree * parent, map<string, pair<int, string>> & offsetTable, int & offset) {
	if (parent->value == "dcl") {
		offsetTable[parent->children[1]->tokens[0]].first = offset;
		offsetTable[parent->children[1]->tokens[0]].second = parent->children[0]->tokens[0];
		offset -= 4;
	}
	for (int i = 0; i < parent->children.size(); i++) {
		offsetTableGeneration(parent->children[i], offsetTable, offset);
	}
}
	

void prolouge() {
	string output = ".import print\n.import init\n.import new\n.import delete\nlis $4\n.word 4\nlis $10\n.word print\nlis $11\n.word 1\nlis $12\n.word init\nlis $13\n.word new\nlis $14\n.word delete\nsub $29, $30, $4\n";
	cout << output;
}

void push (string reg) {
	string output = "sw " + reg + ", -4($30)\nsub $30, $30, $4\n";
	cout << output;
}

void pop (string reg) {
	string output = "add $30, $30, $4\nlw " + reg + ", -4($30)\n";
	cout << output;
}

void epilogue (int num) {
	for (int i = 0; i < num; i++) {
		cout << "add $30, $30, $4\n";
	}
	cout << "jr $31\n";
}

void dcls (Tree * parent) {
	if (parent->value == "NUM") {
		cout << "lis $5\n.word " + parent->tokens[0] + "\n";
	       	push("$5");
	} else if (parent->value == "NULL") {
		cout << "lis $5\n.word 1\n";
		push("$5");
	}
	for (int i = 0; i < parent->children.size(); i++) {
                dcls(parent->children[i]);
        }
}

string lvalue (Tree * parent) {
	if (parent->rule == "lvalue ID") {
		return parent->children[0]->tokens[0];
	} else if (parent->rule == "lvalue LPAREN lvalue RPAREN") {
		return lvalue(parent->children[1]);
	}
}

int counting (Tree * parent, int time) {
	if (parent->rule == "arglist expr") {
		time++;
                return time;
        } else if (parent->rule == "arglist expr COMMA arglist") {
		time++;
		return counting(parent->children[2], time);
	}
}




void code (Tree * parent, map<string, pair<int, string>> & offsetTable) {
	if (parent->rule == "statements statements statement") {
		code(parent->children[0], offsetTable);
		code(parent->children[1], offsetTable);
	} else if (parent->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
		code(parent->children[2], offsetTable);
		string el = "else" + to_string(count);
		count++;
		cout << "beq $3, $0, " + el + "\n";
		code(parent->children[5], offsetTable);
		string endif = "endif" + to_string(count);
		count++;
		cout << "beq $0, $0, " + endif + "\n";
		cout << el + ":\n";
		code(parent->children[9], offsetTable);
		cout << endif << ":\n";
	} else if (parent->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
		string loop = "loop" + to_string(count);
		count++;
		cout << loop + ":\n";
		code(parent->children[2], offsetTable);
		string endWhile = "enddWhile" + to_string(count);
		count++;
		cout << "beq $3, $0, " + endWhile + "\n";
		code(parent->children[5], offsetTable);
		cout << "beq $0, $0, " + loop + "\n";
		cout << endWhile + ":\n";
	} else if (parent->rule == "factor LPAREN expr RPAREN") {
		code(parent->children[1], offsetTable);
	} else if (parent->rule == "expr term") {
		code(parent->children[0], offsetTable);
	} else if (parent->rule == "term factor") {
		code(parent->children[0], offsetTable);
	} else if (parent->rule == "factor ID") {
		cout << "lw $3, " + to_string(offsetTable[parent->children[0]->tokens[0]].first) + "($29)\n";
	} else if (parent->rule == "factor NUM") {
		cout << "lis $3\n.word " + parent->children[0]->tokens[0] + "\n";
	} else if (parent->rule == "term term STAR factor") {
		code(parent->children[0], offsetTable);
		push("$3");
		code(parent->children[2], offsetTable);
		pop("$5");
		cout << "mult $3, $5\nmflo $3\n";
	} else if (parent->rule == "term term SLASH factor") {
		code(parent->children[0], offsetTable);
		push("$3");
		code(parent->children[2], offsetTable);
		pop("$5");
		cout << "div $5, $3\nmflo $3\n";
	} else if (parent->rule == "term term PCT factor") {
		code(parent->children[0], offsetTable);
		push("$3");
		code(parent->children[2], offsetTable);
		pop("$5");
		cout << "div $5, $3\nmfhi $3\n";
	} else if (parent->rule == "statement lvalue BECOMES expr SEMI") {
		code(parent->children[2], offsetTable);
		code(parent->children[0], offsetTable);
	} else if (parent->rule == "lvalue ID") {
		cout << "sw $3, " + to_string(offsetTable[parent->children[0]->tokens[0]].first) + "($29)\n";
	} else if (parent->rule == "lvalue LPAREN lvalue RPAREN") {
			code(parent->children[1], offsetTable);
	} else if (parent->rule == "lvalue STAR factor") {
		push("$3");
		code(parent->children[1], offsetTable);
		pop("$5");
		cout << "sw $5, 0($3)\n";
	} else if (parent->rule == "test expr LT expr") {
		code(parent->children[0], offsetTable);
		push("$3");
		code(parent->children[2], offsetTable);
		pop("$5");
		if (parent->children[0]->type == "int") {
                	cout << "slt $3, $5, $3\n";
		} else if (parent->children[0]->type == "int*") {
                	cout << "sltu $3, $5, $3\n";
		}
	} else if (parent->rule == "test expr GT expr") {
		code(parent->children[2], offsetTable);
                push("$3");
                code(parent->children[0], offsetTable);
                pop("$5");
		if (parent->children[0]->type == "int") {
                        cout << "slt $3, $5, $3\n";
                } else if (parent->children[0]->type == "int*") {
                        cout << "sltu $3, $5, $3\n";
                }
	} else if (parent->rule == "test expr NE expr") {
		code(parent->children[0], offsetTable);
		push("$3");
		code(parent->children[2], offsetTable);
		pop("$5");
		if (parent->children[0]->type == "int") {
			cout << "slt $6, $3, $5\n";
			cout << "slt $7, $5, $3\n";
			cout << "add $3, $6, $7\n";
		} else if (parent->children[0]->type == "int*") {
			cout << "sltu $6, $3, $5\n";
                        cout << "sltu $7, $5, $3\n";
                        cout << "add $3, $6, $7\n";
		}
	} else if (parent->rule == "test expr EQ expr") {
                code(parent->children[0], offsetTable);
                push("$3");
                code(parent->children[2], offsetTable);
                pop("$5");
		if (parent->children[0]->type == "int") {
                        cout << "slt $6, $3, $5\n";
                        cout << "slt $7, $5, $3\n";
                        cout << "add $3, $6, $7\n";
                } else if (parent->children[0]->type == "int*") {
                        cout << "sltu $6, $3, $5\n";
                        cout << "sltu $7, $5, $3\n";
                        cout << "add $3, $6, $7\n";
                }
                cout << "sub $3, $11, $3\n";
	} else if (parent->rule == "test expr LE expr") {
                code(parent->children[2], offsetTable);
                push("$3");
                code(parent->children[0], offsetTable);
                pop("$5");
		if (parent->children[0]->type == "int") {
                	cout << "slt $3, $5, $3\n";
                	cout << "sub $3, $11, $3\n";
		} else if (parent->children[0]->type == "int*") {
                	cout << "sltu $3, $5, $3\n";
			cout << "sub $3, $11, $3\n";
		}
	} else if (parent->rule == "test expr GE expr") {
                code(parent->children[0], offsetTable);
                push("$3");
                code(parent->children[2], offsetTable);
                pop("$5");
		if (parent->children[0]->type == "int") {
                        cout << "slt $3, $5, $3\n";
                        cout << "sub $3, $11, $3\n";
                } else if (parent->children[0]->type == "int*") {
                        cout << "sltu $3, $5, $3\n";
                        cout << "sub $3, $11, $3\n";
                }
	} else if (parent->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		push("$1");
		code(parent->children[2], offsetTable);
		cout << "add $1, $3, $0\n";
		push("$31");
		cout << "jalr $10\n";
		pop("$31");
		pop("$1");
	} else if (parent->rule == "factor NULL") {
		cout << "add $3, $0, $11\n";
	} else if (parent->rule == "factor STAR factor") {
		code(parent->children[1], offsetTable);
		cout << "lw $3, 0($3)\n";
	} else if (parent->rule == "factor AMP lvalue") {
		if (parent->children[1]->rule == "lvalue ID") {
			cout << "lis $3\n.word " + to_string(offsetTable[parent->children[1]->children[0]->tokens[0]].first) + "\nadd $3, $3, $29\n";
		} else if (parent->children[1]->rule == "lvalue STAR factor") {
			code(parent->children[1]->children[1], offsetTable);
		}
	} else if (parent->rule == "factor NEW INT LBRACK expr RBRACK") {
		code(parent->children[3], offsetTable);
		cout << "add $1, $3, $0\n";
		push("$31");
		cout << "jalr $13\n";
		pop("$31");
		cout << "bne $3, $0, 1\nadd $3, $11, $0\n";
	} else if (parent->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
		code(parent->children[3], offsetTable);
		string label = "skipDelete" + to_string(count);
		count++;
		cout << "beq $3, $11, " + label + "\nadd $1, $3, $0\n";
		push("$31");
		cout << "jalr $14\n";
		pop("$31");
		cout << label + ":\n";
	} else if (parent->rule == "expr expr PLUS term") {
		if ((parent->children[0]->type == "int") && (parent->children[2]->type == "int")) {
			code(parent->children[0], offsetTable);
			push("$3");
			code(parent->children[2], offsetTable);
			pop("$5");
			cout << "add $3, $5, $3\n";
		} else if (parent->children[0]->type == "int*") {
			code(parent->children[0], offsetTable);
			push("$3");
			code(parent->children[2], offsetTable);
			cout << "mult $3, $4\nmflo $3\n";
			pop("$5");
			cout << "add $3, $5, $3\n";
		} else {
			code(parent->children[2], offsetTable);
			push("$3");
			code(parent->children[0], offsetTable);
			cout << "mult $3, $4\nmflo $3\n";
                        pop("$5");
                        cout << "add $3, $5, $3\n";
		}
	} else if (parent->rule == "expr expr MINUS term") {
		if ((parent->children[0]->type == "int") && (parent->children[2]->type == "int")) {
                        code(parent->children[0], offsetTable);
                        push("$3");
                        code(parent->children[2], offsetTable);
                        pop("$5");
                        cout << "sub $3, $5, $3\n";
		} else if (parent->children[2]->type == "int") {
			code(parent->children[0], offsetTable);
                        push("$3");
                        code(parent->children[2], offsetTable);
                        cout << "mult $3, $4\nmflo $3\n";
                        pop("$5");
                        cout << "sub $3, $5, $3\n";
		} else {
			code(parent->children[0], offsetTable);
			push("$3");
			code(parent->children[2], offsetTable);
			pop("$5");
			cout << "sub $3, $5, $3\ndiv $3, $4\nmflo $3\n";
		} 
	} else if (parent->rule == "factor ID LPAREN RPAREN") {
		push("$29");
		push("$31");
		cout << "lis $5\n.word F" + parent->children[0]->tokens[0] + "\njalr $5\n";
		pop("$31");
		pop("$29");
	} else if (parent->rule == "factor ID LPAREN arglist RPAREN") {
		push("$29");
		push("$31");
		code(parent->children[2], offsetTable);
		cout << "lis $5\n.word F" + parent->children[0]->tokens[0] + "\njalr $5\n";
		int time = counting(parent->children[2], 0);
		for (int i = 0; i < time; i++) {
			pop("$5");
		}
		pop("$31");
		pop("$29");
	} else if (parent->rule == "arglist expr") {
		code(parent->children[0], offsetTable);
		push("$3");
	} else if (parent->rule == "arglist expr COMMA arglist") {
		code(parent->children[0], offsetTable);
		push("$3");
		code(parent->children[2], offsetTable);
	}
}

void countArg(Tree * parent, int & argNum) {
	if (parent->value == "dcl") {
		argNum++;
	}
	for (int i = 0; i < parent->children.size(); i++) {
		countArg(parent->children[i], argNum);
	}
}


void generation (Tree * parent) {
	if (parent->value == "main") {
		map<string, pair<int, string>> offsetTable;
                int offset = 0;
                offsetTableGeneration(parent, offsetTable, offset);
                prolouge();
		if (parent->children[3]->children[0]->children.size() == 2) {
			push("$31");
			cout << "jalr $12\n";
			pop("$31");
		} else {
			push("$2");
			cout << "lis $2\n.word 0\n";
			push("$31");
			cout << "jalr $12\n";
			pop("$31");
			pop("$2");
		}
                push("$1");
                push("$2");
                dcls(parent->children[8]);
		code(parent->children[9], offsetTable);
                code(parent->children[11], offsetTable);
                epilogue(offsetTable.size());
		return;
	} else if (parent->value == "procedure") {
		map<string, pair<int, string>> offsetTable;
                int offset = 0;
                offsetTableGeneration(parent, offsetTable, offset);
		
		int argNum = 0;
		countArg(parent->children[3], argNum);
		for (auto & i : offsetTable) {
			i.second.first += 4 * argNum;
		}
		
		cout << "F" + parent->children[1]->tokens[0] + ":\n";
		cout << "sub $29, $30, $4\n";
		dcls(parent->children[6]);
		push("$1");
		push("$2");
		push("$5");
		push("$6");
		push("$7");
		code(parent->children[7], offsetTable);
		code(parent->children[9], offsetTable);
		pop("$7");
		pop("$6");
		pop("$5");
		pop("$2");
		pop("$1");
		cout << "add $30, $29, $4\njr $31\n";
		return;
	}
}
		



void functionGeneration (Tree * parent) {
	if (parent->children.size() == 2) {
		generation(parent->children[0]);
		functionGeneration(parent->children[1]);
	}
}

void mainGeneration (Tree * parent) {
	if (parent->children.size() == 1) {
		generation(parent->children[0]);
	} else {
		mainGeneration(parent->children[1]);
	}
}


int main() {
	Tree * parent = makeTree();
	try {
		symbolTable(parent);

		/*for (auto i : tables) {
			cerr << i.first + ": ";
			for (auto m : i.second.first) {
				cerr << m << " ";
			}
			cerr << endl;
			for (auto p : i.second.second) {
				cerr << p.first << " " << p.second << endl;
			}
		}*/

		mainGeneration(parent->children[1]);
		functionGeneration(parent->children[1]);
		deletingTree (parent);
	}
	catch (ParsingError & e) {
		cerr << e.errorMessage << endl;
		deletingTree (parent);
		return 0;
	}
}
