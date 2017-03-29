#include <iostream>

#include <openvdb/openvdb.h>
#include <openvdb/io/Stream.h>


#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/io/readers/GenericReader.h>
#include <DGtal/images/ImageContainerBySTLVector.h>

using namespace DGtal;
using namespace Z3i;


int main(int argc, char **argv)
{
  
  //Input read
  trace.beginBlock("Reading Vol");
  typedef ImageContainerBySTLVector<Domain, unsigned char> Image;
  Image image = GenericReader<Image>::import(argv[1]);
  trace.endBlock();
  
  //OpenVDB init
  openvdb::initialize();
  openvdb::Int32Grid::Ptr grid = openvdb::Int32Grid::create();
  
  //Feeding the openvdb
  trace.beginBlock("Inserting values");
  openvdb::Int32Grid::Accessor accessor = grid->getAccessor();
  for(auto p: image.domain())
  {
    if (image(p) == 0) continue;
    openvdb::Coord xyz(p[0],p[1],p[2]);
    accessor.setValue(xyz, image(p));
  }
  trace.endBlock();
  
  //Pruning
  trace.beginBlock("Pruning");
  grid->pruneGrid();
  trace.endBlock();
  
  //Export
  trace.beginBlock("Exporting");
  openvdb::GridPtrVecPtr grids(new openvdb::GridPtrVec);
  grids->push_back(grid);
  std::ofstream ofile(argv[2], std::ios_base::binary);
  openvdb::io::Stream(ofile).write(*grids);
  trace.endBlock();
  
  return 0;
}
