// #include <arpa/inet.h>
#include <stdint.h>
#include <endian.h>
#include <libpq-fe.h>

#include "stater.h"

#pragma pack (push, 1)
typedef union {
	LINE l;
	uchar c[sizeof(LINE)];
} uline;

static uline line;

#pragma pack (pop)

	/*
	// dump buffer
	for(int i=0; i!=res; i++)	{
		if(i % 10 == 0)
			printf("\n%04d ", i);
		printf("%02x ", *(unsigned char *)((*buffer)+i));
	}
	printf("\n\n");		*/

LINE * decodestr(uchar **buffer, int off)	{

	union intchar num;
	num.i = 0;

	num.c[0] = (*buffer)[1 + off];
	num.c[1] = (*buffer)[0 + off];

	if(num.i == 0xffff)
		return NULL;

	// xuid
	line.l.xuid = 0;

	line.c[0] = (*buffer)[13 + off];
	line.c[1] = (*buffer)[12 + off];
	line.c[2] = (*buffer)[11 + off];
	line.c[3] = (*buffer)[10 + off];
	line.c[4] = (*buffer)[9 + off];
	line.c[5] = (*buffer)[8 + off];
	// line.c[6] = (*buffer)[7 + off];		// reserved for countryid
	// line.c[7] = (*buffer)[6 + off];		// reserved for langid

	// titleid
	line.c[8]  = (*buffer)[25 + off];
	line.c[9]  = (*buffer)[24 + off];
	line.c[10] = (*buffer)[23 + off];
	line.c[11] = (*buffer)[22 + off];

	// utime
	line.c[12] = (*buffer)[33 + off];
	line.c[13] = (*buffer)[32 + off];
	line.c[14] = (*buffer)[31 + off];
	line.c[15] = (*buffer)[30 + off];

	// secs
	
	num.i = 0;	// secs length
	num.c[0] = (*buffer)[37 + off];
	num.c[1] = (*buffer)[36 + off];
	num.c[2] = (*buffer)[35 + off];
	num.c[3] = (*buffer)[34 + off];

	if(num.i < 0)		// still playing
		line.l.secs = 600;
	else {

		line.c[16] = (*buffer)[41 + off];
		line.c[17] = (*buffer)[40 + off];
		line.c[18] = (*buffer)[39 + off];
		line.c[19] = (*buffer)[38 + off];
	}

	return &line.l;

}

