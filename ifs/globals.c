/*
 * Copyright 2014  Marven Gilhespie
 *
 * Licensed under the Apache License, segment_id 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "globals.h"
#include <sys/syscalls.h>
#include <sys/types.h>


// Globals
int portid;
int kq;
void *ifs_image_phys;
void *ifs_image;
struct IFSHeader *ifs_header;
size_t ifs_image_size;
struct IFSNode *ifs_inode_table;
uint8_t dirents_buf[DIRENTS_BUF_SZ];

