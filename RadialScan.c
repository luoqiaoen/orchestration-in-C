

#include <windows.h>
#include "ansi_c.h"
#include "master.h"
#include "pvcam.h"
#include "ni4882.h"
#include "CoolSNAP.h"
#include "filename.h"
#include "stage.h"
#include "Vortex.h"
#include "RadialScan.h"
#include "zb_serial.h"
extern int BackgroundSubtractFlag;

/*
* main loop ties everything together
* controls the scanning, image gathering, and file generation
*
*/

void main(void){
	float scanAngle;
	char base_directory[255];
	/*
	char backgroundFilename[255];
	int i;
	char filename[512];
	char image_filename[512];
	*/

	if (BackgroundSubtractFlag) {
		printf("Background subtract flag set, aborting!");
		return;
	}

	if (get_base_directory(base_directory) == -1) {
		return;
	}

	if (!initial_test(base_directory)) {
		return;
	}

	AcquireImage();

	print_time();
	printf("Starting Experiment");
	SaveExperimentDescription(base_directory);
	SaveExperimentDescriptionRadial(base_directory);

	BrianStageReset(1);
	Sleep(5000);
	BrianStageReset(2);
	Sleep(5000);
	BrianStageMoveRelative(1, 640200);
	Sleep(5000);
	BrianStageMoveRelative(2, 640200);
	Sleep(5000);


	//BrianStageMoveRelative(1, -125982);
	//Sleep(2000);


	print_time();
	printf("Vertical stage position %d\n", BrianStageGetPosition(1));
	print_time();
	printf("Horizontal stage position %d\n", BrianStageGetPosition(2));

	getBackgroundImages(1, 2, base_directory);
	enableLaser();


	scanAngle = -M_PI_2;
	while (scanAngle < M_PI_2 - 0.001) {

		print_time();
		printf("Scan Angle %f", scanAngle);

		print_time();
		printf("Getting Line");

		print_time();

		acquireLineAngle(scanAngle, NUM_LINE_SAMPLES);

		print_time();
		printf("Done getting Line");

		print_time();
		printf("Resetting Horizontal Stage");

		BrianStageReset(2);

		print_time();
		printf("Resetting Vertical Stage");

		BrianStageReset(1);


		print_time();
		printf("Incrementing Set ID");

		IncrementSetId();
		scanAngle += M_PI / (NUM_LINES);

	}

	print_time();
	printf("Getting Background Images");


	getBackgroundImages(3, 4, base_directory);

	save_experiment_description_end(base_directory);

	print_time();
	printf("Done");
	CloseBrianStages();

}

/*
* Scans a line at some angle relative to horizontal and saves the images
*
*/

void acquireLineAngle(float angle, int numSamples) {

	int dx, dy;

	int sampleNumber = 0;

	char image_filename[512];

	printf(" ");

	dy = floor(DELTA_RHO * Y_SCALE * sin(angle) + 0.5);
	dx = floor(DELTA_RHO * X_SCALE * cos(angle) + 0.5);

	// move backwards half way
	stage_move_hori(-dx * (numSamples - 1) / 2, 1, 0);
	stage_move_vert(-dy * (numSamples - 1) / 2, 1, 0);
	Sleep(4000);

	for (sampleNumber = 0; sampleNumber < numSamples; sampleNumber++) {

		print_time();

		printf("(%03i)", sampleNumber);
		AcquireImage();

		GetImageFilename(image_filename);

		SaveImage(image_filename);

		stage_move_hori(dx, 1, 0);

		Sleep(STAGE_MOVE_HORI_SLEEP);

		stage_move_vert(dy, 1, 0);

		Sleep(STAGE_MOVE_VERT_SLEEP);

	}

	BrianStageReset(1);
	Sleep(5000);
	BrianStageReset(2);
	Sleep(5000);

}


void SaveExperimentDescriptionRadial(char * save_directory) {
	char filename[512];
	FILE * handle;
	/*
	FILE description[512];
	int month, day, year, hour, minute, second;
	LPSYSTEMTIME time;
	*/
	

	sprintf(filename, "%s\\_setup.txt", save_directory);

	handle = fopen(filename, "a+");

	fprintf(handle, "Number of radial lines: %d\n", NUM_LINES);
	fprintf(handle, "Radial scan step size: %f\n", DELTA_RHO);

	fprintf(handle, "Number of samples per radial line: %d\n", NUM_LINE_SAMPLES);

	fprintf(handle, "X SCALER: %f\n", X_SCALE);
	fprintf(handle, "Y SCALER: %f\n", Y_SCALE);

	fclose(handle);
}

