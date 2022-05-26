## minilib-ports



Compilation of small programs, compiled with [minilib](https://github.com/michael105/minilib),
serving as testcases.

Some are modified, some unchanged (besides the file minilib.conf).

Ideally, they should compile unmodified; 
yet most problems have been a result of unwanted optimizations by gcc.

However, especially the network stack of minilib seems to be a bit 'unmature'. 
(or there might be slight differences to glibc/musl).
Bug reports/fixes are very welcome.


I collect the tools along with the static builds here.


The directory ./bin_x64 contains static binaries for linux, 64bit;
built with gcc 9.3 (Gentoo).



misc 2022


----

```


bin_x64/daft-dhcp-client       2021-09-24             10912
bin_x64/micro-http             2021-09-20             8192
bin_x64/sash                   2022-05-26             32772
bin_x64/sed                    2022-05-26             16656
bin_x64/smu                    2021-09-20             10872
bin_x64/snake                  2021-09-20             8204
bin_x64/snake-mikro            2021-09-20             8204
bin_x64/splitvt                2022-05-26             24728
bin_x64/totp                   2022-05-26             9696
===========================================================
                                        size: 130236 Bytes
```
