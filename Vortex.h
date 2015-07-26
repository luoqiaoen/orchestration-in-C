#ifndef _H_VORTEX
#define _H_VORTEX
#define MANUAL_LASER_CONTROL 0
void getBackgroundImages(int i, int j, char * base_directory);
void disableLaser(void);
void enableLaser(void);
int   InitializeVortex(void);
int   GetVortexStatus(void);
void  SetDiodeCurrent(void);
void  SetPiezoVoltage(void);
void  SetLaserOutput(int);
float ReadDiodeCurrent(void);
float ReadLaserPower(void);
float get_vortex_aux(void);
float VortexGetCurrent(void);
float VortexGetTemp(void);
float VortexGetVoltage(void);
float VortexGetPower(void);
int VortexGetDiodeHours(void);
int VortexGetControllerHours(void);
int VortexGetOutputStatus(void);




void GPIBerr (char *);
int WriteIt(int, char *);
int ReadIt(int, char *, int);
int WaitForSRQ(int, int);
int SerialPoll(int, char *);
int ClearDev(int);
int SendIt(int, char *);
int ReceiveIt(int, void *, int);
int SendItIFC(void);
short MyReadStatusByte(int);
void InitializeGPIBboard(void);

#endif
