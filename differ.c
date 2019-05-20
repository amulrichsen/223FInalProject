//
//  main.c
//  diffv1
//
//  Created by Anette Ulrichsen on 5/6/19.
//  Copyright © 2019 Anette Ulrichsen. All rights reserved.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diff.h"

#define MAXBUF 256
#define MAXSTRING 2048
#define ARGC_ERROR 1

//typedef struct commands commands;
//struct commands {
//    int l1;
//    int l2;
//    char comm;
//    int r1;
//    int r2;
//};

char* strings1[MAXSTRING];
char* strings2[MAXSTRING];
int lines1 = 1;
int lines2 = 1;
commands arr[50]; //change value?
commands pr[50];
int cc = 0;
int pc = 0;

void flags(char flag);
void version(void);
int quick_comp(void); //quick compare files
void read_files(const char* f1, const char* f2);
void def_out(void); //default, one column output
void commands_print(void); //print command preview
void commands_format(void);
void commands_add_print(int rs, int rq, int pad);
void commands_change_print(int ls, int lq, int rs, int rq);
void commands_delete_print(int ls, int lq, int pad);
void side_by_side(void);
void strip_newline(int f, int lnum);
void side_left(void);
void side_compress(void);
void print_c(void);
void print_u(void);

const char* filename1;
const char* filename2;
const char* arg_flag;
const char* add_flag = "x";


int main(int argc, const char * argv[]) {
    char flag;
    
    if (argc < 3) { fprintf(stderr, "Usage: ./diff file1 file2\n");  exit(ARGC_ERROR); }
    else if (argc == 3) {
        arg_flag = 0;
        filename1 = argv[1];
        filename2 = argv[2];
        read_files(argv[1], argv[2]);
    }
    else if (argc == 4) {
        arg_flag = argv[1];
        filename1 = argv[2];
        filename2 = argv[3];
        read_files(argv[2], argv[3]);
    }
    if (argc == 5) {
        arg_flag = argv[1];
        add_flag = argv[2];
        filename1 = argv[3];
        filename2 = argv[4];
        read_files(argv[3], argv[4]);
    }
    
    if(arg_flag != 0) {
        flag = arg_flag[1];
        flags(flag);
    }
    else {
        def_out();
        commands_format();
        commands_print();
    }
    
    printf("\n");
    
    return 0;
}

void flags(char flag) {
    int result;
    
    switch (flag) {
            //version -- prints version, license, and author (you)
        case 'v':
            version();
            break;
            
            //brief -- reports only whether files are different; silent if they are not
        case 'q':
            result = quick_comp();
            if (result == 1) {
                printf("Files %s and %s differ\n", filename1, filename2);
            }
            if (result == 0 && (!strcmp(add_flag, "-s") || !strcmp(add_flag, "-q"))) {
                printf("Files %s and %s are identical\n", filename1, filename2);
            }
            break;
            
            //report-identical-files -- reports if files are the same; usual out if they are not
        case 's':
            result = quick_comp();
            if (result == 0) {
                printf("Files %s and %s are identical\n", filename1, filename2);
            }
            else if (result != 0 && (!strcmp(add_flag, "-s") || !strcmp(add_flag, "-q"))) {
                printf("Files %s and %s differ\n", filename1, filename2);
            }
            else {
                def_out();
                commands_format();
                commands_print();
            }
            break;
            
            //side-by-side -- side by side format && prints common lines
        case 'y':
            if (!strcmp(add_flag, "--left-column")) {
                def_out();
                commands_format();
                side_left();
            }
            else if (!strcmp(add_flag, "--suppress-common-lines")){
                def_out();
                commands_format();
                side_compress();
            }
            else {
                def_out();
                commands_format();
                side_by_side();
            }
            break;
            
        case 'c':
            //if no num, default 3
            //else take num
            print_c();
            break;
            
        case 'u':
            print_u();
            break;
            
            //error
        default:
            break;
    }
}

void version(void) {
    printf("diff v0.0.1\n");
    printf("Copyright (C) 2019 Anette Ulrichsen, girl-genius\n");
    printf("Written by Anette Ulrichsen\n");
}

int quick_comp(void) {
    int return_val = 0;
    int x = 1;
    
    while((x <= lines1) && (x <= lines2) && (return_val != 1)) {
        if(strcmp(strings1[x], strings2[x])) {return_val = 1;}
        ++x;
    }
    
    return return_val;
}

