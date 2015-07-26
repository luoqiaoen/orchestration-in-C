//system inclusion//
#include <windows.h>  
#include "ansi_c.h"

//SDK inclusion//
#include "master.h"
#include "pvcam.h"

//CVI interface GUI stuff//
//#include "dataskt.h" 
//#include <userint.h> 
//#include <ansi_c.h>
//#include <utility.h>

//other codes//
#include "CoolSNAP.h"


//definitions//
#define DATETIME_FORMATSTRING "%Y%d%j_%H%M"
#define DATETIME_BUFFER 16


extern float LaserDiodeTemp;
//extern float PiezoVoltage;
//extern float DiodeCurrent;
//extern double STEP;

int16 Camera;

uns16 *ImageBuffer,
*BackgroundBuffer;

uns32 BufferSize;

uns16 CCDpixelser,
CCDpixelpar;
CCDgainindex;
CCDpixtime;
Spd_Tab_Index;
ExpTimeUnits;
rgn_type Region;

int xSize,
ySize;
unsigned char  *BitMap;
int ColorTable[256];
//int I_TOL;
//int HoldOn;
int BackgroundSubtractFlag = 0;
uns32 ExpTime;
unsigned long int HistogramData[4096];
double HistogramData_Log[4096];
double HistogramPlotData[4096];

FILE *ImageFile;
char filename[260];//char filename[MAX_PATHNAME_LEN];uses utility.h

void SetDefault(void)
{
	ExpTime = 1000;
	BackgroundSubtractFlag = FALSE;
}

void InitializeCamera(void)
{
	char CameraName[CAM_NAME_LEN];


	SetDefault();

	pl_pvcam_init();
	pl_cam_get_name(0, CameraName);
	pl_cam_open(CameraName, &Camera, OPEN_EXCLUSIVE);

	printf("CameraName = %s\n", CameraName);
	printf("Camera = %d\n", Camera);


	Spd_Tab_Index = 2;
	pl_set_param(Camera, PARAM_SPDTAB_INDEX, &Spd_Tab_Index);    //10Mhz readout mode Readout Port 2

	pl_get_param(Camera, PARAM_SER_SIZE, ATTR_CURRENT, &CCDpixelser); //serial dimension of active area
	pl_get_param(Camera, PARAM_PAR_SIZE, ATTR_CURRENT, &CCDpixelpar); //parallel dimension of active area
	pl_get_param(Camera, PARAM_GAIN_INDEX, ATTR_CURRENT, &CCDgainindex); //gain setting
	pl_get_param(Camera, PARAM_PIX_TIME, ATTR_CURRENT, &CCDpixtime); //actual speed in ns

	printf("\n");
	printf("CCDpixelser = %d\n", CCDpixelser);
	printf("CCDpixelpar = %d\n", CCDpixelpar);
	printf("CCDgainindex = %d\n", CCDgainindex);
	printf("CCDpixtime = %d\n", CCDpixtime);

	Region.s1 = 0;
	Region.s2 = CCDpixelser - 1;
	Region.sbin = 1;
	Region.p1 = 0;
	Region.p2 = CCDpixelpar - 1;
	Region.pbin = 1;


	pl_get_param(Camera, PARAM_EXP_RES, ATTR_CURRENT, &ExpTimeUnits); //resolution
	printf("ExptimeUnits = %d\n", ExpTimeUnits);

	if (ExpTimeUnits == EXP_RES_ONE_MILLISEC)
		printf("Milliseconds!!\n");
	if (ExpTimeUnits == EXP_RES_ONE_MICROSEC)
		printf("Microseconds!!\n");

	pl_exp_init_seq(); // prepare to acquire and readout
	pl_exp_setup_seq(Camera, 1, 1, &Region, TIMED_MODE, ExpTime, &BufferSize); // acquire in sequential mode

	printf("BufferSize = %ld\n", BufferSize);

	ImageBuffer = (uns16*)malloc(BufferSize);
	BackgroundBuffer = (uns16*)malloc(BufferSize);

	//HistogramData = malloc(4096*sizeof(unsigned long int));
	//HistogramPlotData = malloc(4096*sizeof(double));   
    //HistogramData_Log = malloc(4096*sizeof(double));   	


	AllocateBitMap();



	// Acquire noise, don't use the flowing in case of normal operation
	/*	ExpTime = 10000;
	();
	sprintf(filename,"C:\\jason\\data\\speckle\\030625\\mode1\\bg10s2.spk");
	ImageFile = fopen(filename,"wb");
	fwrite(ImageBuffer,sizeof(uns16),CCDpixelser*CCDpixelpar,ImageFile);
	fclose(ImageFile);
	*/



}

void AllocateBitMap(void)
{

	long BitMapSize;
	int  i;
	//int  CVIerr;

	xSize = CCDpixelser / 2;   // Displayed images only have 1/4 of the pixels
	ySize = CCDpixelpar / 2;   // of the CCD due to screen resolution limitation


	printf("xSize = %d, ySize = %d\n", xSize, ySize);

	BitMapSize = xSize*ySize;
	BitMap = malloc(BitMapSize*sizeof(unsigned char));

	for (i = 0; i <256; i++)	  {

		ColorTable[i] = i + (i << 8) + (i << 16);
	}

}

