#!/bin/bash

bash load2switch.sh

make

../bfshell -b `pwd`/config.py

