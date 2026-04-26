#!/bin/sh

set -eux

script_path="$(realpath "${0}")"
script_dir="${script_path%/*}"
repo_dir="${script_dir%/*}"
src_dir="${repo_dir}/usr.sbin/dhcpd"
test_dir="${repo_dir}/testing"

. "${script_dir}/common.sh"

init_net

if_basename="dhcpd0_"
if_name_a="${if_basename}${a_suffix}"
if_name_b="${if_basename}${b_suffix}"

destroy_if "${if_name_a}"

create_epair "${if_basename}"

ifconfig "${br0_if}" addm "${if_name_a}"

ifconfig "${if_name_b}" inet "10.254.0.1/24"

lease_file="$(mktemp -t dev-dhcpd-${if_basename})"

trap "rm ${lease_file}" SIGINT SIGQUIT SIGTERM SIGHUP

setfib 0 \
  "${src_dir}/dhcpd" \
  -d \
  -v \
  -c "${test_dir}/server.conf" \
  -l "${lease_file}" \
  -y "${if_name_b}" \
  -Y "${if_name_b}" \
  "${if_name_b}"
