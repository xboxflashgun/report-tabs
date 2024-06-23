#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <libpq-fe.h>

#include "stater.h"

#define TITPOOLSIZE (1000)

static int xuidcomp(XUID *a, XUID *b)	{

	long long diff = (long long int)(a->xuid & 0xffffffffffff) - (long long int)(b->xuid & 0xffffffffffff);
	return (diff < 0 ) ? -1: (diff > 0) ? 1 : 0;

}

static int titleidcomp(TITLEID *a, TITLEID *b)	{

	long long diff = (long long)(a->titleid) - (long long)(b->titleid);
	return (diff < 0 ) ? -1: (diff > 0) ? 1 : 0;

}

ftree *xuids;
ftree *titleids;

static TITINFO *tpool;		// link to allocated pool
static int tpoolcnt;

static int cnt1, cnt2, cntp;

void process_line(uint64 xuid, uint32 titleid, int utime, int secs)	{

	int ut = utime, lt = utime + secs;
	int i, p;
	static XUID el;
	static TITLEID t;

	// counting different titleids
	memset(&t, 0, sizeof(t));
	t.titleid = titleid;
	ftree_upsert(titleids, (ftree_el *)&t);

	memset(&el, 0, sizeof(el));
	el.xuid = xuid;
	p = ftree_upsert(xuids, (ftree_el *)&el);

	// p -- index of xuid found
	TITINFO *pl = (TITINFO *)(xuids->a + offsetof(XUID, pl) + (uint64)p * xuids->so);
	
	for(i = 0; i != MAXTITLES; i++)
		if(pl->titleid[i] == titleid)
			break;
		else if(pl->titleid[i] == 0)
			break;
		else if(pl->titleid[i] == 1)	{
			// pointer to new payload table
			// printf("p=%d i=%d titleid=%d pl->titleid[i]=%d pl=%p secs=%llx\n", p, i, titleid, pl->titleid[i], pl, pl->secs[i]);
			pl = (TITINFO *)(pl->secs[i]);
			i = 0;
		}

	if(i == MAXTITLES)	{

		// do wee need to allocate new pool?
		if(tpoolcnt-- == 0)	{

			tpool = calloc(TITPOOLSIZE+1, sizeof(TITINFO));
			tpoolcnt = TITPOOLSIZE;
			cntp++;
			// printf("   allocated new pool %d: added %d out of %d\n", cntp, cnt2, cnt1);

		}

		// save prev info to new structue and set up t[MAXTITLES-1] to 1
		tpool->titleid[0] = pl->titleid[MAXTITLES-1];
		tpool->secs[0] = pl->secs[MAXTITLES-1];
		pl->titleid[MAXTITLES-1] = 1;
		pl->secs[MAXTITLES-1] = (uint64) tpool;

		i = 1;		// because we just wrote to [0]
		pl = tpool++;
		cnt2++;
	
	}

	pl->titleid[i] = titleid;
	pl->secs[i] += secs;

	cnt1++;

}


void process()	{

	PGresult* res;

	char req[256];
	static uchar *p;
	uchar **buffer = &p;
	LINE *line;

	sprintf(req, "copy (select * from presence) to STDOUT (format BINARY)");
	
	res = PQexec(conn, req);
	if(PQresultStatus(res) != PGRES_COPY_OUT)  {

		printf("error: %s\n", PQerrorMessage(conn));
		exit(1);

	}

	int r;
	r = PQgetCopyData( conn, (char **) buffer, 0 );	//  skip header

	// first row goes after header
	union intchar len;
	len.c[0] = (*buffer)[19];
	len.c[1] = (*buffer)[18];
	len.c[2] = (*buffer)[17];
	len.c[3] = (*buffer)[16];

	line = decodestr(buffer, 19 + len.i);

	PQfreemem(*buffer);

	xuids = ftree_init(65535, sizeof(XUID), xuidcomp);
	titleids = ftree_init(8191, sizeof(TITLEID), titleidcomp);

	process_line(line->xuid, line->titleid, line->utime, line->secs);

	while( PQgetCopyData(conn, (char **) buffer, 0) >= 0 )	{

		line = decodestr(buffer, 0);
		PQfreemem((char *) *buffer);

		if(line == NULL)		// skip NULL secs
			continue;

		process_line(line->xuid, line->titleid, line->utime, line->secs);

	}

	res = PQgetResult(conn);
	printf("COPY result status: %d\n", PQresultStatus(res));
	res = PQgetResult(conn);
	printf("COPY result status: %d\n", PQresultStatus(res));

	printf("Total xuids: %d, titleids: %d\n", xuids->fp, titleids->fp);
	// ftree_dump(xuids, xuids2str);

}


