/*********************************************************************
 * klt_util.h
 *********************************************************************/

#ifndef _KLT_UTIL_H_
#define _KLT_UTIL_H_

typedef struct  {
  int ncols;
  int nrows;
  float *data;
}  _KLT_FloatImageRec, *_KLT_FloatImage;

_KLT_FloatImage _KLTCreateFloatImage(
  int ncols, 
  int nrows);

void _KLTFreeFloatImage(
  _KLT_FloatImage);
	
void _KLTPrintSubFloatImage(
  _KLT_FloatImage floatimg,
  int x0, int y0,
  int width, int height);

void _KLTWriteFloatImageToPGM(
  _KLT_FloatImage img,
  char *filename,
  char *bmpflname);

/* for affine mapping */
void _KLTWriteAbsFloatImageToPGM(
  _KLT_FloatImage img,
  char *filename,float scale);

int GetPath(const char *input, char* addr, char* filename, char *filetype);

int checkAndBuildOutputDir(const char *headdir, char *resultdir, const char *dirname);

#endif


