/*
 *  lazy_database_impl.h
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

#ifndef _LAZY_DATABASE_IMPL_H_
#define _LAZY_DATABASE_IMPL_H_

#include <lazy.h>

#include <stdint.h>
#include <sys/param.h>
#include <dispatch/dispatch.h>

struct lazy_database_s {
    int retain_count;
    dispatch_queue_t db_queue;
    
    char db_path[MAXPATHLEN];
    
    // version of the database
    int version;
};

#endif // _LAZY_DATABASE_IMPL_H_