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

struct lazy_object_id_s {
	uint32_t cid;
	uint32_t oid;
};

struct lazy_object_s {
    LAZY_BASE_HEAD
	
	int _temporary;
	struct lazy_object_id_s _id;
	dispatch_semaphore_t _semaphore;
    
    // references to other objects
    uint32_t _number_of_references;
	struct lazy_object_id_s * _ref_ids;
	struct lazy_object_s ** _ref_objs;
	lz_db _db;
    
    // custom deallocator
    void (^payload_dealloc)();
    
    // payload
    uint32_t _length;
    void * _data;
};

#pragma mark -
#pragma mark Unmarshal Object

lz_obj lz_obj_unmarshal(struct lazy_object_id_s id,
						void * data,
						uint32_t length,
						void(^dealloc)(),
						uint16_t num_ref, struct lazy_object_id_s * refs);

#endif // _LAZY_OBJECT_IMPL_H_