void read_files(const char* f1, const char* f2) {
    char buf[MAXBUF];
    FILE* file1 = fopen(f1, "r");
    FILE* file2 = fopen(f2, "r");
    
    if(!file1 || !file2) {
        printf("Error opening files\n");
        return;
        //throw some error?
    }
    while (!feof(file1) && fgets(buf, MAXBUF, file1) != NULL) { strings1[lines1++] = strdup(buf); }
    while (!feof(file2) && fgets(buf, MAXBUF, file2) != NULL) { strings2[lines2++] = strdup(buf); }
    
    fclose(file1);
    fclose(file2);
}

void def_out(void) {
    int rp = 0; //retain the last line used
    int lp = 0;
    
    int right = 1; //current index
    int left = 1;
    
    int cmp = 1;
    
    while(left < lines1) {
        //compare left and right
        while ((right + rp) < lines2 && cmp != 0) {
            cmp = strcmp(strings1[left + lp], strings2[right + rp]);
            if (cmp != 0) {
                if((left == lines1 - 1 || right == lines2 - 1) && (cmp == -10 || cmp == 10)) {
                    //new line at end of file, ignore
                    cmp = 0;
                    rp = lp = 0;
                }
                else {
                    ++rp;
                }
            }
        }
        if (cmp == 0 && rp == lp) {
            //do nothing, lines match, no changes
            right++;
        }
        else if (cmp != 0) {
            if (right == left && lines1 == lines2 && !strncmp(strings1[left], strings2[right], 5)) {
                //printf("this is true... change\n");
                arr[cc].l1 = arr[cc].l2 = left;
                arr[cc].comm = 'c';
                arr[cc].r1 = arr[cc].r2 = right;
                ++right;
                cc++;
            }
            else if((left < lines1) && (right + rp == lines2)) {
                //delete
                arr[cc].l1 = arr[cc].l2 = left;
                arr[cc].comm = 'd';
                arr[cc].r1 = arr[cc].r2 = right - 1;
                //++right;
                cc++;
            }
            else {
                //change
                arr[cc].l1 = arr[cc].l2 = left;
                arr[cc].comm = 'c';
                arr[cc].r1 = arr[cc].r2 = right;
                ++right;
                cc++;
            }
        }
        else if(left != lines1 - 1 && right != lines2 - 1){
            int temp = strcmp(strings1[left + 1], strings2[right + rp + 1]);
            if (temp == 0) {
                //add
                arr[cc].l1 = arr[cc].l2 = left - 1;
                arr[cc].comm = 'a';
                arr[cc].r1 = right;
                arr[cc].r2 = right + rp - 1;
                right += rp + 1;
                cc++;
            }
            else {
                //delete
                arr[cc].l1 = arr[cc].l2 = left;
                arr[cc].comm = 'd';
                arr[cc].r1 = arr[cc].r2 = right - 1;
                //++right;
                cc++;
            }
        }
        else {
            //add
            arr[cc].l1 = arr[cc].l2 = left - 1;
            arr[cc].comm = 'a';
            arr[cc].r1 = right;
            arr[cc].r2 = right + rp - 1;
            right += rp + 1;
            cc++;
        }
        
        if ((left == lines1 - 1) && (right + rp < lines2)) {
            //add right lines
            for (; right < lines2; right++) {
                arr[cc].l1 = arr[cc].l2 = left;
                arr[cc].comm = 'a';
                arr[cc].r1 = arr[cc].r2 = right;
                cc++;
            }
        }
        left++;
        rp = lp = 0;
        cmp = 1;
        
    }
    
}

void commands_print(void) {
    for (int i = 0; i < pc - 1; i++) {
        //first 2 same
        if (pr[i].l1 == pr[i].l2) {
            if (pr[i].r1 == pr[i].r2) {
                //second same same
                printf("%i%c%i\n", pr[i].l2, pr[i].comm, pr[i].r2);
            }
            else {
                //second two different
                printf("%i%c%i,%i\n", pr[i].l2, pr[i].comm, pr[i].r1, pr[i].r2);
            }
        }
        else {
            if (pr[i].r1 == pr[i].r2) {
                //first two diff
                //second same same
                printf("%i,%i%c%i\n", pr[i].l1, pr[i].l2, pr[i].comm, pr[i].r2);
            }
            else {
                //second two different
                printf("%i,%i%c%i,%i\n", pr[i].l1, pr[i].l2, pr[i].comm, pr[i].r1, pr[i].r2);
            }
        }
        if (pr[i].comm == 'a') {
            commands_add_print(pr[i].r1, pr[i].r2, 0);
        }
        else if (pr[i].comm == 'd') {
            commands_delete_print(pr[i].l1, pr[i].l2, 0);
        }
        else {
            commands_change_print(pr[i].l1, pr[i].l2, pr[i].r1, pr[i].r2);
        }
    }
    
}

