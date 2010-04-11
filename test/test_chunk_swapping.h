/*
 *  test_chunk_swapping.h
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 06.04.10.
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

#ifndef _TEST_CHUNK_SWAPPING_H_
#define _TEST_CHUNK_SWAPPING_H_

#include <check.h>
#include <lazy.h>

#include "lazy_database_impl.h"

START_TEST (test_chunk_swapping) {
	
	int num_obj = 100000;
	
    struct data_s {
        uint64_t i;
        int8_t foo[1024];
    };
    
    __block object_id_t * oids = calloc(sizeof(object_id_t), num_obj);
    
    lz_db db = lz_db_open("./tmp/test.db");
    dispatch_apply(num_obj, dispatch_get_global_queue(0, 0), ^(size_t loop){
        
        struct data_s * data = malloc(sizeof(struct data_s));
        fail_if(data == 0);
        data->i = loop;
        for (int i = 0; i < 1024; i++) {
            data->foo[i] = -1;
        }
        lz_obj obj = lz_obj_new(data, sizeof(struct data_s), ^{
            free(data);
        }, 0);
		 oids[loop] = lazy_database_write_object(db, obj);
		 lz_release(obj);
    });
    lz_release(db);
    lz_wait_for_completion();
    
    db = lz_db_open("./tmp/test.db");
    dispatch_apply(num_obj, dispatch_get_global_queue(0, 0), ^(size_t loop){
        lz_obj obj = lazy_database_read_object(db, oids[loop]);
        fail_if(obj == 0);
        if (obj) {
            lz_obj_sync(obj, ^(void * d, uint32_t s){
                fail_unless(sizeof(struct data_s) == s);
                fail_unless(((struct data_s *)d)->i == loop);
            });
            lz_release(obj);
        }
    });
    lz_release(db);
    lz_wait_for_completion();
    
    free(oids);
} END_TEST

#endif // _TEST_CHUNK_SWAPPING_H_
