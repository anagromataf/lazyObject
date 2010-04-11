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
#include <unistd.h>
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
    FILE * version_fd;
    
    // read version
    snprintf(filename, MAXPATHLEN, "%s/version", path);
    version_fd = fopen(filename, "r");
    if (version_fd) {
        exsits = 1;
        fscanf(version_fd, "%d", &version);
        fclose(version_fd);
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
        
        // create index folder
        snprintf(filename, MAXPATHLEN, "%s/index", path);
        if (mkdir(filename, S_IRWXU)) {
            strerror_r(errno, msg, 1024);
            ERR("Could not create index folder for database '%s': %s", path, msg);
            return 0;
        }
    }
    
    // open data file in append mode and
    // set the file pointer to the begin of the file
    snprintf(filename, MAXPATHLEN, "%s/data", path);
    FILE * write_fd = fopen(filename, "a+");
    assert(write_fd != NULL);
    
    FILE * read_fd = fopen(filename, "r");
    assert(read_fd != NULL);
    
    // create handle
    struct lazy_database_s * db = malloc(sizeof(struct lazy_database_s));
    if (db) {
        LAZY_BASE_INIT(db, ^{
            dispatch_release(db->write_queue);
            dispatch_release(db->read_queue);
            fclose(write_fd);
            fclose(read_fd);
        });
        db->version = version;
        strcpy(db->filename, path);
        
        db->write_file = write_fd;
        db->read_file = read_fd;
        db->write_queue = dispatch_queue_create(NULL, NULL);
        db->read_queue = dispatch_queue_create(NULL, NULL);
        
        DBG("<%i> New database handle created.", db);
    } else {
        ERR("Could not allocate memory to create a new database handle.");
        fclose(write_fd);
        fclose(read_fd);
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
                                 object_id_t id) {
    
    int offset = 0;
    
    // get the number of references for this object
    uint16_t num_ref;
    assert(pread(fileno(db->read_file), &num_ref, sizeof(uint16_t), id + offset) == sizeof(uint16_t));
    offset = sizeof(uint16_t);
    
    // read the size of the payload
    uint32_t data_size;
    assert(pread(fileno(db->read_file), &data_size, sizeof(uint32_t), id + offset) == sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    // read the references from the file
    object_id_t refs[num_ref];
    if (num_ref > 0) {
        int bytes_read = pread(fileno(db->read_file), &refs, sizeof(object_id_t) * num_ref, id + offset);
        assert(bytes_read == sizeof(object_id_t) * num_ref);
        assert(refs);
        offset += sizeof(object_id_t) * num_ref;
    }
    
    // allocate memory for the payload and read it from the file
    void * data = malloc(data_size);
    assert(data);
    pread(fileno(db->read_file), data, data_size, id + offset);
    
    return lz_obj_unmarshal(db,
                            id,
                            data,
                            data_size,
                            ^{free(data);},
                            num_ref, refs);
}

object_id_t lazy_database_write_object(lz_db db,
                                       lz_obj obj) {
    __block object_id_t result;
    dispatch_semaphore_wait(obj->write_lock, DISPATCH_TIME_FOREVER);
    if (obj->is_temp) {
        // OPTIMIZE: Run parallel
        for (int i = 0; i < obj->num_references; i++) {
            obj->reference_ids[i] = lazy_database_write_object(db, obj->reference_objs[i]);
        }
        dispatch_sync(db->write_queue, ^{
            // get the position in the file as the object id
            result = ftell(db->write_file);
            
            // write number of references
            if (fwrite(&(obj->num_references), sizeof(uint16_t), 1, db->write_file) == 0) {
                ERR("Could not write number of references to file.");
                assert(0);
            }
            
            // write payload length
            if(fwrite(&(obj->payload_length), sizeof(uint32_t), 1, db->write_file) == 0) {
                ERR("Could not write payload length to file.");
                assert(0);
            }
            
            // write references
            if (obj->num_references > 0) {
                if(fwrite(obj->reference_ids, sizeof(object_id_t), obj->num_references, db->write_file) == 0) {
                    ERR("Could not write references to file.");
                    assert(0);
                }
            }
            
            // write payload
            if(fwrite(obj->payload_data, 1, obj->payload_length, db->write_file) == 0) {
                ERR("Could not write payload to file.");
                assert(0);
            }
            
            obj->is_temp = 0;
            fflush(db->write_file);
        });
    }
    dispatch_semaphore_signal(obj->write_lock);
    return result;
}

#pragma mark -
#pragma mark Access Root Handle

lz_root lz_db_root(lz_db db, const char * name) {
    unsigned char digest[CC_SHA1_DIGEST_LENGTH];
    char digest_str[CC_SHA1_DIGEST_LENGTH * 2 + 1];
    object_id_t root_id;
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
            lz_release(root->root_obj);
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



