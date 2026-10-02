@echo off
rem Launch Gaffer with the Mini-Tech extension loaded.
rem Usage: gaffer-mini.cmd [gaffer args...]   e.g. gaffer-mini.cmd test MiniGafferTest
rem
rem MINITECH_INSTALL  Mini-Tech install dir (default: .install next to this script)
rem GAFFER_BUILD      Gaffer build dir      (default: first "gaffer-build" folder found
rem                                          1 to 4 levels above this script)

setlocal

if not defined MINITECH_INSTALL set "MINITECH_INSTALL=%~dp0.install"

if not defined GAFFER_BUILD (
    for %%P in ("%~dp0..\gaffer-build" "%~dp0..\..\gaffer-build" "%~dp0..\..\..\gaffer-build" "%~dp0..\..\..\..\gaffer-build") do (
        if not defined GAFFER_BUILD if exist "%%~fP\bin\gaffer.cmd" set "GAFFER_BUILD=%%~fP"
    )
)

if not defined GAFFER_BUILD (
    echo ERROR: no gaffer-build folder found above "%~dp0" - set GAFFER_BUILD 1>&2
    exit /b 1
)
if not exist "%GAFFER_BUILD%\bin\gaffer.cmd" (
    echo ERROR: gaffer.cmd not found in "%GAFFER_BUILD%\bin" - set GAFFER_BUILD 1>&2
    exit /b 1
)
if not exist "%MINITECH_INSTALL%\python" (
    echo ERROR: Mini-Tech not installed in "%MINITECH_INSTALL%" - run cmake --install first 1>&2
    exit /b 1
)

rem Deliberately replace (not prepend to) any inherited paths, so a studio-wide
rem copy of Mini-Tech is never loaded alongside the one under test.
set "GAFFER_EXTENSION_PATHS=%MINITECH_INSTALL%"
set "GAFFER_REFERENCE_PATHS=%MINITECH_INSTALL%\nodes"
set "MINITECH_REF_NODES=1"

call "%GAFFER_BUILD%\bin\gaffer.cmd" %*
exit /b %ERRORLEVEL%
