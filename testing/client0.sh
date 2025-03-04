#!/bin/sh

set -eux

script_path="$(realpath "${0}")"
script_dir="${script_path%/*}"

. "${script_dir}/common.sh"

init_net

if_basename="client0_"
if_name_a="${if_basename}${a_suffix}"
if_name_b="${if_basename}${b_suffix}"

destroy_if "${if_name_a}"

create_epair "${if_basename}"

ifconfig "${br0_if}" addm "${if_name_a}"

lease_file="$(mktemp -t dev-dhcpd-${if_basename})"

trap "rm ${lease_file}" SIGINT SIGTERM SIGQUIT SIGHUP

dhclient -d -l "${lease_file}" "${if_name_b}"
