//
//  diff.h
//  diffv1
//
//  Created by Anette Ulrichsen on 5/19/19.
//  Copyright © 2019 Anette Ulrichsen. All rights reserved.
//

#ifndef diff_h
#define diff_h

#define MAXBUF 256
#define MAXSTRING 2048
#define ARGC_ERROR 1

typedef struct commands commands;
struct commands {
    int l1;
    int l2;
    int r1;
    int r2;
    char comm;
};

extern char* strings1[MAXSTRING];
extern char* strings2[MAXSTRING];
extern int lines1;
extern int lines2;
extern commands arr[50]; //change value?
extern commands pr[50];
extern int cc;
extern int pc;

extern const char* filename1;
extern const char* filename2;
extern const char* arg_flag;
extern const char* add_flag;

#endif /* diff_h */
