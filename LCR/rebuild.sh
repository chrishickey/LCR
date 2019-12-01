#!/bin/sh

rm -rf build/ ./waf-* .lock-script
./waf configure
./waf build
