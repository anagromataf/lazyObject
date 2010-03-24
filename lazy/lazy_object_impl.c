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

#include <stdlib.h>
#include <stdarg.h>

#pragma mark -
#pragma mark Object Livecycle

lz_obj lz_obj_new(void * data,
              uint32_t length,
              void(^dealloc)(void * data, uint32_t length),
              uint16_t num_ref, ...) {
    struct lazy_object_s * obj = malloc(sizeof(struct lazy_object_s));
    if (obj) {
        obj->_obj_queue = dispatch_queue_create(NULL, NULL);
        obj->_retain_count = 1;
        obj->_length = length;
        obj->_data = data;
        obj->_dealloc = dealloc;
        
        // set up references
        obj->_number_of_references = num_ref;
        obj->_references = malloc(sizeof(struct lazy_reference_s) * num_ref);
        if (obj->_references) {
            va_list refs;
            va_start(refs, num_ref);
            int loop;
            for (loop = 0; loop < num_ref; loop++) {
                struct lazy_object_s * ref = va_arg(refs, struct lazy_object_s *);
                lz_obj_retain(ref);
                obj->_references[loop]._obj_handle = ref;
            }
            va_end(refs);
        } else {
            free(obj);
            obj = 0;
            dealloc(data, length);
        }
        DBG("<%i> New object created.", obj);
    } else {
        ERR("Could not allocate memory to create a new object.");
    }
    return obj;
}

void lz_obj_retain(struct lazy_object_s * obj) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), obj->_obj_queue, ^{
        obj->_retain_count++;
        DBG("<%i> Retain count increased.", obj);
    });
}

void lz_obj_release(struct lazy_object_s * obj) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), obj->_obj_queue, ^{
        if (obj->_retain_count > 1) {
            obj->_retain_count--;
            DBG("<%d> Retain count decreased.", obj);
        } else {
            DBG("<%i> Retain count reaches 0.", obj);
            dispatch_group_async(*lazy_object_get_dispatch_group(), dispatch_get_global_queue(0, 0), ^{
                // dealloc payload
                DBG("<%i> Calling custom dealloc to free the payload.", obj);
                obj->_dealloc(obj->_data, obj->_length);
                                     
                // release references
                DBG("<%i> Releasing %i references.", obj, obj->_number_of_references);
                int loop;
                for (loop=0; loop < obj->_number_of_references; loop++) {
                    lz_obj_release(obj->_references[loop]._obj_handle);
                }
                                     
                // release dispatch queue and free memory
                DBG("<%i> Releasing object dispatch queue.", obj);
                dispatch_release(obj->_obj_queue);
                DBG("<%i> Dealloc memory.", obj);
                free(obj);
            });
        };
    });
}

int lz_obj_rc(lz_obj obj) {
    __block int rc;
    dispatch_sync(obj->_obj_queue, ^{
        rc = obj->_retain_count;
    });
    return rc;
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
    return obj->_number_of_references;
}

lz_obj lz_obj_ref(lz_obj obj, uint16_t pos) {
    lz_obj result = lz_obj_weak_ref(obj, pos);
    if (result) {
        lz_obj_retain(result);
    }
    return result;
}

lz_obj lz_obj_weak_ref(lz_obj obj, uint16_t pos) {
    if (obj->_number_of_references > pos) {
        lz_obj result = obj->_references[pos]._obj_handle;
        if (result) {
            return result;
        } else {
            // TODO: Create a new handle
            return 0;
        }
    } else {
        DBG("<%i> Index error. (number of references: %i; requested position: %i)", obj, obj->_number_of_references, pos);
        return 0;
    }
}

#pragma mark -
#pragma mark Check Same Object

void lz_obj_sync(lz_obj obj, void(^handle)(void * data, uint32_t length)) {
    DBG("<%i> Applying synchronous 'payload block'.", obj);
    handle(obj->_data, obj->_length);
}

void lz_obj_async(lz_obj obj, void(^handle)(void * data, uint32_t length)) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), obj->_obj_queue, ^{
        DBG("<%i> Applying asynchronous 'payload function'.", obj);
        handle(obj->_data, obj->_length);
    });
}

