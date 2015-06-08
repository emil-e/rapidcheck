#!/bin/sh
wget "http://downloads.sourceforge.net/project/boost/boost/1.58.0/boost_1_58_0.tar.bz2?r=http%3A%2F%2Fwww.boost.org%2Fusers%2Fhistory%2Fversion_1_58_0.html&ts=1433786576&use_mirror=garr" -O boost_1_58_0.tar.bz2
tar xjvfp boost_1_58_0.tar.bz2
mv boost_1_58_0 boost
rm boost_1_58_0.tar.bz2
