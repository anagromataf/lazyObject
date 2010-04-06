/*
 *  lazy_logging_impl.c
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

#include "lazy_object_impl.h"
#include "lazy_object_dispatch_group.h"
#include "lazy_database_impl.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <Block.h>

#pragma mark -
#pragma mark Object Livecycle

lz_obj lz_obj_new(void * data,
				  uint32_t length,
				  void(^dealloc)(),
				  uint16_t num_ref, ...) {
    struct lazy_object_s * obj = malloc(sizeof(struct lazy_object_s));
    if (obj) {
        LAZY_BASE_INIT(obj, ^{
            // release references
            DBG("<%i> Releasing %i references.", obj, obj->num_references);
            int loop;
            for (loop=0; loop < obj->num_references; loop++) {
                DBG("<%i> Releasing reference to <%i>.", obj, obj->reference_objs[loop]);
                lz_release(obj->reference_objs[loop]);
            }
            DBG("<%i> References released.", obj);
            
            DBG("<%i> Releasing reference to database <%i>.", obj, obj->database);
            lz_release(obj->database);
            
            dispatch_release(obj->write_lock);
            obj->payload_dealloc();
            Block_release(obj->payload_dealloc);
            free(obj->reference_ids);
            free(obj->reference_objs);
        });
        // set up references
        obj->num_references = num_ref;
        obj->reference_objs = calloc(num_ref, sizeof(struct lazy_object_s));
		obj->reference_ids = calloc(num_ref, sizeof(struct lazy_object_id_s));
        if (obj->reference_objs && obj->reference_ids) {
            va_list refs;
            va_start(refs, num_ref);
            int loop;
            for (loop = 0; loop < num_ref; loop++) {
                struct lazy_object_s * ref = va_arg(refs, struct lazy_object_s *);
                obj->reference_objs[loop] = lz_retain(ref);
            }
            va_end(refs);
            obj->write_lock = dispatch_semaphore_create(1);
            obj->is_temp = 1;
            obj->payload_length = length;
            obj->payload_data = data;
            obj->payload_dealloc = Block_copy(dealloc);
            obj->database = 0;
            DBG("<%i> New object created.", obj);
        } else {
			free(obj->reference_ids);
			free(obj->reference_objs);
            free(obj);
            obj = 0;
            dealloc();
        }
    } else {
        ERR("Could not allocate memory to create a new object.");
    }
    return obj;
}

#pragma mark -
#pragma mark Unmarshal Object

lz_obj lz_obj_unmarshal(lz_db db,
                        struct lazy_object_id_s id,
						void * data,
						uint32_t length,
						void(^dealloc)(),
						uint16_t num_ref, struct lazy_object_id_s * refs) {
	struct lazy_object_s * obj = malloc(sizeof(struct lazy_object_s));
    if (obj) {
        LAZY_BASE_INIT(obj, ^{
            // release references
            DBG("<%i> Releasing %i references.", obj, obj->num_references);
            int loop;
            for (loop=0; loop < obj->num_references; loop++) {
                DBG("<%i> Releasing reference to <%i>.", obj, obj->reference_objs[loop]);
                lz_release(obj->reference_objs[loop]);
            }
            DBG("<%i> References released.", obj);
            
            DBG("<%i> Releasing reference to database <%i>.", obj, obj->database);
            lz_release(obj->database);
            
            dispatch_release(obj->write_lock);
            obj->payload_dealloc();
            Block_release(obj->payload_dealloc);
            free(obj->reference_ids);
            free(obj->reference_objs);
        });
        // set up references
        obj->num_references = num_ref;
        obj->reference_objs = calloc(num_ref, sizeof(struct lazy_object_s));
		obj->reference_ids = calloc(num_ref, sizeof(struct lazy_object_id_s));
        if (obj->reference_objs && obj->reference_ids) {
			memcpy(obj->reference_ids, refs, sizeof(struct lazy_object_id_s) * num_ref);
            obj->write_lock = dispatch_semaphore_create(1);
            obj->is_temp = 0;
            obj->id = id;
            obj->payload_length = length;
            obj->payload_data = data;
            obj->payload_dealloc = Block_copy(dealloc);
            obj->database = lz_retain(db);
            DBG("<%i> New object created.", obj);
        } else {
			free(obj->reference_ids);
			free(obj->reference_objs);
            free(obj);
            obj = 0;
            dealloc();
        }
    } else {
        ERR("Could not allocate memory to create a new object.");
    }
    return obj;
}

#pragma mark -
#pragma mark Access Object References

int lz_obj_same(lz_obj obj1, lz_obj obj2) {
    if (obj1 == obj2) {
        return 1;
    } else {
        return 0;
    }
}

#pragma mark -
#pragma mark Access Object Payload

uint16_t lz_obj_num_ref(lz_obj obj) {
    return obj->num_references;
}

lz_obj lz_obj_ref(lz_obj obj, uint16_t pos) {
    lz_obj result = lz_obj_weak_ref(obj, pos);
    return lz_retain(result);
}

lz_obj lz_obj_weak_ref(lz_obj obj, uint16_t pos) {
    if (obj->num_references > pos) {
        lz_obj result = obj->reference_objs[pos];
        if (result) {
            return result;
        } else {
			lz_obj o = lazy_database_read_object(obj->database, obj->reference_ids[pos]);
            (obj->reference_objs[pos]) = o;
			return o;
        }
    } else {
        DBG("<%i> Index error. (number of references: %i; requested position: %i)", obj, obj->num_references, pos);
        return 0;
    }
}

#pragma mark -
#pragma mark Check Same Object

void lz_obj_sync(lz_obj obj, void(^handle)(void * data, uint32_t length)) {
    DBG("<%i> Applying synchronous 'payload block'.", obj);
    handle(obj->payload_data, obj->payload_length);
}

void lz_obj_async(lz_obj obj, void(^handle)(void * data, uint32_t length)) {
    dispatch_group_async(lazy_object_get_dispatch_group(), obj->queue, ^{
        DBG("<%i> Applying asynchronous 'payload function'.", obj);
        handle(obj->payload_data, obj->payload_length);
    });
}

