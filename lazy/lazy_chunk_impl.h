/*
 *  lazy_chunk_impl.h
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 06.04.10.
 *  Copyright 2010 Fraunhofer Institut für Software- und Systemtechnik ISST.
 *
 *  This file is part of lazyObject.
 *	
 *	lazyObject is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *	
 *	lazyObject is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with lazyObject.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LAZY_CHUNK_IMPL_H_
#define _LAZY_CHUNK_IMPL_H_

#include "lazy_base_impl.h"

#include <stdio.h>
#include <sys/param.h>
#include <uuid/uuid.h>

#define CHUNK_FULL -1

struct lazy_chunk_s {
	LAZY_BASE_HEAD;
	
	int is_read_only;
	int flushed;
	
	uuid_t cid;
	
	FILE * file;
	void * file_data;
	int file_size;
	
	lz_db database;
	
	// index
	uint32_t * index;
	int index_length;
	int index_end;
	
	// data
	void * data;
	int data_size;
};

#pragma mark -
#pragma mark Chunk Lifecycle

struct lazy_chunk_s * lazy_chunk_open(lz_db db, uuid_t cid);
struct lazy_chunk_s * lazy_chunk_create(lz_db db);

#pragma mark -
#pragma mark Read & Write

lz_obj lazy_chunk_read(struct lazy_chunk_s * chunk, object_id_t oid);
int lazy_chunk_write(struct lazy_chunk_s * chunk, lz_obj obj);
void lazy_chunk_flush(struct lazy_chunk_s * chunk);

#endif // _LAZY_CHUNK_IMPL_H_
