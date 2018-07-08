#!/usr/bin/env bash
set -o errexit
set -o nounset
set -o xtrace

echo "MSYSTEM=${MSYSTEM}"
export VCV_PLUGIN_NAME=DHE-Modules
export VCV_PLUGIN_DIR="${APPVEYOR_BUILD_FOLDER}"
export VCV_RACK_DIR=/c/tmp/Rack
export VCV_RACK_COMMIT=master
export VCV_HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

env | sort

mkdir -p "${VCV_RACK_DIR}" \
git clone -n https://github.com/VCVRack/Rack.git "${VCV_RACK_DIR}" || true
cd "${VCV_RACK_DIR}"
git checkout ${VCV_RACK_COMMIT}
git pull
git submodule update --init --recursive

make dep > /dev/null
make

cd "${VCV_PLUGIN_DIR}"

make clean test dist RACK_DIR="${VCV_RACK_DIR}"
