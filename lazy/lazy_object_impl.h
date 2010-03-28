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

struct lazy_reference_s {
    struct lazy_object_s * _obj_handle;
};

struct lazy_object_s {
    // reference livecycle
    int _retain_count;
    dispatch_queue_t _obj_queue;
    
    // references to other objects
    uint16_t _number_of_references;
    struct lazy_reference_s * _references;
    
    // custom deallocator
    void (^_dealloc)(void * data, uint32_t size);
    
    // payload
    uint32_t _length;
    void * _data;
};

#pragma mark -
#pragma mark Unmarshal Object

lz_obj lz_obj_unmarshal(void * data,
						uint32_t length,
						void(^dealloc)(void * data, uint32_t length),
						uint16_t num_ref, uint16_t * refs);

#endif // _LAZY_OBJECT_IMPL_H_