void SaveExperimentDescription(char * save_directory) {
	char filename[512];
	FILE * handle;

	sprintf(filename, "%s\\_setup.txt", save_directory);

	handle = fopen(filename, "a+");

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	fprintf(handle, "Start Date: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(handle, "--------------------------\n");
	fprintf(handle, "Description:\n");
	fprintf(handle, "--------------------------\n");

	fprintf(handle, "Shutter Time: %i\n", GetExposureTime());
	fprintf(handle, "CCD Initial Temp: %f\n", GetCCDCurrentTemp() / 100.0);

	fprintf(handle, "Laser Output Status: %i\n", VortexGetOutputStatus());
	fprintf(handle, "Laser Diode Current: %f mA\n", VortexGetCurrent());
	fprintf(handle, "Laser Diode Temperature: %f\n", VortexGetTemp());

	fprintf(handle, "Laser Piezo Voltage: %f V\n", VortexGetVoltage());
	fprintf(handle, "Laser Output Power: %f\n", VortexGetPower());
	fprintf(handle, "Laser Diode Hours: %i\n", VortexGetDiodeHours());
	fprintf(handle, "Laser Controller Hours: %i\n", VortexGetControllerHours());
	fprintf(handle, "X Stage Starting Step %i\n", CENTER_X);
	fprintf(handle, "Y Stage Starting Step %i\n", CENTER_Y);
	fprintf(handle, "Wait after stage move %i mS\n", STAGE_MOVE_HORI_SLEEP);
	fprintf(handle, "Vertical Line Pairs Start %i\n", VERT_LINE_PAIRS_START);
	fprintf(handle, "Vertical Line Pairs Stop %i\n", VERT_LINE_PAIRS_END);
	fprintf(handle, "Horizontal Number of Samples %i\n", BASE_HORIZ_LINE_SAMPLES);
	fprintf(handle, "Vertical Step Size %i\n", VERTICAL_STEP_SIZE);
	fprintf(handle, "Horizontal Step Size %i\n", HORIZ_STEP_SIZE);
	fprintf(handle, "Base Save Directory %s\n", SAVE_DIRECTORY);
	fprintf(handle, "Move Stage %i\n", MOVE_STAGE);
	fprintf(handle, "Move Stage Vertically %i\n", MOVE_STAGE_VERT);
	fprintf(handle, "Use Small Steps: %i\n", USE_SMALL_STEPS);

	fclose(handle);
}

void save_experiment_description_end(char * save_directory) {
	char filename[512];
    //char description[512];
	FILE * handle;

	sprintf(filename, "%s\\_setup.txt", save_directory);

	handle = fopen(filename, "a+");

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	fprintf(handle, "Start Date: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(handle, "CCD Final Temp: %f\n", GetCCDCurrentTemp() / 100.0);

	fprintf(handle, "Laser Output Status: %i\n", VortexGetOutputStatus());
	fprintf(handle, "Laser Diode Current: %f mA\n", VortexGetCurrent());
	fprintf(handle, "Laser Diode Temperature: %f\n", VortexGetTemp());

	fprintf(handle, "Laser Piezo Voltage: %f V\n", VortexGetVoltage());
	fprintf(handle, "Laser Output Power: %f\n", VortexGetPower());
	fprintf(handle, "Laser Diode Hours: %i\n", VortexGetDiodeHours());
	fprintf(handle, "Laser Controller Hours: %i\n", VortexGetControllerHours());

	fclose(handle);
}

int get_base_directory(char * basedir) {
	int loop = 0;
	int exist = 1;

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	while (exist) {
		sprintf(basedir, "%s\\%04i%02i%02i_%03i", SAVE_DIRECTORY, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, loop);

		exist = GetFileAttributes(basedir) != -1;
		if (!exist) {
			printf("\nCreating Directory: %s \n", basedir);
			CreateDirectory(basedir, 0);

			SetFilenameBase(basedir);

			return 0;
		}
		loop++;
	}
	printf("\nUnable to find destination directory");
	return -1;
}

/*
*
*  returns a string that can be used by sprintf to generate the filename given an image number
*/

int initial_test(char * directory) {
	//char filename[512];
	//char description[512];
	int success;
	float ccd_temp;
	success = 1;

	InitializeCamera();
	InitializeVortex();
	ccd_temp = (float)GetCCDCurrentTemp();
	printf("ccd_temp %d \n", ccd_temp);
	InitializingBrianStages();
	if (MOVE_STAGE) {
		// initialize "com3"
		
		Sleep(6000);
		print_time();
		stage_move_vert(CENTER_Y, 1, 1);
		Sleep(6000);
		print_time();
		stage_move_hori(CENTER_X, 1, 1);//should set this to be the radial center
		Sleep(6000);
		print_time();
	    printf("Vertical stage position %d \n", BrianStageGetPosition(1));
		Sleep(2000);
		print_time();
		printf("Horizontal stage position %d \n", BrianStageGetPosition(2));
		
		// Verify that the stage actually moved
		if (MessageBox(NULL, "Did the stage move?", "Stage Test", MB_YESNO) == IDNO) {
			printf("\nStage Did Not Move");
			success = 0;
		}
	}
	
	/*
	sprintf(filename, "%s\\test.txt", directory);
	handle = fopen(filename, "w");

	if (!handle) {
	printf("\nTest File Could Not Be Opened %s", filename);
	success = 0;
	}
	fclose(handle);

	// delete the test file
	DeleteFile(filename);
	*/


	if (abs(GetCCDCurrentTemp() - ccd_temp) > 3.0) {
		printf("\nCCD Temperature not stable (%f) (%f)", ccd_temp / 100.0, GetCCDCurrentTemp() / 100.0);
		//		success = 0;
	}

	if (!success || MessageBox(NULL, "Run Experiment?", "Start Experiment", MB_YESNO) == IDNO) {
		printf("\nExperiment Aborted!!!");
		success = 0;
	}

	return success;

}

int stage_move_vert(int step, int move, int sleep) {
	int new_position;

	int moved = 0;
	int dir;

	new_position = stage_offset(step, move);

	if (move) {

		// moves the stage by step  
		dir = step > 0 ? 1 : -1;
		step = abs(step);

		if (USE_SMALL_STEPS) {
			while (step > VERTICAL_STEP_SIZE) {

				step -= VERTICAL_STEP_SIZE;

				BrianStageMoveRelative(1, dir * VERTICAL_STEP_SIZE);
				// move_vert_stage_d(dir * VERTICAL_STEP_SIZE);


				if (sleep) {
					printf("^");
					Sleep(STAGE_MOVE_VERT_SLEEP);
				}
			}
		}

		if (step > 0) {

			BrianStageMoveRelative(1, dir * step);
			// move_vert_stage_d(dir * step);

			if (sleep) {
				printf("%");
				Sleep(STAGE_MOVE_VERT_SLEEP);
			}
		}

	}
	return new_position;

}

int stage_move_hori(int step, int move, int sleep) {
	int new_position;


	int moved = 0;
	int dir;

	new_position = stage_offset(step, move);

	if (move) {

		// moves the stage by step  
		dir = step > 0 ? 1 : -1;
		step = abs(step);

		if (USE_SMALL_STEPS) {
			while (step > HORIZ_STEP_SIZE) {

				step -= HORIZ_STEP_SIZE;

				BrianStageMoveRelative(2, dir * HORIZ_STEP_SIZE);
				// move_vert_stage_d(dir * VERTICAL_STEP_SIZE);


				if (sleep) {
					printf("^");
					Sleep(STAGE_MOVE_HORI_SLEEP);
				}
			}
		}

		if (step > 0) {

			BrianStageMoveRelative(2, dir * step);
			// move_vert_stage_d(dir * step);

			if (sleep) {
				printf("%");
				Sleep(STAGE_MOVE_VERT_SLEEP);
			}
		}

	}
	return new_position;

}
/*
*  keeps track of the vertical stage offset
*  first argument is how much the stage is going to be or just was moved
*  return argument is new stage offset
*/
int stage_offset(int step, int move) {
	static int offset = 0;

	if (move) {
		offset += step;
	}
	return offset;
}

int getVerticalPosition(void) {
	return BrianStageGetPosition(1);
}

int getHorizontalPosition(void) {
	return BrianStageGetPosition(2);
}

void print_time(void) {
	
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}