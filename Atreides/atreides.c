#include "atreides.h"

void ctrlCHandler() {

}

int main(int argc, char *argv[]) {
	signal(SIGINT, ctrlCHandler);

	return 0;
}
