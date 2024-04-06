/* This file handles messages for directory operations.
 *
 * Created (CheviotOS Filesystem Handler based)
 *   December 2023 (Marven Gilhespie) 
 */

#define LOG_LEVEL_ERROR

#include "ext2.h"
#include "globals.h"
#include <sys/debug.h>


/* @brief   Lookup an item in a directory
 *
 * @param   fsreq, message header received by getmsg.
 */
void ext2_lookup(struct fsreq *req)
{
  struct fsreply reply = {0};
  struct inode *dir_inode;
  struct inode *inode;
  char name[NAME_MAX+1];
  ino_t ino_nr;
  int sz;
  int sc;
    
  memset(&reply, 0, sizeof reply);  
    
  readmsg(portid, msgid, name, req->args.lookup.name_sz, sizeof *req);

  dir_inode = get_inode(req->args.lookup.dir_inode_nr);
  
  if (dir_inode == NULL) {
    replymsg(portid, msgid, -EINVAL, NULL, 0);
    return;
  }

  sc = lookup_dir(dir_inode, name, &ino_nr);

  if (sc != 0) {
    put_inode(dir_inode);
    replymsg(portid, msgid, sc, NULL, 0);
    return;
  }
  
  inode = get_inode(ino_nr);
  
  if (inode == NULL) {
    put_inode(dir_inode);
    replymsg(portid, msgid, -EIO, NULL, 0);
    return;  
  }
  
  reply.args.lookup.inode_nr = inode->i_ino;
  reply.args.lookup.size = inode->odi.i_size;
  reply.args.lookup.uid = inode->odi.i_uid;
  reply.args.lookup.gid = inode->odi.i_gid; 
  reply.args.lookup.mode = inode->odi.i_mode;   // FIXME: Maybe add conversion function for native to ext2
  reply.args.lookup.atime = 0;    // FIXME: set time
  reply.args.lookup.mtime = 0;
  reply.args.lookup.ctime = 0;

  put_inode(inode);
  put_inode(dir_inode);

  replymsg(portid, msgid, 0, &reply, sizeof reply);  
}


/* @brief   Read a directory
 *
 * @param   fsreq, message header received by getmsg.
 */
void ext2_readdir(struct fsreq *req)
{
	static char readdir_buf[512];
  struct fsreply reply = {0};
  struct inode *dir_inode;
  uint32_t sz;
  off64_t cookie;
  size_t dirents_sz;
  size_t dirents_read_sz;

  memset(&reply, 0, sizeof reply);
      
  dir_inode = get_inode(req->args.readdir.inode_nr);

  if (dir_inode == NULL) {
    replymsg(portid, msgid, -EINVAL, NULL, 0);
    return;
  }

  cookie = req->args.readdir.offset;  
  sz = req->args.readdir.sz;
  dirents_sz = (sizeof readdir_buf < sz) ? sizeof readdir_buf : sz;  
  dirents_read_sz = get_dirents(dir_inode, &cookie, readdir_buf, dirents_sz);

  if (dirents_read_sz > 0) {
    writemsg(portid, msgid, readdir_buf, dirents_read_sz, sizeof reply);
  }

  put_inode(dir_inode);  

  reply.args.readdir.offset = cookie;
  replymsg(portid, msgid, dirents_read_sz, &reply, sizeof reply);
}


/* @brief   Create a new directory and populate with "." and ".." entries
 *
 * @param   fsreq, message header received by getmsg.
 */
void ext2_mkdir(struct fsreq *req)
{
  struct fsreply reply = {0};
  mode_t mode;
  uid_t uid;
  gid_t gid;
  int sc, sc1, sc2;
  struct inode *dir_inode;
  struct inode *inode;
  char name[NAME_MAX+1];
          
  readmsg(portid, msgid, name, req->args.mkdir.name_sz, sizeof *req);

  dir_inode = get_inode(req->args.mkdir.dir_inode_nr);

  if(dir_inode == NULL) {
    replymsg(portid, msgid, -ENOENT, NULL, 0);
    return;
  }
    
  mode = req->args.mkdir.mode;
  uid = req->args.mkdir.uid;
  gid = req->args.mkdir.gid;  
    
  sc = new_inode(dir_inode, name, mode, uid, gid, &inode);

  if(sc != 0) {
	  put_inode(dir_inode);
    replymsg(portid, msgid, sc, NULL, 0);
    return;
  }

  sc1 = dirent_enter(inode, ".", inode->i_ino, S_IFDIR);
  sc2 = dirent_enter(inode, "..", dir_inode->i_ino, S_IFDIR);

  if (sc1 == 0 && sc2 == 0) {
	  inode->odi.i_links_count++;
	  dir_inode->odi.i_links_count++;
	  inode_markdirty(dir_inode);
  } else {
	  if (dirent_delete(dir_inode, name) != 0) {
		  panic("extfs: dir disappeared during mkdir: %d ", (int) inode->i_ino);
		}
		
	  inode->odi.i_links_count--;
  }
  
  inode_markdirty(inode);
  put_inode(dir_inode);

  reply.args.mkdir.inode_nr = inode->i_ino;
  reply.args.mkdir.mode = inode->odi.i_mode;
  reply.args.mkdir.size = inode->odi.i_size;
  reply.args.mkdir.uid = inode->odi.i_uid;
  reply.args.mkdir.gid = inode->odi.i_gid;
  reply.args.mkdir.atime = 0;   // FIXME: set time
  reply.args.mkdir.mtime = 0;
  reply.args.mkdir.ctime = 0;

  put_inode(inode);

  replymsg(portid, msgid, 0, &reply, sizeof reply);  
}


/* @brief   Remove a directory
 *
 * @param   fsreq, message header received by getmsg.
 */
void ext2_rmdir(struct fsreq *req)
{
  struct inode *dir_inode;
  struct inode *inode;
  char name[NAME_MAX+1];
  ino_t ino_nr;
  int	sc;

  log_info("ext2_rmdir");
 
  readmsg(portid, msgid, name, req->args.rmdir.name_sz, sizeof *req);

  dir_inode = get_inode(req->args.rmdir.dir_inode_nr);
  
  if (dir_inode == NULL) {
    replymsg(portid, msgid, -EIO, NULL, 0);
    return;
  }
  
  sc = lookup_dir(dir_inode, name, &ino_nr);

  if (sc == 0) {
    inode = get_inode(ino_nr);
  }

  if (sc != 0 || inode == NULL) {
    put_inode(dir_inode);
    replymsg(portid, msgid, -EIO, NULL, 0);
    return;
  }

  if (is_dir_empty(inode) == false) {
    put_inode(inode);
    put_inode(dir_inode);
    replymsg(portid, msgid, -ENOTEMPTY, NULL, 0);  
    return;
  }

  /* TODO: Can "." and ".." in directory be symbolic links to something else?
   * Are they guaranteed to point to this directory and parent?
   * Do we need to reduce the links count of parent directory?
   *
   * Do we need an unlink function that deletes these and the dir?
   * Replacing what is below?
   */

	// TODO: Truncate directory to 0 bytes free up blocks
	  
  sc = dirent_delete(dir_inode, name);

  if (sc == 0) {
	  dir_inode->odi.i_links_count--;
	  dir_inode->i_update |= CTIME;
	  inode_markdirty(dir_inode);	  
	  inode->odi.i_links_count--;
	  inode->i_update |= CTIME;
	  inode_markdirty(inode);	  
  }

  put_inode(inode);
  put_inode(dir_inode);

  replymsg(portid, msgid, sc, NULL, 0);
}


