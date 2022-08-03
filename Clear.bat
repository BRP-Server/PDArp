@echo off

cd /D "%~dp0"

IF exist "P:\PDArp\" (
	echo Removing existing link P:\PDArp
	rmdir "P:\PDArp\"
)

IF exist "C:\Program Files (x86)\Steam\steamapps\common\DayZ\PDArp\" (
	echo Removing existing link C:\Program Files (x86)\Steam\steamapps\common\DayZ\PDArp\
	rmdir "C:\Program Files (x86)\Steam\steamapps\common\DayZ\PDArp\"
)

IF exist "C:\Users\istar\Documents\DayZServer\PDArp\" (
	echo Removing existing link C:\Users\istar\Documents\DayZServer\PDArp\
	rmdir "C:\Users\istar\Documents\DayZServer\PDArp\"
)

echo Done