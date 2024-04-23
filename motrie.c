#include"functions.c"
#include"insertion.c"

void createref (trie * t, unsigned char ref[], int ptr[])
{
//	printf("%u type %d\n",ref[0],t->type);
//	printf("ptr nxt %d mea %d \n",ptr[0],ptr[1]);
	unsigned int var[5];
	char avail[2];			//availability of meaning,next

	mlist *temp;

	var[0] = next;			// current array pos
	
	next += ceil( (float)(t->nsize)/8);

	ref[var[0]] |= (t->type << 5);	//put type

//	printf("%u \n",ref[0]);

	var[1] = 5;			//bits left free in current array ind
					//var[2] for holding to be encoded value
					//var[3] to hold size of pointer
	var[4] = 0;			//loop control variable

	if( ( t->type==1 ) || ( t->type==3 ) ) //char rep
	{
		var[2]=(countval(t->mptr)|countval(t->filled_pos));
		
		if(var[2] < 0 || var[2] >26)
		{
			printf("Exceeded number of meanings\n");
		}
		
		var[3]=5;
		enc(ref,var);
	}

	while (var[4] < 26)
	{
		temp=t->list;
		
		avail[0] = (((t->filled_pos) & (1 << var[4])) >> var[4]);
		avail[1] = (((t->mptr) & (1 << var[4])) >> var[4]);

		if (var[1] == 0)
		{
			var[1] = 8; 
			var[0]++;
		}

		switch ((t->type >> 1))
		{

			case 0:								//only meaning
				if ( ( t->type & 1  ) == 0 )		//0,1		//bitmap 
				{
					adjust(var);

					if (avail[1] == 1)				//meaning available
					{
						while(temp->ch != var[4])
						{
							temp=temp->next;
						}

						ref[var[0]] |= (1 << (--var[1]));	//set bit to 1

						var[2] = temp->addr;	//value of meaning pointer      
						var[3] = ptr[1];	//size of meaning pointer
						enc (ref, var);
					}
					else
					{
						var[1]--;
					}

				}
				else			//char
				{
					if (avail[1] == 1)  //meaning available
					{
						//encode char
						while(temp->ch != var[4])
						{
							temp=temp->next;
						}
						
						var[2]=temp->ch;
						var[3] = 5;
						enc (ref, var);

						//encode meaning
						var[2] = temp->addr;
						var[3] = ptr[1];  //meaning ptr size
						enc (ref, var);
					}

				}
				break;

			case 1:							//only next                     
				if ((t->type & 1) == 0)				//bitmap 
				{
					adjust(var);
					
					if (avail[0] == 1)			//next available
					{
						ref[var[0]] |= (1 << (--var[1]));	//set bit to 1
						var[2] = next;			//value of meaning pointer      
						var[3] = ptr[0];		//size of next pointer
						enc (ref, var);
						createref (t->alpha[var[4]], ref, ptr);
					}
					else
					{
						var[1]--;
					}

				}
				else			//char
				{
					if (avail[0] == 1)
					{
						//encode char
						var[2] = var[4];
						var[3] = 5;
						enc (ref, var);

						//encode next
						var[2] = next;
						var[3] = ptr[0];
						enc (ref,var);

						createref (t->alpha[var[4]], ref, ptr);
					}

				}
				break;

			case 2:		//both 4,5
				if ((t->type & 1) == 0)	//fully filled
				{
					//encode next
					var[2] = next;
					var[3] = ptr[0];
					enc (ref, var);
					
					//call for next node
					createref (t->alpha[var[4]], ref, ptr);
					//search for char and address
					while(temp->ch != var[4])
					{
						temp=temp->next;
					}
					
					//encode address
					var[2] = temp->addr;
					var[3] = ptr[1];
					enc (ref, var);
				}

				else							//partially filled
				{
					adjust(var);

					if (avail[0] == 1)				//if next available
					{
						ref[var[0]] |= (1 << (--var[1]));	//set bit to 1
						
						//encode next
						var[2] = next;
						var[3] = ptr[0];
						enc (ref, var);
						
					}
					else
					{
						var[1]--;
					}

					adjust(var);

					if (avail[1] == 1) //0 -> meaning avail
					{
						ref[var[0]] |= (1 << (--var[1]));	//set bit to 1
						
						while( temp->ch != var[4])
						{
							temp=temp->next;
						}

						var[2] = temp->addr;
						var[3] = ptr[1];
						enc (ref, var);
					}
					else
					{
						var[1]--;
					}

					if(avail[0]==1)
					{
						createref (t->alpha[var[4]], ref, ptr);
					}
				}
				break;

			default:
				printf ("Error in type recognition\n");
				break;

		}
		var[4]++;
	}

}

/***************************************************************************************************************/
int main (int argc, char *file[])
{
	int filesize;	//size of meaningfile
	scanf ("%d", &filesize);

	if(filesize <=0 )
	{
		printf("MEANING FILE IS EMPTY !!!\n");
		return EXIT_FAILURE;
	}

	unsigned int args[3];

	int ptr[2];		//for next and meaning pointer

	unsigned char *refdata;	//refdata array

	FILE *fp[2];	//handeling word and meaning file

	long int loc; 


	fp[0] = fopen (file[1], "r");
	if (fp[0] == NULL)
	{
		printf ("%s : BAD ADDRESS !!\n", file[1]);
		return EXIT_FAILURE;
	}

	fp[1] = fopen (file[2], "r");
	if (fp[1] == NULL)
	{
		printf ("%s : BAD ADDRESS !!\n", file[2]);
		return EXIT_FAILURE;
	}

	trie *t = gen_word (fp[0], fp[1]);	//get words from file and build the tree , returns top of the tree

	fclose (fp[0]);
	fclose (fp[1]);



	check(t); //get no of nodes,words

	ptr[0] = findn (nnodes, wc, filesize);	//finding size of our reference data , returns pointer size
	ptr[1] = ceil (log2 (filesize));	//meaning pointer
	assign_type (ptr[0], ptr[1], &t);	//assigns type to nodes and calculates size of refdata

	ptr[0] = ceil (log2 ((tsize / 8) + nnodes));	//optimized next poinnter

	tsize=0;
	
	assign_type(ptr[0],ptr[1],&t);

	ptr[0] = ceil (log2 ((tsize / 8) + nnodes));

	refdata =(unsigned char *) calloc (((tsize / 8) + (nnodes)),sizeof (unsigned char));
	
	FILE * d = fopen("dicnry","w+");
	if(d==NULL)
	{
		printf("Failed to create refdata file\n");
		return EXIT_FAILURE;
	}
	
	createref (t, refdata, ptr);

	if(next >0)
	{
		printf("REFDATA CREATED SUCESSFULLY!!!\n");
	}
	else
	{
		printf("REFDATA CREATION FAILED\n");
		return EXIT_FAILURE;
	}

	fwrite((void*)refdata,sizeof(unsigned char),next,d);	
	
	printf("**STATS**\nPOINTER SIZE %d BITS\nMPTR SIZE %d BITS\nNO OF WORDS %d\nNO OF NODES %d\nNEXT %d\n",ptr[0],ptr[1],wc,nnodes,next);

	fclose(d);

	d=fopen("searchinput","w+");
	
	fprintf(d,"%d\n%d\n%d",next,ptr[0],ptr[1]);	
		
	fclose(d);
	int x=0,z;
/*	while(x<next)
        {       z=0;
		printf("\n%u\n",refdata[x]);
                while(z<8)
                {
                        printf("%d ",((refdata[x] & (1<<z))>>z));
                        z++;
                }
                x++;
        }
*/
	
	return 0;
}
