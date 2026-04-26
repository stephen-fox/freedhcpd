#!/bin/sh

set -eux

script_path="$(realpath "${0}")"
script_dir="${script_path%/*}"
repo_dir="${script_dir%/*}"
src_dir="${repo_dir}/usr.sbin/dhcpd"
test_dir="${repo_dir}/testing"

. "${script_dir}/common.sh"

init_net

if_basename="dhcpd_peer_"
if_name_a="${if_basename}${a_suffix}"
if_name_b="${if_basename}${b_suffix}"

destroy_if "${if_name_a}"

create_epair "${if_basename}"

ifconfig "${br0_if}" addm "${if_name_a}"

jail_name="dev-dhcpd-peer"

lease_file="$(mktemp -t ${jail_name})"

# We need to remove the jail manually because, for whatever
# reason, FreeBSD persists the jail even when no processes
# are running in it.
trap \
  "jail -r ${jail_name} && rm ${lease_file}" \
  SIGINT SIGQUIT SIGTERM SIGHUP

jail \
  -c \
  name=${jail_name} \
  path=/ \
  host.hostname=${jail_name} \
  vnet \
  vnet.interface="${if_name_b}" \
  exec.start="/sbin/ifconfig ${if_name_b} inet 10.254.0.2/24 up" \
  exec.start="/sbin/route add default 10.254.0.1" \
  exec.start="${src_dir}/dhcpd
  -d
  -v
  -c ${test_dir}/server.conf
  -l ${lease_file}
  -y ${if_name_b}
  -Y ${if_name_b}
  ${if_name_b}"
