/*
 *  lazy_logging_impl.h
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 22.03.10.
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

#ifndef _LAZY_OBJECT_IMPL_H_
#define _LAZY_OBJECT_IMPL_H_

#include <lazy.h>
#include <dispatch/dispatch.h>

#include "lazy_base_impl.h"

typedef uint64_t object_id_t;

struct lazy_object_s {
	LAZY_BASE_HEAD
	
	// object persistence
	object_id_t oid;
	int is_temp;
	lz_db database;
	dispatch_semaphore_t write_lock;

	// references to other objects
	uint16_t num_references;
	object_id_t * reference_ids;
	lz_obj * reference_objs;
    
	// object payload
	void (^payload_dealloc)();
	uint32_t payload_length;
	void * payload_data;
};

#pragma mark -
#pragma mark Unmarshal Object

lz_obj lz_obj_unmarshal(lz_db db,
                        object_id_t oid,
							void * data,
							uint32_t length,
							void(^dealloc)(),
							uint16_t num_ref,
							object_id_t * refs);

#endif // _LAZY_OBJECT_IMPL_H_
