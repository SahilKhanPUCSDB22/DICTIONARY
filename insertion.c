/****************************************************************************************************************/
mlist * insmem (mlist * node, unsigned long int mptr, char c)
{
	mlist *temp=node;
	if (temp == NULL)
	{
		temp = (mlist *) malloc (sizeof (mlist));
		node=temp;
	}
	else
        {
      		while (temp->next != NULL)
      		{
      			temp = temp->next;
      		}
		temp->next = (mlist *) malloc (sizeof (mlist));
		temp = temp->next;
	}

	temp->next=NULL;
	temp->ch = ind (c);
	temp->addr = mptr;
	return node;
}
/***************************************************************************************************************/
void insert (trie * t, char *c, unsigned int mpointer, char *word)
{
	char ch = ind (*c);

	if (*(c + 1) != '\0')
	{
		if (t->filled_pos == 0)	//for checking whether level was not visited
		{
			t->alpha = (trie **) malloc (sizeof (trie *) * 26);
		}
		if (((t->filled_pos) & (1 << ch)) == 0)	//for checking whether the alpha in current level was not visited
		{
			t->alpha[ch] = (trie *) malloc (sizeof (trie));
			t->alpha[ch]->filled_pos = 0;
			t->alpha[ch]->mptr = 0;
			t->alpha[ch]->list = NULL;
			t->filled_pos |= (1 << ch);	//put a bit marker for visited alphabets in the level
		}
		c++;
		return insert (t->alpha[ch], c, mpointer, word);
	}
	else
	{
		if ((t->mptr & (1 << ch)) == 0)
		{
			t->list = insmem ( t->list, mpointer, (*c));
			t->mptr |= (1 << ch);
		}
	}

}
/**************************************************************************************************************/
int validword (char *w)		// for allowing only non-special char containing strings into out trie
{
	char i = 0;
	while ((*w) != '\0' && i == 0)
	{
		if ((*w) < 65 || (*w) > 90 && (*w) < 97 || (*w) > 122)	//not a valid character is encountered
		{
			i = 1;		//stop looking further
		}
		w++;
	}
	return i;
}
/***************************************************************************************************************/
trie * gen_word (FILE * fp, FILE * mp)
{
	trie *t = (trie *) malloc (sizeof (trie));	// creating our top level 
	t->filled_pos = 0;
	t->mptr = 0;
	t->list == NULL;

	char *word = (char *) malloc (sizeof (char) * size);	//to read the file word by word
	char ch, flag = 0, i = 0;

	unsigned long mptr = 0;

	while ((ch = fgetc (fp)) != EOF)
	{
		if (ch == ' ' || ch == '\n' || ch == '\t')
		{
			if (flag == 1)	//word creation finished
			{
				*(word + i) = '\0';
				if (validword (word) == 0)	//validate the word
				{
				//	printf("%s %ld\n",word,mptr);
					insert (t, word, mptr, word);	//insert the word
					//printf("%d\n",t->list->addr);
				}
				while (fgetc (mp) != '\n')
				{
					mptr++;
				}
				mptr++;
				i = 0;
				flag = 0;		//reset to begin again for next word
			}
		}
		else
		{
			*(word + (i++)) = ch;
			flag = 1;		//word is being created
		}
	}

	return t;
}
