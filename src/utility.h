#pragma once
#define internal static
#include <cstdio> // do smt...

internal void writeEntireFile(char* buffer, int sizeOfBuffer, const char* filename) // length of the string
{
	FILE *fp = fopen(filename, "w");
	fwrite(buffer, sizeof(char), sizeOfBuffer, fp);
	fclose(fp);
}

internal char* readEntireFile(const char* filename, int &bufferSize)
{
	char *buffer = NULL;
	FILE *fp = fopen(filename, "r");

	if (fp != NULL) {
		/* Go to the end of the file. */
		if (fseek(fp, 0L, SEEK_END) == 0) {
			long bufsize = ftell(fp);
			if (bufsize == -1) { /* Error */ }

			bufferSize = (int)bufsize;
			buffer = (char *)malloc(sizeof(char) * (bufsize + 1));

			/* Go back to the start of the file. */
			if (fseek(fp, 0L, SEEK_SET) != 0) { /* Handle error here */ }

			size_t newLen = fread(buffer, sizeof(char), bufsize, fp);

			if (newLen == 0) {
				fputs("Error reading file", stderr);
			}
			else
			{
				// mitä vittuuuuu
				// buffer[++newLen] = '\0'; /* Just to be safe. */
				// bufferSize = newLen + 1;
			}
		}
		fclose(fp);
	}
	return buffer;
}

////////////////////////////////////////////////////////////
// EVEN MORE FILE STUFF

// TODO: TEE concatista päheen nopee kappa!
// päheen nopee https://stackoverflow.com/questions/21880730/c-what-is-the-best-and-fastest-way-to-concatenate-strings
internal char* mystrcat(char* dest, char* src)
{
	while (*dest) dest++;
	while (*dest++ = *src++);
	return --dest;
}
char* enumss[] = { "Sword", "Shield" };
char* pathss[] = { "\\path", "\\pathtosecond" };
internal void insertSmt(char buffer[], char* array[])
{
	for (int i = 0; i < 2; ++i)
	{
		strcat(buffer, "\t");
		strcat(buffer, array[i]);
		strcat(buffer, ",\n");
	}
	strcat(buffer, "\n");
}
internal void insertText(char buffer[], char* array[])
{
	for (int i = 0; i < 2; ++i)
	{
		strcat(buffer, "\t\"");
		strcat(buffer, array[i]);
		strcat(buffer, "\",\n");
	}
	strcat(buffer, "\n");
}
internal void insertSingleEnum(char buffer[], const char* str, const char* enumType)
{
	strcat(buffer, enumType);
	strcat(buffer, str);
	strcat(buffer, ",\n");
}
internal void insertSingleCharArrayElement(char buffer[], std::string str)
{
	for (int i = 0; i < str.size(); ++i)
	{
		if (str[i] == '\\')
		{
			str[i] = '/';
		}
	}
	strcat(buffer, "\t\"");
	strcat(buffer, str.c_str());
	strcat(buffer, "\",\n");
}


internal void createTextureEnums(TextureFile* files, int size, AssetFileInfo* info)
{
	const char* enumStart = "enum Textures {\n";
	const char* enumEnd = "\tTexture_Max\n\
};\n\n";
	const char* pathStart = "const std::string paths[Texture_Max] =\n\
{\n";
	const char* musicPathStart = "const std::string music_paths[Music_Max] =\n{\n";
	const char* effectsPathStart = "const std::string effects_paths[Effect_Max] =\n{\n";

	const char* pathEnd = "};\n\n";

	const char* musicEnumStart = "enum Musics {\n";
	const char* musicEnumEnd = "\tMusic_Max\n\
};\n\n";
	const char* effectEnumStart = "enum Effects {\n";
	const char* effectEnumEnd = "\tEffect_Max\n\
};\n\n";


	char buffer[2048]{};
	strcpy(buffer, enumStart);
	// insertSmt(buffer, enumss);
	for (int i = 0; i < size; ++i)
	{
		insertSingleEnum(buffer, files[i].name.c_str(), "\tTexture_");
	}
	strcat(buffer, enumEnd);

	strcat(buffer, pathStart);

	// insertText(buffer, pathss);
	for (int i = 0; i < size; ++i)
	{
		insertSingleCharArrayElement(buffer, files[i].path);
	}
	strcat(buffer, pathEnd);


	// music
	int total = info->textureCount;
	strcat(buffer, musicEnumStart);
	for (int i = info->textureCount; i < info->musicCount + total; ++i)
	{
		insertSingleEnum(buffer, files[i].name.c_str(), "\tMusic_");
	}
	strcat(buffer, musicEnumEnd);

	strcat(buffer, musicPathStart);
	for (int i = info->textureCount; i < info->musicCount + total; ++i)
	{
		insertSingleCharArrayElement(buffer, files[i].path);
	}
	strcat(buffer, pathEnd);

	total += info->musicCount + 1; // TODO: IMPORTANT GET RID OF .INL FILE in folder
	strcat(buffer, effectEnumStart);
	for (int i = total; i < total + info->effectCount - 1; ++i)
	{
		insertSingleEnum(buffer, files[i].name.c_str(), "\tTexture_");
	}
	strcat(buffer, effectEnumEnd);

	strcat(buffer, effectsPathStart);
	for (int i = total; i < info->effectCount + total - 1; ++i)
	{
		insertSingleCharArrayElement(buffer, files[i].path);
	}
	strcat(buffer, pathEnd);




	int sizeOfString = strlen(buffer);
	if (sizeOfString > 2048) // oma allokaattori ei alligaattori
	{
		assert(false);
	}
	writeEntireFile(buffer, sizeOfString, "../src/enumtest.cpp");
};
