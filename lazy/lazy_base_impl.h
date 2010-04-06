/*
 *  lazy_base_impl.h
 *  lazyObject
 *
 *  Created by Tobias Kräntzer on 29.03.10.
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

#ifndef _LAZY_BASE_IMPL_H_
#define _LAZY_BASE_IMPL_H_

#include <lazy.h>
#include <dispatch/dispatch.h>
#include <Block.h>

#define RETAIN(obj) lz_retain((struct lazy_base_s *)obj)
#define RELEASE(obj) lz_release((struct lazy_base_s *)obj)

#define LAZY_BASE_HEAD dispatch_queue_t queue; \
			           int rc; \
                       void (^dealloc)();

#define LAZY_BASE_INIT(obj, d) obj->queue = dispatch_queue_create(0, 0); \
                               obj->rc = 1; \
                               obj->dealloc = Block_copy(d);

struct lazy_base_s {
    LAZY_BASE_HEAD
};

#endif // _LAZY_BASE_IMPL_H_
