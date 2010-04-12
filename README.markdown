# Lazy Object

Lazy Object is a persistence layer for objects in C. It makes heavy use of [Grand Central Dispatch][gcd] and [Blocks][blocks] to allow concurrent processing. Moreover, the library follows a copy-on-write paradigm, thus each handle of an object is always valid and cannot be modified.

## Objects and References

A lazy object (*lz_obj*) is a reference counted handle for a block of bytes (payload), which furthermore can contain references to other lazy objects. The structure of this block and how the references are used is up to the application and does not affect the behavior of this library.

![Example of an graph with lazy objects](doc/images/object_graph.png "Example of an graph with lazy objects")

Each object is created with a pointer to the data, its length and a block which is responsible for deallocating the data and optionally with a list of references to other objects.

<pre>
// allocat some bytes
int myLength = 1024;
void * myData = malloc(length);

// store a structure in this block
myData = ...

// create the object handle
lz_obj obj = lz_obj_new(myData, myLength, ^{
    free(myData);
}, 0);

// use the object handle
foo(obj);

// release the reference if not needed anymore
lz_release(obj);

// if the retain count reaches 0, the custom block (free(myData);) is called and
// the object handle is deallocated
</pre>

The references are stored in the order given in the function call. If, for example, a new object should represent a dictionary-like structure, all references to other objects are given in the list and the mapping is done in the private data structure. The references in the mapping must point to the position in the list of references given in the function call.

<pre>
// define the structure for the private data
struct dict_s {
    uint16_t key;
    uint16_t value;
};

// create some objects as keys and values
lz_obj key1, key2, key3, ...;
lz_obj value1, value2, value3, ...;

// allocate memory for 5 entries
int dictLength = sizeof(struct dict_s) * 5;
struct dict_s * dictData = malloc(dictLength);

// set the position of the references in the structure
dictData[0].key = 0;
dictData[0].value = 5;

dictData[1].key = ...;

// create the object
lz_obj dict = lz_obj_new(dictData, dictLength, ^{
    free(dictData);
}, 10, key1, key2, key3, key4, key5, value1, value2, value3, value4, value5);
</pre>

Now we created a small object graph with an object referring to 10 other objects. To access the content of the newly created dictionary, we have to call either `lz_obj_sync()` or `lz_obj_async()`. Using this function to access the content of the objects assures that the library is aware of each access to the payload.

<pre>
__block lz_obj key, value;

// get the key and value of the fourth element
lz_obj_sync(dict, ^(void * data, uint32_t size){
    struct dict_s * d = data;
    key = lz_obj_ref(dict, d[3].key);
    value = lz_obj_ref(dict, d[3].value);
});

// use key and value

// release the references
lz_release(key);
lz_release(value);
</pre>

After having completed this step, we have the references of the key and the value of the fourth element in the mapping. If these references are not needed anymore, we have to release them.

If the references are only used inside the block, you can use the function `lz_obj_weak_ref()` which returns an object handle for the position without increasing the retain count (it is not safe to call this function outside a block handled by `lz_obj_sync()` or `lz_obj_async()` for the same object).

## Database and Root Objects

Up to this point we have only created objects which weren't stored in the file system. To achieve this, we have to create a database handle and within this a root object handle.

The database handle is the representation of the whole database in the filesystem. The root object handle could be seen as a named tag for an object. Only objects, which appear in the object graph of objects, which are stores through a root object are made persistent.

<pre>
// open the database and create a root handle with the name 'dict'.
lz_db db = lz_db_open("test.lazyObject");
lz_root root = lz_db_root(db, "dict");

// store our previous created dictionary object under the
// name 'dict' in the database
lz_root_set_sync(root, dict, ^{
    // handler is called after the objects are stored in the file system
});
</pre>

![Illustration of a root handle](doc/images/root_object.png "Illustration of a root handle")

As soon as the function `lz_root_set_sync()` is called, the whole object graph is traversed and each object which has not already been stored in the file system is saved. At the end, a pointer with the label of the root object (in our case *dict*) is set to the object passed in this function.

If we need the dictionary (e.g., after a restart of the application), we can call the function `lz_root_get_sync()` on the appropriate handle.

<pre>
// open the database and create a root handle with the name 'dict'.
lz_db db = lz_db_open("test.lazyObject");
lz_root root = lz_db_root_sync(db, "dict");
__block lz_obj dict;

lz_root_get(root, ^(lz_obj obj){
    dict = obj;
});
</pre>

## System Logging

The default log handler prints all messages to `stderr`. If you want to use your own logging facility you can set your own log handler. At the moment the log handler should be set before any other function of the library is used (particularly in `main()`).

<pre>
// using syslog for logging
openlog("MyApp", LOG_PERROR, LOG_USER);
lz_set_logger(^(int level, const char *msg, ...){
    va_list args;
    va_start(args, msg);
    vsyslog(level, msg, args);
    va_end(args);
});
</pre>

## Work Scheduling

Thus we make heavy use of concurrent processing, we have to wait at least at the end of the application until all pending operations are done. Therefore, we have to call the function `lz_wait_for_completion()`.

<pre>
// wait until all pending operations are done
lz_wait_for_completion();
</pre>

[gcd]: http://developer.apple.com/mac/library/documentation/Performance/Reference/GCD_libdispatch_Ref/Reference/reference.html "Grand Central Dispatch"
[blocks]:http://developer.apple.com/mac/library/documentation/Cocoa/Conceptual/Blocks/Articles/00_Introduction.html "Blocks"
