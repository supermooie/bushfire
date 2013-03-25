// Copyright NVIDIA Corporation 2009-2010
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
// *AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
// BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES

#include <iostream>
#include <fstream>
#include <sstream>

#include "SimpleScene.h"

#include "nvsg/BlendAttribute.h"
#include "nvsg/CgFx.h"
#include "nvsg/DirectedLight.h"
#include "nvsg/Face.h"
#include "nvsg/FaceAttribute.h"
#include "nvsg/GeoNode.h"
#include "nvsg/Material.h"
#include "nvsg/Node.h"
#include "nvsg/Scene.h"
#include "nvsg/StateSet.h"
#include "nvsg/Transform.h"
#include "nvsg/Triangles.h"
#include "nvmath/nvmath.h"
#include "nvmath/Vecnt.h"
#include "MeshGenerator.h"
#include "nvutil/Tools.h"
#include "nvutil/DbgNew.h" // this must be the last include

using namespace nvsg;
using namespace nvmath;
using namespace nvutil;

// Hold spheres data from CSV file
struct SphereData {
  float x;
  float y;
  float z;
  float radius;
};

// const unsigned int NUM_SPHERES = 50;

struct TransformPositions {
  float x;
  float y;
};

void getXTransformPositions(std::vector<TransformPositions>& tps, const bool is_x) {
  std::cout << "getXTransformPositions" << std::endl;

  std::string filename = is_x ? "data/x_bbox.asc" :
  "data/y_bbox.asc";

  std::ifstream file(filename.c_str());
  std::string line;
  unsigned i = 0;

  while (file.good()) {
    getline(file, line);

    if (is_x)
      tps[i].x = atof(line.c_str());
    else
      tps[i].y = atof(line.c_str());

    // std::cout << i << " " << atof(line.c_str()) << std::endl;
    ++i;
  }
}

