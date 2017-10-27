#!/bin/sh
curl -s -L -o boost_1_65_1.tar.bz2 https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.tar.bz2
tar xjfp boost_1_65_1.tar.bz2
mv boost_1_65_1 boost
rm boost_1_65_1.tar.bz2
