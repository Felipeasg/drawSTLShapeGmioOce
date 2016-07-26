# drawShapeOce
Basic STL shape drawing using opencascade, qt5 and gmio

This example is used to draw basic shapes using Stl_Mesh and gmio that create
an AIS_Shape and diplay using an AIS_Interative_context.  

![alt tag](https://github.com/Felipeasg/drawSTLShapeGmioOce/blob/master/stlshape.png)

# Build and run

To build you need [gmio](https://github.com/fougue/gmio), [OpenCascade Community Edition 6.8.0](https://github.com/tpaviot/oce) and Qt5.

After download and install the packages type:

```
$ qmake-qt5 drawShape.pro
$ make -j8
```

Put some STL file in the root of project and run the executable **drawShape**.

`$ ./drawShape`

Click on the button Draw STL and enjoy!


