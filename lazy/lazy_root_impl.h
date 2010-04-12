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

#include <stdio.h>
#include <sys/param.h>
#include <dispatch/dispatch.h>

#include "lazy_base_impl.h"
#include "lazy_database_impl.h"
#include "lazy_object_impl.h"

struct lazy_root_s {
    LAZY_BASE_HEAD
    
    char filename[MAXPATHLEN];
    FILE * file;
    int root_is_bound;
    object_id_t root_obj_id;
    lz_db database;
    lz_obj root_obj;
};

#endif // _LAZY_ROOT_IMPL_H_
