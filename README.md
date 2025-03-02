# freedhcpd

## Updating upstream branch

```sh
# Note: Requires the git-filter-repo package.
git clone https://github.com/openbsd/src openbsd-dhcpd
cd openbsd-dhcpd
git filter-repo --path usr.sbin/dhcpd --path-rename usr.sbin/dhcpd:src
```
