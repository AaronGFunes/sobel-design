#include "./filter.h"
#include <string.h>
#include <stdlib.h>

/* copy src image structure into dst (pixel data included) */
static int cloneImage(const BMPImage_t *src, BMPImage_t *dst)
{
  memset(dst, 0, sizeof(BMPImage_t));

  int srcHeight  = abs(src->height);
  int srcRowSize = ((src->width * src->bitDepth + 31) / 32) * 4;
  int pixelSize  = srcRowSize * srcHeight;

  /* file header */
  memcpy(dst->fileHeader, src->fileHeader, BMP_FILE_HEADER_SIZE);

  /* info header */
  dst->infoHeaderSize = src->infoHeaderSize;
  dst->infoHeader = (unsigned char *)malloc(dst->infoHeaderSize);
  if (!dst->infoHeader)
    {
      fprintf(stderr, "ERROR: Memory allocation failed for info header\n");
      return -1;
    }
  memcpy(dst->infoHeader, src->infoHeader, dst->infoHeaderSize);

  /* color table (present for 8 bpp) */
  dst->colorTabSize = src->colorTabSize;
  if (dst->colorTabSize > 0)
    {
      dst->colorTab = (unsigned char *)malloc(dst->colorTabSize);
      if (!dst->colorTab)
        {
          fprintf(stderr, "ERROR: Memory allocation failed for color table\n");
          free(dst->infoHeader);
          dst->infoHeader = NULL;
          return -1;
        }
      memcpy(dst->colorTab, src->colorTab, dst->colorTabSize);
    }

  /* pixel data */
  dst->pixel = (unsigned char *)malloc(pixelSize);
  if (!dst->pixel)
    {
      fprintf(stderr, "ERROR: Memory allocation failed for pixel data\n");
      free(dst->colorTab);
      free(dst->infoHeader);
      dst->colorTab  = NULL;
      dst->infoHeader = NULL;
      return -1;
    }
  memcpy(dst->pixel, src->pixel, pixelSize);

  dst->width    = src->width;
  dst->height   = src->height;
  dst->bitDepth = src->bitDepth;
  dst->offset   = src->offset;

  return 0;
}

int applyFilter(const BMPImage_t *src,BMPImage_t *dst) {
  if(cloneImage(src,dst) !=0){
    return -1;
  }

  int bytesPerPixel = dst->bitDepth/8;
    
  printf("Bytes per Pixel : %u\n",bytesPerPixel);
  unsigned char *px = dst->pixel;
  printf("Size Pixel : %u\n",sizeof(*px));
  int srcWidth= dst->width;
  int srcHeight = abs(dst->height);
  int srcRowSize = ((srcWidth * dst->bitDepth +31)/32)*4;
  int numColors = dst->colorTabSize/4;
  unsigned char pixel_0_0; // ,x,y
  unsigned char pixel_1_0; // ,x,y
  unsigned char pixel_2_0; // ,x,y
  unsigned char pixel_0_1; // ,x,y
  unsigned char pixel_1_1; // ,x,y
  unsigned char pixel_2_1; // ,x,y
  unsigned char pixel_0_2; // ,x,y
  unsigned char pixel_1_2; // ,x,y
  unsigned char pixel_2_2; // ,x,y
  for (int y = 0; y<srcHeight;y++){
    unsigned char *row = dst->pixel + y*srcRowSize;
    for (int x=0; x<srcWidth;x++){
      unsigned char *px = row + x*bytesPerPixel;
      //      px[0] = px[0] *2;
      printf("Pixel Location:(%u,%u),",x+1,srcHeight-y);
      printf("Pixel value: %u\n",px[0]);        
      if(y <1){
        if(x<1){
          pixel_0_0 = 0;
          pixel_1_0 = px[srcRowSize];
          pixel_2_0 = px[srcRowSize + 1];
          pixel_0_1 = 0;
          pixel_1_1 = px[0];
          pixel_2_1 = px[1];
          pixel_0_2 = 0;
          pixel_1_2 = 0;
          pixel_2_2 = 0;
          
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
        } else if ( x>=srcWidth-1 ) {
          pixel_0_0 = px[srcRowSize - 1];
          pixel_1_0 = px[srcRowSize];
          pixel_2_0 = 0;
          pixel_0_1 = px[-1];
          pixel_1_1 = px[0];
          pixel_2_1 = 0;
          pixel_0_2 = 0;
          pixel_1_2 = 0;
          pixel_2_2 = 0;
          
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);

        } else {
          pixel_0_0 = px[srcRowSize - 1];
          pixel_1_0 = px[srcRowSize];
          pixel_2_0 = px[srcRowSize+1];  
          pixel_0_1 = px[-1];
          pixel_1_1 = px[0];
          pixel_2_1 = px[1];
          pixel_0_2 = 0;
          pixel_1_2 = 0;
          pixel_2_2 = 0;
          
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
          px[0] = 255;
        }        
      } else if (y >= srcHeight-1)
        if(x<1){
          pixel_0_0 = 0;
          pixel_1_0 = 0;
          pixel_2_0 = 0;
          pixel_0_1 = 0;
          pixel_1_1 = px[0];
          pixel_2_1 = px[1];
          pixel_0_2 = 0;
          pixel_1_2 = px[-srcRowSize];
          pixel_2_2 = px[-srcRowSize+1];
          
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
        } else if ( x>=srcWidth-1 ) {
          pixel_0_0 = 0;
          pixel_1_0 = 0;
          pixel_2_0 = 0;
          pixel_0_1 = px[-1];
          pixel_1_1 = px[0];
          pixel_2_1 = 0;
          pixel_0_2 = px[-srcRowSize-1];                
          pixel_1_2 = px[-srcRowSize];  
          pixel_2_2 = 0;
          
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);

        } else {          //get row above                    //column left
          pixel_0_0 = 0;
          pixel_1_0 = 0;
          pixel_2_0 = 0;
          pixel_0_1 = px[-1];
          pixel_1_1 = px[0];
          pixel_2_1 = px[1];
          pixel_0_2 = px[-srcRowSize-1];
          pixel_1_2 = px[-srcRowSize];
          pixel_2_2 = px[-srcRowSize+1];
          
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
          
          px[0] = 255;
        }              
      else {
        pixel_0_0 = px[srcRowSize - 1];
        pixel_1_0 = px[srcRowSize];    
        pixel_2_0 = px[srcRowSize+1];  
        pixel_0_1 = px[-1];
        pixel_1_1 = px[0];
        pixel_2_1 = px[1];
        pixel_0_2 = px[-srcRowSize-1];
        pixel_1_2 = px[-srcRowSize];
        pixel_2_2 = px[-srcRowSize+1];
          
        printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
               ,pixel_0_0,pixel_1_0,pixel_2_0
               ,pixel_0_1,pixel_1_1,pixel_2_1
               ,pixel_0_2,pixel_1_2,pixel_2_2);
      }
    }           
  }
  
  return 0;
}

