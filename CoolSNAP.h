#ifndef _COOLSNAP_H
#define _COOLSNAP_H

void SetDefault(void);
void InitializeCamera(void);
void AllocateBitMap(void);
void CleanUpCamera(void);
int  GetExposureTime(void);
void AcquireImage(void);
void SubtractBackground(void);
void SaveBackgroundImage(char * filename);
void SaveImage(char * filename);
void SaveImageFile(char *, int);
void LoadImageFile(char *);
void AcquireBackground(void);
void CalcHistogram(void);
double CalcMean(void);
int  GetCCDCurrentTemp(void);
void EstimateNoise(void);

#endif
