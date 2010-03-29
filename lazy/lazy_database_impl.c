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
#include "lazy_database_chunk_impl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <Block.h>

// OS X only
#include <CommonCrypto/CommonDigest.h>


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
			version = 1;
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
		
		// create index folder
        snprintf(filename, MAXPATHLEN, "%s/index", path);
        if (mkdir(filename, S_IRWXU)) {
            strerror_r(errno, msg, 1024);
            ERR("Could not create index folder for database '%s': %s", path, msg);
            return 0;
        }
    }
    
    // create handle
    struct lazy_database_s * db = malloc(sizeof(struct lazy_database_s));
    if (db) {
        LAZY_BASE_INIT(db, ^{
            RELEASE(db->chunk);
        });
        db->version = version;
        strcpy(db->filename, path);
		db->chunk = lazy_database_chunk_open(db, 1, CHUNK_RW);
        DBG("<%i> New database handle created.", db);
    } else {
        ERR("Could not allocate memory to create a new database handle.");
    }
    return db;
}

#pragma mark -
#pragma mark Database Version

int lz_db_version(lz_db db) {
	return db->version;
}

#pragma mark -
#pragma mark Read & Write Objects

lz_obj lazy_database_read_object(lz_db db,
								 struct lazy_object_id_s id) {
	assert(id.cid == 1);
	lz_obj obj = lazy_database_chunk_read_object(db->chunk, id);
	return obj;
}

struct lazy_object_id_s lazy_database_write_object(lz_db db,
												   lz_obj obj) {
	dispatch_semaphore_wait(obj->write_lock, DISPATCH_TIME_FOREVER);
	if (obj->is_temp) {
		dispatch_apply(obj->num_references, dispatch_get_global_queue(0, 0), ^(size_t i){
			if ((obj->reference_ids[i].cid) == 0) {
				obj->reference_ids[i] = lazy_database_write_object(db, obj->reference_objs[i]);
			}
		});
		obj->id = lazy_database_chunk_write_object(db->chunk, obj);
	}
	dispatch_semaphore_signal(obj->write_lock);
	return obj->id;
}

#pragma mark -
#pragma mark Access Root Handle

lz_root lz_db_root(lz_db db, const char * name) {
	
	unsigned char digest[CC_SHA1_DIGEST_LENGTH];
	char digest_str[CC_SHA1_DIGEST_LENGTH * 2 + 1];
	struct lazy_object_id_s root_id;
	FILE * fd;
	char msg[1024];
    char filename[MAXPATHLEN];
	int exsits = 0;
	
	// sha1(name)
	CC_SHA1(name, strlen(name), digest);
	// OPTIMIZE: find a better way to convert the digest to a string
	snprintf(digest_str,
			 CC_SHA1_DIGEST_LENGTH * 2 + 1,
			 "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			 digest[0],digest[1],digest[2],digest[3],digest[4],digest[5],digest[6],digest[7],digest[8],digest[9],digest[10],digest[11],digest[12],digest[13],digest[14],digest[15],digest[16],digest[17],digest[18],digest[19]);
	
    // read root obj
    snprintf(filename, MAXPATHLEN, "%s/index/%s", db->filename, digest_str);
    fd = fopen(filename, "r+");
    if (fd) {
        exsits = 1;
		assert(sizeof(root_id) == fread(&root_id, sizeof(root_id), sizeof(root_id), fd));
		fclose(fd);
    } else {
        switch (errno) {
            case ENOENT:
                exsits = 0;
                break;
            default:
                strerror_r(errno, msg, 1024);
                ERR("Could not read root object '%s' (%s): %s", name, digest_str, msg);
                return 0;
        }
    }
	
    struct lazy_root_s * root = malloc(sizeof(struct lazy_root_s));
    if (root) {
        LAZY_BASE_INIT(root, ^{
            if (root->root_obj) {
                lz_release(root->root_obj);
            }
            lz_release(root->database);
        });
		strcpy(root->filename, filename);
		root->_exsits = exsits;
		root->root_obj_id = root_id;
        lz_retain(db);
        root->database = db;
        root->root_obj = 0;
        DBG("<%i> New root handle created.", root);
    } else {
        ERR("Could not allocate memory to create a new root handle.");
    }
    return root;
}