void commands_format(void) {
    int index = pc = 0;
    char commcurr = arr[index].comm;
    while(index <= cc) {
        pr[pc].l1 = arr[index].l1;
        pr[pc].l2 = arr[index].l2;
        pr[pc].comm = arr[index].comm;
        pr[pc].r1 = arr[index].r1;
        pr[pc].r2 = arr[index].r2;
        ++index;
        
        while((arr[index].comm == commcurr) && (pr[pc].l2 == arr[index].l2 - 1) && (pr[pc].r2 == arr[index].r2 - 1)) {
            pr[pc].l2 = arr[index].l2;
            pr[pc].r2 = arr[index].r2;
            ++index;
        }
        
        while((arr[index].comm == commcurr) && (pr[pc].l2 == arr[index].l2 - 1) && (pr[pc].r2 == arr[index].r2)) {
            pr[pc].l2 = arr[index].l2;
            ++index;
        }
        
        while((arr[index].comm == commcurr) && (pr[pc].l2 == arr[index].l2) && (pr[pc].r2 == arr[index].r2 - 1)) {
            pr[pc].r2 = arr[index].r2;
            ++index;
        }
        
        commcurr = arr[index].comm;
        ++pc;
    }
}

void commands_add_print(int rs, int rq, int pad) {
    int i = rs - pad;
    int max = rq + pad;
    
    if (rs - pad <= 0) {
        i = 0;
    }
    for (; i <= max && i < lines2; i++) {
        if(i == rs && pad > 0) { printf("+"); }
        printf("> %s", strings2[i]);
    }
}

void commands_change_print(int ls, int lq, int rs, int rq) {
    for (; rs <= rq; rs++) {
        printf ("< %s", strings2[rs]);
    }
    printf("\n---\n");
    for (; ls <= lq; ls++) {
        printf ("> %s", strings1[ls]);
    }
}

void commands_delete_print(int ls, int lq, int pad) {
    int i = ls - pad;
    int max = lq + pad;
    
    if (ls - pad <= 0) {
        i = 0;
    }
    for (; i <= max && i < lines1; i++) {
        if(i == ls && pad > 0) { printf("-"); }
        printf("< %s", strings1[i]);
    }
}

void side_by_side(void) {
    int lastleft = 1;
    int lastright = 1;
    
    while (lastleft <= pr[0].l1 && lastright <= pr[0].r1 && !strcmp(strings1[lastleft], strings2[lastright])) {
        strip_newline(1, lastleft);
        if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
        printf("%-51s" ,strings1[lastleft]);
        strip_newline(2, lastright);
        if(!strcmp(strings2[lastright], "")) { strings2[lastright] = "\n"; }
        printf("%s", strings2[lastright]);
        lastleft++;
        lastright++;
        printf("\n");
    }
    
    for(int i = 0; i < pc; i++) {
        if (pr[i].comm == 'd') {
            //print left
            for (int j = pr[i].l1; j <= pr[i].l2; j++) {
                strip_newline(1, j);
                printf("%-45s",strings1[j]);
                printf("<\n");
            }
            lastleft = pr[i].l2 + 1;
        }
        else if (pr[i].comm == 'a') {
            //print right
            for (int j = pr[i].r1; j <= pr[i].r2; j++) {
                printf("%50s %s", ">", strings2[j]);
            }
            lastright = pr[i].r2 + 1;
        }
        else if (pr[i].comm == 'c'){
            //print change on same line
            for (int j = pr[i].r1; j <= pr[i].r2; j++) {
                strip_newline(1, lastleft);
                if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
                printf("%-45s",strings1[lastleft]);
                strip_newline(2, lastright);
                if(!strcmp(strings2[lastright], "")) { strings2[lastright] = "\n"; }
                printf("%s %s", "|", strings2[lastright]);
                printf("\n");
                ++lastleft;
                ++lastright;
            }
            lastright = pr[i].r2 + 1;
        }
        while (lastleft <= pr[i+1].l1 && lastright <= pr[i+1].r1) {
            strip_newline(1, lastleft);
            if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
            printf("%-51s" ,strings1[lastleft]);
            strip_newline(2, lastright);
            if(!strcmp(strings2[lastright], "")) { strings2[lastright] = "\n"; }
            printf("%s", strings2[lastright]);
            lastleft++;
            lastright++;
            printf("\n");
        }
    }
    
    while (lastleft < lines1 && lastright < lines2) {
        strip_newline(1, lastleft);
        if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
        printf("%-51s" ,strings1[lastleft]);
        strip_newline(2, lastright);
        if(!strcmp(strings2[lastright], "")) { strings2[lastright] = "\n"; }
        printf("%s", strings2[lastright]);
        lastleft++;
        lastright++;
        printf("\n");
    }
    
}

