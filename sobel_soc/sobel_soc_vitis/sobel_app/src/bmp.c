#include "bmp.h"
#include "filter.h"
#include "platform.h"
#include "ff.h"
#include "xil_printf.h"

/*

C code referenced from https://github.com/subrata05/image-processing-in-c/

 */
/* Helper: Read little-endian values safely */
static uint32_t readU32LE(const unsigned char *data, int offset)
{
  return (uint32_t)data[offset] |
    ((uint32_t)data[offset + 1] << 8) |
    ((uint32_t)data[offset + 2] << 16) |
    ((uint32_t)data[offset + 3] << 24);
}

const char* fatfs_err_to_string(FRESULT err) {
    switch (err) {
        case FR_OK:                      return "FR_OK (Success)";
        case FR_DISK_ERR:                return "FR_DISK_ERR (A hard error occurred in the low level disk I/O layer)";
        case FR_INT_ERR:                 return "FR_INT_ERR (Assertion failed)";
        case FR_NOT_READY:               return "FR_NOT_READY (The physical drive cannot work)";
        case FR_NO_FILE:                 return "FR_NO_FILE (Could not find the file)";
        case FR_NO_PATH:                 return "FR_NO_PATH (Could not find the path)";
        case FR_INVALID_NAME:            return "FR_INVALID_NAME (The path name format is invalid)";
        case FR_DENIED:                  return "FR_DENIED (Access denied or directory full)";
        case FR_EXIST:                   return "FR_EXIST (Access denied due to prohibited access)";
        case FR_INVALID_OBJECT:          return "FR_INVALID_OBJECT (The file/directory object is invalid)";
        case FR_WRITE_PROTECTED:         return "FR_WRITE_PROTECTED (The physical drive is write protected)";
        case FR_INVALID_DRIVE:           return "FR_INVALID_DRIVE (The logical drive number is invalid)";
        case FR_NOT_ENABLED:             return "FR_NOT_ENABLED (The volume has no work area)";
        case FR_NO_FILESYSTEM:           return "FR_NO_FILESYSTEM (There is no valid FAT volume)";
        case FR_MKFS_ABORTED:            return "FR_MKFS_ABORTED (The f_mkfs() aborted)";
        case FR_TIMEOUT:                 return "FR_TIMEOUT (Could not get a grant to access the volume)";
        case FR_LOCKED:                  return "FR_LOCKED (The operation is rejected according to the file sharing policy)";
        case FR_NOT_ENOUGH_CORE:         return "FR_NOT_ENOUGH_CORE (LFN working buffer could not be allocated)";
        case FR_TOO_MANY_OPEN_FILES:     return "FR_TOO_MANY_OPEN_FILES (Number of open files > FF_FS_LOCK)";
        case FR_INVALID_PARAMETER:       return "FR_INVALID_PARAMETER (Given parameter is invalid)";
        default:                         return "Unknown error";
    }
}
static uint16_t readU16LE(const unsigned char *data, int offset)
{
  return (uint16_t)data[offset] | ((uint16_t)data[offset + 1] << 8);
}

static int32_t readS32LE(const unsigned char *data, int offset)
{
  return (int32_t)readU32LE(data, offset);
}

//const char  *inputFile  = NULL;
//const char  *outputFile = DEFAULT_OUTPUT_FILE;
//static FATFS fatfs;
//FRESULT res;
//TCHAR *Path="0/";

void printUsage(const char *programName)
{
  xil_printf("Usage: %s <input_file> [output_file]\n", programName);
  xil_printf("  input_file   : Source BMP file to load (required)\n");
  xil_printf("  output_file  : Destination BMP file (optional, default: output.bmp)\n");
  xil_printf("\nExamples:\n");
  xil_printf("  %s photo.bmp                    # Output: output.bmp\n", programName);
  xil_printf("  %s photo.bmp copy.bmp          # Output: copy.bmp\n", programName);
}

