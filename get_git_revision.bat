@echo off
setlocal enabledelayedexpansion

REM Get git revision
for /f "tokens=*" %%i in ('git rev-parse --short HEAD') do set GIT_REVISION=%%i

REM Generate header
set output=GitRevision.generated.cpp
(
    echo #include "GitRevision.h"
    echo const StringView GitRevisionLiteral = U"!GIT_REVISION!";
) > !output!

endlocal
