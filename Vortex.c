#include <windows.h> 
#include "ni4882.h"
#include "ansi_c.h"
#include "filename.h"
#include "RadialScan.h"
#include "CoolSNAP.h"
#include "Vortex.h"   
int GPIBBoard = 0;

int   Vortex;
    
float LaserDiodeTemp,
      SensedPiezoVoltage,
      SensedOutputPower,
      SensedDiodeCurrent;
int   LaserDiodeHours,
      ControllerHours,
      LaserOutputStatus;

float PiezoVoltage,	   // these are the set point values
      DiodeCurrent;    
      
short MAV = 16;   

    
void getBackgroundImages(int i, int j, char * base_directory) {

	char backgroundFilename[512];

	print_time();
	printf("Getting background Images");
	GetVortexStatus();

	disableLaser();

	// The image is acquired to the normal image buffer and not the background image buffer.
	AcquireImage();
	sprintf(backgroundFilename, "%s\\background_%d", base_directory, i);
	SaveImage(backgroundFilename);

	AcquireImage();
	sprintf(backgroundFilename, "%s\\background_%d", base_directory, j);
	SaveImage(backgroundFilename);
}

void disableLaser(void) {
	// save a couple background images

	if (MANUAL_LASER_CONTROL) {
		MessageBox(NULL, "Turn Off Laser.", "Laser Control", MB_OK);
		return;
	}

	SetLaserOutput(0);

	Sleep(1000);

}

void enableLaser(void) {
	int failCount;

	if (MANUAL_LASER_CONTROL) {
		MessageBox(NULL, "Turn On Laser.", "Laser Control", MB_OK);
		return;
	}

	Sleep(500);
	GetVortexStatus();
	Sleep(500);
	SetLaserOutput(1);
	Sleep(5000);

	failCount = 0;
	while (!VortexGetOutputStatus()) {
		print_time();
		printf("Laser is powering up");
		Sleep(2000);

		if (failCount++ >= 60) {
			print_time();
			printf("Failed to power up");
			return;
		}

		if (failCount % 10 == 0) {
			SetLaserOutput(1);
		}
	}
}

int InitializeVortex(void)
{
	printf("Vortex Address %i\n", MY_VORTEX_ADDR);
	Vortex = ibdev(GPIBBoard,MY_VORTEX_ADDR, 0, T1s, 1, 0x140A); // last param = 0???

    GetVortexStatus();  
    
    return 0;
}

float get_vortex_aux(void) {
	float aux_input;
	float current;

    char StatusByte = 0;
    char CommandStr[80];
    char Buffer[80];

	int i;
	
	float step_size = 0.1f;


	return 0;
	current = 48.0;
	DiodeCurrent = current;
	SetDiodeCurrent(); 
	
	
	for(i=0; i < 0; i++) {
		printf("%i ", i);


    	sprintf(CommandStr,":SENS:VOLT:AUX");
    	WriteIt(Vortex,CommandStr);
    	StatusByte = 0;
    	while (!(StatusByte & MAV))         // wait for Vortex to respond
    	    SerialPoll(Vortex,&StatusByte);
	    ReadIt(Vortex,Buffer,80);

    	aux_input = atof(Buffer);


		if(aux_input > 2.5) {
			printf(" %f %f ", current, step_size);
			current = current - step_size;
			
			
		}
		else if (aux_input < 1.5) {
			current += step_size;
			
		}
		
		DiodeCurrent = current;
		printf("%f %f \n", DiodeCurrent, current);
		SetDiodeCurrent();
		Sleep(100);
	}
	

    printf("Aux Input is %f\n", aux_input);
	printf("Aux String is %s\n", Buffer);
	// set current :SOUR:CURR <VAL>
	

	return aux_input;
}

