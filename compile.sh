#!/bin/sh
g++ `Magick++-config --cxxflags --cppflags` -lX11 -o wpc wallpapercut.cpp `Magick++-config --ldflags --libs`