#include <stdio.h>
#include <string.h>
#include "bmp.h"
#include "filter.h"
#include "platform.h"
#include "ff.h"
#include "xil_printf.h"
#define DEFAULT_OUTPUT_FILE "output.bmp"

/*

  C code referenced from https://github.com/subrata05/image-processing-in-c/

*/
  const char  *inputFile  = NULL;
  const char  *outputFile = DEFAULT_OUTPUT_FILE;
  static FATFS fatfs;

int main(){
	   init_platform();
	   FRESULT res;
	   TCHAR *Path="0/";

  print("Program Start\n");
  inputFile="eye_bw.bmp";
  if (inputFile == NULL){
	  xil_printf( "ERROR: No input file specified!\n\n");
    return XST_FAILURE;
  }

  res=f_mount(&fatfs,Path,1);
  if(res != FR_OK){
	  xil_printf("Error:Card mount failed(%d)\n\r",res);
	  return XST_FAILURE;
  }


  /* ---- Load ---- */
  BMPImage_t srcImg, dstImg;
  printf("Loading: %s\n", inputFile);
  if (bmpLoad(inputFile, &srcImg) != 0)
    return XST_FAILURE;

  printf("Loaded: %dx%d, %d bits/pixel\n",
         srcImg.width, srcImg.height, srcImg.bitDepth);

  //Apply filter
//  if(applyFilter(&srcImg,&dstImg) != 0){
//    bmpFree(&srcImg);
//    return -1;
//  }

  
  printf("Result: %dx%d\n", srcImg.width, srcImg.height);

  if (bmpSave(outputFile, &srcImg) != 0)
    {
      bmpFree(&srcImg);
      bmpFree(&dstImg);
      return -1;
    }
  bmpFree(&dstImg);
  
  
  bmpFree(&srcImg);
  print("Done!\n");
  cleanup_platform();

  return 0;
}

