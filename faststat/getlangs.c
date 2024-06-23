#include <stdlib.h>
#include <libpq-fe.h>

#include "stater.h"

CL cl[512];		// country-lang pairs
int cls;		// number of country-lang pairs

CL lang[512];	// languages
int langs;

CL coun[512];	// countries
int couns;

uint64 totsecs;
int players;

int clcompare(const void *a, const void *b)	{

	return (int)((CL *)b)->pl - ((CL *)a)->pl;

}

void updatelist(CL *l, int cl, int *cnt)	{

	int p;
	for(p = 0; l[p].cl; p++)
		if(l[p].cl == cl)
			break;

	if(l[p].cl)
		l[p].pl++;
	else	{
		l[p].cl = cl;
		l[p].pl = 0;
		(*cnt)++;
		qsort(l, p, sizeof(CL), clcompare);
	}

}


void setlang(uint64 xuid, int cid, int lid)	{

	int p;

	for(p = xuids->root; p != -1;)	{
		uint64 x = ((XUID *)ftree_get(xuids, p))->xuid & 0xffffffffffff;
		if(x > xuid)
			p = ftree_get(xuids, p)->l;
		else if(x < xuid)
			p = ftree_get(xuids, p)->r;
		else
			break;
	}

	if( p == -1 )		// no xuid
		return;

	((XUID *)ftree_get(xuids, p))->xuid |= (uint64)cid << 48;
	((XUID *)ftree_get(xuids, p))->xuid |= (uint64)lid << 56;

	int c = cid | (lid << 8);

	updatelist(cl, c, &cls);
	updatelist(lang, lid, &langs);
	updatelist(coun, cid, &couns);

}

void readlangs()	{

	int r;
	
	union llichar ll;
	union intchar i;

	uint64 xuid = 0;
	int cid = 0, lid = 0;

	char *buf;

	printf("Reading country/language info\n");

	r = PQsendQueryParams(conn, "select xuid,countryid,langid from gamers where countryid is not null",
			0,
			NULL,
			NULL,
			NULL,
			NULL,
			1);			// binary format

	if( ! r )	{

		printf("Error '%s' reading langs/countries tab\n", PQerrorMessage(conn));
		exit(1);

	}

	r = PQsetSingleRowMode(conn);

	if ( ! r )	{

		printf("Error '%s' setting SingleRowMode\n", PQerrorMessage(conn));
		exit(1);

	}

	while( (res = PQgetResult(conn)) != NULL)	{

		ExecStatusType rs;

		rs = PQresultStatus(res);
		if( rs != PGRES_SINGLE_TUPLE)	{
			PQclear(res);
			continue;
		}

		buf = PQgetvalue(res, 0, 0);		// xuid
		ll.c[0] = buf[7];
		ll.c[1] = buf[6];
		ll.c[2] = buf[5];
		ll.c[3] = buf[4];
		ll.c[4] = buf[3];
		ll.c[5] = buf[2];
		// ll.c[6] = buf[1];				// 0x09
		// ll.c[7] = buf[0];				// 0x00
		xuid = ll.i;

		buf = PQgetvalue(res, 0, 1);		// countryid
		i.c[0] = buf[1];
		i.c[1] = buf[0];
		cid = i.i;

		buf = PQgetvalue(res, 0, 2);		// langid
		i.c[0] = buf[1];
		i.c[1] = buf[0];
		lid = i.i;

		setlang(xuid, cid, lid);

		PQclear(res);

	}

	// for(int i=0; i != cls; i++)
	//	printf("  %04x %7d\n", cl[i].cl, cl[i].pl);

	printf("Well done cls=%d\n",cls);

}


