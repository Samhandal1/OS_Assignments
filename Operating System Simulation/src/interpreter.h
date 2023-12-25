#ifndef INTERPRETER_H
#define INTERPRETER_H
int interpreter(char* command_args[], int args_size);
int my_cd(char* dirname);
int my_touch(char* filename);
int my_mkdir(char *dirname);
int help();
#endif