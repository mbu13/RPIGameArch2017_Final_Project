@echo off

mkdir ga4-win64
cd ga4-win64
cmake -G "Visual Studio 14 2015 Win64" ../../src/engine
cd ..
