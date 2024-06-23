#define uint64 unsigned long long
#define uint32 unsigned int
#define uint16 unsigned short int
#define uchar  unsigned char

#define MAXTITLES (12)

#pragma pack (push, 1)

extern void process();

extern PGconn* conn;
extern PGresult* res;

// ftree: definitions
typedef struct {
	int l, r;
	// payload of unknown size
} ftree_el;

typedef struct {
	uchar *a;		// pointer to array of elements ftree_el
					// uchar because we don't know actual size of ftree_el
	int N;			// max number of elements (size of array)
	int fp;			// free index pointer, rebalance if fp == N
	int root;		// index of root element
	int so;			// sizeof(ftree_el + payload)
	int maxdepth;	// maximum depth
	int (*comp)(ftree_el *a, ftree_el *b);	// compare function
} ftree;

extern ftree *ftree_init(int N, int so, int (*comp)());
extern int ftree_upsert(ftree *f, ftree_el *el);
extern void ftree_dump(ftree *f, char *strfunc(int));
extern void ftree_free(ftree *f);

#define ftree_get(f,p) ((ftree_el *) (f->a + (uint64)p * f->so))

extern char *xuids2str(int);
extern char *titleid2str(int);

extern void grouptitles(void);

extern ftree *xuids;
extern ftree *titleids;

union intchar {
	int i;
	char c[4];
};

union llichar {
	uint64 i;
	char c[8];
};

// copy.c
typedef struct {
	uint64 xuid;
	uint32 titleid;
	int utime;
	int secs;
} LINE;

// process.c
typedef struct {
    uint64 secs[MAXTITLES];
    uint32 titleid[MAXTITLES];
} TITINFO;

typedef struct {
    ftree_el ftr;
    uint64 xuid;
    TITINFO pl;         // payload
} XUID;

typedef struct {
	uint64 secs;
	uint32 players;
	uint16 cl;
} TITDATA;

typedef struct {
    ftree_el ftr;
    uint32 titleid;     // ftree payload
	uint32 players;
	uint64 secs;
	TITDATA *tdcl;		// country-lang
	TITDATA *tdc;		// country
	TITDATA *tdl;		// lang
} TITLEID;

typedef struct {
	uint64 secs;
	int pl;
	uint16 cl;
} CL;		// country-lang

extern int cls;			// number of country-lang pairs
extern CL cl[];			// sorted pairs of country-lang

extern int langs;		// number of langs
extern CL lang[];

extern int couns;		// number of countries
extern CL coun[];

extern uint64 totsecs;
extern int players;

void dump_tree();

LINE * decodestr(uchar **buf, int offset);

void readlangs();
void report();

#pragma pack (pop)
