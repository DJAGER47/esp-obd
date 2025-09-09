#!/bin/bash
cd build && cmake .. && make
cd unity-app/build && timeout 10s ./unity_app