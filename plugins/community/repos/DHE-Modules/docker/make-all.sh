#!/usr/bin/env bash
set -o errexit
set -o nounset
set -o xtrace

cd "$(dirname "$0")/"
docker image build -t rack:env rack-env/
docker image build -t rack:dep rack-dep/
docker image build -t rack:build rack-build/
