
@rem    author          Oliver Blaser
@rem    date            26.02.2023
@rem    copyright       GNU GPLv3 - Copyright (c) 2023 Oliver Blaser



setlocal

set /p VERSIONSTR=<dep_vstr.txt
set EXENAME=phodime
set OUTDIRNAME=phodime_win
set OUTDIR=packed\%OUTDIRNAME%
set ARCHIVE=%EXENAME%_win_v%VERSIONSTR%.zip

rmdir /s /q %OUTDIR%

xcopy /i vs\Release\%EXENAME%.exe %OUTDIR%\%EXENAME%\

copy ..\license.txt %OUTDIR%\%EXENAME%\
@rem copy dep_readme.txt %OUTDIR%\%EXENAME%\readme.txt
@echo.>%OUTDIR%\%EXENAME%\readme.txt
@echo.    Photo Directory Merger>>%OUTDIR%\%EXENAME%\readme.txt
@echo.  ==========================>>%OUTDIR%\%EXENAME%\readme.txt
@echo.>>%OUTDIR%\%EXENAME%\readme.txt
@echo.https://github.com/oblaser/photo-directory-merger>>%OUTDIR%\%EXENAME%\readme.txt
@echo.>>%OUTDIR%\%EXENAME%\readme.txt
@echo.phodime v%VERSIONSTR%>>%OUTDIR%\%EXENAME%\readme.txt
@echo.Copyright (c) 2023 Oliver Blaser.>>%OUTDIR%\%EXENAME%\readme.txt
@echo.License: GNU GPLv3 <http://gnu.org/licenses/>.>>%OUTDIR%\%EXENAME%\readme.txt
@echo.This is free software. There is NO WARRANTY.>>%OUTDIR%\%EXENAME%\readme.txt


cd %OUTDIR%
"C:\Program Files\7-Zip\7z.exe" a %ARCHIVE% %EXENAME%\
cd ..\..

del packed\%ARCHIVE%
move %OUTDIR%\%ARCHIVE% packed\%ARCHIVE%

endlocal
