#ifndef _error_h
#define _error_h

#include <cstdio> // freopen, fprintf, stderr
#include <cstdlib> // exit, EXIT_FAILURE
#include <cstdarg> // vargs macros


// reports msg to error stream and exits program
// utilizes variable arguments (vargs)
void error(char *file, int line, char *msg, ...)
{
	va_list argp;
	va_start(argp, msg);

	FILE *log;
	fopen_s(&log, "error.txt", "a");
	fprintf(log, "| %s | %s | ", __DATE__, __TIME__);
	fprintf(log, "%s %d | ", file, line);
	vfprintf(log, msg, argp);
	fprintf(log, "\n");

	fclose(log);
	va_end(argp);
	exit(EXIT_FAILURE);
}

#endif
