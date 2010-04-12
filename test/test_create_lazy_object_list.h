/*
 *  test_create_lazy_object_list.h
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

#ifndef _TEST_CREATE_LAZY_OBJECT_LIST_H_
#define _TEST_CREATE_LAZY_OBJECT_LIST_H_

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <lazy.h>

lz_obj create_string_obj(const char * str) {
    lz_obj result;
    char * value = malloc(strlen(str));
    value = strcpy(value, str);
    if (value) {
        result = lz_obj_new(value, strlen(value) + 1, ^{
            free(value);
        }, 0, 0);
        if (!result) {
            free(value);
        }
        return result;
    } else {
        return 0;
    }
}

START_TEST (test_create_lazy_object_list) {
	
    lz_obj strA = create_string_obj("A");
    lz_obj strB = create_string_obj("B");
    lz_obj strC = create_string_obj("C");
    
    char * text = "Eine Liste mit A, B und C!";
    
    __block lz_obj list = lz_obj_new(text, strlen(text) + 1, ^{}, 3, strA, strB, strC);
    
    fail_if(list == 0);
    
    lz_release(strA);
    lz_release(strB);
    lz_release(strC);
    
    lz_db db = lz_db_open("./tmp/test.db");
    lz_root root = lz_db_root(db, "list");
    lz_root_set_sync(root, list, ^{});
    lz_release(list);
    
    lz_release(root);
    lz_release(db);
    
    lz_wait_for_completion();
    
    db = lz_db_open("./tmp/test.db");
    root = lz_db_root(db, "list");
    lz_root_get_sync(root, ^(lz_obj obj){
        list = obj;
    });
    
    lz_obj_sync(list, ^(void * data, uint32_t size){
        fail_unless(strcmp(data, text) == 0);
        
        int loop;
        for (loop=0; loop < lz_obj_num_ref(list); loop++) {
            lz_obj r = lz_obj_weak_ref(list, loop);
            lz_obj_sync(r, ^(void * data, uint32_t size){
                switch (loop) {
                    case 0:
                        fail_unless(strcmp(data, "A") == 0);
                        break;
                    case 1:
                        fail_unless(strcmp(data, "B") == 0);
                        break;
                    case 2:
                        fail_unless(strcmp(data, "C") == 0);
                        break;
                    default:
                        fail();
                }
            });
        }
    });
    
    lz_release(root);
    lz_release(db);
    lz_release(list);
	lz_wait_for_completion();
    
} END_TEST

#endif // _TEST_CREATE_LAZY_OBJECT_LIST_H_
