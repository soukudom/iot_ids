#!/bin/bash

g++ siot-detector.cpp fields.cpp ConfigParser.cpp Analyzer.cpp -o prg -ltrap -lpcap -lunirec --std=c++11 -Wno-write-strings
