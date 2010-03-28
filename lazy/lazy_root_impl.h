/*
 *  lazy_root_impl.h
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

#ifndef _LAZY_ROOT_IMPL_H_
#define _LAZY_ROOT_IMPL_H_

#include <lazy.h>

#include <sys/param.h>
#include <dispatch/dispatch.h>

#include "lazy_database_impl.h"

struct lazy_root_s {
    int _retain_count;
    dispatch_queue_t _root_queue;

	char _path[MAXPATHLEN];
	
	int _exsits;
	struct lazy_object_id_s _obj_id;
	
    lz_db _database;
    lz_obj _obj;
};

#endif // _LAZY_ROOT_IMPL_H_
