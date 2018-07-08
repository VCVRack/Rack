#!/usr/bin/env bash

# This script uses these environment variables:
#
# VCV_RACK_DIR
#
#   The directory in which to clone and build Rack.
#
# VCV_RACK_COMMIT
#
#   Which Rack commit to build.

mkdir -p "${VCV_RACK_DIR}" \
git clone -n https://github.com/VCVRack/Rack.git "${VCV_RACK_DIR}" || true
cd "${VCV_RACK_DIR}"
git checkout ${VCV_RACK_COMMIT}
git pull
git submodule update --init --recursive

make dep > /dev/null
make
