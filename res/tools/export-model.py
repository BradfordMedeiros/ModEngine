#!/usr/bin/env bash
import bpy
import sys

axis_forward='-Z'
axis_up='Y'

output_file = sys.argv[6]

bpy.ops.export_scene.fbx(filepath=output_file, axis_forward=axis_forward, axis_up=axis_up)