#!/usr/bin/env bash

blender ../../../gameresources/building/shelf.blend --background --python $(pwd)/export-model.py -- ./output.fbx
