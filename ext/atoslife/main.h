#ifndef MAIN_H
#define MAIN_H
// int symbolicate(const char* arch, const char *executable, char *addresses[], int numofaddresses);
int symbolicate(const char* arch, const char *executable, const char *loadAddress, char *addresses[], int numofaddresses);
#endif
