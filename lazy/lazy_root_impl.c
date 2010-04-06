/*
 *  lazy_root_impl.h
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

#include "lazy_root_impl.h"
#include "lazy_object_dispatch_group.h"

#include <stdlib.h>

#pragma mark -
#pragma mark Root Objects

lz_obj lz_root_get(lz_root root) {
    __block lz_obj result;
    dispatch_sync(root->queue, ^{
        result = lz_retain(root->root_obj);
        DBG("<%i> Get object <%i>", root, result);
    });
    return result;
}

void lz_root_set(lz_root root, lz_obj obj) {
    dispatch_group_async(lazy_object_get_dispatch_group(), root->queue, ^{
        DBG("<%i> Set object <%i>", root, obj);
        if (!lz_obj_same(obj, root->root_obj)) {
            lz_release(root->root_obj);
            root->root_obj = lz_retain(obj);;
        }
    });    
}

void lz_root_del(lz_root root) {
    dispatch_group_async(lazy_object_get_dispatch_group(), root->queue, ^{
        lz_release(root->root_obj);
        root->root_obj = 0;
    }); 
}

