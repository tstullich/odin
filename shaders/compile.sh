#!/usr/bin/bash

# Compile all shaders with the following extensions
VALIDATOR_PATH="$(command -v glslangValidator)"
if ! [ $VALIDATOR_PATH ]; then
  echo 'Error: glslangValidator is not installed.
       Make sure you have the Vulkan SDK installed and in your PATH variable' >&2
  exit 1
fi

# By GLSL convention we search for the following files that have the given extensions below
find . -type f \( -name "*.glsl" -o -name "*.comp" -o -name "*.frag" -o -name "*.vert" \) | sed -e 's,^\./,,' | xargs $VALIDATOR_PATH -V