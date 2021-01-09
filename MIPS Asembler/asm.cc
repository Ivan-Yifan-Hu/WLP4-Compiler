#include <iostream>
#include <string>
#include <vector>
#include "scanner.h"
#include <map>
#include <vector>
#include <stdexcept>

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains the main function of your program. By default, it just
 * prints the scanned list of tokens back to standard output.
 */



std::map<std::string, int> label;
std::map<int, int> labelCheck;



void parsing(std::vector<Token> tokenLine, std::map<std::string, int> reg, int &pc, int cur) {
	if (tokenLine[0].getKind() == Token::ID) {
		if ((tokenLine[0].getLexeme() == "add") ||
		   (tokenLine[0].getLexeme() == "sub") ||
	   	   (tokenLine[0].getLexeme() == "slt") ||
	           (tokenLine[0].getLexeme() == "sltu")) {
			if ((tokenLine.size() == 6) && 
			(tokenLine[2].getKind() == Token::COMMA) && 
			(tokenLine[4].getKind() == Token::COMMA) &&
			(reg.count(tokenLine[1].getLexeme()) == 1) &&
		        (reg.count(tokenLine[3].getLexeme()) == 1) &&
			(reg.count(tokenLine[5].getLexeme()) == 1)) {
				pc += 4;
				return;
			} else {
				throw std::invalid_argument("Invalid registers");
			}
		 } else if ((tokenLine[0].getLexeme() == "mult") ||      
                   (tokenLine[0].getLexeme() == "multu") ||       
                   (tokenLine[0].getLexeme() == "div") ||          
                   (tokenLine[0].getLexeme() == "divu")) {
			if ((tokenLine.size() == 4) && 
                        (tokenLine[2].getKind() == Token::COMMA) && 
                        (reg.count(tokenLine[1].getLexeme()) == 1) &&
                        (reg.count(tokenLine[3].getLexeme()) == 1)) {
				pc += 4;
				return;
			 } else {
				throw std::invalid_argument("Invalid registers");
			 }
		 } else if ((tokenLine[0].getLexeme() == "mfhi") ||
                   (tokenLine[0].getLexeme() == "mflo") ||
                   (tokenLine[0].getLexeme() == "lis")) {
			if ((tokenLine.size() == 2) &&
			(reg.count(tokenLine[1].getLexeme()) == 1)) {
				pc += 4;
				return;
			} else {
				throw std::invalid_argument("Invalid registers");
			}
		 } else if ((tokenLine[0].getLexeme() == "lw") ||
                   (tokenLine[0].getLexeme() == "sw")) {
			 if ((tokenLine.size() == 7) &&
			(tokenLine[2].getKind() == Token::COMMA) &&
			(reg.count(tokenLine[1].getLexeme()) == 1) &&
			(reg.count(tokenLine[5].getLexeme()) == 1) &&
			(tokenLine[4].getLexeme() == "(") &&
			(tokenLine[6].getLexeme() == ")") &&
			(((tokenLine[3].getKind() == Token::INT) &&
			 (tokenLine[3].toLong() <= 32767) &&
			 (tokenLine[3].toLong() >= -32768)) ||
			 ((tokenLine[3].getKind() == Token::HEXINT) &&
                         (tokenLine[3].toLong() <= 0xffff)))) {
				 pc += 4;
				 return;
			 } else {
				 throw std::invalid_argument("Invalid registers or numbers");
			 }
		 } else if ((tokenLine[0].getLexeme() == "beq") ||
                   (tokenLine[0].getLexeme() == "bne")) {
			 if ((tokenLine.size() == 6) &&
			(tokenLine[2].getKind() == Token::COMMA) &&
			(tokenLine[4].getKind() == Token::COMMA) &&
                        (reg.count(tokenLine[1].getLexeme()) == 1) &&
                        (reg.count(tokenLine[3].getLexeme()) == 1) &&
			(((tokenLine[5].getKind() == Token::INT) &&
                         (tokenLine[5].toLong() <= 32767) &&
                         (tokenLine[5].toLong() >= -32768)) ||
			 ((tokenLine[5].getKind() == Token::HEXINT) &&
                         (tokenLine[5].toLong() <= 0xffff)) ||
			 (tokenLine[5].getKind() == Token::ID))) {
				 if (tokenLine[5].getKind() == Token::ID) {
					 labelCheck[pc - 4] = cur + 5;
				 }
				 pc += 4;
				 return;
			 } else {
				 throw std::invalid_argument("Invalid registers or numbers");
			 }
		 } else if ((tokenLine[0].getLexeme() == "jr") ||
                   (tokenLine[0].getLexeme() == "jalr")) {
			 if ((tokenLine.size() == 2) &&
			(reg.count(tokenLine[1].getLexeme()) == 1)) {
				 pc += 4;
				 return;
			 } else {
				 throw std::invalid_argument("invalid registers");
			 }
		 } else {
			throw  std::invalid_argument("invalid ID");
		}
	} else if (tokenLine[0].getKind() == Token::WORD) {
		if ((tokenLine.size() == 2) &&
		(((tokenLine[1].getKind() == Token::INT) &&
		(tokenLine[1].toLong() <= 4294967295) &&
		(tokenLine[1].toLong() >= -2147483648)) ||
		((tokenLine[1].getKind() == Token::HEXINT) &&
		(tokenLine[1].toLong() <= 0xffffffff)) ||
		(tokenLine[1].getKind() == Token::ID))) {
			if (tokenLine[1].getKind() == Token::ID) {
				labelCheck[pc - 4] = cur + 1;
			}
			pc += 4;
			return;
		} else {
			throw std::invalid_argument("i is too large or too small");
		}
	} else if (tokenLine[0].getKind() == Token::LABEL) {
		if (label.count(tokenLine[0].getLexeme()) == 0) {
			label[tokenLine[0].getLexeme()] = pc - 4;
			if (tokenLine.size() > 1) {
				tokenLine.erase(tokenLine.begin());
				parsing(tokenLine, reg, pc, cur + 1);
				return;
			} else {
				return;
			}
		} else {
			throw std::invalid_argument("Label already exist");
		}
	} else {
		throw std::invalid_argument("ERROR");
	}
}




