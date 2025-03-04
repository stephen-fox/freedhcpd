#!/bin/sh

set -eux

group="dhcpd_dev"

destroy_if() {
  local if_name="${1}"

  if ifconfig "${if_name}" > /dev/null 2>&1
  then
    ifconfig "${if_name}" destroy
  fi

  return 0
}

for if_name in $(ifconfig -g "${group}")
do
  destroy_if "${if_name}"
done
