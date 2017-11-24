/*********************************************************************
 * klt_util.c
 *********************************************************************/

/* Standard includes */
#include <assert.h>
#include <stdlib.h>  /* malloc() */
#include <math.h>		/* fabs() */

/* Our includes */
#include "base.h"
#include "error.h"
#include "pnmio.h"
#include "klt.h"
#include "klt_util.h"


/*********************************************************************/

float _KLTComputeSmoothSigma(
  KLT_TrackingContext tc)
{
  return (tc->smooth_sigma_fact * max(tc->window_width, tc->window_height));
}


/*********************************************************************
 * _KLTCreateFloatImage
 */

_KLT_FloatImage _KLTCreateFloatImage(
  int ncols,
  int nrows)
{
  _KLT_FloatImage floatimg;
  int nbytes = sizeof(_KLT_FloatImageRec) +
    ncols * nrows * sizeof(float);

  floatimg = (_KLT_FloatImage)  malloc(nbytes);
  if (floatimg == NULL)
    KLTError("(_KLTCreateFloatImage)  Out of memory");
  floatimg->ncols = ncols;
  floatimg->nrows = nrows;
  floatimg->data = (float *)  (floatimg + 1);

  return(floatimg);
}


/*********************************************************************
 * _KLTFreeFloatImage
 */

void _KLTFreeFloatImage(
  _KLT_FloatImage floatimg)
{
  free(floatimg);
}


/*********************************************************************
 * _KLTPrintSubFloatImage
 */

void _KLTPrintSubFloatImage(
  _KLT_FloatImage floatimg,
  int x0, int y0,
  int width, int height)
{
  int ncols = floatimg->ncols;
  int offset;
  int i, j;

  assert(x0 >= 0);
  assert(y0 >= 0);
  assert(x0 + width <= ncols);
  assert(y0 + height <= floatimg->nrows);

  fprintf(stderr, "\n");
  for (j = 0 ; j < height ; j++)  {
    for (i = 0 ; i < width ; i++)  {
      offset = (j+y0)*ncols + (i+x0);
      fprintf(stderr, "%6.2f ", *(floatimg->data + offset));
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
}
	

/*********************************************************************
 * _KLTWriteFloatImageToPGM
 */

void _KLTWriteFloatImageToPGM(
  _KLT_FloatImage img,
  char *filename,
  char *bmpflname)
{
  int npixs = img->ncols * img->nrows;
  float mmax = -999999.9f, mmin = 999999.9f;
  float fact;
  float *ptr;
  uchar *byteimg, *ptrout;
  int i;

  /* Calculate minimum and maximum values of float image */
  ptr = img->data;
  for (i = 0 ; i < npixs ; i++)  {
    mmax = max(mmax, *ptr);
    mmin = min(mmin, *ptr);
    ptr++;
  }
	
  /* Allocate memory to hold converted image */
  byteimg = (uchar *) malloc(npixs * sizeof(uchar));

  /* Convert image from float to uchar */
  fact = 255.0f / (mmax-mmin);
  ptr = img->data;
  ptrout = byteimg;
  for (i = 0 ; i < npixs ; i++)  {
    *ptrout++ = (uchar) ((*ptr++ - mmin) * fact);
  }

  /* Write uchar image to PGM */
  pgmWriteFile(filename, byteimg, img->ncols, img->nrows);

  bmpGrayWriteFile(bmpflname, byteimg, img->ncols, img->nrows);
  
  /* Free memory */
  free(byteimg);
}

/*********************************************************************
 * _KLTWriteAbsFloatImageToPGM
 */

void _KLTWriteAbsFloatImageToPGM(
  _KLT_FloatImage img,
  char *filename,float scale)
{
  int npixs = img->ncols * img->nrows;
  float fact;
  float *ptr;
  uchar *byteimg, *ptrout;
  int i;
  float tmp;
	
  /* Allocate memory to hold converted image */
  byteimg = (uchar *) malloc(npixs * sizeof(uchar));

  /* Convert image from float to uchar */
  fact = 255.0f / scale;
  ptr = img->data;
  ptrout = byteimg;
  for (i = 0 ; i < npixs ; i++)  {
    tmp = (float) (fabs(*ptr++) * fact);
    if(tmp > 255.0) tmp = 255.0;
    *ptrout++ =  (uchar) tmp;
  }

  /* Write uchar image to PGM */
  pgmWriteFile(filename, byteimg, img->ncols, img->nrows);

  /* Free memory */
  free(byteimg);
}


int GetPath(const char *input, char* addr, char* filename, char *filetype)
{
	if (input == NULL || addr == NULL || filename == NULL || filetype == NULL)
		return -1;
	//获取文件类型
	strncpy(filetype, input + strlen(input) - 4, 4);
	filetype[4] = '\0';

	//获取文件名
	//如果有路径
	if (strchr(input, '/') || strchr(input, '\\'))
	{
		char buf_input[_MAX_PATH];
		char *ptHead, *pt;
		int len = 0;

		strcpy(buf_input, input);
		ptHead = buf_input;
		pt = buf_input + strlen(buf_input) - 4;
		do{
			pt--;//指针回退
			if (pt[0] == '/' || pt[0] == '\\')
				break;
			len++;
		} while (pt > ptHead);
		//文件名
		strncpy(filename, pt + 1, len);
		filename[len] = '\0';
		//文件路径
		strncpy(addr, input, strlen(input) - len - 4);
		addr[strlen(input) - len - 4] = '\0';
	}
	else{//直接是文件，没有路径
		strncpy(filename, input, strlen(input) - 4);
		filename[strlen(input) - 4] = '\0';
		strcpy(addr, "./");//当前路径
	}
	return 0;
}

//输出目录文件夹
int checkAndBuildOutputDir(const char *headdir, char *resultdir, const char *dirname)
{
	int ret = 0;
	if (headdir == NULL || dirname == NULL || resultdir == NULL)
		return -1;
	sprintf(resultdir, "%s%s", headdir, dirname);
	if (_access(resultdir, 0) == -1)//路径不存在，则创建
	{
		ret = _mkdir(resultdir);
	}
	return ret;
}