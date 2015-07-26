#include "ansi_c.h"
#include <windows.h>
#include "zb_serial.h"
#include "stage.h"

z_port port;
void InitializingBrianStages(void){
	char *device_name = "COM3";
	unsigned char cmd[6];
	if (zb_connect(&port, device_name) != Z_SUCCESS)
	{
		printf("Could not connect to device %s.\n", device_name);
		printf("Please check that you typed in the correct name,\n");
		printf("and check that the requested port is not currently being used.\n");
	}
	cmd[0] = 0;
	cmd[1] = 1;

	BrianSendCommand(cmd);
	Sleep(6000);
}

void CloseBrianStages(void) {
	zb_disconnect(port);
}


void BrianStageReset(int deviceNumber){
	unsigned char cmd[6];

	cmd[0] = deviceNumber;
	cmd[1] = 1;

	BrianSendCommand(cmd);
}

void BrianSendCommand(unsigned char cmd[6]){
	int j;
	zb_send(port, cmd);
	for (j = 0; j < 6; j++) {
		cmd[j] = 0;
	}
	zb_receive(port, cmd);
}

void BrianStageMoveRelative(int deviceNumber, unsigned int distance) {
	char cmd[6];
	char * ptr;

	cmd[0] = deviceNumber;
	cmd[1] = 21;

	ptr = (char *)&distance;
	cmd[2] = ptr[0];
	cmd[3] = ptr[1];
	cmd[4] = ptr[2];
	cmd[5] = ptr[3];
	
	BrianSendCommand(cmd);

	Sleep(600);
}


void BrianStageMoveAbsolute(int deviceNumber, unsigned int position) {
	char cmd[6];
	char * ptr;

	cmd[0] = deviceNumber;
	cmd[1] = 20;

	ptr = (char *)&position;
	cmd[2] = ptr[0];
	cmd[3] = ptr[1];
	cmd[4] = ptr[2];
	cmd[5] = ptr[3];

	BrianSendCommand(cmd);

}

int BrianStageGetPosition(int deviceNumber) {
	unsigned char cmd[6];
	long currentPosition;

	cmd[0] = deviceNumber;
	cmd[1] = 60;

	cmd[2] = 0;
	cmd[3] = 0;
	cmd[4] = 0;
	cmd[5] = 0;
	BrianSendCommand(cmd);

	currentPosition = cmd[2];
	currentPosition |= (cmd[3] << 8);
	currentPosition |= (cmd[4] << 16);
	currentPosition |= (cmd[5] << 24);


	//printf("Current Position is: %i (%02x %02x %02x %02x)\n", currentPosition, cmd[2], cmd[3], cmd[4], cmd[5]);

	return currentPosition;
}