int bmpLoad(const char *filename, BMPImage_t *img)
{
  //load img block of ory with size of BMP struct all zeros
  memset(img, 0, sizeof(BMPImage_t));
  FIL fp;
  FRESULT res;
  TCHAR *Path="0/";
  uint32_t byte_read_out;

  //read file as binary only, assign to pointer fp
   res = f_open(&fp,filename, FA_READ);
  if (res != FR_OK)
    {
      xil_printf("ERROR: Cannot open input file '%s'\n", filename);
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }

  /* Read file header (14 bytes) */
  res= f_read(&fp,img->fileHeader, BMP_FILE_HEADER_SIZE, byte_read_out);
  if (res != FR_OK)// != BMP_FILE_HEADER_SIZE)
    {
      xil_printf("ERROR: Cannot read file header from '%s'\n", filename);
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }
  xil_printf("%c\n",img->fileHeader[0]);
  xil_printf("%c\n",img->fileHeader[1]);

  /* Verify BMP signature */
  if (img->fileHeader[0] != 'B' || img->fileHeader[1] != 'M')
    {
      xil_printf("ERROR: '%s' is not a valid BMP file (signature mismatch)\n", filename);
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }

  /* Read info header size */
  // each call of fread, file position indicator for the stream is advanced by number of characters read.
  unsigned char sizeBuf[4];
  res = f_read(&fp,sizeBuf,4,byte_read_out);
  xil_printf("%u\n",sizeBuf);
  if ( res != FR_OK)
    {
      xil_printf("ERROR: Cannot read info header size from '%s'\n", filename);
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }
  img->infoHeaderSize = readU32LE(sizeBuf, 0);

  if (img->infoHeaderSize < BMP_MIN_INFO_SIZE)
    {
      xil_printf("ERROR: Unsupported BMP header size (%s) \n", img->infoHeaderSize);
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }
  xil_printf("BMP header size %u \n", img->infoHeaderSize);

  /* Allocate and read full info header */
  img->infoHeader = (unsigned char *)malloc(img->infoHeaderSize);
  if (!img->infoHeader)
    {
      xil_printf("ERROR: memory allocation failed for info header\n");
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }

  memcpy(img->infoHeader, sizeBuf, 4);
  //read rest of bytes for infoHeader
  res = f_read(&fp,img->infoHeader + 4, img->infoHeaderSize - 4,byte_read_out);
  if (res != FR_OK)
    {
      xil_printf("ERROR: Cannot read info header from '%s'\n", filename);
      free(img->infoHeader);
      img->infoHeader = NULL;
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }

  /* Extract fields */
  img->offset = readU32LE(img->fileHeader, 10);
  xil_printf("BMP data offset %u \n", img->offset);
  img->width = readS32LE(img->infoHeader, 4);
  xil_printf("BMP image width %u \n", img->width);
  img->height = readS32LE(img->infoHeader, 8);
  xil_printf("BMP image height %u \n", img->height);
  img->bitDepth = readU16LE(img->infoHeader, 14);
  xil_printf("BMP bits per pixel %u \n", img->bitDepth);

  /* Validate bit depth — only 8, 24, and 32 bpp are supported.
     1bpp and 4bpp pack multiple pixels per byte and require
     nibble/bit-level handling not yet implemented.
     16bpp has sub-format ambiguity (RGB555 vs RGB565) not yet handled. */
  if (img->bitDepth != BMP_SUPPORTED_BPP_8 && img->bitDepth != BMP_SUPPORTED_BPP_24 && img->bitDepth != BMP_SUPPORTED_BPP_32)
    {
      xil_printf("ERROR: Unsupported bit depth %u bpp in '%s'.\n",
              img->bitDepth, filename);
      xil_printf("       Supported formats: 8bpp, 24bpp, 32bpp.\n");
      xil_printf("       1bpp, 4bpp, and 16bpp support is not yet implemented.\n");
      free(img->infoHeader);
      img->infoHeader = NULL;
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }

  /* Read color table if present */
  uint32_t totalHeader = BMP_FILE_HEADER_SIZE + img->infoHeaderSize;
  img->colorTabSize = (int)(img->offset - totalHeader);
  xil_printf("BMP Size of color table:  %u \n", img->colorTabSize);

  if (img->colorTabSize > 0)
    {
      img->colorTab = (unsigned char *)malloc(img->colorTabSize);
      if (!img->colorTab)
        {
          xil_printf("ERROR: Memory allocation failed for color table\n");
          free(img->infoHeader);
          img->infoHeader = NULL;
          f_mount(NULL,Path,0);
          return XST_FAILURE;
        }
      res = f_read(&fp,img->colorTab,img->colorTabSize,byte_read_out);
      if (res != FR_OK)
        {
          xil_printf("ERROR: Cannot read color table from '%s'\n", filename);
          free(img->colorTab);
          free(img->infoHeader);
          img->colorTab = NULL;
          img->infoHeader = NULL;
          f_mount(NULL,Path,0);
          return XST_FAILURE;
        }
    }

  /* Read pixel data */
  //bitmap width * 8bits per byte 
  int rowSize = ((img->width * img->bitDepth + 31) / 32) * 4;
  int pixelSize = rowSize * abs(img->height);
  xil_printf("Row Size: %u\n",rowSize);
  xil_printf("Pixel Size: %u\n",pixelSize);


  img->pixel = (unsigned char *)malloc(pixelSize);
  xil_printf("Pixel Location: %x\n",&img->pixel);
  if (!img->pixel)
    {
      xil_printf("ERROR: Memory allocation failed for pixel data\n");
      free(img->colorTab);
      free(img->infoHeader);
      img->pixel = NULL;
      img->colorTab = NULL;
      img->infoHeader = NULL;
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }

  //move pointer to initial pixel location
  res = f_lseek(&fp, img->offset);
  if (res != FR_OK){
      xil_printf("ERROR: Seek command failed\n");
      free(img->colorTab);
      free(img->infoHeader);
      img->pixel = NULL;
      img->colorTab = NULL;
      img->infoHeader = NULL;
      f_mount(NULL,Path,0);
      return XST_FAILURE;
  }


  xil_printf("size of pixel : %u\n",sizeof(img->pixel));
  xil_printf("size of pixel : %u\n",sizeof(img->infoHeader));
  res = f_read(&fp,img->pixel,307200,byte_read_out);

  if (res != FR_OK)
    {
      xil_printf("ERROR: '%s (code:%d')\n", fatfs_err_to_string(res),res);
      xil_printf("ERROR: Cannot read pixel data from '%s'\n", filename);
      free(img->pixel);
      free(img->colorTab);
      free(img->infoHeader);
      img->pixel = NULL;
      img->colorTab = NULL;
      img->infoHeader = NULL;
      f_mount(NULL,Path,0);
      return XST_FAILURE;
    }
  //8 bits per pixle
  int bytesPerPixel = img->bitDepth/8;
    
  xil_printf("Bytes per Pixel : %u\n",bytesPerPixel);
  unsigned char *px = img->pixel;
  xil_printf("Size Pixel : %u\n",sizeof(*px));
  int srcWidth= img->width;
  int srcHeight = abs(img->height);
  int srcRowSize = ((srcWidth * img->bitDepth +31)/32)*4;
  int numColors = img->colorTabSize/4;
  xil_printf("Num of colors: %u\n",numColors);
  xil_printf("RowSize: %u\n",srcRowSize);

  //0_0 is top left
  //2_0 is top right
/*  unsigned char pixel_0_0; // ,x,y
  unsigned char pixel_1_0; // ,x,y
  unsigned char pixel_2_0; // ,x,y
  unsigned char pixel_0_1; // ,x,y
  unsigned char pixel_1_1; // ,x,y
  unsigned char pixel_2_1; // ,x,y
  unsigned char pixel_0_2; // ,x,y
  unsigned char pixel_1_2; // ,x,y
  unsigned char pixel_2_2; // ,x,y

  for (int y = 0; y<srcHeight;y++){
    unsigned char *row = img->pixel + y*srcRowSize;
    for (int x=0; x<srcWidth;x++){
      unsigned char *px = row + x*bytesPerPixel;
      xil_printf("Pixel Location:(%u,%u),",x+1,srcHeight-y);
      xil_printf("Pixel value: %u\n",px[0]);
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
          
          xil_printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
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
          
          xil_printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
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
          
          xil_printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);

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
          
          xil_printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
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
          
          xil_printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
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
          
          xil_printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);

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
          
          xil_printf(" |%u %u %u|\n |%u %u %u|\n |%u %u %u|\n"
                 ,pixel_0_0,pixel_1_0,pixel_2_0
                 ,pixel_0_1,pixel_1_1,pixel_2_1
                 ,pixel_0_2,pixel_1_2,pixel_2_2);

      }
    }   
  }
  */
    
  res = f_close(&fp);
  if(res != FR_OK) {
      xil_printf("ERROR: Cannot close file '%s'\n", filename);
  }
  return 0;
}

