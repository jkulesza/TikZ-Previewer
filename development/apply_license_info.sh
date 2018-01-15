#!/bin/bash

shopt -s nullglob
for f in ../src/*.cpp ../src/*.h
do
	echo "Applying license header to: $f"
  cat LICENSE $f > $f.new
  mv $f.new $f
done
