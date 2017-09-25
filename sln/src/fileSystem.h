#pragma once
#include <cstdio>
#include <string>

inline bool FileExist(const std::string& name);
inline bool FileExist(const char* name);
static FILETIME Win32GetLastWriteTime(const char* path);

// #include <dirent.h> <-- tuolta saattaa löytyä kivaa

///*
//DIR *dir;
//struct dirent *ent;
//if ((dir = opendir("c:\\src\\")) != NULL) {
////	/* print all the files and directories within directory */
//	while ((ent = readdir(dir)) != NULL) {
//		printf("%s\n", ent->d_name);
//	}
//	closedir(dir);
//}
//else {
//	/* could not open directory */
//	perror("");
//	return EXIT_FAILURE;
//}