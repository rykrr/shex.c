/* shex.c: Simple Hex Editor    */
/* Copyright (c) 2017 Ryan Kerr */

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#define HEXLIM 65536*2

typedef struct hex {
    unsigned char hex;
    struct hex *prev, *next;
} HEX;

HEX *TOP = NULL;
HEX *IX[HEXLIM];

int len = 0, cur = -1, row = 0;

void stat(const char *s) {
    attron(A_REVERSE);
    int x, y;
    getmaxyx(stdscr, y, x);
    for(int i=0; i<x; i++)
        mvprintw(y-1, i, " ");
    
    mvprintw(y-1, 0, "  %s", s);
    mvprintw(y-1, x-11, "%04X/%04X", cur, len-1);
    refresh();
    attroff(A_REVERSE);
}

void reindex() {
    
    if(len<0) {
        for(int i=0; i<HEXLIM; i++)
            IX[i] = NULL;
        return;
    }
    
    int c = 0;
    for(HEX *h = TOP; h; h=h->next)
        IX[c++] = h;
    
    len = c;
    for(;c<HEXLIM;)
        IX[c++] = NULL;
}

void del(HEX *h) {
    
    if(!h) {
        //printf("Warning: Attempted NULL deletion\n");
        return;
    }
    
    if(h->next && h->prev) {
        h->prev->next = h->next;
        h->next->prev = h->prev;
        free(h);
    }
    else if(!h->next && h->prev) {
        h->prev->next = NULL;
        free(h);
    }
    else if(h->next && !h->prev) {
        h->next->prev = NULL;
        TOP = h->next;
        
        free(h);
    }
    else {
        free(TOP);
        TOP = NULL;
        //printf("Warning: Last element erased\n");
    }
    
    len--;
    reindex();
}

void delall() {
    while(TOP)
        del(TOP);
}

void new(HEX *h, int fb, int v) {
    
    if(len == HEXLIM)
        return;
    
    if(!h && !TOP) {
        TOP = malloc(sizeof(HEX));
        *TOP = (HEX){v, NULL, NULL};
        cur = 0;
        len = 1;
        reindex();
        return;
    }
    else if(!h) {
        //printf("Warning: Attempted NULL append\n");
        return;
    }
    
    if(fb && !h->prev) {
        TOP = malloc(sizeof(HEX));
        *TOP = (HEX){v, NULL, h};
        h->prev = TOP;
        reindex();
        return;
    }
    
    HEX *x = (fb?h->prev:h->next);
    
    HEX *n = malloc(sizeof(HEX));
        *n = (HEX){v, fb?(x?x:NULL):h, fb?h:(x?x:NULL)};
        
    if(!fb) {
        if(x)
            h->next->prev = n;
        h->next = n;
    }
    else {
        if(x)
            h->prev->next = n;
        h->prev = n;
    }
    
    //cur++;
    len++;
    reindex();
}

void load(const char *p) {
    
    unsigned char buf[HEXLIM];
    
    //FILE *f = fopen("bin_output", "rb");
    FILE *f = fopen(p, "rb");
    
    if(f) {
        int l = fread(buf, sizeof(unsigned char), HEXLIM, f);
        
        delall();
        new(NULL, 0, buf[0]);
        
        HEX *c = TOP;
        for(int i=1; i<l; i++) {
            new(c, 0, buf[i]);
            c=c->next;
        }
        fclose(f);
    }
}

int save(char *p, int q) {
    
    if(!TOP)
        return 0;
    
    unsigned char *buf = malloc(len);
    
    int i=0;
    for(HEX *b = TOP; b && i<len; b=b->next)
        buf[i++] = b->hex;
    
    if(!q) {
        stat("Do you want to save? (y/n)");
        char c = getch();
        if(c != 'y') {
            stat("");
            return 0;
        }
    }
    
    //FILE *f = fopen("output", "wb");
    FILE *f = fopen(p, "wb");
    
    if(f) {
        fwrite(buf, sizeof(unsigned char), i, f);
        stat("File saved");
        fclose(f);
        return 0;
    }
    else {
        stat("File failed to save");
        return -1;
    }
    
    if(buf)
        free(buf);
}

void init() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, 1);
}

