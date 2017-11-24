**********************************************************************

【目标】
本代码在经典KLT代码基础上，学习金字塔LK光流原理。
代码：实现前后两帧图的100个特征点的追踪（如特征点不足，将少于100个），并输出结果图（包括特征点追踪图、金字塔层图、内部迭代图）。

【简介】
1、KLT原本的相关doc、guidance：见目录“./doc-offical”

2、KLT代码实现的是：
（1）首先，在第k帧图上选取N个特征点；
（2）其次，利用经典金字塔LK光流，求得这些特征点在k+1帧图上的坐标位置（有些点可能追踪不到）
（3）最后，高亮标注这些追踪到的点，并输出到ppm和bmp图中。

3、在KLT基础上作了优化：
（1）增加了example_trk_PYLK.cpp：可以读取灰度图，支持pgm和bmp(仅bmp灰度图)格式；
（2）在KLT源代码基础上，有做不少修改：主要是trackFeatures.c、pnmio.cpp、klt_util.c、klt.c
——如有需要，可以对比查看。

4、详细的PYLK金字塔LK光流可见void KLTTrackFeatures(……）函数；

5、PYLK算法原理：可参看附带资料，包括ppt或"PYLK算法-伪代码.png"（不在代码中）

6、最初的KLT代码，见：http://www.ces.clemson.edu/~stb/klt。（修改不少，但一些核心的函数没变化）

【操作说明】
1、运行run_klt.bat。
输入两帧图：修改run_klt.bat下输入路径。
输入命令参数时，命令参数是两张灰度图（同类型）。可以是两张bmp或两张pgm。
——备注：输出的图像文件是沿用输入的文件名（命令参数中）。

2、输入图片格式：
两张同类型的图，支持bmp、pgm格式（bmp仅限灰度图）。


【注意】
1、输入图像：仅限输入bmp灰度图和pgm图；

2、输入文件的目录可以随意指定，不仅限与此目录"/pic"；
输出的结果图（包括特征点追踪图、金字塔层图、内部迭代图），随着输入目录。查看时请到输入图像的路径下的“/result”中。

3、两个输入图片文件的路径可以不同。输出结果图，见第一个输入参数的同级路径下的“/result”文件夹。

4、输出结果图的命名，沿用输入图的名称。

5、如果多次运行，原本的“/result”下的同名文件将被覆盖，非同名文件仍会保留。


原版**********************************************************************
NOTICE:

This code is now in the public domain.  The Stanford Office of 
Technology Licensing has removed all licensing restrictions.

**********************************************************************

KLT
An implementation of the Kanade-Lucas-Tomasi feature tracker

Version 1.3.4

Authors: Stan Birchfield
         stb@clemson.edu	

         Thorsten Thormaehlen
         thormae@tnt.uni-hannover.de
         (implemented affine code)

         Thanks to many others for various bug fixes.
 
Date: August   30, A.D. 2007
      May      10, A.D. 2007
      March    28, A.D. 2006
      November 21, A.D. 2005
      August   17, A.D. 2005
      June     16, A.D. 2004
      October   7, A.D. 1998

The code can be obtained from http://www.ces.clemson.edu/~stb/klt
(alternatively http://www.vision.stanford.edu/~birch/klt),
where the official manuals reside.  For your convenience, unofficial 
manuals have been placed in the current subdirectory 'doc'.