int bmpSave(const char *filename, const BMPImage_t *img)
{
	  FIL fp;
	  FRESULT res;
	  TCHAR *Path="0/";
	  uint32_t byte_read_out;

  res = f_open(&fp,filename, FA_CREATE_ALWAYS);
  if (res != FR_OK)
	{
	  xil_printf("ERROR: '%s (code:%d')\n", fatfs_err_to_string(res),res);
	  xil_printf("ERROR: Cannot create output file '%s'\n", filename);
	  return XST_FAILURE;
	}
  res = f_open(&fp,filename, FA_WRITE);
  if (res != FR_OK)
    {
      xil_printf("ERROR: '%s (code:%d')\n", fatfs_err_to_string(res),res);
      xil_printf("ERROR: Cannot write to file '%s'\n", filename);
      return XST_FAILURE;
    }

  int rowSize = ((img->width * img->bitDepth + 31) / 32) * 4;
  int pixelSize = rowSize * abs(img->height);

  res = f_write(&fp,img->fileHeader,BMP_FILE_HEADER_SIZE, byte_read_out);
  if (res != FR_OK)
    {
      xil_printf("ERROR: Failed to write BMP header to '%s'\n", filename);
      f_close(&fp);
      return XST_FAILURE;
    }
  res = f_write(&fp, img->infoHeader, img->infoHeaderSize, byte_read_out);
  if (res != FR_OK)
    {
      xil_printf("ERROR: Failed to write Info header to '%s'\n", filename);
      f_close(&fp);
      return XST_FAILURE;
    }

/*  if (fwrite(img->fileHeader, 1, BMP_FILE_HEADER_SIZE, fp) != BMP_FILE_HEADER_SIZE ||
      fwrite(img->infoHeader, 1, img->infoHeaderSize, fp) != img->infoHeaderSize)
    {
      fprintf(stderr, "ERROR: Failed to write headers to '%s'\n", filename);
      fclose(fp);
      return -1;
    }
*/

  if (img->colorTabSize > 0)
    {
	  res = f_write(&fp , img->colorTab, img->colorTabSize, byte_read_out);
      if (res != FR_OK)
        {
    	  xil_printf("ERROR: Failed to write color table to '%s'\n", filename);
          f_close(&fp);
          return XST_FAILURE;
        }
    }
//   int bytesPerPixel = img->bitDepth/8;
//
//  //  printf("Bytes per Pixel : %u\n",bytesPerPixel);
//  unsigned char *px = img->pixel;
//  //  printf("Size Pixel : %u\n",sizeof(*px));
//  int srcWidth= img->width;
//  int srcHeight = abs(img->height);
//  int srcRowSize = ((srcWidth * img->bitDepth +31)/32)*4;
//  int numColors = img->colorTabSize/4;
//  for (int y = 0; y<srcHeight;y++){
//    unsigned char *row = img->pixel + y*srcRowSize;
//    for (int x=0; x<srcWidth;x++){
//      unsigned char *px = row + x*bytesPerPixel;
//      px[0] = px[0] /2;
//    }
//  }
  
    
  res = f_write(&fp, img->pixel,pixelSize, byte_read_out);
  if (res != FR_OK)
    {
      xil_printf("ERROR: Failed to write pixel data to '%s'\n", filename);
      f_close(&fp);
      return XST_FAILURE;
    }

  f_close(&fp);
  xil_printf("Image saved successfully to '%s'\n", filename);
  return 0;
}

void bmpFree(BMPImage_t *img)
{
  free(img->infoHeader);
  free(img->colorTab);
  free(img->pixel);
  memset(img, 0, sizeof(BMPImage_t));
}
