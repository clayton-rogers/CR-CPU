#include "tokenizer.h"

#include <iostream>
#include <sstream>
#include <locale>
#include <map>


static const std::map<std::string, TokenType> STR_TOKEN_MAP =
{
	{"struct", TokenType::key_struct},
	{"char", TokenType::key_char},
	{"int", TokenType::key_int},
	{"const", TokenType::key_const},
	{"void", TokenType::key_void},
	{"return", TokenType::key_return},
	{"=", TokenType::equals},
	{"+", TokenType::add},
	{"-", TokenType::sub},
	{"*", TokenType::star},
	{"/", TokenType::div},
	{"%", TokenType::percent},
	{"&", TokenType::ampersand},
	{"|", TokenType::pipe},
	{"^", TokenType::hat},
	{".", TokenType::period},
	{",", TokenType::comma},
	{";", TokenType::semi_colon},
	{"<", TokenType::less_than},
	{">", TokenType::greater_than},
	{"{", TokenType::open_bracket},
	{"}", TokenType::close_bracket},
	{"(", TokenType::open_parenth},
	{")", TokenType::close_parenth},
	{"[", TokenType::open_square_bracket},
	{"]", TokenType::close_square_bracket},
};

static bool is_known_token(std::string token) {
	return STR_TOKEN_MAP.count(token) == 1;
}

static TokenType get_token_type(std::string token) {
	try {
		return STR_TOKEN_MAP.at(token);
	} catch (std::out_of_range e) {
		return TokenType::unk;
	}
}

TokenList tokenize(const std::string& code) {
	const std::locale locale("en_US.UTF-8");
	TokenList tl;

	Token current_token;
	bool is_in_middle_of_token = false;

	auto end_current_token = [&]() {
		current_token.token_type = get_token_type(current_token.value);
		tl.push_back(current_token);
		current_token = Token();// reset current token
		is_in_middle_of_token = false;
	};

	for (int i = 0; i < code.length(); ++i) {
		const char current_char = code.at(i);
		const std::string current_char_str(1, current_char);

		if (std::isspace(current_char, locale)) {
			if (is_in_middle_of_token) {
				end_current_token();
			} else {
				// ignore
			}
		} else if (is_known_token(current_char_str)) {
			if (is_in_middle_of_token) {
				end_current_token();
			}
			current_token.value = current_char_str;
			end_current_token();
		} else {
			// add the current character to the current token
			current_token.value.append(1, current_char);
			is_in_middle_of_token = true;
		}
	}

	return tl;
}

std::string print_tokens(TokenList tl) {
	const std::map<TokenType, std::string> token_strings =
	{
		{TokenType::unk, "unk"},
		{TokenType::user, "user"},
		{TokenType::key_struct, "key_struct"},
		{TokenType::key_char, "key_char"},
		{TokenType::key_int, "key_int"},
		{TokenType::key_void, "key_void"},
		{TokenType::key_return, "key_return"},
		{TokenType::key_const, "key_const"},
		{TokenType::equals, "equals"},
		{TokenType::add, "add"},
		{TokenType::sub, "sub"},
		{TokenType::star, "star"},
		{TokenType::div, "div"},
		{TokenType::percent, "percent"},
		{TokenType::ampersand, "ampersand"},
		{TokenType::pipe, "pipe"},
		{TokenType::hat, "hat"},
		{TokenType::period, "period"},
		{TokenType::comma, "comma"},
		{TokenType::semi_colon, "semi_colon"},
		{TokenType::less_than, "less_than"},
		{TokenType::greater_than, "greater_than"},
		{TokenType::open_bracket, "open_bracket"},
		{TokenType::close_bracket, "close_bracket"},
		{TokenType::open_parenth, "open_parenth"},
		{TokenType::close_parenth, "close_parenth"},
		{TokenType::open_square_bracket, "open_square_bracket"},
		{TokenType::close_square_bracket, "close_square_bracket"},
	};

	std::stringstream ss;
	for (const auto& token : tl) {
		ss << token.value << "\t\t" << token_strings.at(token.token_type) << std::endl;
	}

	return ss.str();
}
