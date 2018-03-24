#!/bin/bash

g++ DataDetector.cpp fields.cpp ConfigParser.cpp Analyzer.cpp -o prg -ltrap -lpcap -lunirec --std=c++11 -Wno-write-strings -pthread
#g++ siot-detector.cpp fields.cpp ConfigParser.cpp -o prg -ltrap -lpcap -lunirec --std=c++11 -Wno-write-strings
