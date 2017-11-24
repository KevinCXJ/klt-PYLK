/**********************************************************************
Finds the 100 best features in an image, and tracks these
features to the next image.  Saves the feature
locations (before and after tracking) to text files and to PPM files, 
and prints the features to the screen.
//��������������������������������������������
��������
KLT��track features�����������ʱ�����õ��ǽ�����LK������PYLK��ԭ��
Ϊ�˿��ӻ����PYLK�ĵ������̣��˴�������example1.c�Ļ������Ż���
��˵����
1������bmp��ʽʵ�ֿ��ӻ���ԭ��example1.c�������ppm��pgm��ʽ����
2����������ǰ��֡img1��img2������������꣨*.txt�ļ�������������ͼ(./pyramid/)�����ڵ���ͼ��./innerStep/����
3��_trackFeature���õ���PYLK�������㷨ԭ���ɲο�PYLK������α���롣
ע��_trackFeature����Դ����Ļ������޸ģ��ӿ��ѱ䡣
**********************************************************************/

#include "pnmio.h"
#include "klt.h"
#include <stdio.h>
#include <iostream>
#include <io.h>  
#include <fstream>  
#include <direct.h>
using namespace std;

#define DEFAULTPATH "../pic/" //������vs�µ��ԡ�����ǽű�����exe�����޸�·���ɣ�"../../pic/"
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
		
		//��������ļ�������
		char fileName_1[_MAX_FNAME];
		char fileName_2[_MAX_FNAME];
		char dir_1[_MAX_DIR];
		char dir_2[_MAX_DIR];
		char fltype_1[8];
		char fltype_2[8];
		//������·��
		char dir_result[_MAX_PATH];
		//�������ļ������Ƽ�·��
		char out_ppmfile[_MAX_PATH];
		char out_bmpfile[_MAX_PATH];
		char out_feature[_MAX_PATH];

		//����context����ʼ��������
		//��ʼ���������klt.c
		tc = KLTCreateTrackingContext();
		KLTPrintTrackingContext(tc);
		fl = KLTCreateFeatureList(nFeatures);

#pragma region ����img�������·��������ȡ�ļ�
		//Ĭ������·��:"../pic/"
		if (argc == 1){	
			strcpy(dir_1, DEFAULTPATH);
			strcpy(fileName_1, DEFAULT_FILE1);
			strcpy(fileName_2, DEFAULT_FILE2);
			strcpy(fltype_1, ".bmp");

			//������vs���ԣ�����ǲ���bat�ű�����exe�����޸�·���ɣ�"../../pic/"
			img1 = bmpGrayReadFile("../pic/1.bmp", NULL, &ncols, &nrows);
			img2 = bmpGrayReadFile("../pic/2.bmp", NULL, &ncols, &nrows);
		}
		//�������ļ�.bmp/.pgm
		else if (argc == 3){
			//����ļ��Ƿ�ͬ����
			if (0 != strcmp(argv[1] + strlen(argv[1]) - 4,
				argv[2] + strlen(argv[2]) - 4))
			{
				cout << "input err: 2 files type should be same" << endl;
				return 0;
			}				

			//��ȡ�ļ���,�Լ�������루������·����ֱ�ӵ��ļ�����
			GetPath(argv[1],dir_1, fileName_1,fltype_1);
			GetPath(argv[2],dir_2, fileName_2,fltype_2);

#if 0 	/*ʹ��windows API����Ϊ��ƥ��lunux��������API��		*/
			//ʹ��windows API����Ϊ��ƥ��lunux��������API��
			//ʹ��API�ӿ�
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
		else{//����������ݴ���
			cout << "input error: numbers of input not match" << endl;
			return 0;
		}
#pragma endregion
		
		//ѡȡ��Ӧ�����������㣬��Ŀ����context�����á�
		KLTSelectGoodFeatures(tc, img1, ncols, nrows, fl);
		/*
		//��ӡ������
		printf("\nIn first image:\n");
		for (int i = 0; i < fl->nFeatures; i++) {
			printf("Feature #%d:  (%f,%f) with value of %d\n",
				i, fl->feature[i]->x, fl->feature[i]->y,
				fl->feature[i]->val);
		}*/
		
		//������·��
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

		//���ý�����LK��������img2��׷�������㣺img2ƥ�䵽img1�ϡ�
		KLTTrackFeatures(tc, img1, img2, ncols, nrows, fl, dir_result,fileName_1, fileName_2);
		/*
		//��ӡ��img2��׷�ٵ���Ӧ��img1��������
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

