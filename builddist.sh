#!/bin/sh

if [ "$1" = "" ]; then
   echo "USAGE: $0 version"
   exit 1
fi

dist=jsonhandle-$1
file=jsonhandle-$1.zip

cd examples
make clean
cd ..

rm -rf $dist $file
mkdir $dist

cp -r CHANGELOG  examples  LICENSE  README.md  src $dist

zip -r $file $dist

rm -rf $dist

echo created $file
