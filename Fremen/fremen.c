#include "fremen.h"

int main(int argc,char *argv[]) {
	if (argc < 2) return 1;
	return executeProgram(argv[1], &argv[1]);
}
