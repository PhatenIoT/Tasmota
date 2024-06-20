#!/bin/bash
#
# generate all precompiled Berry structures from multiple modules
#
# Included in the Platformio build process with `pio-tools/gen-berry-structures.py
#
PYTHON_EXECUTABLE="C:\Users\Administrator\AppData\Local\Programs\Python\Python312\python.exe"
rm -Rf ./generate/be_*.h
$PYTHON_EXECUTABLE tools/coc/coc -o generate src default ../berry_tasmota/src ../berry_mapping/src ../berry_int64/src ../../libesp32_lvgl/lv_binding_berry/src ../../libesp32_lvgl/lv_haspmota/src/solidify ../berry_matter/src/solidify ../berry_matter/src/embedded ../berry_matter/src ../berry_animate/src/solidify ../berry_animate/src ../../libesp32_lvgl/lv_binding_berry/src/solidify ../../libesp32_lvgl/lv_binding_berry/generate -c default/berry_conf.h
