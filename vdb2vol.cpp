#include <iostream>
#include <openvdb/openvdb.h>
#include <openvdb/io/Stream.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/io/readers/GenericReader.h>
#include <DGtal/io/writers/GenericWriter.h>
#include <DGtal/images/ImageContainerBySTLVector.h>

using namespace DGtal;
using namespace Z3i;

int main(int argc, char **argv)
{
  // Initialize the OpenVDB library.  This must be called at least
  // once per program and may safely be called multiple times.
  openvdb::initialize();
  
  // Create a VDB file object.
  openvdb::io::File file(argv[1]);
  // Open the file.  This reads the file header, but not any grids.
  file.open();

  // Loop over all grids in the file and retrieve a shared pointer
  // to the one named "LevelSetSphere".  (This can also be done
  // more simply by calling file.readGrid("LevelSetSphere").)
  openvdb::GridBase::Ptr baseGrid; // = file.readGrid();

  //We read the first grid
  baseGrid = file.readGrid(file.beginName().gridName());
  
  file.close();
  
  // From the example above, "LevelSetSphere" is known to be a FloatGrid,
  // so cast the generic grid pointer to a FloatGrid pointer.
  openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
  
  
  openvdb::CoordBBox bbox = grid->evalActiveVoxelBoundingBox();
  
  Point lower( bbox.min().x(), bbox.min().y(), bbox.min().z());
  Point upper( bbox.max().x(), bbox.max().y(), bbox.max().z());
  Domain dom(lower,upper);
  ImageContainerBySTLVector<Domain,unsigned char> image(dom);
  
  trace.info() <<"BBox: "<<lower<<"x "<<upper<<std::endl;
  
  // Visit and update all of the grid's active values, which correspond to
  // voxels on the narrow band.
  auto cpt=0;
  float minval,maxval;
  grid->evalMinMax(minval, maxval);
  
  for (openvdb::FloatGrid::ValueOnIter iter = grid->beginValueOn(); iter; ++iter)
  {
    ++cpt;
    Point p( iter.getCoord().x(), iter.getCoord().y(), iter.getCoord().z() );
//    image.setValue(p,  static_cast<unsigned char>(std::round(1.0+255.0*iter.getValue()/maxval)));
    image.setValue(p,128);
  }
  
  image>> "output.vol";
  std::cout <<"Read "<<cpt<<" active voxels.  "<< grid->activeVoxelCount() <<std::endl;
  
  return 0;
}
