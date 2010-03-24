/*
 *  lazy_database_impl.c
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

#include "lazy_database_impl.h"
#include "lazy_root_impl.h"
#include "lazy_object_dispatch_group.h"

#include <stdlib.h>
#include <string.h>

#pragma mark -
#pragma mark Database Livecycle

lz_db lz_db_open(const char * path) {
    struct lazy_database_s * db = malloc(sizeof(struct lazy_database_s));
    if (db) {
        db->_db_queue = dispatch_queue_create(NULL, NULL);
        db->_retain_count = 1;
        DBG("<%i> New database handle created.", db);
    } else {
        ERR("Could not allocate memory to create a new database handle.");
    }
    return db;
}

void lz_db_retain(lz_db db) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), db->_db_queue, ^{
        db->_retain_count++;
        DBG("<%i> Retain count increased.", db);
    });
}

void lz_db_release(lz_db db) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), db->_db_queue, ^{
        if (db->_retain_count > 1) {
            db->_retain_count--;
            DBG("<%d> Retain count decreased.", db);
        } else {
            DBG("<%i> Retain count reaches 0.", db);
            dispatch_group_async(*lazy_object_get_dispatch_group(), dispatch_get_global_queue(0, 0), ^{
                // release dispatch queue and free memory
                DBG("<%i> Releasing database dispatch queue.", db);
                dispatch_release(db->_db_queue);
                DBG("<%i> Dealloc memory.", db);
                free(db);
            });
        };
    });
}

#pragma mark -
#pragma mark Access Root Handle

lz_root lz_db_root(lz_db db, const char * name) {
    struct lazy_root_s * root = malloc(sizeof(struct lazy_root_s));
    if (root) {
        root->_root_queue = dispatch_queue_create(NULL, NULL);
        root->_retain_count = 1;
        lz_db_retain(db);
        root->_database = db;
        root->_name = malloc(strlen(name));
        root->_name = strcpy(root->_name, name);
        root->_root_obj = 0;
        DBG("<%i> New root handle created.", root);
    } else {
        ERR("Could not allocate memory to create a new root handle.");
    }
    return root;
}


