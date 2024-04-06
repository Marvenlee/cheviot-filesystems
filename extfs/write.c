/* This file is the counterpart of "read.c". It contains the code
 * for writing files.
 *
 * Created (MFS based):
 *   February 2010 (Evgeniy Ivanov)
 *
 * Updated (CheviotOS Filesystem Handler based)
 *   December 2023 (Marven Gilhespie) 
 */
 
#define LOG_LEVEL_WARN

#include "ext2.h"
#include "globals.h"


/* @brief   Write data to a file
 *
 * @param   ino_nr, inode number of file to write
 * @param   nbytes, number of bytes in file to write
 * @param   position, offset in file to start writing to.
 * @return  Number of bytes written or negative errno on failure
 */
ssize_t write_file(ino_t ino_nr, size_t nbytes, off64_t position)
{
  off64_t file_size;      // file size
  size_t off;             // offset in block
  size_t total_xfered;    // total bytes written
  size_t chunk_size;      // size of partial block of data we are currently writing
  struct inode *inode;    // inode of file to write to
  int sc;                 // result

  if ((inode = find_inode(ino_nr)) == NULL) {
    log_error("write file to unknown inode");
  	return -EINVAL;
  }
  
  file_size = inode->odi.i_size;
  
  if (file_size < 0) {
    file_size = MAX_FILE_POS;
  }
  
  if (position > (off_t) (sb_max_size - nbytes)) {
    log_error("position out of bounds");
	  return -EFBIG;
	}

  sc = 0;
  total_xfered = 0;
  
  /* Split the transfer into chunks that don't span blocks. */
  while (nbytes != 0) {
	  off = (unsigned int) (position % sb_block_size);
	  chunk_size = sb_block_size - off;
	  if (chunk_size > nbytes) {
		  chunk_size = nbytes;
    }
    
	  sc = write_chunk(inode, position, off, chunk_size, total_xfered);

	  if (sc != 0) {
	    break;
    }
    
	  nbytes -= chunk_size;
	  total_xfered += chunk_size;
	  position += chunk_size;
  }

  if (S_ISREG(inode->odi.i_mode) || S_ISDIR(inode->odi.i_mode)) {
	  if (position > file_size) {
	    inode->odi.i_size = position;
	  }
  }

  if (sc != 0) {
    log_error("write file error:%d", sc);
  	return sc;
  }
  
  inode->i_update |= CTIME | MTIME;
  inode_markdirty(inode);
  return total_xfered;
}


/* @brief   Write all or part of a block
 *
 * @param   inode, pointer to inode for file to be rd/wr
 * @param   position, position within file to read or write
 * @param   off, offset within the current block
 * @param   chunk, number of bytes to read or write
 * @param   data, structure for (remote) user buffer
 * @param   msg_off, offset in message buffer
 * @param   block_size, block size of FS operating on
 * @return  0 on success, negative errno on failure
 */ 
int write_chunk(struct inode *inode, off64_t position, size_t off, size_t chunk_size, size_t msg_off)
{
  struct buf *buf = NULL;
  ino_t ino = NO_INODE;
  uint64_t ino_off = rounddown(position, sb_block_size);
  block_t block;
  int sc = 0;

  ino = inode->i_ino;
  block = read_map_entry(inode, position);
  
  if (block == NO_BLOCK) {
	  if ((buf = new_block(inode, position)) == NULL) {
		  return -EIO;
		}
  } else {
	  if (chunk_size == sb_block_size) {
  	  buf = get_block(cache, block, BLK_CLEAR);
	  } else if (off == 0 && position >= inode->odi.i_size) {
  	  buf = get_block(cache, block, BLK_CLEAR);
		} else {
  	  buf = get_block(cache, block, BLK_READ);
		}
  }

  assert(buf != NULL);
  
  sc = readmsg(portid, msgid, (uint8_t *)buf->data+off, chunk_size, msg_off + sizeof (struct fsreq));	  
  block_markdirty(buf);
  put_block(cache, buf);
  
  if (sc != chunk_size) {
    return -EIO;
  }
  
  return 0;
}

