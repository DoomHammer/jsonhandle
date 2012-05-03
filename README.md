
**JsonHandle** is a very simple JSON interface for C++.  Its supports encode
and decode from char buffers and STL strings & streams.  The typing is a
lot like duck typing in JavaScript.  Handles tranparently transform between
types based on usage.  Designed to efficiently use memory and improve
performance.

Intended for use in messaging, streaming support for a continuous stream
of objecs is supported.

When used for configuration files, there is an option to save for
human readbility or compact for size.

## WORKING WITH HANDLES

Arrays: access using [] operator or at() method with numeric index.
      size() will return the number of entries.
      Items can be removed using erase() or clear() for all.

``` c++
   JsonHandle root;
   root[0] = "Thomas";
   root.erase(0); 
```

Objects: access using [] operator or at() method with char * ot std::string
      index.  If an numeric index is used the value at that index will be
      returned.  The keys can be enumerated using the keys method.
      size() will return the number of entries.
      Items can be removed using erase() or clear() for all.

``` c++
   JsonHandle root;
   root["name"] = "Thomas";
   root.erase("name");
```

Strings: set using setString() or fetch using stringValue() methods
      Alternatively use the = operator.

``` c++
   JsonHandle root;
   root["first"] = "Thomas";
   root["last"].setString("Davis");

   cout << "First name is " << root["name"].stringValuePtr() << endl;
```

Numbers: set using setLong() or setDouble() and fetch using longValue()
      and doubleValue() methods.
      Alternatively use the = operator.

``` c++
   JsonHandle root;
   root["age"].setLong(14);
   root["rate"].setDouble(0.1234);

   cout << "First age is " << root["age"].longValue() << endl;
```

Boolean: set using setBoolean() and fetch using booleanValue()
      Alternatively use the = operator.

``` c++
   JsonHandle root;
   root["old"].setBoolean(false);

   cout << "First age is " << root["age"].longValue() << endl;
```

Null: set using setNull() and fetch using isNull()

``` c++
   JsonHandle root;
   root["enemy"].setNull();

   cout << "First age is " << root["age"].longValue() << endl;
```

References: are created using the copy contructor.   The assignment (=)
       operator makes a depp copy instead is that is desired.

``` c++
    JsonHandle A;
    JsonHandle B(A["list"]), C;
    B[0] = (int64_t)0;
    B[1] = (int64_t)1;       // B is [ 0, 1 ], A is {"list": [ 0, 1 ] }
    C[0] = (int64_t)2;
    B = C;                   // B is [ 2 ], A is {"list": [ 2 ] }
    C[0] = (int64_t)3;
    A["list"] = C;           // B is [ 3 ], A is {"list": [ 3 ] }
```

## EXAMPLES

Examples are located in the examples direcory with a Linux Makefile.

* example1 : file writer
* example2 : file reader
* example3 : stream writer
* example4 : stream reader

## BUILDING

JsonHandle.h must be included from source files using the API and hence
needs to be in the include path.

Copy src/JsonHandle.cpp src/JsonHandle.h src/_JS0.cpp src/_JS0.h to the build and compile
them along with the other source code.

As an alternative, build them into a library and use that.
