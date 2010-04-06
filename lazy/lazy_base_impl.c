/*
 *  lazy_base_impl.c
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 29.03.10.
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

#include "lazy_base_impl.h"
#include "lazy_object_dispatch_group.h"

#include <Block.h>

#pragma mark -
#pragma mark Memory Management

void * lz_retain(lz_base obj) {
    if (obj.base) {
        dispatch_group_async(lazy_object_get_dispatch_group(), obj.base->queue, ^{
            obj.base->rc++;
            DBG("<%i> Retain count increased.", obj);
        });
    }
    return obj.base;
}

void * lz_release(lz_base obj) {
    if (obj.base) {
        dispatch_group_async(lazy_object_get_dispatch_group(), obj.base->queue, ^{
            if (obj.base->rc > 1) {
                obj.base->rc--;
                DBG("<%d> Retain count decreased.", obj);
            } else {
                DBG("<%i> Retain count reaches 0.", obj);
                dispatch_group_async(lazy_object_get_dispatch_group(), dispatch_get_global_queue(0, 0), ^{
                    obj.base->dealloc();
                    Block_release(obj.base->dealloc);
                    dispatch_release(obj.base->queue);
                    free(obj);
                });
            };
        });
    }
    return obj.base;
}

int lz_rc(lz_base obj) {
    __block int rc;
    dispatch_sync(obj.base->queue, ^{
        rc = obj.base->rc;
    });
    return rc;
}
