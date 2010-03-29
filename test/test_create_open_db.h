/*
 *  test_create_open_db.c
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 27.03.10.
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

#ifndef _TEST_CREATE_OPEN_DB_H_
#define _TEST_CREATE_OPEN_DB_H_

#include <check.h>
#include <lazy.h>

START_TEST (test_create_open_db) {
    
	lz_db db;
	
	db = lz_db_open("./tmp/test.db");
	fail_unless(lz_db_version(db) == 1);
	lz_release(db);
	lz_wait_for_completion();
	
	db = lz_db_open("./tmp/test.db");
	fail_unless(lz_db_version(db) == 1);
	lz_release(db);
	lz_wait_for_completion();
	
} END_TEST

#endif // _TEST_CREATE_OPEN_DB_H_
