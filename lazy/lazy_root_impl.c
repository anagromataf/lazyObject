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
#include <assert.h>

#pragma mark -
#pragma mark Root Objects

void _get(lz_root root, void(^handler)(lz_obj obj)) {
    if (root->root_is_bound) {
        if (root->root_obj) {
            handler(lz_retain(root->root_obj));
        } else {
            root->root_obj = lazy_database_read_object(root->database, root->root_obj_id);
            handler(lz_retain(root->root_obj));
        }
    } else {
        handler(0);
    }
    Block_release(handler);
}

void lz_root_get_sync(lz_root root, void(^result_handler)(lz_obj)) {
    void(^handler)(lz_obj) = Block_copy(result_handler);
    dispatch_sync(root->queue, ^{
        _get(root, handler);
    });
}

void lz_root_get_async(lz_root root, void(^result_handler)(lz_obj)) {
    void(^handler)(lz_obj) = Block_copy(result_handler);
    dispatch_group_async(lazy_object_get_dispatch_group(), root->queue, ^{
        _get(root, handler);
    });
}

void _set(lz_root root, lz_obj obj, void(^handler)()) {
    if (!lz_obj_same(obj, root->root_obj)) {
        lz_release(root->root_obj);
        
        root->root_obj = lz_retain(obj);
        root->root_obj_id = lazy_database_write_object(root->database, obj);
        root->root_is_bound = 1;
        int objects_written = fwrite(&(root->root_obj_id), sizeof(object_id_t), 1, root->file);
        assert(objects_written == 1);
        fflush(root->file);
    }
    handler();
    Block_release(handler);
}

void lz_root_set_sync(lz_root root, lz_obj obj, void(^result_handler)()) {
    void(^handler)() = Block_copy(result_handler);
    dispatch_sync(root->queue, ^{
        _set(root, obj, handler);
    });   
}

void lz_root_set_async(lz_root root, lz_obj obj, void(^result_handler)()) {
    void(^handler)() = Block_copy(result_handler);
    dispatch_group_async(lazy_object_get_dispatch_group(), root->queue, ^{
        _set(root, obj, handler);
    });    
}

void _del(lz_root root, void(^handler)()) {
    if (root->root_is_bound) {
        lz_release(root->root_obj);
        root->root_obj = 0;
        root->root_is_bound = 0;
        object_id_t o = OBJECT_ID_UNKNOWN;
        int objects_written = fwrite(&o, sizeof(object_id_t), 1, root->file);
        assert(objects_written == 1);
        fflush(root->file);
    }
    handler();
    Block_release(handler);
}

void lz_root_del_sync(lz_root root, void(^result_handler)()) {
    void(^handler)() = Block_copy(result_handler);
    dispatch_sync(root->queue, ^{
        _del(root, handler);
    }); 
}

void lz_root_del_async(lz_root root, void(^result_handler)()) {
    void(^handler)() = Block_copy(result_handler);
    dispatch_group_async(lazy_object_get_dispatch_group(), root->queue, ^{
        _del(root, handler);
    }); 
}

