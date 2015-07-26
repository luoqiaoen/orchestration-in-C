#ifndef _H_STAGE
#define _H_STAGE
#define STEP_TO_DISTANCE 0.09921875
void InitializingBrianStages(void);
void CloseBrianStages(void);
void BrianConfigureStages(void);
void BrianStageReset(int);
void BrianSendCommand(unsigned char cmd[6]);
void BrianStageMoveRelative(int deviceNumber, unsigned int distance);
void BrianStageMoveAbsolute(int deviceNumber, unsigned int distance);
int BrianStageGetPosition(int deviceNumber);
#endif