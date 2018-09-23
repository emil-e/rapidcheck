#!/bin/sh
curl -s -L -o boost_1_68_0.tar.bz2 https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.bz2
tar xjfp boost_1_68_0.tar.bz2
mv boost_1_68_0 boost
rm boost_1_68_0.tar.bz2
