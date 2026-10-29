#include <stdio.h>
#include <string.h>
#include "./bmp.h"
#include "./filter.h"

#define DEFAULT_OUTPUT_FILE "output.bmp"

/*

  C code referenced from https://github.com/subrata05/image-processing-in-c/

*/
int main(){
  const char  *inputFile  = NULL;
  const char  *outputFile = DEFAULT_OUTPUT_FILE;

  inputFile="eye_bw.bmp";
  if (inputFile == NULL){
    fprintf(stderr, "ERROR: No input file specified!\n\n");
    return -1;
  }
  /* ---- Load ---- */
  BMPImage_t srcImg, dstImg;
  printf("Loading: %s\n", inputFile);
  if (bmpLoad(inputFile, &srcImg) != 0)
    return -1;

  printf("Loaded: %dx%d, %d bits/pixel\n",
         srcImg.width, srcImg.height, srcImg.bitDepth);

  //Apply filter
  if(applyFilter(&srcImg,&dstImg) != 0){
    bmpFree(&srcImg);
    return -1;
  }

  
  printf("Result: %dx%d\n", dstImg.width, dstImg.height);

  if (bmpSave(outputFile, &dstImg) != 0)
    {
      bmpFree(&srcImg);
      bmpFree(&dstImg);
      return -1;
    }
  bmpFree(&dstImg);
  
  
  bmpFree(&srcImg);
  printf("Done!\n");
  return 0;
}