void synthesis(std::vector<Token> tokenLine, std::map<std::string, int> reg, int &pc) {
	if (tokenLine[0].getLexeme() == "add") {
		int binary = (0 << 26) | (reg[tokenLine[3].getLexeme()] << 21) | (reg[tokenLine[5].getLexeme()] << 16) | (reg[tokenLine[1].getLexeme()] << 11) | 32;
		unsigned char c = binary >> 24;
		std::cout << c;
		c = binary >> 16;
		std::cout << c;
		c = binary >> 8;
		std::cout << c;
		c = binary;
		std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "sub") {
                int binary = (0 << 26) | (reg[tokenLine[3].getLexeme()] << 21) | (reg[tokenLine[5].getLexeme()] << 16) | (reg[tokenLine[1].getLexeme()] << 11) | 34;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "slt") {
                int binary = (0 << 26) | (reg[tokenLine[3].getLexeme()] << 21) | (reg[tokenLine[5].getLexeme()] << 16) | (reg[tokenLine[1].getLexeme()] << 11) | 42;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "sltu") {
                int binary = (0 << 26) | (reg[tokenLine[3].getLexeme()] << 21) | (reg[tokenLine[5].getLexeme()] << 16) | (reg[tokenLine[1].getLexeme()] << 11) | 43;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "mult") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 21) | (reg[tokenLine[3].getLexeme()] << 16) | 24;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "multu") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 21) | (reg[tokenLine[3].getLexeme()] << 16) | 25;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "div") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 21) | (reg[tokenLine[3].getLexeme()] << 16) | 26;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "divu") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 21) | (reg[tokenLine[3].getLexeme()] << 16) | 27;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "mfhi") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 11) | 16;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "mflo") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 11) | 18;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
        } else if (tokenLine[0].getLexeme() == "lis") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 11) | 20;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
        } else if (tokenLine[0].getLexeme() == "lw") {
		int padding = tokenLine[3].toLong();
                if (padding < 0) {
                        padding = padding + 32768 * 2;
                }
                int binary = (35 << 26) | (reg[tokenLine[5].getLexeme()] << 21) | (reg[tokenLine[1].getLexeme()] << 16) | padding;;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "sw") {
		int padding = tokenLine[3].toLong();
		if (padding < 0) {
			padding = padding + 32768 * 2;
		}
                int binary = (43 << 26) | (reg[tokenLine[5].getLexeme()] << 21) | (reg[tokenLine[1].getLexeme()] << 16) | padding;;
		unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "jr") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 21) | 8;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "jalr") {
                int binary = (0 << 26) | (reg[tokenLine[1].getLexeme()] << 21) | 9;
                unsigned char c = binary >> 24;
                std::cout << c;
                c = binary >> 16;
                std::cout << c;
                c = binary >> 8;
                std::cout << c;
                c = binary;
                std::cout << c;
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "beq") {
		if ((tokenLine[5].getKind() == Token::INT) || (tokenLine[5].getKind() == Token::HEXINT)) {
			int padding = tokenLine[5].toLong();
                	if (padding < 0) {
                        	padding = padding + 32768 * 2;
                	}
			int binary = (4 << 26) | (reg[tokenLine[1].getLexeme()] << 21) |(reg[tokenLine[3].getLexeme()] << 16) | padding;
			unsigned char c = binary >> 24;
		       	std::cout << c;
	 		c = binary >> 16;
			std::cout << c;
	 		c = binary >> 8;
			std::cout << c;
			c = binary;
			std::cout << c;
		} else {
			int a = label[tokenLine[5].getLexeme() + ":"];
			int padding = (a - pc) / 4;
			if (padding < 0) {
                                padding = padding + 32768 * 2;
                        }
			int binary = (4 << 26) | (reg[tokenLine[1].getLexeme()] << 21) |(reg[tokenLine[3].getLexeme()] << 16) | padding;
			unsigned char c = binary >> 24;
                        std::cout << c;
                        c = binary >> 16;
                        std::cout << c;
                        c = binary >> 8;
                        std::cout << c;
                        c = binary;
                        std::cout << c;
		}
		pc += 4;
	} else if (tokenLine[0].getLexeme() == "bne") {
                if ((tokenLine[5].getKind() == Token::INT) || (tokenLine[5].getKind() == Token::HEXINT)) {
			int padding = tokenLine[5].toLong();
                        if (padding < 0) {
                                padding = padding + 32768 * 2;
                        }
			int binary = (5 << 26) | (reg[tokenLine[1].getLexeme()] << 21) |(reg[tokenLine[3].getLexeme()] << 16) | padding;
			unsigned char c = binary >> 24;
                        std::cout << c;
                        c = binary >> 16;
                        std::cout << c;
                        c = binary >> 8;
                        std::cout << c;
                        c = binary;
                        std::cout << c;
                } else {
			int a = label[tokenLine[5].getLexeme() + ":"];
			int padding = (a - pc) / 4;
                        if (padding < 0) {
                                padding = padding + 32768 * 2;
                        }
			int binary = (5 << 26) | (reg[tokenLine[1].getLexeme()] << 21) |(reg[tokenLine[3].getLexeme()] << 16) | padding ;
			unsigned char c = binary >> 24;
                        std::cout << c;
                        c = binary >> 16;
                        std::cout << c;
                        c = binary >> 8;
                        std::cout << c;
                        c = binary;
                        std::cout << c;
                }
		pc += 4;
	} else if (tokenLine[0].getKind() == Token::WORD) {
		if ((tokenLine[1].getKind() == Token::INT) || (tokenLine[1].getKind() == Token::HEXINT)) {
			int binary = tokenLine[1].toLong();
			unsigned char c = binary >> 24;
			std::cout << c;
	  		c = binary >> 16;
	 		std::cout << c;
			c = binary >> 8;
			std::cout << c;
			c = binary;
			std::cout << c;
		} else {
			int binary = label[tokenLine[1].getLexeme() + ":"];
			unsigned char c = binary >> 24;
                        std::cout << c;
                        c = binary >> 16;
                        std::cout << c;
                        c = binary >> 8;
                        std::cout << c;
                        c = binary;
                        std::cout << c;
		}
		pc += 4;
	} else {
		tokenLine.erase(tokenLine.begin());
                synthesis(tokenLine, reg, pc);
	}
}

			


