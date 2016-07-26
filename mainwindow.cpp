#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include <QIODevice>
#include <QDir>
#include <QElapsedTimer>

/* OpenCascade */
#include <TopoDS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>

#include <gp_Ax2.hxx>

#include <MeshVS_Mesh.hxx>
#include <MeshVS_Tool.hxx>

#include <StlMesh_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <StlMesh_MeshExplorer.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <AIS_Shape.hxx>

/*  gmio */
#include <gmio_core/error.h>
#include <gmio_stl/stl_io.h>
#include "stl_occ_mesh.h"
#include <gmio_support/stl_occ_mesh.h>
#include <gmio_support/stream_qt.h>

#include <iostream>
#include <ctime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    occView = new OccView(this);

    pushButtonDrawBox = new QPushButton("Draw box");
    pushButtonDrawCone = new QPushButton("Draw cone");
    pushButtonDrawSphere = new QPushButton("Draw sphere");
    pushButtonDrawSTL = new QPushButton("Draw STL");


    QWidget *w = new QWidget();

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    QVBoxLayout *verticalLayout = new QVBoxLayout();

    horizontalLayout->addWidget(pushButtonDrawBox);
    horizontalLayout->addWidget(pushButtonDrawCone);
    horizontalLayout->addWidget(pushButtonDrawSphere);    
    horizontalLayout->addWidget(pushButtonDrawSTL);

    verticalLayout->addWidget(occView);
    verticalLayout->addItem(horizontalLayout);

    w->setLayout(verticalLayout);

    setCentralWidget(w);

    connect(pushButtonDrawBox, SIGNAL(clicked(bool)), this, SLOT(on_drawBox()));
    connect(pushButtonDrawCone, SIGNAL(clicked(bool)), this, SLOT(on_drawCone()));
    connect(pushButtonDrawSphere, SIGNAL(clicked(bool)), this, SLOT(on_drawSphere()));
    connect(pushButtonDrawSTL, SIGNAL(clicked(bool)), this, SLOT(on_drawSTL()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_drawBox()
{
    TopoDS_Shape aTopoBox = BRepPrimAPI_MakeBox(3.0, 4.0, 5.0).Shape();

    occView->DrawShape(aTopoBox);
}

void MainWindow::on_drawCone()
{
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(0.0, 10.0, 0.0));

    TopoDS_Shape aTopoCone = BRepPrimAPI_MakeCone(anAxis, 3.0, 0.0, 5.0).Shape();

    occView->DrawShape(aTopoCone);
}

void MainWindow::on_drawSphere()
{
    gp_Ax2 anAxis;
    anAxis.SetLocation(gp_Pnt(5.0, 5, 0.0));

    TopoDS_Shape aTopoSphere = BRepPrimAPI_MakeSphere(anAxis, 3.0).Shape();

    occView->DrawShape(aTopoSphere);
}

void MainWindow::on_drawSTL()
{
    int error = 0;

    TopoDS_Compound ResultShape;
    BRep_Builder CompoundBuilder;
    CompoundBuilder.MakeCompound(ResultShape);

    const char* filepath = "./stlfile.stl";

    Handle_StlMesh_Mesh mesh = new StlMesh_Mesh;
    gmio_stl_mesh_creator_occmesh mesh_creator(mesh);
    QElapsedTimer timer;

    timer.start();
    // Read, using default options(NULL)
    char old_lcnum[64];
    strncpy(old_lcnum, setlocale(LC_NUMERIC, NULL), sizeof(old_lcnum));
    setlocale(LC_NUMERIC, "C");
    error = gmio_stl_read_file(filepath, &mesh_creator, NULL);
    if (error != GMIO_ERROR_OK)
        std::cerr << "gmio error: 0x" << std::hex << error << std::endl;
    setlocale(LC_NUMERIC,old_lcnum);

    qDebug() << "Gmio gmio_stl_read_file" << timer.elapsed() << "milliseconds";

    std::cout << "Nbr of domains: " << mesh_creator.mesh()->NbDomains() << std::endl;
    std::cout << "Nbr of trianbles: " << mesh_creator.mesh()->NbTriangles() << std::endl;
    std::cout << "Nbr of vertices: " << mesh_creator.mesh()->NbVertices() << std::endl;


    Standard_Integer NumberDomains = mesh_creator.mesh()->NbDomains();
    Standard_Integer iND;
    gp_XYZ p1, p2, p3;
    TopoDS_Vertex Vertex1, Vertex2, Vertex3;
    TopoDS_Face AktFace;
    TopoDS_Wire AktWire;
    Standard_Real x1, y1, z1;
    Standard_Real x2, y2, z2;
    Standard_Real x3, y3, z3;

    StlMesh_MeshExplorer aMExp (mesh_creator.mesh());

    timer.start();
    for (iND=1;iND <= NumberDomains; iND++) {
        for (aMExp.InitTriangle (iND); aMExp.MoreTriangle (); aMExp.NextTriangle ())
        {

            aMExp.TriangleVertices (x1,y1,z1,x2,y2,z2,x3,y3,z3);
            p1.SetCoord(x1,y1,z1);
            p2.SetCoord(x2,y2,z2);
            p3.SetCoord(x3,y3,z3);

            if ((!(p1.IsEqual(p2,0.0))) && (!(p1.IsEqual(p3,0.0))))
            {
                Vertex1 = BRepBuilderAPI_MakeVertex(p1);
                Vertex2 = BRepBuilderAPI_MakeVertex(p2);
                Vertex3 = BRepBuilderAPI_MakeVertex(p3);

                AktWire = BRepBuilderAPI_MakePolygon( Vertex1, Vertex2, Vertex3, Standard_True);

                if( !AktWire.IsNull())
                {
                    AktFace = BRepBuilderAPI_MakeFace( AktWire);
                    if(!AktFace.IsNull())
                        CompoundBuilder.Add(ResultShape,AktFace);
                }
            }
        }
    }
    qDebug() << "Create shape took" << timer.elapsed() << "milliseconds";


    TopoDS_Shape aShape = ResultShape;

    occView->DrawShape(ResultShape);

//    Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(mesh_creator.mesh());

//    occView->mContext->Display(shape);
//    occView->FitAll();

}

