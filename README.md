# Anirniq Shared code

This repository contains all drivers, interfaces and other source code shared between the different Anirniq modules.

## CAN ID

The ID field of a standard CAN frame is 11-bit long, divided as follows

```none
0123456789AB
rr           2 bit reserved
  nnn        3 bit node id
     mmmmmmm 6 bit message (register) id
```

The two reserved bit should be set to `11` when sending messages.

The node id are as follows:

- `000` Mission
- `001` Communication
- `010` Acquisition
- `011` Motherboard
