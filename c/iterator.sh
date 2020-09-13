#!/bin/bash
FILES=~/Desktop/Lifesense/Gitlab/Step_algorithm/data/Benchmark/FalseSteps/*/*.csv
for f in $FILES
do
  # take action on each file. $f store current file name
  ./step $f
done