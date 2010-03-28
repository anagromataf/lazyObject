/*
 *  lazy_database_chunk_impl.h
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 28.03.10.
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

#ifndef _LAZY_DATABASE_CHUNK_IMPL_H_
#define _LAZY_DATABASE_CHUNK_IMPL_H_

#include <lazy.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/param.h>

#include <dispatch/dispatch.h>

#include "lazy_database_impl.h"

enum lazy_database_chunk_mode {
	CHUNK_RO,
	CHUNK_RW
};

struct lazy_database_chunk_s {
	int retain_count;
	dispatch_queue_t queue;
	
	enum lazy_database_chunk_mode mode;
	
	char filename[MAXPATHLEN];
	int file_size;
	FILE * file;
	
	struct _chunk_header_s * chunk;
	
	int index_length;
	int index_end;
	uint32_t * index;
	void * data;
};

#pragma mark -
#pragma mark Live Cycle

struct lazy_database_chunk_s * lazy_database_chunk_open(lz_db db,
														uint32_t cid,
														enum lazy_database_chunk_mode mode);

void lazy_database_chunk_retain(struct lazy_database_chunk_s * chunk);
void lazy_database_chunk_release(struct lazy_database_chunk_s * chunk);

#pragma mark -
#pragma mark Read & Write Object

lz_obj lazy_database_chunk_read_object(struct lazy_database_chunk_s * chunk, uint32_t oid);
uint32_t lazy_database_chunk_write_object(struct lazy_database_chunk_s * chunk, lz_obj obj);


#endif // _LAZY_DATABASE_CHUNK_IMPL_H_