void draw(int o, int s) {
    
    static int off = 0;
    static int oll = 0;
    
    if(!oll)
        oll = len;
    
    int dx, mx, my, mod, mid;
    getmaxyx(stdscr, my ,mx);
    
    my--;
    //mod = (mx/6)-2;
    mod = 32;
    mid = (mx-mod)/2;
    mod = 12;
    dx  = len/mod + (len%mod?1:0);
    
    for(int i=0; i<my; i++)
        mvprintw(i,0, "\n");
    
    int c = cur;
    
    if(len < oll && !s && !o)
        s=-1;
    if(len > oll && !s && !o)
        s=1;
    
    if(s) {
        if(0<s && cur+1<len) {
            cur++;
            if(cur%mod==0)
                o=1;
        }
        
        if(s<0 && 0<=cur-1) {
            cur--;
            if(cur%mod==mod-1)
                o=-1;
        }
        stat("");
    }
        
    if(o) {
        if(!s) {
            if(0<o && cur+mod<len)
                cur+=mod;
            else if(0<o && len-1<cur+mod)
                cur=len-1;
            else if(o<0 && 0<cur-mod)
                cur-=mod;
            else if(o<0 && cur-mod==0)
                cur=0;
        }
        
        if(c<cur) {
            if(row<my-1 && row<dx-1)
                row++;
            else if(off<dx-my)
                off++;
        }
        if(cur<c) {
            if(0<row)
                row--;
            else if(0<off)
                off--;
        }
        stat("");
    }
    
    for(int i=0; i<my; i++)
        mvprintw(i, 0, "\n");
    
    for(int i=0; (i+off)<dx && i<my; i++) {
        mvprintw(i, mid-6, "%04X\n", i+off);
        mvprintw(i, mid+(3*mod)+1, "%04X\n", i+off);
        for(int j=0; j<mod && mod*(i+off)+j<len; j++) {
            if(cur == (off+i)*mod+j)
                attron(A_BOLD);
            else if(!IX[(off+i)*mod+j]->hex)
                attron(A_DIM);
            
            if(!IX[(off+i)*mod+j]->hex)
                mvprintw(i, mid+(3*j), "--");
            else
                mvprintw(i, mid+(3*j), "%02X", IX[(off+i)*mod+j]->hex);
            
            attroff(A_BOLD|A_DIM);
        }
    }
    
    oll=len;
    
    mvprintw(row, mid-(mod-3), "[[");
    mvprintw(row, mid+(3*(mod+2)), "]]");
    
    if(len) {
        attron(A_BOLD);
        mvprintw(row, mid+3*(cur%mod)-1, "[");
        mvprintw(row, mid+3*(cur%mod)-1, "[");
        mvprintw(row, mid+3*(cur%mod)+2, "]");
        attroff(A_BOLD);
    }
}

int main(int argc, char *argv[]) {
    
    printf("shex.c: Simple Hex Editor\nCopyright (c) 2017 Ryan Kerr\n");
    
    setbuf(stdout, NULL);
    reindex();
    
    if(argc == 2)
        load(argv[1]);
    else
        exit(404);
    
    init();
    
    if(save(argv[1], 1)) {
        printf("Insufficient file privileges");
        return 403;
    }
    
    cur = 0;
    draw(0, 0);
    stat(argv[1]);
    getch();
    
    int xmode = 0;
    int modi = 0;
    int quit = 0;
    
    do {
        char c = getch();
        int C = c;
        switch(C) {
            case 'j':
                draw(1, 0);
                break;
            case 'k':
                draw(-1, 0);
                break;
            case 'l':
                draw(0, 1);
                break;
            case 'h':
                draw(0, -1);
                break;
            case 'n':
                new(IX[cur], 0, 0);
                draw(0, 0);
                break;
            case 'i':
                new(IX[cur], 1, 0);
                draw(0, 0);
                break;
            case 's':
                save(argv[1], 0);
                draw(0, 0);
                break;
            case 'r':
                del(IX[cur]);
                draw(0, 0);
                break;
            case 'g':
                if(len)
                    cur = 0;
                draw(0, 0);
                break;
            case 'q':
                stat("Are you sure you want to quit? (y/n)");
                char x = getch();
                if(x == 'y')
                    quit = 1;
                stat("");
            case KEY_RESIZE:
                endwin();
                init();
                draw(0, 0);
                stat(argv[1]);
                break;
            default:
                if(len) {
                    if('0' <= c && c <= '9') {
                        IX[cur]->hex &= (xmode?0xF0:0x0F);
                        IX[cur]->hex |= (c-'0')<<(xmode++?0:4);
                        modi++;
                    }
                    else if('A' <= c && c <= 'F') {
                        IX[cur]->hex &= (xmode?0xF0:0x0F);
                        IX[cur]->hex |= (c-'A'+10)<<(xmode++?0:4);
                        modi++;
                    }
                    else if('a' <= c && c <= 'f') {
                        IX[cur]->hex &= (xmode?0xF0:0x0F);
                        IX[cur]->hex |= (c-'a'+10)<<(xmode++?0:4);
                        modi++;
                    }
                    draw(0, 0);
                }
        }
        xmode %= 2;
        
        if(!modi)
            xmode = 0;
        modi = 0;
    } while(!quit);
    endwin();
    delall();
    return 0;
}
