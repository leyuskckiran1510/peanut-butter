# About [CIVETWEB](./civetweb.o) 

This was  compiled at machine with such specs:-
```
Linux local 6.5.0-44-generic #44~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC x86_64 GNU/Linux
```

# For your system you might want to follow these steps to prepear the civetweb libary

# for unix like system (macos,unix,linux ...)
```bash
git clone --depth=1 https://github.com/civetweb/civetweb webtech
cd webtech
make lib
cp ./out/src/civetweb.o path/to/peanut_butter/lib/civetweb.o

# optional
cd ./..
rm -rf ./webtech

```

# for windows
```ps
git clone --depth=1 https://github.com/civetweb/civetweb webtech
cd webtech
mingw32-make CC=gcc lib
copy .\out\src\civetweb.* path\to\peanut_butter\lib\civetweb.*

# optional
cd .\..
rm -Recursive -Force .\webtech
```

