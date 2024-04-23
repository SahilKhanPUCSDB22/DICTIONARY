#include"functions.c"
#include"insertion.c"
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#define size 46

unsigned int wc, nnodes, tsize, next;

typedef struct mlist
{
	struct mlist *next;
	unsigned char ch;
	unsigned int addr;
} mlist;

typedef struct trie
{
	struct trie **alpha;
	mlist *list;
	int mptr, filled_pos,nsize;
	unsigned char type;
} trie;

