#include "Cronos.h"

int main() {
	for (int i = 0; i < 4; i++) {
        printf("\n[ + ] Sleeping\n");
        CronosSleep(10);
	    printf("[ + ] Code executed!\n");
	}
}