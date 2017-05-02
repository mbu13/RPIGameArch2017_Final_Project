@echo off

mkdir ga4-win64
cd ga4-win64
cmake -G "MinGW Makefiles" -DVIDEO_OPENGLES=0 ../../src/engine
cd ..
