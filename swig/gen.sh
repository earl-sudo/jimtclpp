
cp jimtclpp.i csharp
cd csharp
swig -csharp  jimtclpp.i
rm jimtclpp.i
cd ..

cp jimtclpp.i tcl
cd tcl
swig -tcl8  jimtclpp.i
rm jimtclpp.i
cd ..

cp jimtclpp.i python
cd python
swig -python  jimtclpp.i
rm jimtclpp.i
cd ..

cp jimtclpp.i java
cd java
swig -java  jimtclpp.i
rm jimtclpp.i
cd ..


