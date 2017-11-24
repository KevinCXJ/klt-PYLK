/*********************************************************************
 * pnmio.c
 *
 * Various routines to manipulate PNM files.
 *********************************************************************/


/* Standard includes */
#include <stdio.h>   /* FILE  */
#include <stdlib.h>  /* malloc(), atoi() */
#include <fstream>
#include "pnmio.h"
/* Our includes */


#ifdef __cplusplus
extern "C" {
#endif
#include "error.h"
#define LENGTH 80

	

/*********************************************************************/

static void _getNextString(
  FILE *fp,
  char *line)
{
  int i;

  line[0] = '\0';

  while (line[0] == '\0')  {
    fscanf(fp, "%s", line);
    i = -1;
    do  {
      i++;
      if (line[i] == '#')  {
        line[i] = '\0';
        while (fgetc(fp) != '\n') ;
      }
    }  while (line[i] != '\0');
  }
}


/*********************************************************************
 * pnmReadHeader
 */

void pnmReadHeader(
  FILE *fp, 
  int *magic, 
  int *ncols, int *nrows, 
  int *maxval)
{
  char line[LENGTH];
	
  /* Read magic number */
  _getNextString(fp, line);
  if (line[0] != 'P')
    KLTError("(pnmReadHeader) Magic number does not begin with 'P', "
             "but with a '%c'", line[0]);
  sscanf(line, "P%d", magic);
	
  /* Read size, skipping comments */
  _getNextString(fp, line);
  *ncols = atoi(line);
  _getNextString(fp, line);
  *nrows = atoi(line);
  if (*ncols < 0 || *nrows < 0 || *ncols > 10000 || *nrows > 10000)
    KLTError("(pnmReadHeader) The dimensions %d x %d are unacceptable",
             *ncols, *nrows);
	
  /* Read maxval, skipping comments */
  _getNextString(fp, line);
  *maxval = atoi(line);
  fread(line, 1, 1, fp); /* Read newline which follows maxval */
	
  if (*maxval != 255)
    KLTWarning("(pnmReadHeader) Maxval is not 255, but %d", *maxval);
}


/*********************************************************************
 * pgmReadHeader
 */

void pgmReadHeader(
  FILE *fp, 
  int *magic, 
  int *ncols, int *nrows, 
  int *maxval)
{
  pnmReadHeader(fp, magic, ncols, nrows, maxval);
  if (*magic != 5)
    KLTError("(pgmReadHeader) Magic number is not 'P5', but 'P%d'", *magic);
}


/*********************************************************************
 * ppmReadHeader
 */

void ppmReadHeader(
  FILE *fp, 
  int *magic, 
  int *ncols, int *nrows, 
  int *maxval)
{
  pnmReadHeader(fp, magic, ncols, nrows, maxval);
  if (*magic != 6)
    KLTError("(ppmReadHeader) Magic number is not 'P6', but 'P%d'", *magic);
}


/*********************************************************************
 * pgmReadHeaderFile
 */

void pgmReadHeaderFile(
  char *fname, 
  int *magic, 
  int *ncols, int *nrows, 
  int *maxval)
{
  FILE *fp;

  /* Open file */
  if ( (fp = fopen(fname, "rb")) == NULL)
    KLTError("(pgmReadHeaderFile) Can't open file named '%s' for reading\n", fname);

  /* Read header */
  pgmReadHeader(fp, magic, ncols, nrows, maxval);

  /* Close file */
  fclose(fp);
}


/*********************************************************************
 * ppmReadHeaderFile
 */

void ppmReadHeaderFile(
  char *fname, 
  int *magic, 
  int *ncols, int *nrows, 
  int *maxval)
{
  FILE *fp;

  /* Open file */
  if ( (fp = fopen(fname, "rb")) == NULL)
    KLTError("(ppmReadHeaderFile) Can't open file named '%s' for reading\n", fname);

  /* Read header */
  ppmReadHeader(fp, magic, ncols, nrows, maxval);

  /* Close file */
  fclose(fp);
}


/*********************************************************************
 * pgmRead
 *
 * NOTE:  If img is NULL, memory is allocated.
 */

unsigned char* pgmRead(
  FILE *fp,
  unsigned char *img,
  int *ncols, int *nrows)
{
  unsigned char *ptr;
  int magic, maxval;
  int i;

  /* Read header */
  pgmReadHeader(fp, &magic, ncols, nrows, &maxval);

  /* Allocate memory, if necessary, and set pointer */
  if (img == NULL)  {
    ptr = (unsigned char *) malloc(*ncols * *nrows * sizeof(char));
    if (ptr == NULL)  
      KLTError("(pgmRead) Memory not allocated");
  }
  else
    ptr = img;

  /* Read binary image data */
  {
    unsigned char *tmpptr = ptr;
    for (i = 0 ; i < *nrows ; i++)  {
      fread(tmpptr, *ncols, 1, fp);
      tmpptr += *ncols;
    }
  }

  return ptr;
}


/*********************************************************************
 * pgmReadFile
 *
 * NOTE:  If img is NULL, memory is allocated.
 */

unsigned char* pgmReadFile(
  char *fname,
  unsigned char *img,
  int *ncols, int *nrows)
{
  unsigned char *ptr;
  FILE *fp;

  /* Open file */
  if ( (fp = fopen(fname, "rb")) == NULL)
    KLTError("(pgmReadFile) Can't open file named '%s' for reading\n", fname);

  /* Read file */
  ptr = pgmRead(fp, img, ncols, nrows);

  /* Close file */
  fclose(fp);

  return ptr;
}


/*********************************************************************
 * pgmWrite
 */

void pgmWrite(
  FILE *fp,
  unsigned char *img, 
  int ncols, 
  int nrows)
{
  int i;

  /* Write header */
  fprintf(fp, "P5\n");
  fprintf(fp, "%d %d\n", ncols, nrows);
  fprintf(fp, "255\n");

  /* Write binary data */
  for (i = 0 ; i < nrows ; i++)  {
    fwrite(img, ncols, 1, fp);
    img += ncols;
  }
}


/*********************************************************************
 * pgmWriteFile
 */

void pgmWriteFile(
  char *fname, 
  unsigned char *img, 
  int ncols, 
  int nrows)
{
  FILE *fp;

  /* Open file */
  if ( (fp = fopen(fname, "wb")) == NULL)
    KLTError("(pgmWriteFile) Can't open file named '%s' for writing\n", fname);

  /* Write to file */
  pgmWrite(fp, img, ncols, nrows);

  /* Close file */
  fclose(fp);
}


/*********************************************************************
 * ppmWrite
 */

void ppmWrite(
  FILE *fp,
  unsigned char *redimg,
  unsigned char *greenimg,
  unsigned char *blueimg,
  int ncols, 
  int nrows)
{
  int i, j;

  /* Write header */
  fprintf(fp, "P6\n");
  fprintf(fp, "%d %d\n", ncols, nrows);
  fprintf(fp, "255\n");

  /* Write binary data */
  for (j = 0 ; j < nrows ; j++)  {
    for (i = 0 ; i < ncols ; i++)  {
      fwrite(redimg, 1, 1, fp); 
      fwrite(greenimg, 1, 1, fp);
      fwrite(blueimg, 1, 1, fp);
      redimg++;  greenimg++;  blueimg++;
    }
  }
}


/*********************************************************************
 * ppmWriteFileRGB
 */

void ppmWriteFileRGB(
  char *fname, 
  unsigned char *redimg,
  unsigned char *greenimg,
  unsigned char *blueimg,
  int ncols, 
  int nrows)
{
  FILE *fp;

  /* Open file */
  if ( (fp = fopen(fname, "wb")) == NULL)
    KLTError("(ppmWriteFileRGB) Can't open file named '%s' for writing\n", fname);

  /* Write to file */
  unsigned char *red = redimg;
  unsigned char *green = greenimg;
  unsigned char *blue = blueimg;
  ppmWrite(fp, red, green, blue, ncols, nrows);

  /* Close file */
  fclose(fp);
}

/*************************************
* bmpWriteFileRGB
*/

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef long LONG;

//定义BMP文件头
//是一个结构体类型，该结构的长度是固定的，为14个字节。其定义如下：
typedef struct FileHeader
{
	//unsigned short  bfType;      //位图文件类型，必须是0x424D，即字符串“BM”，也就是说，所有的“*.bmp”文件的头两个字节都是“BM”。  
	unsigned int    bfSize;      //位图文件大小，包括这14个字节。  
	unsigned short  bfReserved1; //Windows保留字，暂不用。  
	unsigned short  bfReserved2; //Windows保留字，暂不用。  
	unsigned int    bfOffBits;   //从文件头到实际的位图数据的偏移字节数，bitmap文件前3个部分(文件头、信息头、颜色表)的长度之和。 
}FileHeader;

//定义BMP信息头
//也是一个结构体类型的数据结构，该结构的长度也是固定的，为40个字节。其定义如下：
typedef struct InfoHeader
{
	unsigned int    biSize;          //本结构的长度，为40个字节。  
	int             biWidth;         //位图的宽度，以像素为单位。  
	int             biHeight;        //位图的高度，以像素为单位。  
	unsigned short  biPlanes;        //目标设备的级别，必须是1。  
	unsigned short  biBitCount;      //每个像素所占的位数（bit），其值必须为1（黑白图像）、4（16色图）、8（256色）、24（真彩色图），新的BMP格式支持32位色。  
	unsigned int    biCompression;   //位图压缩类型，有效的值为BI_RGB（未经压缩）、BI_RLE8、BI_RLE4、BI_BITFILEDS（均为Windows定义常量）。这里只讨论未经压缩的情况，即biCompression=BI_RGB。  
	unsigned int    biSizeImage;     //实际的位图数据占用的字节数，该值的大小在第4部分位图数据中有具体解释。  
	int             biXPelsPerMeter; //指定目标设备的水平分辨率，单位是像素/米。  
	int             biYPelsPerMeter; //指定目标设备的垂直分辨率，单位是像素/米。  
	unsigned int    biClrUsed;       //位图实际用到的颜色数，如果该值为零，则用到的颜色数为2的biBitCount次幂。  
	unsigned int    biClrImportant;  //位图显示过程中重要的颜色数，如果该值为零，则认为所有的颜色都是重要的。  
}InfoHeader;

typedef struct tagRGBQUAD{
	BYTE rgbBlue; //该颜色的蓝色分量
	BYTE rgbGreen; //该颜色的绿色分量
	BYTE rgbRed; //该颜色的红色分量
	BYTE rgbReserved; //保留值
}RGBQUAD;//调色板定义

//像素信息
typedef struct tagIMAGEDATA
{
	BYTE blue;
	BYTE green;
	BYTE red;
}IMAGEDATA;

int WriteRGBBMP_Head(FILE *BMPfp, int ImageWidth, int ImageHeight, PixelType pxlTyp)
{
	FileHeader bfh;
	InfoHeader bih;
	int biBitCount = 24;//每个像素所占的位数（bit）。
	if (pxlTyp == GRAY)
		biBitCount = 8;//24位
	else if (pxlTyp == RGB)
		biBitCount = 24;//24位
	
	int ColorTableSize = 0;//彩色图像没有调色板
	int DataSizePerLine = (ImageWidth*biBitCount / 8 + 3) / 4 * 4; //图像格式规定一个扫描行所占的字节数必须是4的倍数，不足4的倍数则要对其进行扩充。 
	if (BMPfp == NULL)
	{
		printf("无法创建保存图片\n");
		exit(0);
	}
	//写位图文件头 
	//bfh.bfType = 0x4D42;
	unsigned short bfType = 0x4D42; //由于sizeof(FileHeader)时出现字节对齐现象(即sizeof(FileHeader)=16,!=14)，因此单独输入此部分  
	bfh.bfSize = 54 + DataSizePerLine * ImageHeight;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = 54 + ColorTableSize;
	//写位图信息头
	bih.biSize = 40;
	bih.biWidth = ImageWidth;
	bih.biHeight = ImageHeight;
	bih.biPlanes = 1;
	if (pxlTyp == GRAY)
		bih.biBitCount = 1;
	else if (pxlTyp == RGB)
		bih.biBitCount = 24;

	bih.biCompression = 0;
	bih.biSizeImage = DataSizePerLine*ImageHeight;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
	//写入文件头、信息头、调色板
	fwrite(&bfType, sizeof(unsigned short), 1, BMPfp);
	fwrite(&bfh, sizeof(FileHeader), 1, BMPfp);//bfh指向被写入的文件头，fp指向被改写的文件
	fwrite(&bih, sizeof(InfoHeader), 1, BMPfp);//bin指向被写入的信息头，fp指向被改写的文件  
	//fwrite(bColors, sizeof(tagRGBQUAD), 256, BMPfp); //调色板写入文件

	return DataSizePerLine;
}

//WriteGrayBMP
int WriteGrayBMP_Head(FILE *BMPfp, int ImageWidth, int ImageHeight, PixelType pxlTyp)
{
	FileHeader bfh;
	InfoHeader bih;
	RGBQUAD strPla[256];//256色调色板
	int biBitCount = 24;//每个像素所占的位数（bit）。
	if (pxlTyp == GRAY)
		biBitCount = 8;//24位
	else if (pxlTyp == RGB)
		biBitCount = 24;//24位

	int ColorTableSize = 256 * 4;//注：如果是彩色图，没有调色板，值为0
	int DataSizePerLine = (ImageWidth*biBitCount / 8 + 3) / 4 * 4; //图像格式规定一个扫描行所占的字节数必须是4的倍数，不足4的倍数则要对其进行扩充。 
	if (BMPfp == NULL)
	{
		printf("无法创建保存图片\n");
		exit(0);
	}
	//写位图文件头 
	//bfh.bfType = 0x4D42;
	unsigned short bfType = 0x4D42; //由于sizeof(FileHeader)时出现字节对齐现象(即sizeof(FileHeader)=16,!=14)，因此单独输入此部分  
	bfh.bfSize = 54 + DataSizePerLine * ImageHeight + ColorTableSize;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = 54 + ColorTableSize;
	//写位图信息头
	bih.biSize = 40;
	bih.biWidth = ImageWidth;
	bih.biHeight = ImageHeight;
	bih.biPlanes = 1;
	if (pxlTyp == GRAY)
		bih.biBitCount = 8;
	else if (pxlTyp == RGB)
		bih.biBitCount = 24;

	bih.biCompression = 0;
	bih.biSizeImage = DataSizePerLine*ImageHeight;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
	//写调色板
	memset(strPla, 0, 256 * sizeof(RGBQUAD));
	for (int i = 0; i < 256; i++){
		strPla[i].rgbBlue = i;
		strPla[i].rgbGreen = i;
		strPla[i].rgbRed = i;
		//strPla[i].rgbReserved = 0;
	}

	//写入文件头、信息头、调色板
	fwrite(&bfType, sizeof(unsigned short), 1, BMPfp);
	fwrite(&bfh, sizeof(FileHeader), 1, BMPfp);//bfh指向被写入的文件头，fp指向被改写的文件
	fwrite(&bih, sizeof(InfoHeader), 1, BMPfp);//bin指向被写入的信息头，fp指向被改写的文件  
	fwrite(strPla, sizeof(tagRGBQUAD), 256, BMPfp); //调色板写入文件

	return DataSizePerLine;
}

void bmpWrite(
	FILE *fp,
	unsigned char *redimg,
	unsigned char *greenimg,
	unsigned char *blueimg,
	int ncols,
	int nrows)
{
	WriteRGBBMP_Head(fp, ncols, nrows, RGB);

	for (int i = nrows - 1; i >= 0; i--)
	{
		for (int j = 0; j < ncols; j++)
		{
			fwrite(blueimg + i*ncols + j, 1, sizeof(BYTE), fp);
			fwrite(greenimg + i*ncols + j, 1, sizeof(BYTE), fp);
			fwrite(redimg + i*ncols + j, 1, sizeof(BYTE), fp);//注意三条语句的顺序：否则颜色会发生变化
		}
	}
}

void bmpWriteFileRGB(
	char *fname,
	unsigned char *redimg,
	unsigned char *greenimg,
	unsigned char *blueimg,
	int ncols,
	int nrows)
{
	FILE *fp;

	/* Open file */
	if ((fp = fopen(fname, "wb")) == NULL)
		KLTError("(ppmWriteFileRGB) Can't open file named '%s' for writing\n", fname);

	unsigned char *red = redimg;
	unsigned char *green = greenimg;
	unsigned char *blue = blueimg;
	/* Write to file */
	bmpWrite(fp, red, green, blue, ncols, nrows);

	/* Close file */
	fclose(fp);
}

void bmpGrayWriteFile(
	char *fname,
	unsigned char *img,
	int ncols,
	int nrows)
{
	FILE *fp;

	/* Open file */
	if ((fp = fopen(fname, "wb")) == NULL)
		KLTError("(ppmWriteFileRGB) Can't open file named '%s' for writing\n", fname);

	/* Write to file */
	//写bmp文件头、位图信息头、调色板
	WriteGrayBMP_Head(fp, ncols, nrows, GRAY);

	//写位图像素数据
	for (int i = nrows - 1; i >= 0; i--)
	{
		for (int j = 0; j < ncols; j++)
		{
			fwrite(img + i*ncols + j, 1, sizeof(BYTE), fp);//gray图,可以理解为：只需写一个通道
			//fwrite(greenimg + i*ncols + j, 1, sizeof(BYTE), fp);
			//fwrite(redimg + i*ncols + j, 1, sizeof(BYTE), fp);
		}
	}

	/* Close file */
	fclose(fp);
}

unsigned char *bmpGrayReadFile(
	char *fname,
	unsigned char *img,
	int *ncols, int *nrows)
{
	unsigned char *ptr;
	FILE *fpi;
	int count;//图片的宽度和高度
	/* Open file */
	fpi = fopen(fname, "rb");
	if (fpi == NULL)
		KLTError("(bmpGrayReadFile) Can't open file named '%s' for reading\n", fname);

	//先读取文件类型
	WORD bfType;
	fread(&bfType, 1, sizeof(WORD), fpi);
	if (0x4d42 != bfType)
	{
		KLTError("(bmpGrayReadFile) fileType '%s' is not .bmp\n", fname);
	}
	//读取bmp文件的文件头和信息头
	FileHeader strHead;
	RGBQUAD strPla[256];//256色调色板
	InfoHeader strInfo;
	fread(&strHead, 1, sizeof(FileHeader), fpi);
	fread(&strInfo, 1, sizeof(InfoHeader), fpi);

	//读取调色板
	//对于调色板数据没有使用的场合，经直接移动fp的指针fp->_ptr和fp->cnt，不成功，有错！
	if (strHead.bfOffBits > 54){
		int cnt = (strHead.bfOffBits - 54) / 4;
		for (unsigned int nCounti = 0; nCounti < cnt; nCounti++)
		{
			fread((char *)&(strPla[nCounti].rgbBlue), 1, sizeof(BYTE), fpi);
			fread((char *)&(strPla[nCounti].rgbGreen), 1, sizeof(BYTE), fpi);
			fread((char *)&(strPla[nCounti].rgbRed), 1, sizeof(BYTE), fpi);
			fread((char *)&(strPla[nCounti].rgbReserved), 1, sizeof(BYTE), fpi);
		}
	}

	*ncols = strInfo.biWidth;
	*nrows = strInfo.biHeight;
	//图像每一行的字节数必须是4的整数倍
	*ncols = (*ncols * sizeof(char)+3) / 4 * 4;

	count = *ncols * *nrows;
	if (img == NULL)  {
		ptr = (unsigned char *)malloc(count * sizeof(char));
		if (ptr == NULL)
			KLTError("(pgmRead) Memory not allocated");
		else
			memset(ptr, 0, count*sizeof(unsigned char));
	}
	else
		ptr = img;
		
	//读出图片的像素数据
	for (int i = *nrows - 1; i >= 0; i--)
	{
		fread(ptr + *ncols*i, sizeof(unsigned char)* (*ncols), 1, fpi);
	}
	fclose(fpi);

	return ptr;
}


#ifdef __cplusplus
}
#endif
