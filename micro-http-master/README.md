 A minimal webserver, for statically generated websites.
 I'm using it to preview web projects locally;
 I wouldn't recommend using this server accessible from the outside.
 
 It is mainly a testcase for minilib, (https://github.com/michael105/minilib)
 and compiles to 8.2kB statically linked.

 I started with another project, (https://github.com/venkat24/micro-http)
 leaving this mostly as it is.

 Several things I'd implement in another way,
 but even the other coding style did serve for testing 
 minilib. (e.g. several unneccessary calls to malloc and free)

 To download and compile with minilib, please run 'make -f minilib.conf'.
 (Tested yet with gcc 9.3 only)

 misc 2021

----


[![forthebadge](http://forthebadge.com/images/badges/built-with-love.svg)](http://forthebadge.com)

# micro-http

A tiny static file server written in C, with functionality similar to Python's SimpleHTTPServer. Presents a file listing for directories. Handles multiple requests asynchronously.

* Build by running `make` or `make serve`.

* Add `serve` to your `$PATH`. 

* Run `serve 9000` to serve the current working directory. Visit localhost:9000.
