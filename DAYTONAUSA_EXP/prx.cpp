#include "stdafx.h"

#include <cell/codec/gifdec.h>
#include <cell/codec/jpgdec.h>
#include <cell/codec/jpgenc.h>
#include <cell/codec/pngcom.h>
#include <cell/codec/pngdec.h>
#include <cell/codec/pngenc.h>
#include <cell/gcm.h>

#include <sysutil\sysutil_msgdialog.h>
#include <sysutil\sysutil_oskdialog.h>
#include <sysutil\sysutil_oskdialog_ext.h>

#include <cellstatus.h>
#include <sys/prx.h>
#include <sys/random_number.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "printf.h"

using namespace cell::Gcm;

#define THREAD_NAME         "ogl_main_thread"
#define STOP_THREAD_NAME    "ogl_main_stop_thread"

#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)

static sys_ppu_thread_t ogl_main_tid = -1;

SYS_MODULE_INFO( DAYTONAUSA_EXP, 0, 1, 1);
SYS_MODULE_START( _DAYTONAUSA_EXP_prx_entry );

SYS_LIB_DECLARE_WITH_STUB( LIBNAME, SYS_LIB_AUTO_EXPORT, STUBNAME );
SYS_LIB_EXPORT( _DAYTONAUSA_EXP_export_function, LIBNAME );

void my_dialog2(int button, void *userdata)
{
	switch (button) {
	case CELL_MSGDIALOG_BUTTON_OK:

	case CELL_MSGDIALOG_BUTTON_NONE:

	case CELL_MSGDIALOG_BUTTON_ESCAPE:
		//dialog_action = 1;
		break;
	default:
		break;
	}
}

void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
 
// Converts a given integer x to string str[].
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
 
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
 
    reverse(str, i);
    str[i] = '\0';
    return i;
}

void ftoa(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
 
    // Extract floating part
    float fpart = n - (float)ipart;
 
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
 
    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
 
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10.0f, afterpoint);
 
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

int myAtoi(char* str)
{
    // Initialize result
    int res = 0;
  
    // Iterate through all characters
    // of input string and update result
    // take ASCII character of corresponding digit and
    // subtract the code from '0' to get numerical
    // value and multiply res by 10 to shuffle
    // digits left to update running total
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
  
    // return result.
    return res;
}

static void ogl_main_thread(uint64_t arg) {
	printf("DAYTONAUSA EXP started \n");
	sys_timer_usleep(5 * 1000 * 1000);

	int isInGame = 0;
	int toSave = 0;
	int score = 0;

	while (1) {
		//printf("DAYTONAUSA_EXP.sprx \n");
		unsigned char *scoreBuf = (unsigned char *)0x30081DBE0;
		unsigned char *isInGameBuf = (unsigned char *)0x3004FFF49;

		float value = (*(float*)scoreBuf);
		int inGameValue = (*(int*) isInGameBuf);

		//printf("SCORE BUFFER: %02X:%02X:%02X:%02X\n", scoreBuf[0], scoreBuf[1], scoreBuf[2], scoreBuf[3]);

		if(inGameValue == 0) {
			//printf("MENU \n");
			isInGame = 0;

			if(toSave == 1) {
				toSave = 0;
				printf("SAVE TO DISK !");

				int LOG;
				uint64_t sw = 4096;
				int ret = cellFsOpen("/dev_hdd0/customprx/DAYTONAUSA_EXP.txt", CELL_FS_O_RDWR | /*CELL_FS_O_CREAT*/  CELL_FS_O_APPEND, &LOG, NULL, 0);
				if (ret == 1)
				{
					cellFsClose(LOG);
				} else {
					char buffer[12] = { ' ' };
					sprintf(buffer, "\nXP: %d", score);
					cellFsWrite(LOG, (const void *)buffer, (uint64_t)12, &sw);
					cellFsClose(LOG);
				}
			}
		} else {
			//printf("GAME \n");
			isInGame = 1;

			char res[20];
			float n = value;
			ftoa(n, res, 0);

			score = myAtoi(res);
			if(isInGame == 1 && score > 0) {
				printf("SCORE:  %s \n", res);
				toSave = 1;
			}
		}

		//printf("IN GAME: %d \n", inGameValue);

		sys_timer_usleep(2 * 1000 * 1000); //1 second delay
	}
	//cellMsgDialogOpen(CELL_MSGDIALOG_TYPE_PROGRESSBAR_SINGLE, "DAYTONA USA EXP!", my_dialog2, (void*)0x0000aaab, NULL);
}

// An exported function is needed to generate the project's PRX stub export library
extern "C" int _DAYTONAUSA_EXP_export_function(void)
{
    return CELL_OK;
}

extern "C" int _DAYTONAUSA_EXP_prx_entry(void)
{
	sys_ppu_thread_create(&ogl_main_tid, ogl_main_thread, NULL, 0, 0x4000, SYS_PPU_THREAD_CREATE_JOINABLE, THREAD_NAME);
    return SYS_PRX_RESIDENT;
}
