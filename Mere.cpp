#include "Outils.h"
#include "Heure.h"
#include <unistd.h>

int main(void) {

	// VT220 Si SSH
	// XTERM par défaut
	InitialiserApplication (VT220);
	sleep(10);
	TerminerApplication(true);

	return 0;
	

}
