# fbsddeps
Displays the FreeBSD packages upon which a given compiled binary or shared library depends.

## Build
You'll want mkfbsdmnfst in order to build a package.

```
./configure
gmake package
```

## Install
May be installed using ```pkg install ...```  For example:

```pkg install fbsddeps-1.0.0.txz```
