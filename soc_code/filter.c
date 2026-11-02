#include "filter.h"
#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "ff.h"
#include "xparameters.h"
#include "xil_printf.h"
#include "xil_io.h"

#define SOBEL_BASE_ADDR (0x43C00000)

#define START_REG_ADDR (SOBEL_BASE_ADDR + 0x00)
#define PIXEL_0_0_ADDR (SOBEL_BASE_ADDR + 0x04)
#define PIXEL_1_0_ADDR (SOBEL_BASE_ADDR + 0x08)
#define PIXEL_2_0_ADDR (SOBEL_BASE_ADDR + 0x0C)

#define PIXEL_0_1_ADDR (SOBEL_BASE_ADDR + 0x10)
#define PIXEL_1_1_ADDR (SOBEL_BASE_ADDR + 0x14)
#define PIXEL_2_1_ADDR (SOBEL_BASE_ADDR + 0x18)
#define PIXEL_0_2_ADDR (SOBEL_BASE_ADDR + 0x1C)

#define PIXEL_1_2_ADDR  (SOBEL_BASE_ADDR + 0x20)
#define PIXEL_2_2_ADDR  (SOBEL_BASE_ADDR + 0x24)
#define PIXEL_OUT_ADDR  (SOBEL_BASE_ADDR + 0x28)
#define PIXEL_DONE_ADDR (SOBEL_BASE_ADDR + 0x2C)


