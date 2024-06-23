#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

#include "stater.h"

PGconn* conn = NULL;
PGresult* res = NULL;

int main(int argc, char *argv[])      {

	conn = PQconnectdb("dbname=global");
	
	if(PQstatus(conn) != CONNECTION_OK)     {

		printf("%s\n", PQerrorMessage(conn));
		return 1;

	}

	printf("   Processing\n");
	process();

	readlangs();

	grouptitles();

	report();

	PQfinish(conn);

	return 0;

}
