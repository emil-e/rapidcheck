#!/bin/sh
curl -s -L -o boost_1_59_0.tar.bz2 "http://downloads.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.tar.bz2?r=http%3A%2F%2Fwww.boost.org%2Fusers%2Fhistory%2Fversion_1_59_0.html&ts=1441002160&use_mirror=netassist"
tar xjfp boost_1_59_0.tar.bz2
mv boost_1_59_0 boost
rm boost_1_59_0.tar.bz2
