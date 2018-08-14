#!/bin/bash

# script argument
ARG=$1

# Rack base dir
RACK_DIR="../.."

# build plugin
make

# quit if compiling fails
[[ $? != 0 ]] && exit 1

# build and run
if [[ ${ARG} == "run" ]]; then
    pushd $RACK_DIR
        make run
    popd
fi

