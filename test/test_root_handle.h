/*
 *  test_root_handle.h
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

#ifndef _TEST_ROOT_HANDLE_H_
#define _TEST_ROOT_HANDLE_H_

#include <check.h>
#include <lazy.h>

START_TEST (test_root_handle) {
    
    // setup db and create a root handle
    lz_db db = lz_db_open("./tmp/test.db");
    lz_root root = lz_db_root(db, "index");
    
    // create an object and set it as root for index.
    lz_obj obj = lz_obj_new("Foo", 4, ^{}, 0);
    lz_root_set_sync(root, obj, ^{});

    lz_release(root);
    
    // get the root object and compare it
    // with the previous set object
    root = lz_db_root(db, "index");
    __block lz_obj rootObj;
    lz_root_get_sync(root,^(lz_obj obj){
        rootObj = obj;
    });
    fail_unless(lz_obj_same(obj, rootObj));
    
    // release the objects
    lz_release(obj);
    lz_release(rootObj);
    
    lz_root_del_sync(root, ^{});    
    lz_root_get_sync(root, ^(lz_obj obj){
        fail_unless(obj == 0);
    });
    
    // release the other handles
    lz_release(root);
    lz_release(db);
    lz_wait_for_completion();

} END_TEST

#endif // _TEST_ROOT_HANDLE_H_