void CleanUpCamera(void)
{
	pl_cam_close(Camera);
	pl_pvcam_uninit();
	free(ImageBuffer);
	free(BitMap);
}

int GetExposureTime(void) {
	return ExpTime;
}

void AcquireImage(void)
{
	int16 Status;
	uns32 NotUsed;

	pl_exp_init_seq();
	pl_exp_setup_seq(Camera, 1, 1, &Region, TIMED_MODE, ExpTime, &BufferSize);

	printf(".");
	pl_exp_start_seq(Camera, ImageBuffer);

	while (pl_exp_check_status(Camera, &Status, &NotUsed) &&
		(Status != READOUT_COMPLETE && Status != READOUT_FAILED)) {
		Sleep(50);
	}
	// monitor status
	//     pl_exp_finish_seq(Camera,Buffer,0)  // un-needed? p66

	pl_exp_uninit_seq();

	if (BackgroundSubtractFlag) {
		SubtractBackground();
	}

}

void SubtractBackground(void)
{

	unsigned long int i;

	for (i = 0; i < CCDpixelser*CCDpixelpar; i++)  {
		if (ImageBuffer[i] > BackgroundBuffer[i])
			ImageBuffer[i] -= BackgroundBuffer[i];
		else
			ImageBuffer[i] = 0;
	}


}

void SaveBackgroundImage(char * filename) {
	SaveImageFile(filename, 0);
}

void SaveImage(char * filename) {
	SaveImageFile(filename, 1);
}

void SaveImageFile(char *filename, int image)
{
	FILE *ImageFile;
	uns16* img;

	if (image) {
		img = ImageBuffer;
	}
	else {
		img = BackgroundBuffer;
	}

	ImageFile = fopen(filename, "wb");
	fwrite(img, sizeof(uns16), CCDpixelser*CCDpixelpar, ImageFile);
	fwrite(&LaserDiodeTemp, sizeof(float), 1, ImageFile);
	fclose(ImageFile);
}


void LoadImageFile(char *filename)
{
	FILE *ImageFile;

	ImageFile = fopen(filename, "rb");
	fread(ImageBuffer, sizeof(uns16), CCDpixelser*CCDpixelpar, ImageFile);
	fclose(ImageFile);

}

void AcquireBackground(void)
{
	unsigned long int i;

	//SetLaserOutput(0);
	//Sleep(5*1000);

	AcquireImage();
	for (i = 0; i<CCDpixelser*CCDpixelpar; i++) {
		BackgroundBuffer[i] = ImageBuffer[i];
	}
	AcquireImage();
	for (i = 0; i<CCDpixelser*CCDpixelpar; i++) {
		BackgroundBuffer[i] += ImageBuffer[i];
	}

	for (i = 0; i<CCDpixelser*CCDpixelpar; i++) {
		BackgroundBuffer[i] /= 2;
	}

	CalcHistogram();

	//SetLaserOutput(1);
	//Sleep(5*1000);
}

void CalcHistogram(void)
{
	unsigned long int Limit,
		i;
	unsigned int mu = 0;
	double mu_2;
	// 	if (ADCResolution == 12)
	Limit = 4096;
	//	else 
	//	    Limit = 65536;

	for (i = 0; i < Limit; i++) {
		HistogramData[i] = 0;
	}

	for (i = 0; i < CCDpixelser*CCDpixelpar; i++) {
		HistogramData[ImageBuffer[i]]++;
		mu += ImageBuffer[i];
	}
	mu_2 = ((double)mu) / (CCDpixelser*CCDpixelpar);
}

double CalcMean(void) {
	unsigned long int i;
	double mu;

	mu = 0;
	for (i = 0; i < CCDpixelser*CCDpixelpar; i++) {
		mu += ImageBuffer[i];
	}
	return mu / (CCDpixelser * CCDpixelpar);
}

int GetCCDCurrentTemp(void){
	int16 CCDCurrentTemperature;


	pl_get_param(Camera, PARAM_TEMP, ATTR_CURRENT, &CCDCurrentTemperature);

	return CCDCurrentTemperature;
}

void EstimateNoise(void)
{

	char DIR[256], Filename[256], ext[16];
	int i;

	clock_t t0, t1;

	sprintf(DIR, "D:\\jason\\data\\temp\\bg.");
	sprintf(ext, "0a");
	ExpTime = 1;

	t0 = clock();

	for (i = 1; i <= 5; i++)
	{
		sprintf(Filename, DIR);

		ext[0]++;
		strcat(Filename, ext);
		AcquireImage();
		SaveImageFile(Filename, 1);

		ext[1]++;
		sprintf(Filename, DIR);
		strcat(Filename, ext);
		AcquireImage();
		SaveImageFile(Filename, 1);

		ext[1]--;
		ExpTime = ExpTime * 10;

	}

	t1 = clock();
	printf("%d secs elipses.", (int)difftime(t1, t0) / 1000);
}