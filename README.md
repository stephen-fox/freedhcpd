# freedhcpd

## Updating upstream branch

Note: Requires the git-filter-repo package.

```sh
git clone https://github.com/openbsd/src openbsd-src
cp -r openbsd-src openbsd-dhcpd

cd openbsd-dhcpd
git filter-repo --path usr.sbin/dhcpd --path-rename usr.sbin/dhcpd:src

cd ../freedhcpd
git remote add tmp ../openbsd-dhcpd
git checkout upstream
git pull tmp master
git checkout -
```
