#!/usr/bin/env bash
set -o errexit
set -o nounset
set -o xtrace

echo "MSYSTEM=${MSYSTEM}"
export VCV_PLUGIN_NAME=Erratic
export VCV_PLUGIN_DIR="${APPVEYOR_BUILD_FOLDER}"
export VCV_RACK_DIR=/c/tmp/Rack
export VCV_RACK_COMMIT=v0.6
export VCV_HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

env | sort

mkdir -p "${VCV_RACK_DIR}"
git clone -n --branch=${VCV_RACK_COMMIT} https://github.com/VCVRack/Rack.git "${VCV_RACK_DIR}" || true
cd "${VCV_RACK_DIR}"
git submodule update --init --recursive

make dep > /dev/null
make

cd "${VCV_PLUGIN_DIR}"

make clean test dist RACK_DIR="${VCV_RACK_DIR}"
