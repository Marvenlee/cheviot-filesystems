/* This file handles the main event loop.
 *
 * Created (CheviotOS Filesystem Handler based)
 *   December 2023 (Marven Gilhespie) 
 */


#define LOG_LEVEL_WARN

#include <stdint.h>
#include <time.h>
#include "ext2.h"
#include "globals.h"


/* @brief   Ext filesystem handler main loop
 *
 */
int main(int argc, char *argv[])
{
  struct kevent ev;
  iorequest_t req;
  int sc;
  int nevents;
  struct sigaction sact;
  
  init(argc, argv);

  sact.sa_handler = &sigterm_handler;
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
  
  if (sigaction(SIGTERM, &sact, NULL) != 0) {
    exit(-1);
  }
  
  EV_SET(&ev, portid, EVFILT_MSGPORT, EV_ADD | EV_ENABLE, 0, 0, 0); 
  kevent(kq, &ev, 1, NULL, 0, NULL);

  while (!shutdown) {
    nevents = kevent(kq, NULL, 0, &ev, 1, NULL);
  
    if (nevents == 1 && ev.ident == portid && ev.filter == EVFILT_MSGPORT) {
      while ((sc = getmsg(portid, &msgid, &req, sizeof req)) == sizeof req) {      
        switch (req.cmd) {
          case CMD_READ:
            ext2_read(&req);
            break;

          case CMD_WRITE:
            ext2_write(&req);
            break;

          case CMD_LOOKUP:
            ext2_lookup(&req);
            break;

          case CMD_CLOSE:
            ext2_close(&req);
            break;
          
          case CMD_CREATE:
            ext2_create(&req);
            break;

          case CMD_READDIR:
            ext2_readdir(&req);
            break;

          case CMD_UNLINK:
            ext2_unlink(&req);
            break;

          case CMD_RMDIR:
            ext2_rmdir(&req);
            break;

          case CMD_MKDIR:
            ext2_mkdir(&req);
            break;

          case CMD_MKNOD:
            ext2_mknod(&req);
            break;

          case CMD_RENAME:
            ext2_rename(&req);
            break;

          case CMD_CHMOD:
            ext2_chmod(&req);
            break;

          case CMD_CHOWN:
            ext2_chown(&req);
            break;

          case CMD_TRUNCATE:
            ext2_truncate(&req);
            break;

          // TODO: Add VNODEATTR

          default:
            log_warn("extfs: unknown command: %d", req.cmd);
            replymsg(portid, msgid, -ENOTSUP, NULL, 0);
            break;
        }
      }

      if (sc != 0) {
        log_error("ext2fs: getmsg err = %d, %s", sc, strerror(-sc));
        exit(-1);
      }
    }
  }

  exit(0);
}


/*
 *
 */
void sigterm_handler(int signo)
{
  shutdown = true;
}


