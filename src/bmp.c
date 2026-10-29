#include "./bmp.h"
#include "./filter.h"

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

static uint16_t readU16LE(const unsigned char *data, int offset)
{
  return (uint16_t)data[offset] | ((uint16_t)data[offset + 1] << 8);
}

static int32_t readS32LE(const unsigned char *data, int offset)
{
  return (int32_t)readU32LE(data, offset);
}

void printUsage(const char *programName)
{
  printf("Usage: %s <input_file> [output_file]\n", programName);
  printf("  input_file   : Source BMP file to load (required)\n");
  printf("  output_file  : Destination BMP file (optional, default: output.bmp)\n");
  printf("\nExamples:\n");
  printf("  %s photo.bmp                    # Output: output.bmp\n", programName);
  printf("  %s photo.bmp copy.bmp          # Output: copy.bmp\n", programName);
}

int bmpLoad(const char *filename, BMPImage_t *img)
{
  //load img block of memory with size of BMP struct all zeros
  memset(img, 0, sizeof(BMPImage_t));

  //read file as binary only, assign to pointer fp
  FILE *fp = fopen(filename, "rb");
  if (!fp)
    {
      fprintf(stderr, "ERROR: Cannot open input file '%s'\n", filename);
      return -1;
    }

  /* Read file header (14 bytes) */
  if (fread(img->fileHeader, 1, BMP_FILE_HEADER_SIZE, fp) != BMP_FILE_HEADER_SIZE)
    {
      fprintf(stderr, "ERROR: Cannot read file header from '%s'\n", filename);
      fclose(fp);
      return -1;
    }

  /* Verify BMP signature */
  if (img->fileHeader[0] != 'B' || img->fileHeader[1] != 'M')
    {
      fprintf(stderr, "ERROR: '%s' is not a valid BMP file (signature mismatch)\n", filename);
      fclose(fp);
      return -1;
    }

  /* Read info header size */
  // each call of fread, file position indicator for the stream is advanced by number of characters read.
  unsigned char sizeBuf[4];
  if (fread(sizeBuf, 1, 4, fp) != 4)
    {
      fprintf(stderr, "ERROR: Cannot read info header size from '%s'\n", filename);
      fclose(fp);
      return -1;
    }
  img->infoHeaderSize = readU32LE(sizeBuf, 0);

  if (img->infoHeaderSize < BMP_MIN_INFO_SIZE)
    {
      fprintf(stderr, "ERROR: Unsupported BMP header size (%s) \n", img->infoHeaderSize);
      fclose(fp);
      return -1;
    }
  printf("BMP header size %u \n", img->infoHeaderSize);

  /* Allocate and read full info header */
  img->infoHeader = (unsigned char *)malloc(img->infoHeaderSize);
  if (!img->infoHeader)
    {
      fprintf(stderr, "ERROR: Memory allocation failed for info header\n");
      fclose(fp);
      return -1;
    }

  memcpy(img->infoHeader, sizeBuf, 4);
  //read rest of bytes for infoHeader
  if (fread(img->infoHeader + 4, 1, img->infoHeaderSize - 4, fp) != img->infoHeaderSize - 4)
    {
      fprintf(stderr, "ERROR: Cannot read info header from '%s'\n", filename);
      free(img->infoHeader);
      img->infoHeader = NULL;
      fclose(fp);
      return -1;
    }

  /* Extract fields */
  img->offset = readU32LE(img->fileHeader, 10);
  printf("BMP data offset %u \n", img->offset);
  img->width = readS32LE(img->infoHeader, 4);
  printf("BMP image width %u \n", img->width);
  img->height = readS32LE(img->infoHeader, 8);
  printf("BMP image height %u \n", img->height);
  img->bitDepth = readU16LE(img->infoHeader, 14);
  printf("BMP bits per pixel %u \n", img->bitDepth);

  /* Validate bit depth — only 8, 24, and 32 bpp are supported.
     1bpp and 4bpp pack multiple pixels per byte and require
     nibble/bit-level handling not yet implemented.
     16bpp has sub-format ambiguity (RGB555 vs RGB565) not yet handled. */
  if (img->bitDepth != BMP_SUPPORTED_BPP_8 && img->bitDepth != BMP_SUPPORTED_BPP_24 && img->bitDepth != BMP_SUPPORTED_BPP_32)
    {
      fprintf(stderr, "ERROR: Unsupported bit depth %u bpp in '%s'.\n",
              img->bitDepth, filename);
      fprintf(stderr, "       Supported formats: 8bpp, 24bpp, 32bpp.\n");
      fprintf(stderr, "       1bpp, 4bpp, and 16bpp support is not yet implemented.\n");
      free(img->infoHeader);
      img->infoHeader = NULL;
      fclose(fp);
      return -1;
    }

  /* Read color table if present */
  uint32_t totalHeader = BMP_FILE_HEADER_SIZE + img->infoHeaderSize;
  img->colorTabSize = (int)(img->offset - totalHeader);
  printf("BMP Size of color table:  %u \n", img->colorTabSize);

  if (img->colorTabSize > 0)
    {
      img->colorTab = (unsigned char *)malloc(img->colorTabSize);
      if (!img->colorTab)
        {
          fprintf(stderr, "ERROR: Memory allocation failed for color table\n");
          free(img->infoHeader);
          img->infoHeader = NULL;
          fclose(fp);
          return -1;
        }
      if (fread(img->colorTab, 1, img->colorTabSize, fp) != (size_t)img->colorTabSize)
        {
          fprintf(stderr, "ERROR: Cannot read color table from '%s'\n", filename);
          free(img->colorTab);
          free(img->infoHeader);
          img->colorTab = NULL;
          img->infoHeader = NULL;
          fclose(fp);
          return -1;
        }
    }

  /* Read pixel data */
  //bitmap width * 8bits per byte 
  int rowSize = ((img->width * img->bitDepth + 31) / 32) * 4;
  int pixelSize = rowSize * abs(img->height);

  img->pixel = (unsigned char *)malloc(pixelSize);
  if (!img->pixel)
    {
      fprintf(stderr, "ERROR: Memory allocation failed for pixel data\n");
      free(img->colorTab);
      free(img->infoHeader);
      img->pixel = NULL;
      img->colorTab = NULL;
      img->infoHeader = NULL;
      fclose(fp);
      return -1;
    }

  //move pointer to initial pixel location
  fseek(fp, img->offset, SEEK_SET);
  if (fread(img->pixel, 1, pixelSize, fp) != (size_t)pixelSize)
    {
      fprintf(stderr, "ERROR: Cannot read pixel data from '%s'\n", filename);
      free(img->pixel);
      free(img->colorTab);
      free(img->infoHeader);
      img->pixel = NULL;
      img->colorTab = NULL;
      img->infoHeader = NULL;
      fclose(fp);
      return -1;
    }
  //8 bits per pixle
  int bytesPerPixel = img->bitDepth/8;
    
  printf("Bytes per Pixel : %u\n",bytesPerPixel);
  unsigned char *px = img->pixel;
  printf("Size Pixel : %u\n",sizeof(*px));
  int srcWidth= img->width;
  int srcHeight = abs(img->height);
  int srcRowSize = ((srcWidth * img->bitDepth +31)/32)*4;
  int numColors = img->colorTabSize/4;
  printf("Num of colors: %u\n",numColors);
  printf("RowSize: %u\n",srcRowSize);

  //0_0 is top left
  //2_0 is top right
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
    unsigned char *row = img->pixel + y*srcRowSize;
    for (int x=0; x<srcWidth;x++){
      unsigned char *px = row + x*bytesPerPixel;
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
    
  fclose(fp);
  return 0;
}

int bmpSave(const char *filename, const BMPImage_t *img)
{
  FILE *fp = fopen(filename, "wb");
  if (!fp)
    {
      fprintf(stderr, "ERROR: Cannot create output file '%s'\n", filename);
      return -1;
    }

  int rowSize = ((img->width * img->bitDepth + 31) / 32) * 4;
  int pixelSize = rowSize * abs(img->height);

  if (fwrite(img->fileHeader, 1, BMP_FILE_HEADER_SIZE, fp) != BMP_FILE_HEADER_SIZE ||
      fwrite(img->infoHeader, 1, img->infoHeaderSize, fp) != img->infoHeaderSize)
    {
      fprintf(stderr, "ERROR: Failed to write headers to '%s'\n", filename);
      fclose(fp);
      return -1;
    }

  if (img->colorTabSize > 0)
    {
      if (fwrite(img->colorTab, 1, img->colorTabSize, fp) != (size_t)img->colorTabSize)
        {
          fprintf(stderr, "ERROR: Failed to write color table to '%s'\n", filename);
          fclose(fp);
          return -1;
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
  
    

  if (fwrite(img->pixel, 1, pixelSize, fp) != (size_t)pixelSize)
    {
      fprintf(stderr, "ERROR: Failed to write pixel data to '%s'\n", filename);
      fclose(fp);
      return -1;
    }

  fclose(fp);
  printf("Image saved successfully to '%s'\n", filename);
  return 0;
}

void bmpFree(BMPImage_t *img)
{
  free(img->infoHeader);
  free(img->colorTab);
  free(img->pixel);
  memset(img, 0, sizeof(BMPImage_t));
}
