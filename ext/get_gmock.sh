#!/bin/sh
curl -s -L -o gmock-1.7.0.zip https://googlemock.googlecode.com/files/gmock-1.7.0.zip
unzip -q gmock-1.7.0.zip
mv gmock-1.7.0 gmock
rm gmock-1.7.0.zip
