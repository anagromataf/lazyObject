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

#include "lazy_database_chunk_impl.h"
#include "lazy_base_impl.h"
#include "lazy_object_impl.h"

struct lazy_database_s {
    LAZY_BASE_HEAD
    
    char filename[MAXPATHLEN];
    
    // version of the database
    int version;
	
	struct lazy_database_chunk_s * chunk;
};

#pragma mark -
#pragma mark Read & Write Objects

lz_obj lazy_database_read_object(lz_db db, struct lazy_object_id_s id);
struct lazy_object_id_s lazy_database_write_object(lz_db db, lz_obj obj);

#endif // _LAZY_DATABASE_IMPL_H_