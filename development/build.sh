#!/bin/bash

# This is a convenience script setup by the original author to build TikZ
# Previewer outside the Qt Creator environment.  It has the effect of also
# regenerating the icon(s) and reassessing (via qmake) the list of code
# contributors based on the git log entries.

# Rebuild icon.
cd icon && ./make_icon.sh && cd ..

# Build application.
if [ -d ../build ]; then rm -rf ../build; fi
mkdir ../build && cd ../build
qmake -config release ../src
make

# Perform corrections to dynamically linked libraries.
ln -s ../development/setup_dylibs_for_deployment.sh
./setup_dylibs_for_deployment.sh

