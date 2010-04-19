/*
 *  lazy.h
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

#ifndef _LAZY_H_
#define _LAZY_H_

#include <stdint.h>

typedef struct lazy_object_s * lz_obj;
typedef struct lazy_database_s * lz_db;
typedef struct lazy_root_s *lz_root;

typedef union {
    struct lazy_base_s * base;
    struct lazy_object_s * obj;
    struct lazy_database_s * db;
    struct lazy_root_s * root;
} lz_base __attribute__((transparent_union));

#pragma mark -
#pragma mark Work Scheduling

void lz_wait_for_completion();

#pragma mark -
#pragma mark Memory Management

void * lz_retain(lz_base obj);
void * lz_release(lz_base obj);

int lz_rc(lz_base obj);

#pragma mark -
#pragma mark Object Livecycle

lz_obj lz_obj_new(void * data,
                  uint32_t length,
                  void(^dealloc)(),
                  uint16_t num_ref, ...);

lz_obj lz_obj_new_v(void * data,
                    uint32_t length,
                    void(^dealloc)(),
                    uint16_t num_ref,
                    lz_obj * refs);

#pragma mark -
#pragma mark Check if objects are the same

int lz_obj_same(lz_obj obj1, lz_obj obj2);

#pragma mark -
#pragma mark Access Object Payload

void lz_obj_sync(lz_obj, void(^)(void * data, uint32_t length));
void lz_obj_async(lz_obj, void(^)(void * data, uint32_t length));

#pragma mark -
#pragma mark Access Object References

uint16_t lz_obj_num_ref(lz_obj obj);

lz_obj lz_obj_weak_ref(lz_obj obj, uint16_t pos);
lz_obj lz_obj_ref(lz_obj obj, uint16_t pos);

#pragma mark -
#pragma mark Database Livecycle

lz_db lz_db_open(const char * path);

#pragma mark -
#pragma mark Database Version

int lz_db_version(lz_db db);

#pragma mark -
#pragma mark Access Root Handle

lz_root lz_db_root(lz_db db, const char * name);

#pragma mark -
#pragma mark Root Objects

void lz_root_get_sync(lz_root root, void(^result_handler)(lz_obj obj));
void lz_root_get_async(lz_root root, void(^result_handler)(lz_obj obj));

void lz_root_set_sync(lz_root root, lz_obj obj, void(^result_handler)());
void lz_root_set_async(lz_root root, lz_obj obj, void(^result_handler)());

void lz_root_del_sync(lz_root root, void(^result_handler)());
void lz_root_del_async(lz_root root, void(^result_handler)());

#endif // _LAZY_H_



