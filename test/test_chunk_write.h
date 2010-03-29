/*
 *  test_chunk_write.h
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 28.03.10.
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

#ifndef _TEST_CHUNK_WRITE_H_
#define _TEST_CHUNK_WRITE_H_

#include <check.h>
#include <lazy.h>

#include "lazy_database_chunk_impl.h"

START_TEST (test_chunk_write) {

	struct lazy_database_chunk_s * chunk;
	
    lz_db db = lz_db_open("./tmp/test.db");
    lz_obj objA = lz_obj_new("Foo", 4, ^{}, 0, 0);
	lz_obj objB = lz_obj_new("Bar", 4, ^{}, 0, 0);
    
	chunk = db->chunk;
	
	int oidA = lazy_database_chunk_write_object(chunk, objA);
	fail_unless(oidA == 0);
	
	int oidB = lazy_database_chunk_write_object(chunk, objB);
	fail_unless(oidB == 1);
	
	lz_release(objA);
	lz_release(objB);
	lz_release(db);
    lz_wait_for_completion();
	
	
	db = lz_db_open("./tmp/test.db");
    
	chunk = db->chunk;
	
	objA = lazy_database_chunk_read_object(chunk, oidA);
	lz_obj_sync(objA, ^(void * data, uint32_t size){
        fail_unless(strcmp(data, "Foo") == 0);
    });
	
	objB = lazy_database_chunk_read_object(chunk, oidB);
	lz_obj_sync(objB, ^(void * data, uint32_t size){
        fail_unless(strcmp(data, "Bar") == 0);
    });
    
	lz_release(objA);
	lz_release(objB);
	lz_release(db);
    lz_wait_for_completion();
	
} END_TEST

#endif // _TEST_CHUNK_WRITE_H_