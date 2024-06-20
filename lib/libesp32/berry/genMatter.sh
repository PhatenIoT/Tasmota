#!/bin/bash
#
# generate all precompiled Berry structures from multiple modules
#
# Included in the Platformio build process with `pio-tools/gen-berry-structures.py
#

#name: Make Berry and Tasmota Berry code
cd ../berry_tasmota
../berry/berry -s -g solidify_all.be

# Berry Animate Code
#cd ../berry_animate
#../berry/berry -s -g solidify_all.be

# LVGL Berry Code
#cd ../lv_binding_berry
#../../libesp32/berry/berry -s -g solidify_all.be

# HASPmota Berry Code
#cd ../lv_haspmota
#../../libesp32/berry/berry -s -g solidify_all.be

# Matter Berry Code
cd ../berry_matter
../berry/berry -s -g solidify_all.be