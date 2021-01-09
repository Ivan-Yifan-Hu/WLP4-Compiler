#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <utility>
#include <set>
#include <sstream>
#include <array>
#include "scanner.h"

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains helpers for asm.cc and you don't need to modify it.
 * Furthermore, while this code may be helpful to understand starting with
 * the DFA assignments, you do not need to understand it to write the assembler.
 */

Token::Token(Token::Kind kind, std::string lexeme):
  kind(kind), lexeme(std::move(lexeme)) {}

  Token:: Kind Token::getKind() const { return kind; }
const std::string &Token::getLexeme() const { return lexeme; }

std::ostream &operator<<(std::ostream &out, const Token &tok) {
  switch (tok.getKind()) {
    case Token::ID:         out << "ID";         break;
    case Token::NUM:        out << "NUM";        break;
    case Token::LPAREN:     out << "LPAREN";     break;
    case Token::RPAREN:     out << "RPAREN";     break;
    case Token::LBRACE:     out << "LBRACE";     break;
    case Token::RBRACE:     out << "RBRACE";     break;
    case Token::RETURN:     out << "RETURN";     break;
    case Token::IF:         out << "IF";         break;
    case Token::ELSE:       out << "ELSE";       break;
    case Token::WHILE:      out << "WHILE";      break;
    case Token::PRINTLN:    out << "PRINTLN";    break;
    case Token::WAIN:       out << "WAIN";       break;
    case Token::BECOMES:    out << "BECOMES";    break;
    case Token::INT:        out << "INT";        break;
    case Token::NE:         out << "NE";         break;
    case Token::EQ:         out << "EQ";         break;
    case Token::LT:         out << "LT";         break;
    case Token::GT:         out << "GT";         break;
    case Token::LE:         out << "LE";         break;
    case Token::GE:         out << "GE";         break;
    case Token::PLUS:       out << "PLUS";       break;
    case Token::MINUS:      out << "MINUS";      break;
    case Token::STAR:       out << "STAR";       break;
    case Token::SLASH:      out << "SLASH";      break;
    case Token::PCT:        out << "PCT";        break;
    case Token::COMMA:      out << "COMMA";      break;
    case Token::SEMI:       out << "SEMI";       break;
    case Token::NEW:        out << "NEW";        break;
    case Token::DELETE:     out << "DELETE";     break;
    case Token::LBRACK:     out << "LBRACK";     break;
    case Token::RBRACK:     out << "RBRACK";     break;
    case Token::AMP:        out << "AMP";        break;
    case Token::NUL:        out << "NULL";       break;
  }

  out << " " << tok.getLexeme();

  return out;
}

/*
int64_t Token::toLong() const {
  std::istringstream iss;
  int64_t result;

  if (kind == INT) {
    iss.str(lexeme);
  } else if (kind == HEXINT) {
    iss.str(lexeme.substr(2));
    iss >> std::hex;
  } else if (kind == REG) {
    iss.str(lexeme.substr(1));
  } else {
    // This should never happen if the user calls this function correctly
    return 0;
  }

  iss >> result;
  return result;
}
*/

ScanningFailure::ScanningFailure(std::string message):
  message(std::move(message)) {}

const std::string &ScanningFailure::what() const { return message; }

/* Represents a DFA (which you will see formally in class later)
 * to handle the scanning
 * process. You should not need to interact with this directly:
 * it is handled through the starter code.
 */
class AsmDFA {
  public:
    enum State {
      // States that are also accepting states
      ID = 0,
      ZERO,
      NUM,
      NE,
      LR,
      BECOMES,
      EQ,
      OP,
      SLASH,
      T1,
      T2,
      WHITESPACE,
      COMMENT,

      // States that are not accepting states
      N,
      START,
      FAIL,

      // Hack to let this be used easily in arrays. This should always be the
      // final element in the enum, and should always point to the previous
      // element.

      LARGEST_STATE = FAIL
    };

  private:
    /* A set of all accepting states for the DFA.
     * Currently non-accepting states are not actually present anywhere
     * in memory, but a list can be found in the constructor.
     */
    std::set<State> acceptingStates;

    /*
     * The transition function for the DFA, stored as a map.
     */

    std::array<std::array<State, 128>, LARGEST_STATE + 1> transitionFunction;

    /*
     * Converts a state to a kind to allow construction of Tokens from States.
     * Throws an exception if conversion is not possible.
     */
    Token::Kind stateToKind(State s, std::string & input) const {
      switch(s) {
        case ID:
		if (input == "wain") {
			return Token::WAIN;
		} else if (input == "int") {
			return Token::INT;
		} else if (input == "if") {
       			return Token::IF;
		} else if (input == "else") {
			return Token::ELSE;
		} else if (input == "while") {
                        return Token::WHILE;
                } else if (input == "println") {
                        return Token::PRINTLN;
                } else if (input == "return") {
                        return Token::RETURN;
                } else if (input == "NULL") {
                        return Token::NUL;
                } else if (input == "new") {
                        return Token::NEW;
                } else if (input == "delete") {
                        return Token::DELETE;
                } else {
			return Token::ID;
		}
        case ZERO:
       		return Token::NUM;
        case NUM:
	  	return Token::NUM;
        case NE:
	  	return Token::NE;
        case LR:
		if (input == "(") {
                        return Token::LPAREN;
                } else if (input == ")") {
                        return Token::RPAREN;
                } else if (input == "{") {
                        return Token::LBRACE;
                } else if (input == "}") {
                        return Token::RBRACE;
                } else if (input == "[") {
                        return Token::LBRACK;
                } else {
                        return Token::RBRACK;
                }
        case BECOMES:
		return Token::BECOMES;
        case EQ:       
	     	return Token::EQ;
        case OP:
		if (input == "+") {
                        return Token::PLUS;
                } else if (input == "-") {
                        return Token::MINUS;
                } else if (input == "*") {
                        return Token::STAR;
                } else if (input == "%") {
                        return Token::PCT;
                } else if (input == ",") {
                        return Token::COMMA;
                } else if (input == ";") {
			return Token::SEMI;
		} else {
			return Token::AMP;
		}
	case SLASH:
		return Token::SLASH;
        case T1:
		if (input == "<") {
			return Token::LT;
		} else {
			return Token::GT;
		}
        case T2:
		if (input == "<=") {
			return Token::LE;
		} else {
			return Token::GE;
		}
        case WHITESPACE:
	       	return Token::WHITESPACE;
        case COMMENT:  
	      	return Token::COMMENT;
        default: throw ScanningFailure("ERROR: Cannot convert state to kind.");
      }
    }


