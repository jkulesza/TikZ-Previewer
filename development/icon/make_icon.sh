#!/bin/bash

# Process from
# https://blog.macsales.com/28492-create-your-own-custom-icons-in-10-7-5-or-later
# followed.

if [ -f icon_magnifying_glass.icns ]; then rm icon_magnifying_glass.icns; fi
if [ -d icon_magnifying_glass.iconset ]; then rm -rf icon_magnifying_glass.iconset; fi

pdflatex icon_magnifying_glass.tikz.tex

convert -resize 1024x1024 -density 1000 icon_magnifying_glass.tikz.pdf icon_512x512@2x.png
convert -resize 512x512   -density 1000 icon_512x512@2x.png icon_512x512.png
convert -resize 512x512   -density 1000 icon_512x512@2x.png icon_256x256@2x.png
convert -resize 256x256   -density 1000 icon_512x512@2x.png icon_256x256.png
convert -resize 256x256   -density 1000 icon_512x512@2x.png icon_128x128@2x.png
convert -resize 128x128   -density 1000 icon_512x512@2x.png icon_128x128.png
convert -resize 64x64     -density 1000 icon_512x512@2x.png icon_32x32@2x.png
convert -resize 32x32     -density 1000 icon_512x512@2x.png icon_32x32.png
convert -resize 32x32     -density 1000 icon_512x512@2x.png icon_16x16@2x.png
convert -resize 16x16     -density 1000 icon_512x512@2x.png icon_16x16.png

mkdir -p ../../src/icons
convert icon_256x256.png ../../src/icons/icon_app_256.ico

mkdir icon_magnifying_glass.iconset
mv icon_*png icon_magnifying_glass.iconset
iconutil -c icns icon_magnifying_glass.iconset
