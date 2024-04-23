#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
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

/****************************************************************************************************************/

#define gind(x) ((x>96) ? (x-97) : (x-65))

#define adjust(x) \
	do{ \
		if(x[1]==0) \
		{ \
			x[1]=8; \
			x[0]++; \
		} \
	}while(0)
/***************************************************************************************************************/
int ind (char ch)
{
	if (ch >= 65 && ch <= 90)	//for direct index access when capital alphabet is encountered
	{
		ch = ch - 65;
	}
	if (ch >= 97 && ch <= 122)	//same as above but for small alphabet
	{
		ch = ch - 97;
	}
	return ch;
}

/****************************************************************************************************************/
unsigned char countval (unsigned int val)
{
	unsigned char i = 0, count = 0;
	while (i < 26)
	{
		if ((val & (1 << i)) != 0)	//alpha was visited
		{
			count++;
		}
		i++;
	}

	return count;
}
/****************************************************************************************************************/
void check_ntype (trie * t, int ncount[])
{
	unsigned char i = 0, count = 0;	//i for bit checking count to keep count of alphabets in current level
	if ((t->filled_pos > 0) || (t->mptr > 0))
	{
		count = countval (t->filled_pos);
//		wc = wc + (countval (t->mptr));
		ncount[count - 1] = ncount[count - 1] + 1;	//increment total count of 'count' type of node 

		i = 0;
		while (i < 26)
		{
			if ((t->filled_pos & (1 << i)) != 0)	//alpha was visited
			{
				check_ntype (t->alpha[i], ncount);	//go a level down
			}
			i++;
		}
	}
}
/***************************************************************************************************************/
int findn (int nodes, int word, int fsize)
{
	int c[3];
	c[0] = 26 * (nodes);
	c[1] = 55 * (nodes) + word * (ceil (log2 (fsize)));
	c[2] = 0;

	while ((pow (2, c[2]) - (c[0] * c[2])) < c[1])
	{
		c[2]++;
	}

	return c[2];
}
/****************************************************************************************************************/
void assign_type (unsigned int nodeptr, unsigned int meaningptr, trie ** tp)
{
	trie* t = *(tp);
	int val[2];
	int nsize = 3;
	if (t->filled_pos > 0 || t->mptr > 0)
	{
		val[0] = countval (t->filled_pos);
		val[1] = countval (t->mptr);

		if (val[0] > 0 && val[1] > 0)	//both next and meaning
		{
			t->type = 2;
			if (val[0] == 26 && val[1] == 26)
			{
				nsize = nsize + (26 * ( nodeptr + meaningptr));	//fully filled
				t->type <<= 1;
			}
			else
			{
				nsize = nsize + (26 + 26 + (val[0] * nodeptr) + (val[1] * meaningptr));	//partially filled
				t->type = ((t->type << 1) | 1);
			}
		}
		else if (val[0] > 0)	//only next
		{
			t->type = 1;
			if ((val[0] * 5) > 26)
			{
				nsize = nsize + (26 + (val[0] * nodeptr));
				t->type = t->type << 1;
			}
			else
			{
				nsize = nsize + 5 + ((5 + (nodeptr)) * val[0]);  //char rep
				t->type = ((t->type << 1) | 1);
			}
		}
		else if (val[1] > 0)
		{
			t->type = 0;
			if ((val[1] * 5) > 26)	//only meaning
			{
				nsize = nsize + (26 + (val[1] * meaningptr));	//bitmap
				t->type = t->type << 1;
			}
			else
			{
				nsize = nsize +5+ ((5 + (meaningptr)) * val[1]);	//char rep
				t->type = ((t->type << 1) | 1);
			}
		}

		t->nsize = nsize;
		tsize = tsize + nsize;

		char i = 0;
		while (i < 26)
		{
			if ((t->filled_pos & (1 << i)) != 0)	//alpha was visited
			{
				assign_type (nodeptr, meaningptr, &(t->alpha[i]));	//go a level down
			}
			i++;
		}
	}

}
/****************************************************************************************************************/

void enc (unsigned char ref[], unsigned int args[])
{
	//args[0]=current
	//args[1]=left
	//args[2]=value
	//args[3]=pointer length
	int len = args[3];
	int val = args[2];

//	printf("current %d left %d val %d size %d \n",args[0],args[1],args[2],args[3]);

	adjust(args);

	while (len > 0)
	{
		if (len <= args[1])
		{
//			printf("%u\n",ref[0]);
			ref[args[0]] |= (val << (args[1] - len));
//			printf("%u\n",ref[0]);
			if(args[1]==len) //left == pointersize
			{
				args[1]=8;
				args[0]++;
			}
			else
			{
				args[1]-=len;
			}
			len = 0;
		}
		else
		{
			ref[args[0]] = ref[args[0]] | ( ( val >> (len -= args[1]) ) );
			args[1] = 8;
			args[0]++;
		}
	}
//	printf("left after enc %d curr after %u\n",args[1],ref[0]);
}
/**************************************************************************************************************/
void jump(unsigned int args[])
{
	adjust(args);

	if (args[2] <= args[1])
	{
		if(args[2]==args[1])
		{
			args[0]++;
			args[1]=8;
		}
		else
		{
			args[1] -= args[2];
		}
	}
	else
	{
		args[2] -= args[1];
		args[0]++;
		args[0] += (args[2] / 8);
		args[1] = 8 - ( args[2] - (8 * (args[2] / 8)) );
	}
}

/***************************************************************************************************************/

long int getptr(unsigned char ref[] , unsigned int args[])
{
//	printf("current %d left %d val %d \n",args[0],args[1],args[2]);

	long int re=0;
	unsigned char x=0;

	adjust(args);

	while(args[2] > 0)
	{
		if(args[2]<=args[1])
		{
			re=re<<args[2];
			
			x=(char)(ref[args[0]] << (8-args[1]));
			x >>= (8-args[2]);
			re |=x;
			
			if(args[1]==args[2])
			{
				args[1]=8;
				args[0]++;
			}
			else
			{
				args[1]-=args[2];
			}
			args[2]=0;
		}
		else
		{
			re= re<<args[1];
			
			x=(char)(ref[args[0]] << (8-args[1]) );
			x >>= (8-args[1]);
			re |= x;	
			
			args[2] -= args[1];
			args[1]=8;
			args[0]++;
		}
	}
//	printf("re=%ld\n",re);
	return re;
}


/***************************************************************************************************************/
void check( trie* t )
{
	if(t->mptr >0 || t->filled_pos>0)
	{
		wc += (countval(t->mptr));
		nnodes++;
		
		mlist *temp=t->list;
		
		int i=0;
		while(i<26)
		{
			if( (t->filled_pos & (1<<i) ) !=0)
			{
				check(t->alpha[i]);
			}
			i++;
		}
	}
}
/***************************************************************************************************************/

void intrie( trie *t , char *st )
{
	char c=(*st)-97;
	if( *(st+1)!='\0')
	{
		if( ( t->filled_pos & (1<<c) )  !=0 )
		{
			printf("%c pres\n",*st);
			st++;
			intrie(t->alpha[ c ] ,st);
		}
		else
		{
			printf("Not present!!\n");
		}
	}
	else
	{
		if( ( t->mptr & (1<<c ) ) !=0 )
		{
			mlist * temp =t->list;

			while( temp->ch != (c ) )
			{
				temp=temp->next;
			}

			printf("Present\n");
		}
		else
		{
			printf("Meaning absent\n");
		}
	}
}

