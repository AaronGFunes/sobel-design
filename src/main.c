#include <stdio.h>
#include <string.h>
#include "./bmp.h"

#define DEFAULT_OUTPUT_FILE "output.bmp"

int main(){
  const char  *inputFile  = NULL;
  const char  *outputFile = DEFAULT_OUTPUT_FILE;

  inputFile="eye_bw.bmp";
  if (inputFile == NULL)
    {
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
  

  if (bmpSave(outputFile, &srcImg) != 0)
    {
      bmpFree(&srcImg);
      return -1;
    }
    bmpFree(&srcImg);
    printf("Done!\n");
    return 0;
}

