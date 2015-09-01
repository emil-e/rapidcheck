#!/bin/sh
curl -s -L -o boost_1_59_0.tar.bz2 "http://downloads.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fboost%2Ffiles%2Fboost%2F1.59.0%2Fboost_1_59_0.tar.bz2%2Fdownload&ts=1441133751&use_mirror=skylink"
tar xjfp boost_1_59_0.tar.bz2
mv boost_1_59_0 boost
rm boost_1_59_0.tar.bz2