SimpleScene::SimpleScene()
{
  m_sceneHandle = Scene::create();
  WritableObject<Scene> theScene(m_sceneHandle);
  // theScene->setBackColor(  Vec4f( 71.0f/255.0f, 111.0f/255.0f, 0.0f, 1.0f ) );

  // Create BlendAttribute to render the transparent materials correctly.
  BlendAttributeSharedPtr blendAttribute = BlendAttribute::create();
  {
    BlendAttributeWriteLock ba( blendAttribute );
    ba->setBlendFunctions( BF_SOURCE_ALPHA, BF_ONE_MINUS_SOURCE_ALPHA );
  }

  // Render front and back faces of the transparent objects and do two sided ligting on them.
  FaceAttributeSharedPtr faceAttribute = FaceAttribute::create();
  {
    FaceAttributeWriteLock fa( faceAttribute );
    fa->setCullMode( false, false );
    fa->setTwoSidedLighting( true );
  }

  // Create four materials/textures
  StateSetSharedPtr stateSet[5];

  stateSet[0] = createDefinedMaterial( Vec3f( 0.0f, 0.0f, 0.0f ),  // ambientColor,
                                       Vec3f( 1.0f, 1.0f, 1.0f ),  // diffuseColor,
                                       // Vec3f( 1.0f, 1.0f, 1.0f ),  // specularColor,
                                       // Vec3f( 1.0f, 1.0f, 1.0f ),  // specularColor,
                                       Vec3f( 1.0f, 0.0f, 0.0f ),  // specularColor,
                                       // 20.0f,                      // specularExponent,
                                       200.0f,                      // specularExponent,
                                       Vec3f( 0.0f, 0.0f, 0.0f ),  // emissiveColor,
                                       1.0f,                       // opacity,
                                       0.5f,                       // reflectivity,
                                       1.0f );                     // indexOfRefraction


  {
    StateSetWriteLock ss( stateSet[0] );
    // No BlendAttribute or FaceAttribute needed for the opaque material.
    ss->setName( "White Material" );
  }
  stateSet[1] = createDefinedMaterial( Vec3f( 0.0f, 0.0f, 0.0f ),  // ambientColor
                                       Vec3f( 1.0f, 0.0f, 0.0f ),  // diffuseColor
                                       Vec3f( 1.0f, 1.0f, 1.0f ),  // specularColor
                                       40.0f,                      // specularExponent
                                       Vec3f( 0.0f, 0.0f, 0.0f ),  // emissiveColor
                                       0.7f,                       // opacity
                                       0.3f,                       // reflectivity
                                       1.33f );  // Water          // indexOfRefraction
  {
    StateSetWriteLock ss( stateSet[1] );
    ss->addAttribute( blendAttribute );
    ss->addAttribute( faceAttribute );
    ss->setName( "Red Material" );
  }

  stateSet[2] = createDefinedMaterial( Vec3f( 0.0f, 0.0f, 0.0f ),  // ambientColor
                                       Vec3f( 0.0f, 1.0f, 0.0f ),  // diffuseColor
                                       Vec3f( 1.0f, 1.0f, 1.0f ),  // specularColor
                                       60.0f,                      // specularExponent
                                       Vec3f( 0.0f, 0.0f, 0.0f ),  // emissiveColor
                                       0.5f,                       // opacity
                                       0.3f,                       // reflectivity
                                       1.45f );  // Glass          // indexOfRefraction
  {
    StateSetWriteLock ss( stateSet[2] );
    ss->addAttribute( blendAttribute );
    ss->addAttribute( faceAttribute );
    ss->setName( "Green Material" );
  }

  stateSet[3] = createDefinedMaterial( Vec3f( 0.0f, 0.0f, 0.0f ),  // ambientColor
                                       Vec3f( 0.0f, 0.0f, 1.0f ),  // diffuseColor
                                       Vec3f( 1.0f, 1.0f, 1.0f ),  // specularColor
                                       80.0f,                      // specularExponent
                                       Vec3f( 0.0f, 0.0f, 0.0f ),  // emissiveColor
                                       0.3f,                       // opacity
                                       0.3f,                       // reflectivity
                                       1.7f );                     // indexOfRefraction
  {
    StateSetWriteLock ss( stateSet[3] );
    ss->addAttribute( blendAttribute );
    ss->addAttribute( faceAttribute );
    ss->setName( "Blue Material" );
  }

  stateSet[4] = createDefinedMaterial( Vec3f( 0.0f, 0.0f, 0.0f ),  // ambientColor,
                                       Vec3f( 1.0f, 1.0f, 1.0f ),  // diffuseColor,
                                       Vec3f( 1.0f, 1.0f, 1.0f ),  // specularColor,
                                       100.0f,                      // specularExponent,
                                       Vec3f( 0.0f, 0.0f, 0.0f ),  // emissiveColor,
                                       0.1f,                       // opacity,
                                       0.0f,                       // reflectivity,
                                       1.0f );                     // indexOfRefraction


  {
    StateSetWriteLock ss( stateSet[4] );
    // No BlendAttribute or FaceAttribute needed for the opaque material.
    ss->setName( "White Material" );
  }

  // std::ifstream file("/home/jonathan/Desktop/spheres.csv.tt");
  std::ifstream file("data/fire.csv");

  // // std::ifstream file("/home/jonathan/Desktop/spheres.dat");
  // // std::ifstream file("/home/jonathan/Desktop/tt3");
  std::string line;
  int line_number = 0;

  std::vector<SphereData> sds = std::vector<SphereData>(NUM_SPHERES);

  while (file.good()) {
    getline(file, line);
    std::istringstream linestream(line);
    std::string item;
    int item_num = 0;
    while (getline (linestream, item, ',')) {
    // while (getline (linestream, item, ' ')) {
      item_num++;
      // std::cout << "Item #" << item_num << ": " << item << std::endl;
      switch (item_num) {
        case 1: // X
        sds[line_number].x = atof(item.c_str());
        break;
        case 2: // Y
        sds[line_number].y = atof(item.c_str());
        break;
        case 3: // Z
        sds[line_number].z = atof(item.c_str());
        break;
        case 4: // Sphere radius
        sds[line_number].radius = atof(item.c_str());
        break;
        default:
        std::cerr << "Error: format of CSV file - incorrent number of columns" << std::endl;
        exit(1);
        break;
      }
      // Create class to hold CSV entries
    }
    ++line_number;
    // std::cout << std::endl;
  }

  std::cout << "line number: " << line_number << std::endl;

  for (int i = 0; i < NUM_SPHERES; ++i) {
    m_sphere_drawable[i] = createSphere(32, 16, sds[i].radius);

    // m_sphere_drawable[i] = createCube();

    // m_sphere_drawable[i] = createQuadStrip(32,16, 2.3);
    // m_sphere_drawable[i] = createQuadSet(32,16, 2.3);
    // m_sphere_drawable[i] = createTriSet(32,16, 2.3);

    // m_sphere_drawable[i] = createTetrahedron();
    // m_sphere_drawable[i] = createOctahedron();
    // m_sphere_drawable[i] = createDodecahedron();
    // m_sphere_drawable[i] = createIcosahedron();
    // m_sphere_drawable[i] = createTorus(32,16);
  }

  for (int i = 0; i < NUM_CUBES; ++i) {
    m_cube_drawable[i] = createCube();
  }

  GeoNodeSharedPtr sphere_node[NUM_SPHERES];
  for ( int i=0 ; i<NUM_SPHERES ; i++ )
  {
    sphere_node[i] = GeoNode::create();
    WritableObject<GeoNode> wgn( sphere_node[i] );
    wgn->addDrawable( stateSet[4], m_sphere_drawable[i] );
  }

  GeoNodeSharedPtr cube_node[NUM_CUBES];
  for ( int i=0 ; i<NUM_CUBES; i++ )
  {
    cube_node[i] = GeoNode::create();
    WritableObject<GeoNode> wgn( cube_node[i] );
    wgn->addDrawable( stateSet[0], m_cube_drawable[i] );
  }

  std::vector<TransformPositions> transform_positions = std::vector<TransformPositions>(NUM_SPHERES);
  getXTransformPositions(transform_positions, true);
  getXTransformPositions(transform_positions, false);

  for (int i = 0; i < NUM_SPHERES; ++i) {
    m_sphereTransformHandle[i] = createTransform(sphere_node[i], Vec3f(sds[i].x, sds[i].y, sds[i].z));
  }

  for (int i = 0; i < NUM_CUBES; ++i) {
    m_cubeTransformHandle[i] = createTransform(cube_node[i], Vec3f(transform_positions[i].x,
      transform_positions[i].y, 500));
  }

  // Create four transforms. Cube coordinates are in the range [-1, 1], set them 3 units apart.
  // m_transformHandle[0] = createTransform( sphere_node[0] );
  // TransformWriteLock( m_transformHandle[0] )->setName( "White Object Transform" );

  // m_transformHandle[1] = createTransform( sphere_node[1] , Vec3f( 3.0f, 0.0f, 0.0f ) );
  // TransformWriteLock( m_transformHandle[1] )->setName( "Red Object Transform" );

  // m_transformHandle[2] = createTransform( sphere_node[2] , Vec3f( 0.0f, 3.0f, 0.0f ) , Quatf( Vec3f( 0.0f, 1.0f, 0.0f ), 10.0f) );
  // TransformWriteLock( m_transformHandle[2] )->setName( "Green Object Transform" );

  // m_transformHandle[3] = createTransform( sphere_node[3] , Vec3f( 0.0f, 0.0f, 3.0f ) , Quatf( Vec3f( 0.0f, 0.0f, 1.0f ), 20.0f) );
  // TransformWriteLock( m_transformHandle[3] )->setName( "Blue Object Transform" );

  // Create a point light
  // m_pointLight = createPointLight( Vec3f( 5.0f, 8.0f, 6.0f ) );

  // m_pointLight = createPointLight( Vec3f( sds[200].x, sds[200].y, sds[200].z),
  //   Vec3f(1.0f, 0.0f, 0.0f));

  for (int i = 0; i < NUM_SPHERES; ++i) {
    m_pointLights[i] = createPointLight( Vec3f( sds[i].x, sds[i].y, sds[i].z),
      Vec3f(1.0f, 0.0f, 0.0f));
  }

  GroupSharedPtr groupHdl = Group::create();
  {
    WritableObject<Group> wg(groupHdl);
    for ( int i=0 ; i<NUM_SPHERES ; i++ )
    {
      wg->addChild( m_sphereTransformHandle[i] );
    }

    for ( int i=0 ; i<NUM_CUBES; i++ )
    {
      wg->addChild( m_cubeTransformHandle[i] );
    }

    for (int i = 0; i < NUM_SPHERES; ++i) {
      wg->addLightSource( m_pointLights[i] );
    }

    // wg->addLightSource( m_pointLight );
    wg->setName( "Root Node" );
  }

  theScene->setRootNode( groupHdl );
}

SimpleScene::~SimpleScene()
{
}
