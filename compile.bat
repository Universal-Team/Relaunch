@echo off
color 0f
echo.
SET /p "OutputDevice=Type for which device the files will be generated. (3DS, DSi, Flashcart) : "
echo.
make %OutputDevice%
echo.
pause