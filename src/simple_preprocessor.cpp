#define _CRT_SECURE_NO_WARNINGS 1
#include <stdlib.h>
#include <stdio.h>


char *ReadEntireFIleIntoMemoryAndNullTerminate(char *filename)
{
	char *result = 0;
	FILE *file = fopen(filename, "r");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		result = (char *)malloc(fileSize + 1);
		fread(result, fileSize, 1, file);
		result[fileSize] = 0;

		fclose(file);
	}
	return result;
}

enum token_type
{
	Token_Unknown,
	Token_OpenParem,
	Token_Colon,
	Token_ClosenParen,
	Token_Semicolon,
	Token_Asterisk,
	Token_OpenBracket,
	Token_CloseBracket,
	Token_OpenBraces,
	Token_CloseBraces,

	Token_String,
	Token_Identifier,

	Token_EndOfStream,
};

struct Token
{
	token_type type;
	size_t textLength;
	char *text;
};

struct Tokenizer
{
	char* at;
};

inline bool IsEndOfLine(char c)
{
	return (c == '\n' || c == '\r');
}

inline bool IsWhitespace(char c)
{
	bool result = ((c == ' ') || (c == '\t') || IsEndOfLine(c));
	return result;
}

inline bool IsAlpha(char c)
{
	return ((c >= 'a') && (c <= 'z') ||
			(c >= 'A') && (c <= 'Z'));
}

inline bool IsNumer(char c)
{
	return ((c >= '0') && (c <= '9'));
}


static void EatAllWhitespace(Tokenizer* tokenizer)
{
	while (1)
	{
		if (IsWhitespace(tokenizer->at[0]))
		{
			++tokenizer->at;
		}
		else if (tokenizer->at[0] == '/' && tokenizer->at[1] == '/')
		{
			tokenizer->at += 2;
			while (tokenizer->at[0] && !IsEndOfLine(tokenizer->at[0]))
			{
				++tokenizer->at;
			}
		}
		else if (tokenizer->at[0] == '/' && tokenizer->at[1] == '*')
		{
			tokenizer->at += 2;
			while (tokenizer->at[0] && !(tokenizer->at[0] == '*' && tokenizer->at[1] == '/'))
			{
				++tokenizer->at;
			}

			if (tokenizer->at[0] == '*')
			{
				tokenizer->at += 2;
			}
		}
		else
		{
			break;
		}
	}
}

static Token GetToken(Tokenizer* tokenizer)
{
	EatAllWhitespace(tokenizer);

	Token token{};
	token.textLength = 1;
	token.text = tokenizer->at;
	char c = tokenizer->at[0];
	++tokenizer->at;

	switch (c)
	{
	case '\0': token.type = Token_EndOfStream; break;
	case '(': token.type = Token_OpenParem; break;
	case ')': token.type = Token_ClosenParen; break;
	case ':': token.type = Token_Colon; break;
	case ';': token.type = Token_Semicolon; break;
	case '*': token.type = Token_Asterisk; break;
	case '[': token.type = Token_OpenBracket; break;
	case ']': token.type = Token_CloseBracket; break;
	case '{': token.type = Token_OpenBraces; break;
	case '}': token.type = Token_CloseBraces; break;

	/*case '/':
	{

	} break;
*/
	case '"':
	{
		token.type = Token_String;
		token.text = tokenizer->at;

		while (tokenizer->at[0] && tokenizer->at[0] != '"')
		{
			if ((tokenizer->at[0] == '\\') && tokenizer->at[1])
			{
				++tokenizer->at;
			}
			++tokenizer->at;

		}

		token.textLength = tokenizer->at - token.text;
		if (tokenizer->at[0] == '"')
		{
			++tokenizer->at;
		}
	} break;

	default:
	{
		if (IsAlpha(c))
		{
			token.type = Token_Identifier;
			while (IsAlpha(tokenizer->at[0]) ||
				IsNumer(tokenizer->at[0]) ||
				(tokenizer->at[0] == '_'))
			{
				++tokenizer->at;
				//ParseIdentifier();
			}
			token.textLength = tokenizer->at - token.text;
		}
		else if (IsNumer(tokenizer->at[0]))
		{
			// ParseNumber();
		}
		else
		{
			token.type = Token_Unknown;
		}
	}break;
	}

	return token;
}

inline bool TokenEquals(Token token, char* match)
{
	char *at = match;
	for (int i = 0; i < token.textLength; ++i, at++)
	{
		if ((*at == 0) || (token.text[i] != *at))
		{
			return false;
		}
	}
	return (*at == 0);
}

static bool RequireToken(Tokenizer* tokenizer, token_type type)
{
	Token token = GetToken(tokenizer);
	bool result = token.type == type;
	return result;
}

static void parseIntrospectionParams(Tokenizer * tokenizer)
{
	for (;;)
	{
		Token token = GetToken(tokenizer);
		if ((token.type == Token_ClosenParen) ||
			(token.type == Token_EndOfStream))
		{
			break;
		}
	}
}

static void ParseMember(Tokenizer *tokenizer, Token memberTypeToken)
{
	bool parsing = true;
	bool isPointer = false;
	while (parsing)
	{
		Token token = GetToken(tokenizer);

		//if (token.type == Token_Asterisk)
		//{
		//	isPointer = true;
		//}

		switch (token.type)
		{
		case Token_Asterisk:
			isPointer = true;
			break;

		case Token_Identifier:
			printf("{type_%.*s, %.*s },\n",
				(int)memberTypeToken.textLength, memberTypeToken.text, 
				(int)token.textLength, token.text);
			break;

		case Token_Semicolon:
		case Token_EndOfStream:
			parsing = false;
			break;
		}
	}
}

static void ParseStruct(Tokenizer* tokenizer)
{
	Token nameToken = GetToken(tokenizer);
	if (RequireToken(tokenizer, Token_OpenBraces))
	{
		printf("char *memberOf_%.*s[] = \n", (int)nameToken.textLength, nameToken.text);
		printf("{\n");
		for (;;)
		{
			Token memberToke = GetToken(tokenizer);
			if (memberToke.type == Token_CloseBraces)
			{
				break;
			}
			else
			{
				ParseMember(tokenizer, memberToke);
			}
		}
		printf("}\n");
	}
}

static void parseIntrospectable(Tokenizer *tokenizer)
{
	if (RequireToken(tokenizer, Token_OpenParem))
	{
		parseIntrospectionParams(tokenizer);

		Token typeToken = GetToken(tokenizer);
		if (TokenEquals(typeToken, "struct"))
		{
			ParseStruct(tokenizer);
		}
		else
		{
			fprintf(stderr, "ERROR only supported for struct atm.\n");
		}
	}
	else
	{
		fprintf(stderr, "ERROR missing parentheses.\n");
	}
}



int main(int ArgCount, char ** Args)
{
	char* Filecontents = ReadEntireFIleIntoMemoryAndNullTerminate("I:/Dev/SDL/allegro vanhat/TEST_ENUMERATE/handmade_sim_region.h");

	Tokenizer tokenizer = {};
	tokenizer.at = Filecontents;

	bool parsing = true;
	while (parsing)
	{
		Token token = GetToken(&tokenizer);
		switch (token.type)
		{
		case Token_EndOfStream:
		{
			parsing = false;
		} break;



		case Token_Unknown: break;

		case Token_Identifier:
		{
			if (TokenEquals(token, "introspect"))
			{
				parseIntrospectable(&tokenizer);
			}
		} break;

		default:
		{
			printf("%d: %.*s: \n", token.type, (int)token.textLength, token.text);
		} break;
		}
	}

	getchar();
}