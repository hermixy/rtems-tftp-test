Test program for https://devel.rtems.org/ticket/2375

Works with the pc386 BSP from 4.10.2
and QEMU 2.1.2 (debian 1:2.1+dfsg-12).

To build and run w/ QEMU:

```sh
make RTEMS_MAKEFILE_PATH=<rtems-prefix>/i386-rtems4.10/pc386/ clean all run
```
