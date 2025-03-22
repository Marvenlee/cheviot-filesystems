#ifndef devfs_H
#define devfs_H

//#define NDEBUG

#include <sys/types.h>
#include <stdint.h>
#include <sys/syslimits.h>


/* Constants
 */
#define DEVFS_MAX_INODE 	128
#define DIRENTS_BUF_SZ 		4096
#define DEVFS_NAME_LEN    64
/*
 *
 */
#define ALIGN_UP(val, alignment)                                \
    ((((val) + (alignment) - 1)/(alignment))*(alignment))
#define ALIGN_DOWN(val, alignment)                              \
            ((val) - ((val) % (alignment)))



/*
 *
 */
struct DevfsNode {
  char name[DEVFS_NAME_LEN];
  int32_t inode_nr;
  int32_t parent_inode_nr;
  mode_t mode;
  int32_t uid;
  int32_t gid;
  uint32_t file_offset;
  uint32_t file_size;
} __attribute__((packed));



/*
 * Driver Configuration settings
 */
struct Config
{
  char pathname[PATH_MAX + 1];
  uid_t uid;
  gid_t gid;
  mode_t mode;
  dev_t dev;
};


// prototypes

// init.c
void init (int argc, char *argv[]);
int process_args(int argc, char *argv[]);
int init_devfs(void);
int mount_device(void);

#endif

