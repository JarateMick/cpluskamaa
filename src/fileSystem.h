#include <cstdio>
#include <string>
// #include <dirent.h>

inline bool FileExist(const std::string& name) {
	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

inline bool FileExist(const char* name) {
	if (FILE *file = fopen(name, "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}
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