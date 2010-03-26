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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>

#pragma mark -
#pragma mark Database Livecycle

lz_db lz_db_open(const char * path) {
    
    char msg[1024];
    char filename[MAXPATHLEN];
    int version;
    int exsits = 0;
    FILE * fd;
    
    // read version
    snprintf(filename, MAXPATHLEN, "%s/version", path);
    fd = fopen(filename, "r");
    if (fd) {
        exsits = 1;
        fscanf(fd, "%d", &version);
        fclose(fd);
        INFO("Database with version %d exsists.", version);
    } else {
        switch (errno) {
            case ENOENT:
                exsits = 0;
                break;
            default:
                strerror_r(errno, msg, 1024);
                ERR("Could not read version of database '%s': %s", path, msg);
                return 0;
        }
    }
    
    // create a database structure
    if (!exsits) {
        INFO("Database does not exsist. Setting up default structure for version 1.");
        
        // create database folder
        if (mkdir(path, S_IRWXU)) {
            strerror_r(errno, msg, 1024);
            char cwd[1024];
            getcwd(cwd, 1024);
            ERR("Could not create folder for database '%s' in '%s': %s", path, cwd, msg);
            return 0;
        }
        
        // create version info
        FILE * fd = fopen(filename, "w+");
        if (fd) {
            fprintf(fd, "1");
            fclose(fd);
        } else {
            strerror_r(errno, msg, 1024);
            ERR("Could not create version info for database '%s': %s", path, msg);
            return 0;
        }
        
        // create chunks folder
        snprintf(filename, MAXPATHLEN, "%s/chunks", path);
        if (mkdir(filename, S_IRWXU)) {
            strerror_r(errno, msg, 1024);
            ERR("Could not create chunk folder for database '%s': %s", path, msg);
            return 0;
        }
    }
    
    // create handle
    struct lazy_database_s * db = malloc(sizeof(struct lazy_database_s));
    if (db) {
        db->db_queue = dispatch_queue_create(NULL, NULL);
        db->retain_count = 1;
        
        db->version = version;
        strcpy(db->db_path, path);
        
        DBG("<%i> New database handle created.", db);
    } else {
        ERR("Could not allocate memory to create a new database handle.");
    }
    return db;
}

void lz_db_retain(lz_db db) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), db->db_queue, ^{
        db->retain_count++;
        DBG("<%i> Retain count increased.", db);
    });
}

void lz_db_release(lz_db db) {
    dispatch_group_async(*lazy_object_get_dispatch_group(), db->db_queue, ^{
        if (db->retain_count > 1) {
            db->retain_count--;
            DBG("<%i> Retain count decreased.", db);
        } else {
            DBG("<%i> Retain count reaches 0.", db);
            dispatch_group_async(*lazy_object_get_dispatch_group(), dispatch_get_global_queue(0, 0), ^{
                // release dispatch queue and free memory
                DBG("<%i> Releasing database dispatch queue.", db);
                dispatch_release(db->db_queue);
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


