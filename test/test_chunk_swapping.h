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
	
	int num_obj = 1000;
	
	struct data_s {
		uint64_t i;
		char foo[1024 * 10];
	} data;
	
	int size = sizeof(struct data_s);
    
	lz_obj_id oids[num_obj]; 
	
	lz_db db = lz_db_open("./tmp/test.db");
	
	for (int loop = 0; loop < num_obj; loop++) {
		data.i = loop;
		lz_obj obj = lz_obj_new(&data, size, ^{}, 0);
		lazy_database_write_object(db, obj);
		oids[loop].oid	= obj->oid;
		uuid_copy(oids[loop].cid, obj->cid);
		lz_release(obj);
	}
	
	lz_release(db);
	lz_wait_for_completion();
	
	db = lz_db_open("./tmp/test.db");
	
	for (int loop = 0; loop < num_obj; loop++) {
		lz_obj obj = lazy_database_read_object(db, oids[loop]);
		fail_if(obj == 0);
		if (obj) {
			lz_obj_sync(obj, ^(void * d, uint32_t s){
				struct data_s * data = d;
				fail_unless(size == s);
				fail_unless(data->i == loop);
			});
			lz_release(obj);
		}
	}
	
	lz_release(db);
	lz_wait_for_completion();
	
} END_TEST

#endif // _TEST_CHUNK_SWAPPING_H_
