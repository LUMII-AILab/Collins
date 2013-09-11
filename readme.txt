
Requirements
============

...

Optional
--------

Sparsehash : https://code.google.com/p/sparsehash/


Compiling
=========

$ cd <source dir>
$ mkdir build; cd build
$ cmake ..
$ make


To choose different hash map, use:
$ cmake -D USE_MAP=0 ..
where:
	USE_MAP=-1          --> std::map
	USE_MAP=0 (default) --> std::unordered_map
	USE_MAP=1           --> std::dense_hash_map
	USE_MAP=2           --> std::sparse_hash_map

To link statically on Linux use:
$ cmake -D Boost_USE_STATIC_LIBS=ON -D CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO=-static-libstdc++ ..

