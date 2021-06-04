#!/bin/bash


#qsub -l nodes=1:gpu:ppn=2 -d . run.sh

make clean
make
