#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

#include "stater.h"

static void rebalance(ftree *f);

static char *(*el2str)(int p);

char *xuids2str(int p)	{

	static char buf[64];
	sprintf(buf, "%llu", ((XUID *)ftree_get(xuids, p))->xuid);	// + 0x9000000000000);
	return buf;

}

char *titleid2str(int p)	{

	static char buf[32];
	sprintf(buf, "%u", ((TITLEID *)ftree_get(titleids, p))->titleid);
	return buf;

}

static void subdump(ftree *f, int p)	{

	ftree_el *el = (ftree_el *) (f->a + p * f->so);
	printf("    p=%2d l=%2d r=%2d el=%s\n", p, el->l, el->r, el2str(p));
	if( el->l != -1 )
		subdump(f, el->l);
	if( el->r != -1 )
		subdump(f, el->r);

}

void ftree_dump(ftree *f, char *strfunc())	{

	el2str = strfunc;
	printf("  tree dump root=%d\n", f->root);
	subdump(f, f->root);

}

ftree *ftree_init(int N, int so, int (*comp)())	{

	ftree *f = calloc(1, sizeof(ftree));
	f->N = N;
	f->so = so;
	f->comp = comp;
	f->root = -1;			// no elements yet
	f->a = calloc(N, so);
	return f;

}

int ftree_upsert(ftree *f, ftree_el *el)	{

	el->l = el->r = -1;		// no children of a new el

	if(f->root == -1)	{

		f->root = 0;			// this is the first element
		f->fp++;
		memcpy(f->a, el, f->so);
		f->maxdepth = 1;

		return 0;

	}

	int p, np, md = 0;
	for(p = np = f->root; p != -1; md++ )	{
		if( f->comp(ftree_get(f, p),el) == 0)
			break;
		np = p;
		if( f->comp(el, ftree_get(f, p)) > 0)
			p = ftree_get(f, p)->r;
		else
			p = ftree_get(f, p)->l;
	}

	if(md > f->maxdepth)
		f->maxdepth = md;

	if(p == -1)		{

		p = f->fp++;						// next element in tree
		ftree_el *pp = ftree_get(f,p);		// this element
		ftree_el *npp = ftree_get(f, np);	// link to parent el

		memcpy(pp, el, f->so);			// copy new node to array

		if( f->comp(el, npp) > 0)
			npp->r = p;
		else
			npp->l = p;

	}

	if(f->fp == f->N)
		rebalance(f);

	return p;

}

static int *sorted;
static int sp;			// sorted pointer

// extract to sorted from xuids tree
static void mksort(ftree *f, int i)	{

	if(ftree_get(f, i)->l != -1)
		mksort(f, ftree_get(f, i)->l);

	sorted[sp++] = i;

	if(ftree_get(f, i)->r != -1)
		mksort(f, ftree_get(f, i)->r);

}


// create rebalanced tree from sorted array
static void merge(ftree *f)	{

	int i, st, l, r;

	for(i=0; i < f->fp; i += 2)
		ftree_get(f, sorted[i])->l = ftree_get(f, sorted[i])->r = -1;

	for(st = 4; st/2 <= f->fp; st *= 2) {
		for(i = st/2 - 1; i < f->fp; i += st)   {
			
			l = i - st/4;
			r = i + st/4;

			ftree_get(f, sorted[i])->l = sorted[l];
			ftree_get(f, sorted[i])->r = (r >= f->fp) ? -1 : sorted[r];

			// printf("    %d ===> (%d) - (%d)     sorted: %d ===> (%d) - (%d)\n", i, l, r, sorted[i], sorted[l], sorted[r]);

		}

	}

	f->root = sorted[st/4 - 1];

}

static void rebalance(ftree *f)	{

	sorted = calloc(f->N, sizeof(int));

	printf("Rebalancing with maxdepth=%d  N=%d\n", f->maxdepth, f->N);

	sp = 0;
	mksort(f, f->root);

	merge(f);

	free(sorted);

	f->N = f->N * 2 + 1;

	f->a = reallocarray(f->a, f->N, f->so);
	if(f->a == NULL)	{

		printf("Out of memory\n");
		exit(1);

	}

	f->maxdepth = 1;
	printf("Rebalancing ended to N=%d\n", f->N);

}

