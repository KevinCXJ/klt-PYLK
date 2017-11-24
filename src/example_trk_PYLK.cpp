/**********************************************************************
Finds the 100 best features in an image, and tracks these
features to the next image.  Saves the feature
locations (before and after tracking) to text files and to PPM files, 
and prints the features to the screen.
//——————————————————————
【概述】
KLT中track features的特征点配对时，采用的是金字塔LK光流（PYLK）原理。
为了可视化输出PYLK的迭代过程，此代码是在example1.c的基础上优化。
【说明】
1、采用bmp格式实现可视化（原本example1.c中输出是ppm和pgm格式）；
2、输出结果是前后帧img1和img2中特征点的坐标（*.txt文件）、金字塔层图(./pyramid/)、层内迭代图（./innerStep/）；
3、_trackFeature采用的是PYLK光流的算法原理，可参看PYLK光流的伪代码。
注：_trackFeature有在源代码的基础上修改，接口已变。
**********************************************************************/

#include "pnmio.h"
#include "klt.h"
#include <stdio.h>
#include <iostream>
#include <io.h>  
#include <fstream>  
#include <direct.h>
using namespace std;

#define DEFAULTPATH "../pic/" //仅限于vs下调试。如果是脚本调用exe，需修改路径成："../../pic/"
#define DEFAULT_FILE1 "1"
#define DEFAULT_FILE2 "2"

#ifdef __cplusplus
extern "C" {
#endif

int RunExample_trk_PYLK(int argc, char* argv[])
	{
		unsigned char *img1, *img2;
		KLT_TrackingContext tc;
		KLT_FeatureList fl;
		int nFeatures = 100;
		int ncols, nrows;
		
		//存放输入文件的名称
		char fileName_1[_MAX_FNAME];
		char fileName_2[_MAX_FNAME];
		char dir_1[_MAX_DIR];
		char dir_2[_MAX_DIR];
		char fltype_1[8];
		char fltype_2[8];
		//存放输出路径
		char dir_result[_MAX_PATH];
		//存放输出文件的名称及路径
		char out_ppmfile[_MAX_PATH];
		char out_bmpfile[_MAX_PATH];
		char out_feature[_MAX_PATH];

		//构建context，初始化参数等
		//初始化情况，见klt.c
		tc = KLTCreateTrackingContext();
		KLTPrintTrackingContext(tc);
		fl = KLTCreateFeatureList(nFeatures);

#pragma region 处理img输入输出路径，并读取文件
		//默认输入路径:"../pic/"
		if (argc == 1){	
			strcpy(dir_1, DEFAULTPATH);
			strcpy(fileName_1, DEFAULT_FILE1);
			strcpy(fileName_2, DEFAULT_FILE2);
			strcpy(fltype_1, ".bmp");

			//仅用于vs调试，如果是采用bat脚本调用exe，需修改路径成："../../pic/"
			img1 = bmpGrayReadFile("../pic/1.bmp", NULL, &ncols, &nrows);
			img2 = bmpGrayReadFile("../pic/2.bmp", NULL, &ncols, &nrows);
		}
		//读输入文件.bmp/.pgm
		else if (argc == 3){
			//检查文件是否同类型
			if (0 != strcmp(argv[1] + strlen(argv[1]) - 4,
				argv[2] + strlen(argv[2]) - 4))
			{
				cout << "input err: 2 files type should be same" << endl;
				return 0;
			}				

			//获取文件名,自己拆解输入（可能是路径或直接的文件名）
			GetPath(argv[1],dir_1, fileName_1,fltype_1);
			GetPath(argv[2],dir_2, fileName_2,fltype_2);

#if 0 	/*使用windows API——为了匹配lunux，不采用API！		*/
			//使用windows API——为了匹配lunux，不采用API！
			//使用API接口
			char path_buffer[_MAX_PATH];
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];

			_splitpath(argv[1], drive, dir, fname, ext);
			printf("Path extracted with _splitpath:\n");
			printf("  Drive: %s\n", drive);
			printf("  Dir: %s\n", dir);
			printf("  Filename: %s\n", fname);
			printf("  Ext: %s\n", ext);
#endif	

			if (0 == strcmp(fltype_1, ".bmp"))
			{				
				img1 = bmpGrayReadFile(argv[1], NULL, &ncols, &nrows);
				img2 = bmpGrayReadFile(argv[2], NULL, &ncols, &nrows);
			}
			else if (0 == strcmp(fltype_1, ".pgm"))
			{				
				img1 = pgmReadFile(argv[1], NULL, &ncols, &nrows);
				img2 = pgmReadFile(argv[2], NULL, &ncols, &nrows);
			}
			else{
				cout << "input err:file type should be .bmp or .pgm" << endl;
				return 0;
			}
		}	
		else{//输入参数数据错误
			cout << "input error: numbers of input not match" << endl;
			return 0;
		}
#pragma endregion
		
		//选取相应数量的特征点，数目可在context中设置。
		KLTSelectGoodFeatures(tc, img1, ncols, nrows, fl);
		/*
		//打印特征点
		printf("\nIn first image:\n");
		for (int i = 0; i < fl->nFeatures; i++) {
			printf("Feature #%d:  (%f,%f) with value of %d\n",
				i, fl->feature[i]->x, fl->feature[i]->y,
				fl->feature[i]->val);
		}*/
		
		//检查输出路径
		if (0 != checkAndBuildOutputDir(dir_1, dir_result, "result"))
		{
			cout << "create output dir failed" << endl;
			//KLTError("create output dir failed\n");
			return 0;
		}
		sprintf(out_ppmfile, "%s/%s_feat.ppm", dir_result, fileName_1);
		sprintf(out_bmpfile, "%s/%s_feat.bmp", dir_result, fileName_1);
		KLTWriteFeatureListToPPMandBMP(fl, img1, ncols, nrows, out_ppmfile, out_bmpfile); // "pic/1.ppm");
		sprintf(out_feature, "%s/%s_feat.txt", dir_result, fileName_1);
		KLTWriteFeatureList(fl, out_feature, "%3d");

		//采用金字塔LK光流，在img2上追踪特征点：img2匹配到img1上。
		KLTTrackFeatures(tc, img1, img2, ncols, nrows, fl, dir_result,fileName_1, fileName_2);
		/*
		//打印在img2中追踪到对应于img1的特征点
		printf("\nIn second image:\n");
		for (int i = 0; i < fl->nFeatures; i++) {
			printf("Feature #%d:  (%f,%f) with value of %d\n",
				i, fl->feature[i]->x, fl->feature[i]->y,
				fl->feature[i]->val);
		}*/

		sprintf(out_ppmfile, "%s/%s_feat_trked.ppm", dir_result, fileName_2);
		sprintf(out_bmpfile, "%s/%s_feat_trked.bmp", dir_result, fileName_2);
		KLTWriteFeatureListToPPMandBMP(fl, img2, ncols, nrows, out_ppmfile, out_bmpfile);
		sprintf(out_feature, "%s/%s_feat_trked.txt", dir_result, fileName_2);
		KLTWriteFeatureList(fl, out_feature, "%5.1f");  // text file  
		
		if (img1 != NULL)
			free(img1);
		if (img2 != NULL)
			free(img2);

		//system("pause");
		return 0;
	}
#ifdef __cplusplus
}
#endif

