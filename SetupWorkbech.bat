@echo off

cd /D "%~dp0"

IF exist "P:\PDArp\" (
	echo Removing existing link P:\PDArp
	rmdir "P:\PDArp\"
)

echo Creating link P:\PDArp
mklink /J "P:\PDArp\" "%cd%\PDArp\"

echo Done