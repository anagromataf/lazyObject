/*
 *  lazy_chunk_impl.c
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

#include "lazy_chunk_impl.h"

#include "lazy_database_impl.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>

#define INDEX_TYPE uint32_t

#define CHUNK_DATA_SIZE 1024 * 1024

struct _chunk_header_s {
	uint8_t version;
	object_id_t index_length;
};

struct _object_s {
	uint32_t data_size;
	uint16_t num_ref;
	char data[];
};

#pragma mark -
#pragma mark Chunk Lifecycle

struct lazy_chunk_s * lazy_chunk_open(lz_db db, uuid_t cid) {
	char msg[1024];
	uuid_string_t cids;
	char filename[MAXPATHLEN];
	FILE * file;
	int file_size;
	
	// create filename for this chunk
	uuid_unparse(cid, cids);
	snprintf(filename,
			 MAXPATHLEN,
			 "%s/chunks/%s",
			 db->filename,
			 cids);
	
	// open file
	file = fopen(filename, "r");
	if (!file) {
		strerror_r(errno, msg, 1024);
		ERR("Could not open chunk '%s': %s", filename, msg);
		return 0;
	}
	
	// get file size
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	
	// create the chunk handle
	struct lazy_chunk_s * chunk = malloc(sizeof(struct lazy_chunk_s));
	if (chunk) {
		chunk->is_read_only = 1;
		chunk->flushed = 1;
		
		uuid_copy(chunk->cid, cid);
		chunk->file = file;
		chunk->file_size = file_size;
		chunk->database = db;
		
		// mmap file
		chunk->file_data = mmap(0,
								chunk->file_size,
								PROT_READ,
								MAP_PRIVATE,
								fileno(chunk->file),
								0);
		if (chunk->file_data == MAP_FAILED) {
			// TODO: Error handling
			strerror_r(errno, msg, 1024);
			ERR("Could not mmap chunk '%s': %s", filename, msg);
			assert(0);
		}
		
		struct _chunk_header_s * header = chunk->file_data;
		assert(header->version == 1);
		chunk->index = chunk->file_data
		             + sizeof(struct _chunk_header_s);
		chunk->index_length = header->index_length;
		
		chunk->data = chunk->file_data
		            + sizeof(struct _chunk_header_s)
		            + sizeof(INDEX_TYPE) * header->index_length;
		
		LAZY_BASE_INIT(chunk, ^{
			munmap(chunk->file_data, chunk->file_size);
			fclose(chunk->file);
			DBG("Dealloc chunk '%s'.", cids);
		});
		DBG("Chunk '%s' opened.", cids);
	} else {
		strerror_r(errno, msg, 1024);
		ERR("Could not allocate memory for a chunk: %s", msg);
		fclose(file);
	}
	return chunk;
}

struct lazy_chunk_s * lazy_chunk_create(lz_db db) {
	char msg[1024];
	char filename[MAXPATHLEN];
	uuid_t cid;
	uuid_string_t cids;
	FILE * file;
	int file_size;
	
	// TODO: get next free chunk id
	uuid_generate(cid);
	
	// create filename for this chunk
	uuid_unparse(cid, cids);
	snprintf(filename,
			 MAXPATHLEN,
			 "%s/chunks/%s",
			 db->filename,
			 cids);
	
	// open file
	file = fopen(filename, "w+x");
	if (!file) {
		strerror_r(errno, msg, 1024);
		ERR("Could not create chunk '%s': %s", filename, msg);
		return 0;
	}
	
	// create the chunk handle
	struct lazy_chunk_s * chunk = malloc(sizeof(struct lazy_chunk_s));
	if (chunk) {
		chunk->is_read_only = 0;
		chunk->flushed = 0;
		
		uuid_copy(chunk->cid, cid);
		chunk->file = file;
		chunk->file_size = file_size;
		chunk->database = db;
		
		// create empty index
		chunk->index_length = 10;
		chunk->index = malloc(sizeof(INDEX_TYPE) * chunk->index_length);
		assert(chunk->index);
		chunk->index_end = 0;
		chunk->index[chunk->index_end] = 0;
		
		// create data block
		chunk->data_size = CHUNK_DATA_SIZE;
		chunk->data = malloc(chunk->data_size);
		assert(chunk->data);
		
		LAZY_BASE_INIT(chunk, ^{
			lazy_chunk_flush(chunk);
			fclose(chunk->file);
			free(chunk->index);
			free(chunk->data);
			DBG("Dealloc chunk '%s'.", cids);
		});
		DBG("Chunk '%s' created.", cids);
	} else {
		strerror_r(errno, msg, 1024);
		ERR("Could not allocate memory for a chunk: %s", msg);
		fclose(file);
	}
	return chunk;
}

#pragma mark -
#pragma mark Read & Write

lz_obj lazy_chunk_read(struct lazy_chunk_s * chunk, object_id_t oid) {
	__block lz_obj result;
	dispatch_sync(chunk->queue, ^{
		uuid_string_t cids;
		uuid_unparse(chunk->cid, cids);
		DBG("Unmarshal object with id %i in chunk %s.", oid, cids);
		struct _object_s * obj = chunk->data + chunk->index[oid];
		result = lz_obj_unmarshal(chunk->database,
								  oid,
								  chunk->cid,
								  obj->data + obj->num_ref * sizeof(struct lazy_object_id_s), 
								  obj->data_size,
								  ^{RELEASE(chunk);},
								  obj->num_ref,
								  (struct lazy_object_id_s *)obj->data);
		if (result) {
			RETAIN(chunk);
		}
	});
	return result;
}

int lazy_chunk_write(struct lazy_chunk_s * chunk, lz_obj obj) {
	__block int result;
	dispatch_sync(chunk->queue, ^{
		
		uuid_string_t cids;
		uuid_unparse(chunk->cid, cids);
		
		if (chunk->is_read_only) {
			ERR("Chunk '%s' is read only.", cids);
			result = -1;
			return;
		}
		
		if (chunk->flushed) {
			ERR("Chunk '%s' flushed.", cids);
			result = -1;
			return;
		}
		
		DBG("Try to write object to chunk '%s'.", cids);
		
		// bytes needed for this object
		int bytes_to_write = sizeof(uint32_t) + sizeof(uint16_t)
					       + sizeof(lz_obj_id) * obj->num_references
		                   + obj->payload_length;
		
		int expected_chunk_data_size = chunk->index[chunk->index_end] + bytes_to_write;
		
		if (expected_chunk_data_size > chunk->data_size) {
			DBG("Chunk '%s' is full.", cids);
			result = -1;
		} else {
			int oid = chunk->index_end;
			struct _object_s * obj_data = chunk->data + chunk->index[oid];
			
			// copy object data
			obj_data->num_ref = obj->num_references;
			obj_data->data_size = obj->payload_length;
			
			memcpy(obj_data->data,
				   obj->reference_ids,
				   sizeof(struct lazy_object_id_s) * obj->num_references);
			
			memcpy(obj_data->data + sizeof(struct lazy_object_id_s) * obj->num_references,
				   obj->payload_data,
				   obj->payload_length);
			
			// extend index if nededed
			// TODO: check max index length
			if (chunk->index_length == chunk->index_end + 1) {
				chunk->index_length += 10;
				chunk->index = realloc(chunk->index, sizeof(INDEX_TYPE) * chunk->index_length);
				DBG("Extending index of chunk '%s' to %i.", cids, chunk->index_length);
			}
			
			// set next free position in the chunk
			chunk->index_end++;
			chunk->index[chunk->index_end] = chunk->index[oid] + bytes_to_write;
			
			// dealloc the user payload and
			// set the payload form the chunk
			obj->payload_dealloc();
			Block_release(obj->payload_dealloc);
			RETAIN(chunk);
			obj->payload_dealloc = Block_copy(^{
				RELEASE(chunk);
			});
			
			obj->payload_data = obj_data->data + sizeof(struct lazy_object_id_s) * obj->num_references;
			
			uuid_copy(obj->cid, chunk->cid);
			obj->oid = oid;
			
			obj->is_temp = 0;
			
			result = 0;
		}
	});
	return result;
}

void lazy_chunk_flush(struct lazy_chunk_s * chunk) {
	dispatch_sync(chunk->queue, ^{
		if (chunk->is_read_only != 0 || chunk->flushed != 0) {
			return;
		}
		
		uuid_string_t cids;
		uuid_unparse(chunk->cid, cids);
		DBG("Flushing chunk '%s' (index_length: %i)", cids, chunk->index_end);
		
		// build header
		struct _chunk_header_s header;
		header.version = 1;
		header.index_length = chunk->index_end;
		
		// flush header
		fwrite(&header, sizeof(struct _chunk_header_s), 1, chunk->file);
		
		// flush index
		fwrite(chunk->index, sizeof(INDEX_TYPE), chunk->index_end, chunk->file);
		
		// flush data
		fwrite(chunk->data, 1, chunk->index[chunk->index_end], chunk->file);
		
		fsync(chunk->file);
		
		// set flushed
		chunk->flushed = 1;
		
	});
}


