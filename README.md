# evoplus_cid
Tool to change the CID on Samsung Evo Plus SD Cards. Requires rooted Android device.
Precompiled Android binary included. May also work on regular Linux, but you must use
a real sd controller, not a usb mass storage type sd reader.
See http://richard.burtons.org/2016/07/01/changing-the-cid-on-an-sd-card/
for more details.

**USE AT YOU OWN RISK!**

## Usage:
```
./evoplus_cid <device> <cid> [serial]
device - sd card block device e.g. /dev/block/mmcblk1
cid - new cid, must be in hex (without 0x prefix)
  it can be 32 chars with checksum or 30 chars without, it will
  be updated with new serial number if supplied, the checksum is
  (re)calculated if not supplied or new serial applied
serial - optional, can be hex (0x prefixed) or decimal
  and will be applied to the supplied cid before writing
```

Based on the reverse engineering and code of Sean Beaupre,
see: https://github.com/beaups/SamsungCID

## Compile

`gcc evoplus_cid.c -o evoplus_cid`

You can safely ignore compilation warnings.