void side_left(void) {
    int lastleft = 1;
    int lastright = 1;
    
    while (lastleft <= pr[0].l1 && lastright <= pr[0].r1 && !strcmp(strings1[lastleft], strings2[lastright])) {
        strip_newline(1, lastleft);
        if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
        printf("%-51s" ,strings1[lastleft]);
        printf("(\n");
        lastleft++;
        lastright++;
    }
    
    for(int i = 0; i < pc; i++) {
        if (pr[i].comm == 'd') {
            //print left
            for (int j = pr[i].l1; j <= pr[i].l2; j++) {
                strip_newline(1, j);
                printf("%-45s",strings1[j]);
                printf("<\n");
            }
            lastleft = pr[i].l2 + 1;
        }
        else if (pr[i].comm == 'a') {
            //print right
            for (int j = pr[i].r1; j <= pr[i].r2; j++) {
                printf("%50s %s", ">", strings2[j]);
            }
            lastright = pr[i].r2 + 1;
        }
        else if (pr[i].comm == 'c'){
            //print change on same line
            for (int j = pr[i].r1; j <= pr[i].r2; j++) {
                strip_newline(1, lastleft);
                if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
                printf("%-45s",strings1[lastleft]);
                strip_newline(2, lastright);
                if(!strcmp(strings2[lastright], "")) { strings2[lastright] = "\n"; }
                printf("%s %s", "|", strings2[lastright]);
                printf("\n");
                ++lastleft;
                ++lastright;
            }
            lastright = pr[i].r2 + 1;
        }
        while (lastleft <= pr[i+1].l1 && lastright <= pr[i+1].r1) {
            strip_newline(1, lastleft);
            if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
            printf("%-51s" ,strings1[lastleft]);
            printf("(\n");
            lastleft++;
            lastright++;
        }
    }
    
    while (lastleft < lines1 && lastright < lines2) {
        strip_newline(1, lastleft);
        if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
        printf("%-51s" ,strings1[lastleft]);
        printf("(\n");
        lastleft++;
        lastright++;
    }
}

void side_compress(void) {
    int lastleft = 1;
    int lastright = 1;
    
    for(int i = 0; i < pc; i++) {
        if (pr[i].comm == 'd') {
            //print left
            for (int j = pr[i].l1; j <= pr[i].l2; j++) {
                strip_newline(1, j);
                printf("%-45s",strings1[j]);
                printf("<\n");
            }
            lastleft = pr[i].l2 + 1;
        }
        else if (pr[i].comm == 'a') {
            //print right
            for (int j = pr[i].r1; j <= pr[i].r2; j++) {
                printf("%50s %s", ">", strings2[j]);
            }
            lastright = pr[i].r2 + 1;
        }
        else if (pr[i].comm == 'c'){
            //print change on same line
            for (int j = pr[i].r1; j <= pr[i].r2; j++) {
                strip_newline(1, lastleft);
                if(!strcmp(strings1[lastleft], "")) { strings1[lastleft] = "\n"; }
                printf("%-45s",strings1[lastleft]);
                strip_newline(2, lastright);
                if(!strcmp(strings2[lastright], "")) { strings2[lastright] = "\n"; }
                printf("%s %s", "|", strings2[lastright]);
                printf("\n");
                ++lastleft;
                ++lastright;
            }
            lastright = pr[i].r2 + 1;
        }
    }
}

void strip_newline(int f, int lnum) {
    if (f == 1 && strcmp(strings1[lnum], "\n")) {
        //left
        strings1[lnum] = strtok(strings1[lnum], "\n");
    }
    else if (f == 1 && !strcmp(strings1[lnum], "\n")) {
        //left
        //assign 0
        strings1[lnum] = "";
    }
    else if (f == 2 && strcmp(strings2[lnum], "\n")){
        //right
        strings2[lnum] = strtok(strings2[lnum], "\n");
    }
    else {
        //right
        //assign 0
        strings2[lnum] = "";
    }
    
}

void print_c(void) {
    printf("*** %s\n", filename1);
    printf("--- %s\n", filename2);
    printf("Didn't have time to do this part\n");
    
}

void print_u(void) {
    printf("*** %s\n", filename1);
    printf("--- %s\n", filename2);
    printf("Didn't have time to do this part\n");
}
