#include "tokenizer.h"

#include <iostream>
#include <sstream>
#include <locale>
#include <map>


// For simple terminal tokens we recognize them immediately here.
// Composite or complicated tokens are matched in the parser.
static const std::map<std::string, TokenType> STR_TOKEN_MAP =
{
	{"struct", TokenType::key_struct},
	{"union", TokenType::key_union},
	{"void", TokenType::key_void},
	{"char", TokenType::key_char},
	{"short", TokenType::key_short},
	{"int", TokenType::key_int},
	{"long", TokenType::key_long},
	{"float", TokenType::key_float},
	{"double", TokenType::key_double},
	{"signed", TokenType::key_signed},
	{"unsigned", TokenType::key_unsigned},
	{"return", TokenType::key_return},
	{"case", TokenType::key_case},
	{"break", TokenType::key_break},
	{"continue", TokenType::key_continue},
	{"default", TokenType::key_default},
	{"do", TokenType::key_do},
	{"else", TokenType::key_else},
	{"enum", TokenType::key_enum},
	{"for", TokenType::key_for},
	{"goto", TokenType::key_goto},
	{"if", TokenType::key_if},
	{"switch", TokenType::key_switch},
	{"while", TokenType::key_while},
	{"const", TokenType::key_const},
	{"volatile", TokenType::key_volatile},
	{"typedef", TokenType::key_typedef},
	{"extern", TokenType::key_extern},
	{"static", TokenType::key_extern},
	{"auto", TokenType::key_auto},
	{"register", TokenType::key_register},
	{"=", TokenType::equals},
	{"+", TokenType::add},
	{"-", TokenType::sub},
	{"*", TokenType::star},
	{"~", TokenType::tilda},
	{"!", TokenType::exclam},
	{"/", TokenType::div},
	{"%", TokenType::percent},
	{"&", TokenType::ampersand},
	{"|", TokenType::pipe},
	{"?", TokenType::question},
	{"^", TokenType::hat},
	{".", TokenType::period},
	{",", TokenType::comma},
	{";", TokenType::semi_colon},
	{":", TokenType::colon},
	{"<", TokenType::less_than},
	{">", TokenType::greater_than},
	{"{", TokenType::open_bracket},
	{"}", TokenType::close_bracket},
	{"(", TokenType::open_parenth},
	{")", TokenType::close_parenth},
	{"[", TokenType::open_square_bracket},
	{"]", TokenType::close_square_bracket},
};

static const std::locale LOCALE("");

static bool is_known_token(std::string token) {
	return STR_TOKEN_MAP.count(token) == 1;
}

static TokenType get_token_type(std::string token) {
	try {
		return STR_TOKEN_MAP.at(token);
	} catch (std::out_of_range e) {
		return TokenType::NONE;
	}
}

// TODO better checking for string and identifiers
static bool is_string_literal(std::string s) {
	return s.at(0) == '"';
}

static bool is_identifier(std::string s) {
	return std::isalpha(s.at(0), LOCALE);
}

// For now numeral literals (constants) must be all numbers
static bool is_constant(std::string s) {
	for (const auto& letter : s) {
		if (!std::isdigit(letter, LOCALE)) {
			return false;
		}
	}
	return true;
}

static const std::map<TokenType, std::string> COMPOUND_TOKENS{
	{TokenType::inc_op, "++"},
	{TokenType::ptr_op, "->"},
	{TokenType::left_op, "<<"},
	{TokenType::right_op, ">>"},
	{TokenType::le_op, "<="},
	{TokenType::ge_op, ">="},
	{TokenType::eq_op, "=="},
	{TokenType::ne_op, "!="},
	{TokenType::and_op, "&&"},
	{TokenType::or_op, "||"},
	{TokenType::mul_assign, "*="},
	{TokenType::div_assign, "/="},
	{TokenType::mod_assign, "%="},
	{TokenType::add_assign, "+="},
	{TokenType::sub_assign, "-="},
	{TokenType::and_assign, "&="},
	{TokenType::or_assign,  "|="},
	{TokenType::xor_assign, "^="},
};

