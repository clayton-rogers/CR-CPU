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
		{TokenType::key_case, "key_case"},
		{TokenType::key_break, "key_break"},
		{TokenType::key_continue, "key_continue"},
		{TokenType::key_default, "key_default"},
		{TokenType::key_do, "key_do"},
		{TokenType::key_else, "key_else"},
		{TokenType::key_enum, "key_enum"},
		{TokenType::key_for, "key_for"},
		{TokenType::key_goto, "key_goto"},
		{TokenType::key_if, "key_if"},
		{TokenType::key_switch, "key_switch"},
		{TokenType::key_while, "key_while"},
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
