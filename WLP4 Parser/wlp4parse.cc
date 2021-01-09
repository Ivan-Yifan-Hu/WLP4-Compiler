#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <sstream>
#include <fstream>
using namespace std;

class Tree {
	public:
	string value;
	stack <Tree *> children;
};

void deleting (Tree * parent) {
	while (!parent->children.empty()) {
                deleting(parent->children.top());
                parent->children.pop();
        }
        delete parent;
}

void preOrder (Tree * parent) {
	cout << parent->value << endl;
	while (!parent->children.empty()) {
		preOrder(parent->children.top());
		parent->children.pop();
	}
	delete parent;
}


int main() {
	ifstream infile;
	infile.open("wlp4grammar.txt");
	// construct CFG and DFA using input
	map <string, int> nonTerminals;
	vector <string> allSymbols;
	// table for all derivations
	map <int, vector<string>> derivations;
	int tempNum;
	string q;
	string sym;
	infile >> tempNum;
	for (int i = 0; i < tempNum; i++) {
		infile >> sym;
		allSymbols.emplace_back(sym);
	}
	infile >> tempNum;
	for (int i = 0; i < tempNum; i++) {
		infile >> sym;
		nonTerminals[sym] = i;
	}
	infile >> q;
	infile >> tempNum;
	infile.ignore(256, '\n');
	for (int i = 0; i < tempNum; i++) {
		vector <string> line;
		string derivation;
		getline(infile, derivation);
		stringstream s(derivation);
		string token;
		while (s >> token) {
			line.emplace_back(token);
		}
		derivations[i] = line;
	}
	int stateNum;
	infile >> stateNum;
	map <string, string> reduceTable;
	map <string, string> shiftTable;
	infile >> tempNum;
	infile.ignore(256, '\n');
	for (int i = 0; i < tempNum; i++) {
		string transition;
		getline(infile, transition);
		stringstream s(transition);
		string state1;
		string symbol;
		string form;
		string state2;
		s >> state1 >> symbol >> form >> state2;
		if (form == "reduce") {
			reduceTable[state1 + symbol] = state2;
		} else {
			shiftTable[state1 + symbol] = state2;
		}
	}
	infile.close();

	// Parse input
	// Make Tree
	Tree * parent = new Tree();
	string input;
	stack <Tree *> symStack;
	stack <string> stateStack;
	stateStack.push("0");
	int temp = 0;
	input = "BOF BOF";
	parent->value = input;
	string t;
        string l;
        stringstream s(input);  
	s >> t >> l;
	symStack.push(parent);
	stateStack.push(shiftTable[stateStack.top() + t]);
	while (getline(cin, input)) {
		string token;
		string lexeme;
		stringstream s(input);
		s >> token >> lexeme;
		while (reduceTable.count(stateStack.top() + token) == 1) {
			int index = stoi(reduceTable[stateStack.top() + token]);
			vector <string> derivation = derivations[index];
			int number = derivation.size();
			// Create Tree
			Tree * pa = new Tree();
			for (int i = 0; i < number - 1; i++) {
				pa->value = pa->value + derivation[i] + " ";
			}
			pa->value += derivation[number - 1];
			for (int i = 1; i < number; i++) {
				pa->children.push(symStack.top());
				symStack.pop();
				stateStack.pop();
			}
			symStack.push(pa);
			stateStack.push(shiftTable[stateStack.top() + derivation[0]]);
		}
		temp++;
		Tree * pa = new Tree();
		pa->value = input;
		symStack.push(pa);
		if (shiftTable.count(stateStack.top() + token) == 0) {
			cerr << "ERROR at " << temp << endl;
			while (!symStack.empty()) {
                        	deleting(symStack.top());
                        	symStack.pop();
			}
			return 1;
		} else {
			stateStack.push(shiftTable[stateStack.top() + token]);
		}
	}
	input = "EOF EOF";
        stringstream k(input);
        k >> t >> l;
	while (reduceTable.count(stateStack.top() + t) == 1) {
                        int index = stoi(reduceTable[stateStack.top() + t]);
                        vector <string> derivation = derivations[index];
                        int number = derivation.size();
			// Create Tree
                        Tree * pa = new Tree();
                        for (int i = 0; i < number - 1; i++) {
                                pa->value = pa->value + derivation[i] + " ";
                        }
			pa->value += derivation[number - 1];
                        for (int i = 0; i < number - 1; i++) {
				pa->children.push(symStack.top());
                                symStack.pop();
                                stateStack.pop();
                        }
                        symStack.push(pa);
                        stateStack.push(shiftTable[stateStack.top() + derivation[0]]);
	}
	temp++;
	Tree * pa = new Tree();
        pa->value = input;
        symStack.push(pa);
	if (shiftTable.count(stateStack.top() + t) == 0) {
                cerr << "ERROR at " << temp << endl;
		while (!symStack.empty()) {
			deleting(symStack.top());
			symStack.pop();
		}
		return 1;
        } else {
                stateStack.push(shiftTable[stateStack.top() + t]);
        }
	
	// push start state on stack
	Tree * root = new Tree();
	root->value = "start BOF procedures EOF";
	while (!symStack.empty()) {
		root->children.push(symStack.top());
		symStack.pop();
	}
	symStack.push(root);

	// print parse tree
	preOrder(root);
}