static TokenList colate_multi_op(const TokenList& tl) {
	if (tl.size() == 0) {
		return tl;
	}

	TokenList ret;
	int i = 0;

	auto add_compound_token = [&ret, &i](TokenType tt) {
		Token t;
		t.token_type = tt;
		t.value = COMPOUND_TOKENS.at(tt);
		ret.push_back(t);
		++i;
	};

	for (i = 0; i < static_cast<int>(tl.size())-1; ++i) {
		const Token& cur = tl.at(i);
		const TokenType& next = tl.at(i + 1).token_type;
		switch (cur.token_type) {
		case TokenType::add:
			if (next == TokenType::add) {
				add_compound_token(TokenType::inc_op);
			} else if (next == TokenType::equals) {
				add_compound_token(TokenType::add_assign);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::sub:
			if (next == TokenType::greater_than) {
				add_compound_token(TokenType::ptr_op);
			} else if (next == TokenType::sub) {
				add_compound_token(TokenType::dec_op);
			} else if (next == TokenType::equals) {
				add_compound_token(TokenType::sub_assign);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::less_than:
			if (next == TokenType::less_than) {
				add_compound_token(TokenType::left_op);
			} else if (next == TokenType::equals) {
				add_compound_token(TokenType::le_op);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::greater_than:
			if (next == TokenType::greater_than) {
				add_compound_token(TokenType::right_op);
			} else if (next == TokenType::equals) {
				add_compound_token(TokenType::ge_op);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::equals:
			if (next == TokenType::equals) {
				add_compound_token(TokenType::eq_op);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::exclam:
			if (next == TokenType::equals) {
				add_compound_token(TokenType::ne_op);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::ampersand:
			if (next == TokenType::ampersand) {
				add_compound_token(TokenType::and_op);
			} else if (next == TokenType::equals) {
				add_compound_token(TokenType::and_assign);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::pipe:
			if (next == TokenType::pipe) {
				add_compound_token(TokenType::or_op);
			} else if (next == TokenType::equals) {
				add_compound_token(TokenType::or_assign);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::star:
			if (next == TokenType::equals) {
				add_compound_token(TokenType::mul_assign);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::div:
			if (next == TokenType::equals) {
				add_compound_token(TokenType::div_assign);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::percent:
			if (next == TokenType::equals) {
				add_compound_token(TokenType::mod_assign);
			} else {
				ret.push_back(cur);
			}
			break;
		case TokenType::hat:
			if (next == TokenType::equals) {
				add_compound_token(TokenType::xor_assign);
			} else {
				ret.push_back(cur);
			}
			break;
		default:
			ret.push_back(cur);
		}
	}
	// The loop above ends one early to prevent reading off the end of the array,
	// so we have to add that last token to the output here.
	// TODO technically there is a bug here if the last two characters of a token
	// list are a dual token, but that should never happen since the last should
	// always be '}'
	ret.push_back(tl.at(tl.size() - 1));

	return ret;
}

TokenList tokenize(const std::string& code) {
	
	TokenList tl;

	Token current_token;
	bool is_in_middle_of_token = false;
	bool is_in_string_literal = false;

	auto end_current_token = [&]() {
		current_token.token_type = get_token_type(current_token.value);
		if (current_token.token_type == TokenType::NONE) {
			if (is_string_literal(current_token.value)) {
				current_token.token_type = TokenType::string_literal;
			} else if (is_identifier(current_token.value)) {
				current_token.token_type = TokenType::identifier;
			} else if (is_constant(current_token.value)) {
				current_token.token_type = TokenType::constant;
			} else {
				throw std::logic_error("Could not determine token type: " + current_token.value);
			}
		}
		tl.push_back(current_token);
		current_token = Token();// reset current token
		is_in_middle_of_token = false;
	};

	for (int i = 0; i < static_cast<int>(code.length()); ++i) {
		const char current_char = code.at(i);
		const std::string current_char_str(1, current_char);

		if (std::isspace(current_char, LOCALE) && !is_in_string_literal) {
				if (is_in_middle_of_token) {
					end_current_token();
				} else {
					// ignore
				}
		} else if (is_known_token(current_char_str) && !is_in_string_literal) {
			if (!is_in_string_literal) {
				if (is_in_middle_of_token) {
					end_current_token();
				}
				current_token.value = current_char_str;
				end_current_token();
			}
		} else {
			// add the current character to the current token
			current_token.value.append(1, current_char);
			if (!is_in_middle_of_token) {
				is_in_middle_of_token = true;
				if (current_char == '"') {
					is_in_string_literal = true;
				}
			} else {
				if (is_in_string_literal && current_char == '"') {
					is_in_string_literal = false;
					//end_current_token(); // Not sure if this is required. For now will leave it out.
				}
			}
		}
	}

	return colate_multi_op(tl);
}

std::string print_token_list(TokenList tl) {
	std::stringstream ss;
	for (const auto& token : tl) {
		ss << token.value << "\t\t" << tokenType_to_string(token.token_type) << std::endl;
	}

	return ss.str();
}
