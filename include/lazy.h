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

#pragma mark -
#pragma mark Logging

// TODO: Skip all DEBUG messages in the release build.

#define DBG(...) __lazy_object_logger(7, __VA_ARGS__)
#define INFO(...) __lazy_object_logger(6, __VA_ARGS__)
#define NOTICE(...) __lazy_object_logger(5, __VA_ARGS__)
#define WARNING(...) __lazy_object_logger(4, __VA_ARGS__)
#define ERR(...) __lazy_object_logger(3, __VA_ARGS__)
#define CRIT(...) __lazy_object_logger(2, __VA_ARGS__)
#define ALERT(...) __lazy_object_logger(1, __VA_ARGS__)
#define EMERG(...) __lazy_object_logger(0, __VA_ARGS__)

#pragma mark -
#pragma mark Log Handler

extern void (^__lazy_object_logger)(int level, const char * msg, ...);

void lz_set_logger(void (^)(int level, const char * msg, ...));

#pragma mark -
#pragma mark Work Scheduling

void lz_wait_for_completion();

#pragma mark -
#pragma mark Object Livecycle

lz_obj lz_obj_new(void * data,
                  uint32_t length,
                  void(^dealloc)(void * data, uint32_t length),
                  uint16_t num_ref, ...);

void lz_obj_retain(lz_obj obj);
void lz_obj_release(lz_obj obj);

int lz_obj_rc(lz_obj obj);

#pragma mark -
#pragma mark Access Object Payload

void lz_obj_sync(lz_obj, void(^)(void * data, uint32_t length));
void lz_obj_async(lz_obj, void(^)(void * data, uint32_t length));

#pragma mark -
#pragma mark Access Object References

uint16_t lz_obj_num_ref(lz_obj obj);

lz_obj lz_obj_weak_ref(lz_obj obj, uint16_t pos);
lz_obj lz_obj_ref(lz_obj obj, uint16_t pos);


#endif // _LAZY_H_

