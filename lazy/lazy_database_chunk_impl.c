/*
 *  lazy_database_chunk_impl.c
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

#include "lazy_database_chunk_impl.h"
#include "lazy_object_dispatch_group.h"
#include "lazy_object_impl.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <Block.h>

#define CHUNK_INIT_LENGTH 1024 * 1024

struct _chunk_header_s {
	uint32_t version;
	uint32_t cid;
	uint32_t index_length;
	uint32_t index_offset;
};

struct _object {
	uint32_t length;
	uint32_t num_ref;
	char data[];
};

void lazy_database_chunk_sync(struct lazy_database_chunk_s * chunk);

#pragma mark -
#pragma mark Live Cycle

struct lazy_database_chunk_s * lazy_database_chunk_open(lz_db db,
														uint32_t cid,
														enum lazy_database_chunk_mode mode) {
	char msg[1024];
	char filename[MAXPATHLEN];
	FILE * fd;
	int file_size;
	int init = 0;
	struct _chunk_header_s * chunk;
	
	// create chunk path
	snprintf(filename, MAXPATHLEN, "%s/chunks/%x", db->db_path, cid);
	
	// open file
	fd = fopen(filename, mode == CHUNK_RW ? "r+" : "r");
	if (!fd) {
		switch (errno) {
			case ENOENT:
				init = 1;
				fd = fopen(filename, "w+");
				if (!fd) {
					strerror_r(errno, msg, 1024);
					ERR("Could not open chunk '%s': %s", filename, msg);
					return 0;
				}
				
				if (ftruncate(fileno(fd), CHUNK_INIT_LENGTH)) {
					strerror_r(errno, msg, 1024);
					ERR("Could not truncate new chunk '%s': %s", filename, msg);
					fclose(fd);
					return 0;
				}
				
				break;
				
			default:
				strerror_r(errno, msg, 1024);
				ERR("Could not open chunk '%s': %s", filename, msg);
				return 0;
		}
	}
	
	// get file size
	fseek(fd, 0L, SEEK_END);
	file_size = ftell(fd);
	
	// mmap chunk
	chunk = mmap(0, file_size, mode == CHUNK_RW ? PROT_READ | PROT_WRITE : PROT_READ, MAP_SHARED, fileno(fd), 0);
	if (!chunk) {
		strerror_r(errno, msg, 1024);
		ERR("Could not mmap chunk '%s': %s", filename, msg);
		fclose(fd);
		return 0;
	}
	
	// init chunk if needed
	if (init) {
		chunk->version = 1;
		chunk->cid = cid;
		chunk->index_length = 0;
		chunk->index_offset = 0;
	}
	
	if (chunk->version != 1) {
		ERR("Expecting chunk version 1, got version %i in chunk '%s'.", chunk->version, filename);
		munmap(chunk, file_size);
		fclose(fd);
		return 0;
	}
	
	struct lazy_database_chunk_s * result = malloc(sizeof(struct lazy_database_chunk_s));
	if (result) {
		result->retain_count = 1;
		result->queue = dispatch_queue_create(0, 0);
		
		result->mode = mode;
		
		result->cid = cid;
		strcpy(result->filename, filename);
		result->file_size = file_size;
		result->file = fd;
		
		result->chunk = chunk;
		
		if (mode == CHUNK_RW) {
			result->data = (void*)chunk + sizeof(struct _chunk_header_s);
			
			result->index_length = chunk->index_length + 100;
			result->index_end = chunk->index_length;
			result->index = calloc(result->index_length, sizeof(uint32_t));
			memcpy(result->index,
				   result->data + chunk->index_offset,
				   sizeof(uint32_t) * chunk->index_length);
			result->index[result->index_end] = chunk->index_offset;
		} else {
			result->data = (void*)chunk + sizeof(struct _chunk_header_s);
			result->index = result->data + chunk->index_offset;
		}
	} else {
		ERR("Could not allocate memory for a new chunk.");
		munmap(chunk, file_size);
		fclose(fd);
	}
	return result;
}


void lazy_database_chunk_retain(struct lazy_database_chunk_s * chunk) {
	dispatch_group_async(*lazy_object_get_dispatch_group(), chunk->queue, ^{
        chunk->retain_count++;
        DBG("<%i> Retain count increased.", chunk);
    });
}


void lazy_database_chunk_release(struct lazy_database_chunk_s * chunk) {
	dispatch_group_async(*lazy_object_get_dispatch_group(), chunk->queue, ^{
        if (chunk->retain_count > 1) {
            chunk->retain_count--;
            DBG("<%i> Retain count decreased.", chunk);
        } else {
            DBG("<%i> Retain count of chunk reaches 0.", chunk);
            dispatch_group_async(*lazy_object_get_dispatch_group(), dispatch_get_global_queue(0, 0), ^{
                // release dispatch queue and free memory
				
				if (chunk->mode == CHUNK_RW) {
					lazy_database_chunk_sync(chunk);
				}
				
				munmap(chunk->chunk, chunk->file_size);
				fclose(chunk->file);
				
				DBG("<%i> Releasing chunk dispatch queue.", chunk);
                dispatch_release(chunk->queue);
                DBG("<%i> Dealloc memory.", chunk);
				if (chunk->mode == CHUNK_RW) {
					free(chunk->index);
				}
                free(chunk);
            });
        };
    });
}

#pragma mark -
#pragma mark Sync Chunk

void lazy_database_chunk_sync(struct lazy_database_chunk_s * chunk) {
	dispatch_sync(chunk->queue, ^{
		chunk->chunk->index_length = chunk->index_end;
		chunk->chunk->index_offset = chunk->index[chunk->index_end];
		memcpy(chunk->data + chunk->index[chunk->index_end],
			   chunk->index,
			   sizeof(uint32_t) * chunk->chunk->index_length);
		msync(chunk->chunk, chunk->file_size, MS_SYNC);
    });
}


#pragma mark -
#pragma mark Read & Write Object

lz_obj lazy_database_chunk_read_object(struct lazy_database_chunk_s * chunk, uint32_t oid) {
	struct _object * obj = chunk->data + chunk->index[oid];
	struct lazy_object_id_s id;
	id.cid = chunk->cid;
	id.oid = oid;
	lz_obj result = lz_obj_unmarshal(id, obj->data + obj->num_ref * sizeof(struct lazy_object_id_s), obj->length, ^(void * d, uint32_t l){
		lazy_database_chunk_release(chunk);
	}, obj->num_ref, (struct lazy_object_id_s *)obj->data);
	
	if (result) {
		lazy_database_chunk_retain(chunk);
	}
	
	return result;
}

uint32_t lazy_database_chunk_write_object(struct lazy_database_chunk_s * chunk, lz_obj obj) {
	__block uint32_t result;
	dispatch_sync(chunk->queue, ^{
        
		int bytes_to_write = sizeof(uint32_t) * 2 + sizeof(struct lazy_object_id_s) * obj->_number_of_references + obj->_length;
		
		// TODO: check file size and call ftruncate if needed
		uint32_t oid = chunk->index_end;
		struct _object * data = chunk->data + chunk->index[chunk->index_end];
		data->length = obj->_length;
		data->num_ref = obj->_number_of_references;
		memcpy(data->data,
			   obj->_ref_ids,
			   sizeof(struct lazy_object_id_s) * obj->_number_of_references);
		memcpy(data->data + sizeof(struct lazy_object_id_s) * obj->_number_of_references,
			   obj->_data,
			   obj->_length);
		
		// TODO: extend index list if needed
		chunk->index_end++;
		chunk->index[chunk->index_end] = chunk->index[oid] + bytes_to_write;
		
		obj->_dealloc(obj->_data, obj->_length);
		Block_release(obj->_dealloc);
		lazy_database_chunk_retain(chunk);
		obj->_dealloc = Block_copy(^(void * d, uint32_t l){
			lazy_database_chunk_release(chunk);
		});
		
		obj->_data = data->data + sizeof(struct lazy_object_id_s) * obj->_number_of_references;
		result = oid;
    });
	return result;
}
