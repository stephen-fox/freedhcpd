#!/bin/sh

group="dhcpd_dev"

br0_if="${group}_br0"

a_suffix="bm"
b_suffix="cs"

fib=0

die() {
  local status="${1}"
  shift

  echo "fatal: ${@}" 1>&2
  exit "${status}"
}

destroy_if() {
  local if_name="${1}"

  if ifconfig "${if_name}" > /dev/null 2>&1
  then
    ifconfig "${if_name}" destroy
  fi

  return 0
}

create_epair() {
  local if_name="${1}"

  local a="${if_name}${a_suffix}"
  local b="${if_name}${b_suffix}"

  local tmp="$(ifconfig epair create)"
  tmp="${tmp%?}" # trim the last byte ("a" in this case)

  ifconfig "${tmp}a" name "${a}"
  ifconfig "${tmp}b" name "${b}"

  ifconfig "${a}" fib ${fib} group "${group}" up
  ifconfig "${b}" fib ${fib} group "${group}" up

  return 0
}

create_bridge() {
  local if_name="${1}"

  local tmp="$(ifconfig bridge create fib ${fib})"

  ifconfig "${tmp}" name "${if_name}"

  ifconfig "${if_name}" group "${group}" up

  return 0
}

init_net() {
  if ! ifconfig "${br0_if}" > /dev/null 2>&1
  then
    create_bridge "${br0_if}"
  fi

  return 0
}
