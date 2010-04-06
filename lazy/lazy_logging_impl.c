/*
 *  lazy_logging_impl.c
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

#include "lazy_logging_impl.h"

#include <stdio.h>
#include <stdarg.h>
#include <Block.h>

#pragma mark -
#pragma mark Log Handler

void (^__lazy_object_logger)(int level, const char * msg, ...) = ^(int level, const char * msg, ...){
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
};

void lz_set_logger(void (^log_handler)(int level, const char * msg, ...)) {
    if (__lazy_object_logger != 0) {
        Block_release(__lazy_object_logger);
    }
    __lazy_object_logger = Block_copy(log_handler);
    DBG("New logger set.");
}
