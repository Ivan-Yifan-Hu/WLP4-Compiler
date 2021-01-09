#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using namespace std;

map<string, pair<vector<string>, map<string, string>>> tables;

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



int main() {
	Tree * parent = makeTree();
	try {
		symbolTable(parent);
		for (auto i : tables) {
			cerr << i.first + ": ";
			for (auto m : i.second.first) {
				cerr << m << " ";
			}
			cerr << endl;
			for (auto p : i.second.second) {
				cerr << p.first << " " << p.second << endl;
			}
		}
		deletingTree (parent);
	}
	catch (ParsingError & e) {
		cerr << e.errorMessage << endl;
		deletingTree (parent);
		return 0;
	}
}
