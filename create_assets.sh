#!/usr/bin/env bash
#
# Copyright 2019, David Runge
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# Creates assets for $upstream based on a provided tag in the form of
# ${output_package_name}-${version}.tar.gz' and moves the file to
# ${output_dir}.
# Optionally creates a detached PGP signature for the tarball.
# Requires a writable /tmp folder.

set -euo pipefail

get_absolute_path() {
  cd "$(dirname "$0")" && pwd -P
}

remove_source_dir() {
  rm -rf "${source_dir:?}/${package_name}"* \
    "${source_dir:?}/${output_package_name}"*
}

checkout_project() {
  remove_source_dir
  cd "${source_dir}"
  git clone "$upstream" --recursive
  cd "${package_name}"
  git checkout "${version}"
}

clean_sources() {
  cd "${source_dir}/${package_name}"
  find . \( -iname "*.git*" -o \
         -iname "*.clang-format" -o \
         -iname "*.travis.yml" -o \
         -iname "*create_assets.sh" \) \
         -exec rm -rfv {} +
}

rename_sources() {
  cd "${source_dir}"
  mv -v "${package_name}" "${output_package_name}-${version}"
}

compress_sources() {
  cd "${source_dir}"
  tar cvfz "${output_package_name}-${version}.tar.gz" \
    "${output_package_name}-${version}"
}

move_sources() {
  cd "${source_dir}"
  mv -v "${output_package_name}-${version}.tar.gz" "${output_dir}/"
}

sign_sources() {
  cd "${output_dir}"
  gpg2 --default-key "${signer}" \
       --output "${output_package_name}-${version}.tar.gz.asc" \
       --detach-sign "${output_package_name}-${version}.tar.gz"
}

cleanup_source_dir() {
  cd "${source_dir}"
  rm -rf "${output_package_name}-${version}"
}

print_help() {
  echo "Usage: $0 -v <version tag> -s <signature email>"
  exit 1
}

upstream="https://github.com/vcvrack/rack"
package_name="rack"
output_package_name="rack"
source_dir="/tmp"
version=$(date "+%Y-%m-%d")
signer=""
output_dir=$(get_absolute_path "$0")


if [ ${#@} -gt 0 ]; then
  while getopts 'hv:s:' flag; do
    case "${flag}" in
      h) print_help
          ;;
      s) signer=$OPTARG
          ;;
      v) version=$OPTARG
          ;;
      *)
        echo "Error! Try '${0} -h'."
        exit 1
        ;;
    esac
  done
else
  print_help
fi

checkout_project
clean_sources
rename_sources
compress_sources
move_sources
if [ -n "${signer}" ]; then
  sign_sources
fi
cleanup_source_dir

exit 0

# vim:set ts=2 sw=2 et:
