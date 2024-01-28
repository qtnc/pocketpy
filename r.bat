@echo off
del include\pocketpy\_generated.h
mingw32-make mode=release PK_ENABLE_OS=0 %*
copy libpocketpy.a ..\wxSalonClient\
xcopy include\pocketpy\*.h ..\wxSalonClient\pocketpy\pocketpy\ /D /Y
