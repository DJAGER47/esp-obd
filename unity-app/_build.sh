#!/bin/bash
cd build && cmake .. && make

cmake .. -DENABLE_CLANG_TIDY=ON && make

cd unity-app/build && timeout 10s ./unity_app