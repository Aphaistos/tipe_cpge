@echo off
gcc client.c -o client.exe -lwinmm -lws2_32 -lm
gcc server.c -o server.exe -lws2_32 -lwinmm -lm

pause