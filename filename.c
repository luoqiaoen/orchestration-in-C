#include <Windows.h>
#include "ansi_c.h"
#include "filename.h"  
#include "RadialScan.h"



int smSetId; //global variables are initialized to zero

int GetSetId(void) {
	return smSetId;
}

void IncrementSetId(void) {

	smSetId++;

}
char fnBaseDirectory[512];

void SetFilenameBase(char * base) {

	// no length checking, bad...
	sprintf(fnBaseDirectory, "%s", base);
}


// return a filename based on the current set, and stage positions
void GetImageFilename(char * filename) {

	int set = GetSetId();
	int vertical = getVerticalPosition();
	int horizontal = getHorizontalPosition();

	sprintf(filename, "%s\\o_%03d_%06d_%06d", fnBaseDirectory, set, vertical, horizontal);
	
}


