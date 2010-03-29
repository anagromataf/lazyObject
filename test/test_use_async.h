/*
 *  test_use_async.h
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

#ifndef _TEST_USE_ASYNC_H_
#define _TEST_USE_ASYNC_H_

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <lazy.h>

START_TEST (test_use_async) {
    char * text = "Ein Testtext!";
    
    __block int dealloc_called = 0;
    
    lz_obj obj = lz_obj_new(text, strlen(text) + 1, ^{dealloc_called = 1;}, 0, 0);
    
    fail_if(obj == 0);
    fail_unless(lz_obj_rc(obj) == 1);
    
    lz_obj_async(obj, ^(void * data, uint32_t size){
        fail_unless(strcmp(data, text) == 0);
    });
    
    lz_obj_release(obj);
    lz_wait_for_completion();
    fail_if(dealloc_called == 0);
} END_TEST

#endif // _TEST_USE_ASYNC_H_
