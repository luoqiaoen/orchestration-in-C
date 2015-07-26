#ifndef _RADIAL_SCAN_H
#define _RADIAL_SCAN_H

#define CENTER_Y 1000000 // Y Stage Starting Step
#define CENTER_X 1000000 // X Stage Starting Step

#define DELTA_RHO 75.0 // in units of micrometers
#define NUM_LINE_SAMPLES 80  // number of samples on each radial scan line
#define NUM_LINES 20 // number of radial scans distributed across -pi:pi 

#define Y_SCALE 20.997 // 20.997 // # steps per micrometer
#define X_SCALE 20.997 // # steps per micrometer

#define M_PI 3.14159265359
#define M_PI_2 M_PI/2
#define STAGE_MOVE_VERT_SLEEP 2000

#define VERT_NUM_LINES 1   // number of lines for the second scan dimension - at least 1

#define BASE_HORIZ_LINE_SAMPLES 200 // number of sample in the first scan dimension

#define VERTICAL_STEP_SIZE 150 //mm
#define HORIZ_STEP_SIZE 150 //mm

#define STAGE_MOVE_HORI_SLEEP 2000
#define STAGE_MOVE_VERT_SLEEP 2000
#define USE_SMALL_STEPS 0

#define SAVE_DIRECTORY "T:\\luo\\data"
#define MOVE_STAGE 1
#define MOVE_STAGE_VERT 1

#define VERT_LINE_PAIRS_START 0
#define VERT_LINE_PAIRS_END 0

#define MY_CONTROLLER      ST133
#define MY_CCD             PI_800x1000_B
#define MY_CARD            PCI_COMPLEX_PC_Interface

#define MY_GPIBBOARD_ID    0  // GPIB ID
#define MY_ESP300_ADDR     19
#define MY_VORTEX_ADDR     1 // GPIB address of New Focus Vortex Laser System
#define MY_HPE3631A_ADDR   5

#define MY_DAQ_CARD        1

#define PI 3.1415926536         

void SaveExperimentDescription(char * save_directory);
void SaveExperimentDescriptionRadial(char * save_directory);
void save_experiment_description_end(char * save_directory);
int get_base_directory(char * basedir);
int initial_test(char * directory);
int stage_move_vert(int step, int move, int sleep);
int stage_move_hori(int step, int move, int sleep);
int stage_offset(int step, int move);
int getVerticalPosition(void);
int getHorizontalPosition(void);
void acquireLineAngle(float angle, int numSamples);
void main(void);
void print_time(void);


#endif
