#include"functions.c"

long int search (unsigned char ref[], int ptr[], unsigned int args[], char *w)
{
	unsigned char val[4],ch;
	long int re=-1;

	ch=*w;
	//printf("%c\n",ch);
	val[1] = gind(ch);		//char to be searched
	val[0] = 0;			//loop ctrl

	args[1]=5; 			//left bits

	if ((*(w + 1) != '\0'))
	{
		args[2]=ptr[0];

		switch ( (ref[args[0]] >> 6) )
		{
			case 0: 	//only meaning 0,1
				break;

			case 1: //2,3
				if (((ref[args[0]] >> 5) & 1) == 0)	//bit rep
				{
					while (val[0] < val[1])
					{
						adjust(args);

						if ((ref[args[0]] & (1 << (--args[1]))) != 0)
						{
							jump(args);
						}

						val[0]++;
						args[2]=ptr[0];
					}

					adjust(args);

					if ((ref[args[0]] & (1 << (--args[1]))) != 0)	//next pointer present
					{
						args[2]=ptr[0];
						args[0]=getptr(ref,args);
						w=w+1;
						re=search( ref,ptr,args,w );
					}
				}

				else							//char representation
				{
					args[2]=5;
					val[3]=getptr(ref,args) ; //get no of entries

					args[2]=5;	 	  //reset ptr length

					while( (( ch=getptr(ref,args) ) < val[1])  && (val[0] < val[3]))
					{
						args[2]=ptr[0];
						jump (args);
						args[2]=5;
						val[0]++;
					}

					//printf("%c\n",ch);

					if(ch==val[1])
					{
						args[2]=ptr[0];
						args[0]=getptr(ref,args);
						w=w+1;
						re= search( ref,ptr,args,w );
					}
				}
				break;

			case 2 : //4,5
				if ( ((ref[args[0]] >> 5) & 1) == 1 )	//bitmap rep			
				{
					//printf("start left %d\n",args[1]);
					while(val[0]<val[1])
					{
						adjust(args);

						if ((ref[args[0]] & (1 << (--args[1]))) != 0) //next bit is set
						{
							//printf("npres %c %d\n",val[0]+97,args[1]);
							args[2]=ptr[0];
							jump(args);	
						}

						adjust(args);

						if ((ref[args[0]] & (1 << (--args[1]))) != 0) //meaning bit is set
						{
							//printf("mpres %c %d\n",val[0]+97,args[1]);
							args[2]=ptr[1];
							jump(args);
						}
						val[0]++;
					}

					adjust(args);

					if ((ref[args[0]] & (1 << (--args[1]))) != 0) //next bit is set
					{
						args[2]=ptr[0];
						args[0]=getptr(ref,args);

						w=w+1;
						re=search( ref,ptr,args,w );
					}
				}

				else //word definitely present
				{
					args[2]= val[1] * ( ptr[0] + ptr[1] );
					jump(args);

					args[2]=ptr[0];
					args[0]=getptr(ref,args);

					w=w+1;
					re=search( ref,ptr,args,w );
				}
				break;
		}
	}
	else	//search for meaning  
	{
		//printf("mea\n");
		args[2]=ptr[1];

		switch( (ref[args[0]]>>6) )
		{
			case 0 : // only meaning
				if( ( ( ref[args[0]]>>5 )&1) == 0 ) //bitmap rep
				{
					while(val[0]<val[1])
					{
						adjust(args);

						if( ( ref[args[0]] & (1 << (--args[1]) ) ) != 0 ) //bit set
						{
							jump(args);
						}

						args[2]=ptr[1];
						val[0]++;
					}

					adjust(args);

					if( ( ref[args[0]] & (1 << (--args[1]) ) ) != 0 )  //meaning present
					{
						re = getptr(ref,args);
					}
				}
				else //char representation
				{
					//printf("here\n");
					args[2]=5;
					val[3]=getptr(ref,args);
					//printf("val3 %d\n",val[3]);
					args[2]=5;
					while(( ( ch=getptr(ref,args) ) < val[1]) && (val[0] < val[3]))
					{
						args[2]=ptr[1];
						jump(args);

						args[2]=5;
						val[0]++;
					}
					//printf("%c\n",ch);

					if(ch==val[1])
					{
						args[2]=ptr[1];
						re = getptr(ref,args);
					}
				}
				break;			 

			case 1 : //only nexts
				break;

			case 2 : if( ( ( ref[args[0]]>>5 )&1) != 0 ) //partially filled
				 {
					 while(val[0]<val[1])
					 {
						 adjust(args);
						 if ((ref[args[0]] & (1 << (--args[1]))) != 0) //next bit is set
						 {
							 args[2]=ptr[0];
							 jump(args);
						 }

						 adjust(args);

						 if ((ref[args[0]] & (1 << (--args[1]))) != 0) //meaning bit is set
						 {
							 args[2]=ptr[1];
							 jump(args);
						 }

						 val[0]++;
					 }

					 adjust(args);

					 if ((ref[args[0]] & (1 << (--args[1]))) != 0) //next bit is set
					 {
						 args[2]=ptr[0];
						 jump(args);
					 }

					 if ((ref[args[0]] & (1 << (--args[1]))) != 0) //next bit is set
					 {
						 args[2]=ptr[1];
						 re=getptr(ref,args);
					 }
				 }
				 else
				 {
					 args[2]= val[1] * ( ptr[0] + ptr[1] );
					 args[2]+=ptr[0];

					 jump(args);
					 args[2]=ptr[1];
					 re=getptr(ref,args);
					 //re=search( ref,ptr,args,(w++) );
				 }
				 break;
		}
	}

	return re;
}

int main(int argc , char* file[])
{
	unsigned int fsize,ptr[2],args[4];

	scanf("%d",&fsize);
	scanf("%d",&ptr[0]);
	scanf("%d",&ptr[1]);

//	char * word=file[3];

	args[0]=0;

	FILE *fpm=fopen(file[2],"r");
	if(fpm==NULL)
	{
		printf("Couldnt open meaning file \n");
		return EXIT_FAILURE;
	}
	
	FILE* fp = fopen(file[1],"rb");
	if(fp==NULL)
	{
		printf("Couldnt open refdata file \n");
		return EXIT_FAILURE;
	}

	int sockfd,tempfd;
	struct sockaddr_in server;

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("Socket syscall f\n");
		return 1;
	}

	

	unsigned char *refdata=(unsigned char*) calloc(sizeof(unsigned char),fsize);

	char m,word[47];

	int r =fread((void*)refdata,sizeof(unsigned char),fsize,fp);

	if(r<=0)
	{
		printf("reading failure");
		return EXIT_FAILURE;
	}

	unsigned long int loc;
	


	loc= search(refdata,ptr,args,word);

	if(loc==-1)
	{
		printf("Word not in Dictionary !!!\n");
		return EXIT_FAILURE;
	}

	fseek(fpm,(loc),SEEK_SET);

	printf("MEANING : ");

	m;

	while( (m=fgetc(fpm) ) != '\n')
	{
		printf("%c",m);
	}
	printf("\n");
	return 0;

}