  public:
    /* Tokenizes an input string according to the SMM algorithm.
     * You will learn the SMM algorithm in class around the time of Assignment 6.
     */
    std::vector<Token> simplifiedMaximalMunch(const std::string &input) const {
      std::vector<Token> result;

      State state = start();
      std::string munchedInput;

      // We can't use a range-based for loop effectively here
      // since the iterator doesn't always increment.
      for (std::string::const_iterator inputPosn = input.begin();
           inputPosn != input.end();) {

        State oldState = state;
        state = transition(state, *inputPosn);

        if (!failed(state)) {
          munchedInput += *inputPosn;
          oldState = state;

          ++inputPosn;
        }

        if (inputPosn == input.end() || failed(state)) {
          if (accept(oldState)) {
            result.push_back(Token(stateToKind(oldState, munchedInput), munchedInput));

            munchedInput = "";
            state = start();
          } else {
            if (failed(state)) {
              munchedInput += *inputPosn;
            }
            throw ScanningFailure("ERROR: Simplified maximal munch failed on input: "
                                 + munchedInput);
          }
        }
      }

      return result;
    }

    /* Initializes the accepting states for the DFA.
     */
    AsmDFA() {
      acceptingStates = {ID, ZERO, NUM, NE, BECOMES, SLASH, EQ, LR, OP, T1, T2, WHITESPACE, COMMENT};
      //Non-accepting states are DOT, MINUS, ZEROX, DOLLARS, START

      // Initialize transitions for the DFA
      for (size_t i = 0; i < transitionFunction.size(); ++i) {
        for (size_t j = 0; j < transitionFunction[0].size(); ++j) {
          transitionFunction[i][j] = FAIL;
        }
      }
      char whitespace[] = {32, 9, 10};
      registerTransition(START, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ID);
      registerTransition(START, "0", ZERO);
      registerTransition(START, "123456789", NUM);
      registerTransition(START, "!", N);
      registerTransition(START, "(){}[]", LR);
      registerTransition(START, "=", BECOMES);
      registerTransition(START, "+-*%,;&", OP);
      registerTransition(START, "/", SLASH);
      registerTransition(START, "<>", T1);
      registerTransition(ID, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", ID);
      registerTransition(NUM, "0123456789", NUM);
      registerTransition(N, "=", NE);
      registerTransition(BECOMES, "=", EQ);
      registerTransition(SLASH, "/", COMMENT);
      registerTransition(T1, "=", T2);
      registerTransition(COMMENT, [](int c) -> int { return c != '\n'; },
          COMMENT);
      registerTransition(START, whitespace, WHITESPACE);
      registerTransition(WHITESPACE, whitespace, WHITESPACE);
    }

    // Register a transition on all chars in chars
    void registerTransition(State oldState, const std::string &chars,
        State newState) {
      for (char c : chars) {
        transitionFunction[oldState][c] = newState;
      }
    }

    // Register a transition on all chars matching test
    // For some reason the cctype functions all use ints, hence the function
    // argument type.
    void registerTransition(State oldState, int (*test)(int), State newState) {

      for (int c = 0; c < 128; ++c) {
        if (test(c)) {
          transitionFunction[oldState][c] = newState;
        }
      }
    }

    /* Returns the state corresponding to following a transition
     * from the given starting state on the given character,
     * or a special fail state if the transition does not exist.
     */
    State transition(State state, char nextChar) const {
      return transitionFunction[state][nextChar];
    }

    /* Checks whether the state returned by transition
     * corresponds to failure to transition.
     */
    bool failed(State state) const { return state == FAIL; }

    /* Checks whether the state returned by transition
     * is an accepting state.
     */
    bool accept(State state) const {
      return acceptingStates.count(state) > 0;
    }

    /* Returns the starting state of the DFA
     */
    State start() const { return START; }
};

std::vector<Token> scan(const std::string &input) {
  static AsmDFA theDFA;

  std::vector<Token> tokens = theDFA.simplifiedMaximalMunch(input);

  // We need to:
  // * Remove WHITESPACE and COMMENT tokens entirely.

  std::vector<Token> newTokens;

  for (auto &token : tokens) {
	  if (token.getKind() != Token::WHITESPACE && token.getKind() != Token::Kind::COMMENT) {
		  if (token.getKind() == Token::NUM) {
			  std::stringstream temp(token.getLexeme());
			  int a;
			  temp >> a;
			  if ((a > 2147483647) || (a < 0)) {
				  throw ScanningFailure("ERROR: number out of range: " + token.getLexeme());
			  }
		  }
		  newTokens.push_back(token);
	  }
  }
  return newTokens;
}
