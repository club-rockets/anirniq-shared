# Anirniq Shared code

This repository contains all drivers, interfaces and other source code shared between the different Anirniq modules.

## CAN ID

The ID field of a standard CAN frame is 11-bit long, divided as follows

```none
0123456789AB
nnnnn        5 bit node id
     mmmmmmm 6 bit message (register) id
```

The node id are as follows:

- `00000` Emergency (virtual node id that any board may use to transmit **emergency** messages)
- `00001` Mission
- `00010` Communication
- `00011` Acquisition
- `00100` Motherboard