bool allLabel(std::vector<Token> tokenLine) {
	for (auto &p : tokenLine) {
		if (p.getKind() == Token::LABEL) {
			continue;
		} else {
			return false;
		}
	} 
	return true;
}



int main() {

  std::map<std::string, int> reg;
  for (int a = 0; a <= 31; ++a) {
    std::string temp = "$" + std::to_string(a);
    reg[temp] = a;
  } 


  std::vector<std::vector<Token>> input;

  std::string line;
  try {
    int pc = 4;
    while (getline(std::cin, line)) {	
	

      // For example, just print the scanned tokens
      std::vector<Token> tokenLine = scan(line);
      if (tokenLine.size() == 0) {
	      continue;
      }
      parsing(tokenLine, reg, pc, 0);
      if (allLabel(tokenLine)) {
             continue;
      }
      input.emplace_back(tokenLine);
    }


  } catch (ScanningFailure &f) {
    std::cerr << f.what() << std::endl;
    return 1;
  } catch (std::invalid_argument &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }





  int pc = 0;
  for (auto &s : input) {

	pc += 4;

	  if (labelCheck.count(pc - 4) == 1) {
		  if ((label.count(s[labelCheck[pc-4]].getLexeme() + ":") == 1) &&
                (s[labelCheck[pc-4] - 1].getKind() == Token::WORD)) {
                          continue;
		  } else if ((label.count(s[labelCheck[pc-4]].getLexeme() + ":") == 1) && (((label[s[labelCheck[pc-4]].getLexeme() + ":"] - pc + 4) / 4) >= -32768) && (((label[s[labelCheck[pc-4]].getLexeme() + ":"] - pc + 4) / 4) <= 32767) && (s[labelCheck[pc-4] - 5].getKind() == Token::ID)) {
			  continue;
		  } else {
			  std::cerr << "ERROR: Label is not defined or out of range" << std::endl;
			  return 1;
		  }
	  }
  }


  int pc2 = 4;
  for (auto &s : input) {
	  synthesis(s, reg, pc2);
  }

  for (auto &p : label) {
	  std::string temp = p.first;
	  temp.pop_back();
	  std::cerr << temp << " " << p.second << std::endl;
  }

  return 0;
}
