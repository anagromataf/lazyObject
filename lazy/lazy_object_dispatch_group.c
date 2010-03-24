/*
 *  lazy_object_dispatch_group.c
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

#include "lazy_object_dispatch_group.h"

dispatch_group_t * lazy_object_get_dispatch_group() {
    static dispatch_group_t group = 0;
    static dispatch_once_t predicate = 0;
    dispatch_once(&predicate, ^{
        group = dispatch_group_create();
    });
    return (&group);
}

void lz_wait_for_completion() {
    dispatch_group_t group = * lazy_object_get_dispatch_group();
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
}
