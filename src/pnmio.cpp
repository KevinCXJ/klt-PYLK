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

//����BMP�ļ�ͷ
//��һ���ṹ�����ͣ��ýṹ�ĳ����ǹ̶��ģ�Ϊ14���ֽڡ��䶨�����£�
typedef struct FileHeader
{
	//unsigned short  bfType;      //λͼ�ļ����ͣ�������0x424D�����ַ�����BM����Ҳ����˵�����еġ�*.bmp���ļ���ͷ�����ֽڶ��ǡ�BM����  
	unsigned int    bfSize;      //λͼ�ļ���С��������14���ֽڡ�  
	unsigned short  bfReserved1; //Windows�����֣��ݲ��á�  
	unsigned short  bfReserved2; //Windows�����֣��ݲ��á�  
	unsigned int    bfOffBits;   //���ļ�ͷ��ʵ�ʵ�λͼ���ݵ�ƫ���ֽ�����bitmap�ļ�ǰ3������(�ļ�ͷ����Ϣͷ����ɫ��)�ĳ���֮�͡� 
}FileHeader;

//����BMP��Ϣͷ
//Ҳ��һ���ṹ�����͵����ݽṹ���ýṹ�ĳ���Ҳ�ǹ̶��ģ�Ϊ40���ֽڡ��䶨�����£�
typedef struct InfoHeader
{
	unsigned int    biSize;          //���ṹ�ĳ��ȣ�Ϊ40���ֽڡ�  
	int             biWidth;         //λͼ�Ŀ�ȣ�������Ϊ��λ��  
	int             biHeight;        //λͼ�ĸ߶ȣ�������Ϊ��λ��  
	unsigned short  biPlanes;        //Ŀ���豸�ļ��𣬱�����1��  
	unsigned short  biBitCount;      //ÿ��������ռ��λ����bit������ֵ����Ϊ1���ڰ�ͼ�񣩡�4��16ɫͼ����8��256ɫ����24�����ɫͼ�����µ�BMP��ʽ֧��32λɫ��  
	unsigned int    biCompression;   //λͼѹ�����ͣ���Ч��ֵΪBI_RGB��δ��ѹ������BI_RLE8��BI_RLE4��BI_BITFILEDS����ΪWindows���峣����������ֻ����δ��ѹ�����������biCompression=BI_RGB��  
	unsigned int    biSizeImage;     //ʵ�ʵ�λͼ����ռ�õ��ֽ�������ֵ�Ĵ�С�ڵ�4����λͼ�������о�����͡�  
	int             biXPelsPerMeter; //ָ��Ŀ���豸��ˮƽ�ֱ��ʣ���λ������/�ס�  
	int             biYPelsPerMeter; //ָ��Ŀ���豸�Ĵ�ֱ�ֱ��ʣ���λ������/�ס�  
	unsigned int    biClrUsed;       //λͼʵ���õ�����ɫ���������ֵΪ�㣬���õ�����ɫ��Ϊ2��biBitCount���ݡ�  
	unsigned int    biClrImportant;  //λͼ��ʾ��������Ҫ����ɫ���������ֵΪ�㣬����Ϊ���е���ɫ������Ҫ�ġ�  
}InfoHeader;

typedef struct tagRGBQUAD{
	BYTE rgbBlue; //����ɫ����ɫ����
	BYTE rgbGreen; //����ɫ����ɫ����
	BYTE rgbRed; //����ɫ�ĺ�ɫ����
	BYTE rgbReserved; //����ֵ
}RGBQUAD;//��ɫ�嶨��

//������Ϣ
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
	int biBitCount = 24;//ÿ��������ռ��λ����bit����
	if (pxlTyp == GRAY)
		biBitCount = 8;//24λ
	else if (pxlTyp == RGB)
		biBitCount = 24;//24λ
	
	int ColorTableSize = 0;//��ɫͼ��û�е�ɫ��
	int DataSizePerLine = (ImageWidth*biBitCount / 8 + 3) / 4 * 4; //ͼ���ʽ�涨һ��ɨ������ռ���ֽ���������4�ı���������4�ı�����Ҫ����������䡣 
	if (BMPfp == NULL)
	{
		printf("�޷���������ͼƬ\n");
		exit(0);
	}
	//дλͼ�ļ�ͷ 
	//bfh.bfType = 0x4D42;
	unsigned short bfType = 0x4D42; //����sizeof(FileHeader)ʱ�����ֽڶ�������(��sizeof(FileHeader)=16,!=14)����˵�������˲���  
	bfh.bfSize = 54 + DataSizePerLine * ImageHeight;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = 54 + ColorTableSize;
	//дλͼ��Ϣͷ
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
	//д���ļ�ͷ����Ϣͷ����ɫ��
	fwrite(&bfType, sizeof(unsigned short), 1, BMPfp);
	fwrite(&bfh, sizeof(FileHeader), 1, BMPfp);//bfhָ��д����ļ�ͷ��fpָ�򱻸�д���ļ�
	fwrite(&bih, sizeof(InfoHeader), 1, BMPfp);//binָ��д�����Ϣͷ��fpָ�򱻸�д���ļ�  
	//fwrite(bColors, sizeof(tagRGBQUAD), 256, BMPfp); //��ɫ��д���ļ�

	return DataSizePerLine;
}

