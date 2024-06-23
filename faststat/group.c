#include <stdlib.h>
#include <stddef.h>
#include <libpq-fe.h>

#include "stater.h"

void init_titdata( int p, int off, CL *cl, int cnt )
{

	uchar *t = ((uchar *)ftree_get(titleids, p));
	TITDATA **td = (TITDATA **)(t + off);
	(*td) = (TITDATA *)calloc(cnt+1, sizeof(TITDATA));

	// setting up country/lang list
	for(int i = 0; i != cnt; i++)
		(*td)[i].cl = cl[i].cl;

	// printf("p=%d off=%d cnt=%d l=%d r=%d\n", p, off, cnt, ftree_get(titleids, p)->l, ftree_get(titleids, p)->r);
	if(ftree_get(titleids, p)->l != -1)
		init_titdata(ftree_get(titleids, p)->l, off, cl, cnt);
	if(ftree_get(titleids, p)->r != -1)
		init_titdata(ftree_get(titleids, p)->r, off, cl, cnt);

}

void updatestat(TITDATA *t, int cl, uint64 secs)	{

	while(t->cl != cl)
		t++;

	t->secs += secs;
	t->players++;

}

void updatecl(CL *t, int cl, uint64 secs)	{

	while(t->cl != cl)
		t++;

	t->secs += secs;
	t->pl++;

}

void process_xuid(uint32 titleid, uint64 secs, uint16 clx)	{

	int i;

	for(i = titleids->root; i != -1;)	{
		uint32 t = ((TITLEID *)ftree_get(titleids, i))->titleid;
		if(titleid > t)
			i = ftree_get(titleids, i)->r;
		else if(titleid < t)
			i = ftree_get(titleids, i)->l;
		else
			break;
	}

	if(i == -1)	{
		printf("Fatal error: unable to find titleid=%d\n", titleid);
		exit(1);
	}

	updatestat((TITDATA *)(((TITLEID *)ftree_get(titleids, i))->tdcl), clx, secs);
	updatestat((TITDATA *)(((TITLEID *)ftree_get(titleids, i))->tdc), clx & 0xff, secs);		// country
	updatestat((TITDATA *)(((TITLEID *)ftree_get(titleids, i))->tdl), clx >> 8, secs);			// lang

	((TITLEID *)ftree_get(titleids, i))->players++; 
	((TITLEID *)ftree_get(titleids, i))->secs += secs; 

	updatecl(cl, clx, secs);
	updatecl(lang, clx >> 8, secs);
	updatecl(coun, clx & 0xff, secs);

	players++;
	totsecs += secs;

}

// scan every xuid recursively
void scan_xuid(int p)	{

	int i;
	TITINFO *pl = (TITINFO *)(xuids->a + offsetof(XUID, pl) + (uint64)p * xuids->so);
	uint16 clx = ((XUID *)ftree_get(xuids,p))->xuid >> 48;

	for(i = 0; i != MAXTITLES; i++)
		if(pl->titleid[i] == 1)		{
			pl = (TITINFO *)(pl->secs[i]);
			i = 0;
		} else if(pl->titleid[i] == 0)
			break;
		else
			process_xuid(pl->titleid[i], pl->secs[i], clx);

	if(ftree_get(xuids, p)->l != -1)
		scan_xuid(ftree_get(xuids, p)->l);
	if(ftree_get(xuids, p)->r != -1)
		scan_xuid(ftree_get(xuids, p)->r);

}

void grouptitles()	{

	printf("Grouping\n");
	// ftree_dump(titleids, titleid2str);
	
	// init TITLEID TITDATA
	printf("Country-lang pairs: %d, countries: %d, langs: %d\n", cls, couns, langs);
	init_titdata( titleids->root, offsetof(TITLEID, tdcl), cl, cls );
	init_titdata( titleids->root, offsetof(TITLEID, tdc), coun, couns );
	init_titdata( titleids->root, offsetof(TITLEID, tdl), lang, langs );

	printf("Titleid prepared, scanning xuids tree\n");

	scan_xuid( xuids->root );

}

