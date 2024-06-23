#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

#include "stater.h"

FILE *csv;

void scan_titleids(int p)	{

	uint32 titleid = ((TITLEID *)ftree_get(titleids, p))->titleid;
	TITDATA *t = (TITDATA *)(((TITLEID *)ftree_get(titleids, p))->tdcl);  // titleid data

	while(t->cl)	{
		if(t->players)
			fprintf(csv, "%u,%d,%d,%d,%lld\n", titleid, t->cl & 0xff, t->cl >> 8, t->players, t->secs);
		t++;
	}

	t = (TITDATA *)(((TITLEID *)ftree_get(titleids, p))->tdc);
	while(t->cl)	{
		if(t->players)
			fprintf(csv, "%u,%d,%d,%d,%lld\n", titleid, t->cl, 0, t->players, t->secs);
		t++;
	}

	t = (TITDATA *)(((TITLEID *)ftree_get(titleids, p))->tdl);
	while(t->cl)	{
		if(t->players)
			fprintf(csv, "%u,%d,%d,%d,%lld\n", titleid, 0, t->cl, t->players, t->secs);
		t++;
	}

	TITLEID *ti = ((TITLEID *)ftree_get(titleids, p));
	fprintf(csv, "%u,%d,%d,%d,%lld\n", titleid, 0, 0, ti->players, ti->secs);


	if(ftree_get(titleids, p)->l != -1)
		scan_titleids(ftree_get(titleids, p)->l);
	if(ftree_get(titleids, p)->r != -1)
		scan_titleids(ftree_get(titleids, p)->r);

}

void report()	{

	int i;

	printf("Writing report\n");

	csv = fopen("stattab.csv", "w");
	// csv = fopen("/dev/stdout", "a+");
	if( csv == NULL )	{

		printf("Error creating output CSV\n");
		exit(1);

	}

	scan_titleids( titleids->root );

	for(i = 0; i != cls; i++)
		fprintf(csv, "%u,%d,%d,%d,%lld\n", 0, cl[i].cl & 0xff, cl[i].cl >> 8, cl[i].pl, cl[i].secs);

	for(i = 0; i != couns; i++)
		fprintf(csv, "%u,%d,%d,%d,%lld\n", 0, coun[i].cl, 0, coun[i].pl, coun[i].secs);

	for(i = 0; i != langs; i++)
		fprintf(csv, "%u,%d,%d,%d,%lld\n", 0, 0, lang[i].cl, lang[i].pl, lang[i].secs);

	fprintf(csv, "%u,%d,%d,%d,%lld\n", 0, 0, 0, xuids->fp, totsecs);

	printf("End of report\n");

}