//WriteGrayBMP
int WriteGrayBMP_Head(FILE *BMPfp, int ImageWidth, int ImageHeight, PixelType pxlTyp)
{
	FileHeader bfh;
	InfoHeader bih;
	RGBQUAD strPla[256];//256ɫ��ɫ��
	int biBitCount = 24;//ÿ��������ռ��λ����bit����
	if (pxlTyp == GRAY)
		biBitCount = 8;//24λ
	else if (pxlTyp == RGB)
		biBitCount = 24;//24λ

	int ColorTableSize = 256 * 4;//ע������ǲ�ɫͼ��û�е�ɫ�壬ֵΪ0
	int DataSizePerLine = (ImageWidth*biBitCount / 8 + 3) / 4 * 4; //ͼ���ʽ�涨һ��ɨ������ռ���ֽ���������4�ı���������4�ı�����Ҫ����������䡣 
	if (BMPfp == NULL)
	{
		printf("�޷���������ͼƬ\n");
		exit(0);
	}
	//дλͼ�ļ�ͷ 
	//bfh.bfType = 0x4D42;
	unsigned short bfType = 0x4D42; //����sizeof(FileHeader)ʱ�����ֽڶ�������(��sizeof(FileHeader)=16,!=14)����˵�������˲���  
	bfh.bfSize = 54 + DataSizePerLine * ImageHeight + ColorTableSize;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = 54 + ColorTableSize;
	//дλͼ��Ϣͷ
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
	//д��ɫ��
	memset(strPla, 0, 256 * sizeof(RGBQUAD));
	for (int i = 0; i < 256; i++){
		strPla[i].rgbBlue = i;
		strPla[i].rgbGreen = i;
		strPla[i].rgbRed = i;
		//strPla[i].rgbReserved = 0;
	}

	//д���ļ�ͷ����Ϣͷ����ɫ��
	fwrite(&bfType, sizeof(unsigned short), 1, BMPfp);
	fwrite(&bfh, sizeof(FileHeader), 1, BMPfp);//bfhָ��д����ļ�ͷ��fpָ�򱻸�д���ļ�
	fwrite(&bih, sizeof(InfoHeader), 1, BMPfp);//binָ��д�����Ϣͷ��fpָ�򱻸�д���ļ�  
	fwrite(strPla, sizeof(tagRGBQUAD), 256, BMPfp); //��ɫ��д���ļ�

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
			fwrite(redimg + i*ncols + j, 1, sizeof(BYTE), fp);//ע����������˳�򣺷�����ɫ�ᷢ���仯
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
	//дbmp�ļ�ͷ��λͼ��Ϣͷ����ɫ��
	WriteGrayBMP_Head(fp, ncols, nrows, GRAY);

	//дλͼ��������
	for (int i = nrows - 1; i >= 0; i--)
	{
		for (int j = 0; j < ncols; j++)
		{
			fwrite(img + i*ncols + j, 1, sizeof(BYTE), fp);//grayͼ,�������Ϊ��ֻ��дһ��ͨ��
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
	int count;//ͼƬ�Ŀ�Ⱥ͸߶�
	/* Open file */
	fpi = fopen(fname, "rb");
	if (fpi == NULL)
		KLTError("(bmpGrayReadFile) Can't open file named '%s' for reading\n", fname);

	//�ȶ�ȡ�ļ�����
	WORD bfType;
	fread(&bfType, 1, sizeof(WORD), fpi);
	if (0x4d42 != bfType)
	{
		KLTError("(bmpGrayReadFile) fileType '%s' is not .bmp\n", fname);
	}
	//��ȡbmp�ļ����ļ�ͷ����Ϣͷ
	FileHeader strHead;
	RGBQUAD strPla[256];//256ɫ��ɫ��
	InfoHeader strInfo;
	fread(&strHead, 1, sizeof(FileHeader), fpi);
	fread(&strInfo, 1, sizeof(InfoHeader), fpi);

	//��ȡ��ɫ��
	//���ڵ�ɫ������û��ʹ�õĳ��ϣ���ֱ���ƶ�fp��ָ��fp->_ptr��fp->cnt�����ɹ����д�
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
	//ͼ��ÿһ�е��ֽ���������4��������
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
		
	//����ͼƬ����������
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
