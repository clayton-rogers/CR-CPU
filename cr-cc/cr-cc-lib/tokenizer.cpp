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

static bool is_known_token(const std::string& token)
{
	return STR_TOKEN_MAP.count(token) == 1;
}

static TokenType get_token_type(const std::string& token)
{
	if (STR_TOKEN_MAP.count(token) == 1) {
		return STR_TOKEN_MAP.at(token);
	} else {
		return TokenType::NONE;
	}
}

// TODO better checking for string and identifiers
static bool is_string_literal(const std::string& s)
{
	return s.at(0) == '"';
}

static bool is_identifier(const std::string& s)
{
	return (std::isalpha(s.at(0), LOCALE) || s.at(0) == '_');
}

// For now numeral literals (constants) must be hex or dec
static bool is_constant(const std::string& s)
{
	if (s.at(0) == '\'') {
		// Make sure there is a closing ', but otherwise no restriction
		return s.at(s.length() - 1) == '\'';
	} else if ((int)s.length() > 2 && s.at(0) == '0' && (s.at(1) == 'x' || s.at(1) == 'X')) {
		for (int i = 2; i < (int)s.length(); ++i) {
			if (!std::isxdigit(s.at(i), LOCALE)) {
				return false;
			}
		}
	} else {
		for (const auto& letter : s) {
			if (!std::isdigit(letter, LOCALE)) {
				return false;
			}
		}
	}
	return true;
}

struct CompoundTokenType {
	std::string s;
	TokenType token;
};
static const std::vector<CompoundTokenType> COMPOUND_TOKENS{
	{"++", TokenType::inc_op},
	{"--", TokenType::dec_op},
	{"->", TokenType::ptr_op},
	{"<<", TokenType::left_op},
	{">>", TokenType::right_op},
	{"<=", TokenType::le_op},
	{">=", TokenType::ge_op},
	{"==", TokenType::eq_op},
	{"!=", TokenType::ne_op},
	{"&&", TokenType::and_op},
	{"||", TokenType::or_op},
	{"*=", TokenType::mul_assign},
	{"/=", TokenType::div_assign},
	{"%=", TokenType::mod_assign},
	{"+=", TokenType::add_assign},
	{"-=", TokenType::sub_assign},
	{"&=", TokenType::and_assign},
	{"|=", TokenType::or_assign},
	{"^=", TokenType::xor_assign},
};

static TokenType get_compound_token(char current, char next)
{
	for (const auto& compound : COMPOUND_TOKENS) {
		if (current == compound.s.at(0) &&
			next == compound.s.at(1)) {
			return compound.token;
		}
	}
	return TokenType::NONE;
}

TokenList tokenize(const std::string& code)
{

	TokenList tl;

	Token current_token;
	bool is_in_middle_of_token = false;
	bool is_in_string_literal = false;
	bool is_in_character_literal = false;

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

	for (size_t i = 0; i < code.length(); ++i) {
		const char current_char = code.at(i);
		const char next_char = (i + 1 < code.length()) ? code.at(i + 1) : '\0';
		const std::string current_char_str(1, current_char);
		const TokenType possible_compound = get_compound_token(current_char, next_char);

		if (std::isspace(current_char, LOCALE) && !(is_in_string_literal || is_in_character_literal)) {
			if (is_in_middle_of_token) {
				end_current_token();
			} else {
				// ignore adjacent whitespace
			}
		} else if (possible_compound != TokenType::NONE) {
			// Found a compound
			current_token.token_type = possible_compound;
			current_token.value = "";
			tl.push_back(current_token);
			current_token = Token(); // reset
			is_in_middle_of_token = false;
			++i; // consume the additional token
		} else if (is_known_token(current_char_str) && !(is_in_string_literal || is_in_character_literal)) {
			if (is_in_middle_of_token) {
				end_current_token();
			}
			current_token.value = current_char_str;
			end_current_token();
		} else {
			// add the current character to the current token
			current_token.value.append(1, current_char);
			if (!is_in_middle_of_token) {
				is_in_middle_of_token = true;
				if (current_char == '"') {
					is_in_string_literal = true;
				}
				if (current_char == '\'') {
					is_in_character_literal = true;
				}
			} else {
				if (is_in_string_literal && current_char == '"') {
					is_in_string_literal = false;
					//end_current_token(); // Not sure if this is required. For now will leave it out.
				}
				if (is_in_character_literal && current_char == '\'') {
					is_in_character_literal = false;
				}
			}
		}
	}

	return tl;
}

std::string print_token_list(TokenList tl)
{
	std::stringstream ss;
	for (const auto& token : tl) {
		ss << token.value << "\t\t" << tokenType_to_string(token.token_type) << std::endl;
	}

	return ss.str();
}
