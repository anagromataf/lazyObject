/*
 *  check_lazy_object.c
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

#include <check.h>
#include <stdarg.h>
#include <fcntl.h>
#include <syslog.h>

#include "test_create_lazy_object.h"
#include "test_create_lazy_object_list.h"
#include "test_use_async.h"
#include "test_root_handle.h"
#include "test_create_open_db.h"
#include "test_chunk_write.h"
#include "test_chunk_swapping.h"

#pragma mark -
#pragma mark Fixtures

void setup() {
    system("mkdir ./tmp");
}

void teardown() {
    system("rm -rf ./tmp");
}

#pragma mark -
#pragma mark Lazy Object Suites

Suite * lazy_object_suite(void) {
    
    Suite *s = suite_create("Lazy Object");
    
    TCase *tc_core = tcase_create("Core");
    tcase_add_checked_fixture (tc_core, setup, teardown);
    
    tcase_add_test(tc_core, test_create_lazy_object);
    tcase_add_test(tc_core, test_create_lazy_object_list);
    tcase_add_test(tc_core, test_use_async);
    tcase_add_test(tc_core, test_root_handle);
    tcase_add_test(tc_core, test_create_open_db);
    tcase_add_test(tc_core, test_chunk_write);
    tcase_add_test(tc_core, test_chunk_swapping);
	
    suite_add_tcase(s, tc_core);
    
    return s;
}

#pragma mark -
#pragma mark Main Suite

Suite * main_suite(void) {
    Suite *s = suite_create ("Main Suite");
    TCase *tc_core = tcase_create("Core");
    suite_add_tcase(s, tc_core);
    return s;
}

#pragma mark -
#pragma mark Running Tests

int main(int argc, char ** argv) {
    int number_failed;
    
    openlog("check lazy object", LOG_PERROR, LOG_USER);
    
    SRunner *sr = srunner_create(main_suite());
    srunner_set_fork_status(sr, CK_NOFORK);
    
    // add the suites to the main suite
    srunner_add_suite(sr, lazy_object_suite());
    
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