/* copy src image structure into dst (pixel data included) */
static int cloneImage(const BMPImage_t *src, BMPImage_t *dst)
{
  TCHAR *Path="0/";
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
      xil_printf("ERROR: Memory allocation failed for info header\n");
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }
  memcpy(dst->infoHeader, src->infoHeader, dst->infoHeaderSize);

  /* color table (present for 8 bpp) */
  dst->colorTabSize = src->colorTabSize;
  if (dst->colorTabSize > 0)
    {
      dst->colorTab = (unsigned char *)malloc(dst->colorTabSize);
      if (!dst->colorTab)
        {
          xil_printf("ERROR: Memory allocation failed for color table\n");
          free(dst->infoHeader);
          dst->infoHeader = NULL;
          return XST_FAILURE;
        }
      memcpy(dst->colorTab, src->colorTab, dst->colorTabSize);
    }

  /* pixel data */
  dst->pixel = (unsigned char *)malloc(pixelSize);
  if (!dst->pixel)
    {
      xil_printf("ERROR: Memory allocation failed for pixel data\n");
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


#define DONE_READY_MASK 0x00000001

void wait_for_done_signal(){
	while((Xil_In32(PIXEL_DONE_ADDR) & DONE_READY_MASK)==0){
	}
}



void filter_call(){
/*
    Xil_Out32(PIXEL_0_0_ADDR,pixel_0_0);
    Xil_Out32(PIXEL_1_0_ADDR,pixel_1_0);
    Xil_Out32(PIXEL_2_0_ADDR,pixel_2_0);
    Xil_Out32(PIXEL_0_1_ADDR,pixel_0_1);
    Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
    Xil_Out32(PIXEL_2_1_ADDR,pixel_2_1);
    Xil_Out32(PIXEL_0_2_ADDR,pixel_0_2);
    Xil_Out32(PIXEL_1_2_ADDR,pixel_1_2);
    Xil_Out32(PIXEL_2_2_ADDR,pixel_2_2);
   */
/*    Xil_Out32(PIXEL_0_0_ADDR,pixel_2_2);
    Xil_Out32(PIXEL_1_0_ADDR,pixel_1_2);
    Xil_Out32(PIXEL_2_0_ADDR,pixel_0_2);
    Xil_Out32(PIXEL_0_1_ADDR,pixel_2_1);
    Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
    Xil_Out32(PIXEL_2_1_ADDR,pixel_0_1);
    Xil_Out32(PIXEL_0_2_ADDR,pixel_2_0);
    Xil_Out32(PIXEL_1_2_ADDR,pixel_1_0);
    Xil_Out32(PIXEL_2_2_ADDR,pixel_0_0);
    Xil_Out32(START_REG_ADDR,1);
    wait_for_done_signal();
    Xil_Out32(START_REG_ADDR,0);
    */
}

int applyFilter( BMPImage_t *src, BMPImage_t *dst) {
  if(cloneImage(src,dst) !=0){
    return -1;
  }

	unsigned char pixel_0_0; // ,x,y
	unsigned char pixel_1_0; // ,x,y
	unsigned char pixel_2_0; // ,x,y
	unsigned char pixel_0_1; // ,x,y
	unsigned char pixel_1_1; // ,x,y
	unsigned char pixel_2_1; // ,x,y
	unsigned char pixel_0_2; // ,x,y
	unsigned char pixel_1_2; // ,x,y
	unsigned char pixel_2_2; // ,x,y
	uint32_t pixel_out_reg;
	uint32_t pixel_start_reg;
	uint32_t pixel_done_reg;
    uint32_t pixel_0_0_reg = Xil_In32(PIXEL_0_0_ADDR);
    uint32_t pixel_1_0_reg = Xil_In32(PIXEL_1_0_ADDR);
    uint32_t pixel_2_0_reg = Xil_In32(PIXEL_2_0_ADDR);
    uint32_t pixel_0_1_reg = Xil_In32(PIXEL_0_1_ADDR);
    uint32_t pixel_1_1_reg = Xil_In32(PIXEL_1_1_ADDR);
    uint32_t pixel_2_1_reg = Xil_In32(PIXEL_2_1_ADDR);
    uint32_t pixel_0_2_reg = Xil_In32(PIXEL_0_2_ADDR);
    uint32_t pixel_1_2_reg = Xil_In32(PIXEL_1_2_ADDR);
    uint32_t pixel_2_2_reg = Xil_In32(PIXEL_2_2_ADDR);

  int bytesPerPixel = src->bitDepth/8;
    
  unsigned char *px = src->pixel;
  unsigned char *dst_px = dst->pixel;
  int srcWidth= src->width;
  int srcHeight = abs(src->height);
  int srcRowSize = ((srcWidth * src->bitDepth +31)/32)*4;
  int numColors = src->colorTabSize/4;

  for (int y = 0; y<srcHeight;y++){
	    unsigned char *row = src->pixel + y*srcRowSize;
	    unsigned char *dst_row = dst->pixel + y*srcRowSize;
    for (int x=0; x<srcWidth;x++){
        unsigned char *px = row + x*bytesPerPixel;
        unsigned char *dst_px = dst_row + x*bytesPerPixel;
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
          
/*          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);

          xil_printf("Testing Sobel Filter\n"); */
          Xil_Out32(PIXEL_0_0_ADDR,pixel_0_0);
          Xil_Out32(PIXEL_1_0_ADDR,pixel_1_0);
          Xil_Out32(PIXEL_2_0_ADDR,pixel_2_0);
          Xil_Out32(PIXEL_0_1_ADDR,pixel_0_1);
          Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
          Xil_Out32(PIXEL_2_1_ADDR,pixel_2_1);
          Xil_Out32(PIXEL_0_2_ADDR,pixel_0_2);
          Xil_Out32(PIXEL_1_2_ADDR,pixel_1_2);
          Xil_Out32(PIXEL_2_2_ADDR,pixel_2_2);

          pixel_start_reg = Xil_In32(START_REG_ADDR);
          pixel_1_1_reg = Xil_In32(PIXEL_1_1_ADDR);
          pixel_out_reg = Xil_In32(PIXEL_OUT_ADDR);
          pixel_done_reg =Xil_In32(PIXEL_DONE_ADDR);

/*          xil_printf("Start Reg: %d \n",pixel_start_reg);
          xil_printf("Pixel_1_1: %d \n",pixel_1_1_reg);
          xil_printf("Pixel Out: %d \n",pixel_out_reg);
          xil_printf("Done Reg:  %d \n",pixel_done_reg);
*/
          Xil_Out32(START_REG_ADDR,1);

//          pixel_start_reg = Xil_In32(START_REG_ADDR);
 //         pixel_1_1_reg = Xil_In32(PIXEL_1_1_ADDR);
//          xil_printf("Start Reg: %d \n",pixel_start_reg);
//         xil_printf("Pixel_1_1: %d \n",pixel_1_1_reg);
          wait_for_done_signal();
          Xil_Out32(START_REG_ADDR,0);
          pixel_out_reg = Xil_In32(PIXEL_OUT_ADDR);
          pixel_done_reg =Xil_In32(PIXEL_DONE_ADDR);
//          xil_printf("Pixel Out: %d \n",pixel_out_reg);
//          xil_printf("Done Reg:  %d \n",pixel_done_reg);
//          px[0] = pixel_out_reg;
          dst_px[0] = pixel_out_reg;



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

          Xil_Out32(PIXEL_0_0_ADDR,pixel_0_0);
          Xil_Out32(PIXEL_1_0_ADDR,pixel_1_0);
          Xil_Out32(PIXEL_2_0_ADDR,pixel_2_0);
          Xil_Out32(PIXEL_0_1_ADDR,pixel_0_1);
          Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
          Xil_Out32(PIXEL_2_1_ADDR,pixel_2_1);
          Xil_Out32(PIXEL_0_2_ADDR,pixel_0_2);
          Xil_Out32(PIXEL_1_2_ADDR,pixel_1_2);
          Xil_Out32(PIXEL_2_2_ADDR,pixel_2_2);
          Xil_Out32(START_REG_ADDR,1);
          wait_for_done_signal();
          Xil_Out32(START_REG_ADDR,0);

          pixel_out_reg = Xil_In32(PIXEL_OUT_ADDR);
          pixel_done_reg =Xil_In32(PIXEL_DONE_ADDR);
//          px[0] = pixel_out_reg;
          /*
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
*/
          dst_px[0] = pixel_out_reg;
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
/*
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
                 */
//          px[0] = 255;
          Xil_Out32(PIXEL_0_0_ADDR,pixel_0_0);
          Xil_Out32(PIXEL_1_0_ADDR,pixel_1_0);
          Xil_Out32(PIXEL_2_0_ADDR,pixel_2_0);
          Xil_Out32(PIXEL_0_1_ADDR,pixel_0_1);
          Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
          Xil_Out32(PIXEL_2_1_ADDR,pixel_2_1);
          Xil_Out32(PIXEL_0_2_ADDR,pixel_0_2);
          Xil_Out32(PIXEL_1_2_ADDR,pixel_1_2);
          Xil_Out32(PIXEL_2_2_ADDR,pixel_2_2);
          Xil_Out32(START_REG_ADDR,1);
          wait_for_done_signal();
          Xil_Out32(START_REG_ADDR,0);

          pixel_out_reg = Xil_In32(PIXEL_OUT_ADDR);
          pixel_done_reg =Xil_In32(PIXEL_DONE_ADDR);
          dst_px[0] = pixel_out_reg;
//          px[0] = pixel_out_reg;
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

          Xil_Out32(PIXEL_0_0_ADDR,pixel_0_0);
          Xil_Out32(PIXEL_1_0_ADDR,pixel_1_0);
          Xil_Out32(PIXEL_2_0_ADDR,pixel_2_0);
          Xil_Out32(PIXEL_0_1_ADDR,pixel_0_1);
          Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
          Xil_Out32(PIXEL_2_1_ADDR,pixel_2_1);
          Xil_Out32(PIXEL_0_2_ADDR,pixel_0_2);
          Xil_Out32(PIXEL_1_2_ADDR,pixel_1_2);
          Xil_Out32(PIXEL_2_2_ADDR,pixel_2_2);
          Xil_Out32(START_REG_ADDR,1);
          wait_for_done_signal();
          Xil_Out32(START_REG_ADDR,0);
          pixel_out_reg = Xil_In32(PIXEL_OUT_ADDR);
          pixel_done_reg =Xil_In32(PIXEL_DONE_ADDR);
          dst_px[0] = pixel_out_reg;
//          px[0] = pixel_out_reg;
/*
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
*/
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
          Xil_Out32(PIXEL_0_0_ADDR,pixel_0_0);
          Xil_Out32(PIXEL_1_0_ADDR,pixel_1_0);
          Xil_Out32(PIXEL_2_0_ADDR,pixel_2_0);
          Xil_Out32(PIXEL_0_1_ADDR,pixel_0_1);
          Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
          Xil_Out32(PIXEL_2_1_ADDR,pixel_2_1);
          Xil_Out32(PIXEL_0_2_ADDR,pixel_0_2);
          Xil_Out32(PIXEL_1_2_ADDR,pixel_1_2);
          Xil_Out32(PIXEL_2_2_ADDR,pixel_2_2);
          Xil_Out32(START_REG_ADDR,1);
          wait_for_done_signal();
          Xil_Out32(START_REG_ADDR,0);
          pixel_out_reg = Xil_In32(PIXEL_OUT_ADDR);
          pixel_done_reg =Xil_In32(PIXEL_DONE_ADDR);
          dst_px[0] = pixel_out_reg;
//          px[0] = pixel_out_reg;
/*
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
*/
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
/*
          printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);
 */
          Xil_Out32(PIXEL_0_0_ADDR,pixel_0_0);
          Xil_Out32(PIXEL_1_0_ADDR,pixel_1_0);
          Xil_Out32(PIXEL_2_0_ADDR,pixel_2_0);
          Xil_Out32(PIXEL_0_1_ADDR,pixel_0_1);
          Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
          Xil_Out32(PIXEL_2_1_ADDR,pixel_2_1);
          Xil_Out32(PIXEL_0_2_ADDR,pixel_0_2);
          Xil_Out32(PIXEL_1_2_ADDR,pixel_1_2);
          Xil_Out32(PIXEL_2_2_ADDR,pixel_2_2);
          Xil_Out32(START_REG_ADDR,1);
          wait_for_done_signal();
          Xil_Out32(START_REG_ADDR,0);
          pixel_out_reg = Xil_In32(PIXEL_OUT_ADDR);
          pixel_done_reg =Xil_In32(PIXEL_DONE_ADDR);
          dst_px[0] = pixel_out_reg;
//          px[0] = pixel_out_reg;
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

        Xil_Out32(PIXEL_0_0_ADDR,pixel_0_0);
        Xil_Out32(PIXEL_1_0_ADDR,pixel_1_0);
        Xil_Out32(PIXEL_2_0_ADDR,pixel_2_0);
        Xil_Out32(PIXEL_0_1_ADDR,pixel_0_1);
        Xil_Out32(PIXEL_1_1_ADDR,pixel_1_1);
        Xil_Out32(PIXEL_2_1_ADDR,pixel_2_1);
        Xil_Out32(PIXEL_0_2_ADDR,pixel_0_2);
        Xil_Out32(PIXEL_1_2_ADDR,pixel_1_2);
        Xil_Out32(PIXEL_2_2_ADDR,pixel_2_2);
        Xil_Out32(START_REG_ADDR,1);

        wait_for_done_signal();
        Xil_Out32(START_REG_ADDR,0);

        pixel_out_reg = Xil_In32(PIXEL_OUT_ADDR);
        pixel_done_reg =Xil_In32(PIXEL_DONE_ADDR);

        dst_px[0] = pixel_out_reg;
      }
    }           
  }
  
  return 0;
}

