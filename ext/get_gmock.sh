#!/bin/sh
wget -nv https://googlemock.googlecode.com/files/gmock-1.7.0.zip -O gmock-1.7.0.zip
unzip -q gmock-1.7.0.zip
mv gmock-1.7.0 gmock
rm gmock-1.7.0.zip
