#!/bin/sh

set -eux

script_path="$(realpath "${0}")"
script_dir="${script_path%/*}"

. "${script_dir}/common.sh"

init_net