int GetVortexStatus(void)
{
    char StatusByte = 0;
    char CommandStr[80];
    char Buffer[80];
	int start_time, stop_time;


    sprintf(CommandStr,":SENS:TEMP");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    
	start_time = GetTickCount();
    while (!(StatusByte & MAV)) {        // wait for Vortex to respond
    	
        SerialPoll(Vortex,&StatusByte);
        
        //we should only try for 20 seconds though and then give up.....
		stop_time = GetTickCount();
        
        if(stop_time < start_time) {
			stop_time += 60000;
        }
        if(stop_time - start_time > 20000) {
			MessageBox(NULL, "Unable to communicate with Vortex Laser.", "Error",MB_OK | MB_ICONEXCLAMATION );
        	return -1;
        }
        
    }
    
    get_vortex_aux();
    
    ReadIt(Vortex,Buffer,80);
    LaserDiodeTemp = atof(Buffer);

    sprintf(CommandStr,":SYST:INF:DHO?");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    LaserDiodeHours = atoi(Buffer);
    
    sprintf(CommandStr,":SYST:INF:SHO?");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    ControllerHours = atoi(Buffer);

    sprintf(CommandStr,":SENS:CURR");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    SensedDiodeCurrent = atof(Buffer);

    sprintf(CommandStr,":SENS:POW");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    SensedOutputPower = atof(Buffer);
        
    sprintf(CommandStr,":SENS:VOLT:PIEZ");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    SensedPiezoVoltage = atof(Buffer);

    sprintf(CommandStr,":OUTP?");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    LaserOutputStatus = atoi(Buffer);

	/*
    SetCtrlAttribute(VortexConfigPanel,
                     VORTEPANEL_VORTEXTEMP,
                     ATTR_CTRL_VAL,
                     LaserDiodeTemp);

    SetCtrlAttribute(VortexConfigPanel,
                     VORTEPANEL_LASERDIODEHOURS,
                     ATTR_CTRL_VAL,
                     LaserDiodeHours);
        
    SetCtrlAttribute(VortexConfigPanel,
                     VORTEPANEL_CONTROLHOURS,
                     ATTR_CTRL_VAL,
                     ControllerHours);

    SetCtrlAttribute(VortexConfigPanel,
                     VORTEPANEL_DIODECURRENT,
                     ATTR_CTRL_VAL,
                    SensedDiodeCurrent);
	
    SetCtrlAttribute(VortexConfigPanel,
                     VORTEPANEL_PIEZOVOLTAGE,
                     ATTR_CTRL_VAL,
                     SensedPiezoVoltage);

    SetCtrlAttribute(VortexConfigPanel,
                     VORTEPANEL_LASERPOWER,
                     ATTR_CTRL_VAL,
                     SensedOutputPower);
    SetCtrlVal(VortexConfigPanel,
               VORTEPANEL_LASEROUTPUTIND,
               LaserOutputStatus);*/
               
    return 0;
}        
    
float VortexGetCurrent(void) {
	GetVortexStatus();
	return SensedDiodeCurrent;
}

float VortexGetTemp(void) {
	GetVortexStatus();
	return LaserDiodeTemp;
}

float VortexGetVoltage(void) {
	GetVortexStatus();
	return SensedPiezoVoltage;
}

float VortexGetPower(void) {
	GetVortexStatus();
	return SensedOutputPower;
}

int VortexGetDiodeHours(void) {
	GetVortexStatus();
	return LaserDiodeHours;
}

int VortexGetControllerHours(void) {
	GetVortexStatus();
	return ControllerHours;
}

int VortexGetOutputStatus(void) {

	char CommandStr[255], Buffer[255];
	char StatusByte;
	int LaserOutputStatus;

    sprintf(CommandStr,":OUTP?");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    LaserOutputStatus = atoi(Buffer);

	return LaserOutputStatus;
}
   
void SetDiodeCurrent(void)
{
    char CommandStr[80];
    
    sprintf(CommandStr,":SOUR:CURR %4.1f",DiodeCurrent);
    WriteIt(Vortex,CommandStr);
}

void SetPiezoVoltage(void)
{
    char CommandStr[80];
    
    sprintf(CommandStr,":SOUR:VOLT:PIEZ %5.1f",PiezoVoltage);
    WriteIt(Vortex,CommandStr);
}

void SetLaserOutput(int state)   // state = 1 turns on laser
{								 // state = 0 turns off laser
    char CommandStr[80];
    
    sprintf(CommandStr,":OUTP %1d",state);
    
    WriteIt(Vortex,CommandStr);

}

float ReadDiodeCurrent(void)
{
    char StatusByte;
    char CommandStr[80];
    char Buffer[80];

    float LaserDiodeCurrent;

    sprintf(CommandStr,":SENS:CURR");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    LaserDiodeCurrent = atof(Buffer);
    
    return LaserDiodeCurrent;
}

float ReadLaserPower(void)
{
    char StatusByte;
    char CommandStr[80];
    char Buffer[80];

    float LaserPower;

    sprintf(CommandStr,":SENS:POW");
    WriteIt(Vortex,CommandStr);
    StatusByte = 0;
    while (!(StatusByte & MAV))         // wait for Vortex to respond
        SerialPoll(Vortex,&StatusByte);
    ReadIt(Vortex,Buffer,80);
    LaserPower = atof(Buffer);
    
    return LaserPower;
}

void GPIBerr(char *msg)
{
    printf("GPIB ERROR!");
}

int WriteIt(int Dev, char *CommandStr)
{
    int NumBytes;
    char TmpStr[80];
    char iberrStr[20];
    short Listen = 0;
    
    NumBytes = strlen(CommandStr);
    ibwrt(Dev,CommandStr,NumBytes);
    
    ibwait(Dev,CMPL|TIMO);     // wait for write to complete
    
    if (ibsta & ERR)		// ERR  = 1 << 15, checks error bit in status
        {
        strcpy(TmpStr,"ibwrt error: ");
        strcat(TmpStr,CommandStr);
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
        }
    else
        return 1;
}

int ReadIt(int Dev, char *Buffer, int NumBytes)
{
    char TmpStr[80];
    char iberrStr[20];
    
    ibrd(Dev,Buffer,NumBytes);
    ibwait(Dev,CMPL|TIMO);
    if (ibsta & ERR)
        {
        strcpy(TmpStr,"ibrd error: ");
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
        }
    else
        Buffer[ibcntl-1] = '\0';

    return 1;
}

int WaitForSRQ(int Dev, int Mask)
{
    char TmpStr[80];
    char iberrStr[20];

    ibwait(Dev, Mask);
    if (ibsta & (ERR|TIMO))
        {
        strcpy(TmpStr,"ibwait error: ");
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
    }
    else
        return 1;
}        

int SerialPoll(int Dev, char *Response)
{
    char TmpStr[80];
    char iberrStr[20];

    ibrsp(Dev, Response);
    ibwait(Dev,CMPL|TIMO);
    if (ibsta & ERR)
        {
        strcpy(TmpStr,"ibrsp error: ");
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
    }
    else
        return 1;
}       

int ClearDev(int Dev)
{
    char TmpStr[80];
    char iberrStr[20];

    ibclr(Dev);
    if (ibsta & ERR)
        {
        strcpy(TmpStr,"ibclr error: ");
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
    }
    else
        return 1;
}       


// Below here are commands for the 488.2 command set.

int SendIt(int Dev, char *SendStr)
{
	int  NumBytes;
    char TmpStr[80];
    char iberrStr[20];

    NumBytes = strlen(SendStr);
    Send(GPIBBoard,Dev,SendStr,NumBytes,NLend);
    if (ibsta & ERR)
        {
        strcpy(TmpStr,"Send error:");
        strcat(TmpStr,SendStr);
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
    }
    else
        return 1;
}       

int ReceiveIt(int Dev, void *Buffer, int NumBytes)
{
    char TmpStr[80];
    char iberrStr[20];

    Receive(GPIBBoard, Dev, Buffer, NumBytes, STOPend);
    if (ibsta & ERR)
        {
        strcpy(TmpStr,"Receive error:");
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
    }
    else
        return 1;
}    

int SendItIFC(void)
{
    char TmpStr[80];
    char iberrStr[20];

    SendIFC(GPIBBoard);
    if (ibsta & ERR)
        {
        strcpy(TmpStr,"SendIFC error: ");
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
    }
    else
        return 1;
}

short MyReadStatusByte(int Dev)
{
    char TmpStr[80];
    char iberrStr[20];
    short StatusByte;
    
    ReadStatusByte(GPIBBoard,Dev,&StatusByte);
    if (ibsta & ERR)
        {
        strcpy(TmpStr,"ReadStatusByte error:");
        sprintf(iberrStr," (iberr=%d)",iberr);
        strcat(TmpStr,iberrStr);
        GPIBerr(TmpStr);
        return 0;
    }
    else
        return StatusByte;
}       

    
void InitializeGPIBboard(void)
{
    SendItIFC();    // 488.2 command for interface clear.
}