@echo off

cd /D "%~dp0"

IF exist "P:\PDArp\" (
	echo Removing existing link P:\PDArp
	rmdir "P:\PDArp\"
)

echo Creating link P:\PDArp
mklink /J "P:\PDArp\" "%cd%\PDArp\"

IF exist "C:\Program Files (x86)\Steam\steamapps\common\DayZ\PDArp\" (
	echo "Removing existing link C:\Program Files (x86)\Steam\steamapps\common\DayZ\PDArp"
	rmdir "C:\Program Files (x86)\Steam\steamapps\common\DayZ\PDArp\"
)

echo Creating link C:\Program Files (x86)\Steam\steamapps\common\DayZ\PDArp\
mklink /J "C:\Program Files (x86)\Steam\steamapps\common\DayZ\PDArp\" "%cd%\PDArp\"

echo Creating link C:\Users\istar\Documents\DayZServer\PDArp\
mklink /J "C:\Users\istar\Documents\DayZServer\PDArp\" "%cd%\PDArp\"

echo Done