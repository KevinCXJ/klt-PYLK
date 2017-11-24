@ cd /d %cd%
@ echo off

cd projects/Debug 
::assgin input: the dir of 2 same-type images(type can be pgm or bmp)
Klt.exe ../../pic/1.bmp ../../pic/2.bmp
::Klt.exe ../../pic/1.pgm ../../pic/2.pgm
pause

