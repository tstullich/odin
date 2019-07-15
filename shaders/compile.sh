#!/usr/bin/bash

# Compile all shaders with the following extensions
VALIDATOR_PATH="$(command -v glslangValidator)"
if ! [ $VALIDATOR_PATH ]; then
  echo 'Error: glslangValidator is not installed.
       Make sure you have the Vulkan SDK installed and in your PATH variable' >&2
  exit 1
fi

$VALIDATOR_PATH -V *.frag *.vert