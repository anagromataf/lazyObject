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
#pragma mark Root Handle Livecycle

void lz_root_retain(lz_root root) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), root->_root_queue, ^{
        root->_retain_count++;
        DBG("<%i> Retain count increased.", root);
    });
}

void lz_root_release(lz_root root) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), root->_root_queue, ^{
        if (root->_retain_count > 1) {
            root->_retain_count--;
            DBG("<%d> Retain count decreased.", root);
        } else {
            DBG("<%i> Retain count reaches 0.", root);
            dispatch_group_async(*lazy_object_get_dispatch_group(), dispatch_get_global_queue(0, 0), ^{
                if (root->_root_obj) {
                    lz_obj_release(root->_root_obj);
                }
                // release dispatch queue and free memory
                DBG("<%i> Releasing reference to the database.");
                lz_db_release(root->_database);
                DBG("<%i> Releasing root dispatch queue.", root);
                dispatch_release(root->_root_queue);
                DBG("<%i> Free internal memory.", root);
                free(root->_name);
                DBG("<%i> Dealloc memory.", root);
                free(root);
            });
        };
    });
}

#pragma mark -
#pragma mark Root Objects

lz_obj lz_root_get(lz_root root) {
    __block lz_obj result;
    dispatch_sync(root->_root_queue, ^{
        result = root->_root_obj;
        if (result) {
            // retain the root object
            // to return an object with rc +1
            lz_obj_retain(result);
        }
        DBG("<%i> Get object <%i> as root for '%s'", root, result, root->_name);
    });
    return result;
}

void lz_root_set(lz_root root, lz_obj obj) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), root->_root_queue, ^{
        DBG("<%i> Set object <%i> as root for '%s'", root, obj, root->_name);
        if (!lz_obj_same(obj, root->_root_obj)) {
            // release the old root object
            if (root->_root_obj) {
                lz_obj_release(root->_root_obj);
            }
            // retain the new root object
            if (obj) {
                lz_obj_retain(obj);
                root->_root_obj = obj;
            }
        }
    });    
}

void lz_root_del(lz_root root) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), root->_root_queue, ^{
        if (root->_root_obj) {
            // release the old root object
            lz_obj_release(root->_root_obj);
            root->_root_obj = 0;
        }
    }); 
}

