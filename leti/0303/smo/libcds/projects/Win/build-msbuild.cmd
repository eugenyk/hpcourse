:: Command line arguments:
:: %1 - Visual C++ version: vc10 (2010), vc11 (2012), vc12 (2013)
:: %2 - configuration to build (Release, Debug etc)

set vcXX=%1
set cfg=%2 

set additionalOptions=
if "%vcXX%"=="vc10" (
    set additionalOptions=/tv:4.0
    goto :make
)
if "%vcXX%"=="vc11" (
    set additionalOptions=/tv:4.0
    goto :make
)

:make
for %%A in (x64 Win32) do (
  msbuild %vcXX%\cds.sln /t:Clean /p:Configuration=%cfg% /p:Platform=%%A %additionalOptions%
  msbuild %vcXX%\cds.sln /t:Build /p:Configuration=%cfg% /p:Platform=%%A /fl /flp:LogFile=%vcXX%_%%A.log;Encoding=UTF-8;Verbosity=minimal /v:minimal /maxcpucount %additionalOptions%
)
