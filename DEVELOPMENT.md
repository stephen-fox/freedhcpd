# Development

This document covers development topics for working on the project.

## Virtual test network automation

A virtual network can be created using the shell scripts in the `testing`
directory. These scripts must, unfortunately, be run as root.

Please find a summary of what these scripts do below:

- `destroy-net.sh` - Destroys all network interfaces created by the scripts
- `create-net.sh` - Creates the base network interface(s). This script's
  code is automatically executed by the other scripts
- `clientN.sh` (where "N" is a number) - Creates a DHCP client
- `server0.sh` - Creates a dhcpd server
- `peer-server.sh` - Creates a peer dhcpd server in a separate network
  namespace. Useful for simulating a lease synchronization peer running
  on a different computer

#### Simulating a server and some clients

Execute the following scripts in different shells:

- `client0.sh` (to create a DHCP client)
- `server0.sh` (to create a dhcpd instance)

w00t :)
