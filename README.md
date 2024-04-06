# cheviot-filesystems

A repository containing file system handlers for CheviotOS.

This repository is brought in to the cheviot-project base repository as a
git submodule.

## Overview of File System Handlers

File systems in CheviotOS are user-mode processes. The Virtual File System
Switch within the kernel converts standard system calls such as open(), read(),
write() and lseek() into messages that are passed to file system handlers via
the kernel's inter-process communication mechanisms.  In turn, the file system
handlers will perform read and write operations to the block device drivers.

## ifs

The IFS (or Initial File System) is a special, read-only file system used
to bootstrap the kernel and required file system handlers, drivers and init process.
The IFS uses a simple format similar to ISO9660 CD-ROMs. The *mkifs* tool in the
cheviot-build repository is used to create an IFS image. This image is is merged
with the bootloader.

When the kernel has bootstrapped this IFS process becomes the *root* process.
This process forks and the forked process becomes the IFS handler with the IFS
imaged mapped into it.  After this step the *init* process is started where it can
now start other commands from the *IFS* image.

## devfs

A simple filesystem that implements the */dev* hierarchy where block and character
special devices can be mounted.

## extfs

An implementation of the Linux Ext2 file system. This is derived from Minix sources, the
license of which is in the extfs directory. This is currently work-in-progress. Read operations
and directory traveral works.  Operations for writing or modifying files and directories
are in progress.

The root file system created by the build is formatted as an Ext2 file system.

## fatfs

This implements the FAT file system. This is currently disabled as it is based on the
earlier Kielder sources and requires a rewrite.

# Licenses and Acknowledgements

The extfs file system is derived from the Minix ext2 driver. See the extfs/MINIX_LICENSE
for the license.


