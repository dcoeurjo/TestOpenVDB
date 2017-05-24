#include <openvdb/openvdb.h>
#include <openvdb/io/Stream.h>
#include <iostream>
#include <fstream>


// Populate the given grid with a narrow-band level set representation of a sphere.
// The width of the narrow band is determined by the grid's background value.
// (Example code only; use tools::createSphereSDF() in production.)
template<class GridType>
void
makeSphere(GridType& grid, float radius, const openvdb::Vec3f& c)
{
  typedef typename GridType::ValueType ValueT;
  // Distance value for the constant region exterior to the narrow band
  const ValueT outside = grid.background();
  // Distance value for the constant region interior to the narrow band
  // (by convention, the signed distance is negative in the interior of
  // a level set)
  const ValueT inside = -outside;
  // Use the background value as the width in voxels of the narrow band.
  // (The narrow band is centered on the surface of the sphere, which
  // has distance 0.)
  int padding = int(openvdb::math::RoundUp(openvdb::math::Abs(outside)));
  // The bounding box of the narrow band is 2*dim voxels on a side.
  int dim = int(radius + padding);
  // Get a voxel accessor.
  typename GridType::Accessor accessor = grid.getAccessor();
  // Compute the signed distance from the surface of the sphere of each
  // voxel within the bounding box and insert the value into the grid
  // if it is smaller in magnitude than the background value.
  openvdb::Coord ijk;
  int &i = ijk[0], &j = ijk[1], &k = ijk[2];
  for (i = c[0] - dim; i < c[0] + dim; ++i) {
    const float x2 = openvdb::math::Pow2(i - c[0]);
    for (j = c[1] - dim; j < c[1] + dim; ++j) {
      const float x2y2 = openvdb::math::Pow2(j - c[1]) + x2;
      for (k = c[2] - dim; k < c[2] + dim; ++k) {
        // The distance from the sphere surface in voxels
        const float dist = openvdb::math::Sqrt(x2y2
                                               + openvdb::math::Pow2(k - c[2])) - radius;
        // Convert the floating-point distance to the grid's value type.
        ValueT val = ValueT(dist);
        // Only insert distances that are smaller in magnitude than
        // the background value.
        if (val < inside || outside < val) continue;
        // Set the distance for voxel (i,j,k).
        accessor.setValue(ijk, val);
      }
    }
  }
  // Propagate the outside/inside sign information from the narrow band
  // throughout the grid.
 // openvdb::tools::signedFloodFill(grid.tree());
}


int main()
{
  // Initialize the OpenVDB library.  This must be called at least
  // once per program and may safely be called multiple times.
  openvdb::initialize();
  
  
  // Create an empty floating-point grid with background value 0.
  openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
  
  std::cout << "Testing random access:" << std::endl;
  // Get an accessor for coordinate-based access to voxels.
  openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
  
  // Define a coordinate with large signed indices.
  openvdb::Coord xyz(1000, -200000000, 30000000);
  
  // Set the voxel value at (1000, -200000000, 30000000) to 1.
  accessor.setValue(xyz, 1.0);
  
  
  // Verify that the voxel value at (1000, -200000000, 30000000) is 1.
  std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;
  // Reset the coordinates to those of a different voxel.
  xyz.reset(1000, 200000000, -30000000);
  
  // Verify that the voxel value at (1000, 200000000, -30000000) is
  // the background value, 0.
  std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;
  
  // Set the voxel value at (1000, 200000000, -30000000) to 2.
  accessor.setValue(xyz, 2.0);
  
  // Set the voxels at the two extremes of the available coordinate space.
  // For 32-bit signed coordinates these are (-2147483648, -2147483648, -2147483648)
  // and (2147483647, 2147483647, 2147483647).
  accessor.setValue(openvdb::Coord::min(), 3.0f);
  accessor.setValue(openvdb::Coord::max(), 4.0f);
 
  //Adding a sphere
  makeSphere(*grid, 100, openvdb::Vec3f(0,0,0));
  // Associate some metadata with the grid.
  grid->insertMeta("radius", openvdb::FloatMetadata(50.0));
  // Associate a scaling transform with the grid that sets the voxel size
  // to 0.5 units in world space.
  grid->setTransform(
                     openvdb::math::Transform::createLinearTransform(/*voxel size=*/0.5));
  // Identify the grid as a level set.
  grid->setGridClass(openvdb::GRID_LEVEL_SET);
  // Name the grid "LevelSetSphere".
  grid->setName("LevelSetSphere");

  
  
  std::cout << "Testing sequential access:" << std::endl;
  // Print all active ("on") voxels by means of an iterator.
  for (openvdb::FloatGrid::ValueOnCIter iter = grid->cbeginValueOn(); iter; ++iter) {
    std::cout << "Grid" << iter.getCoord() << " = " << *iter << std::endl;
  }
  // Stream the grids to a file.
  
  openvdb::GridPtrVecPtr grids(new openvdb::GridPtrVec);
  grids->push_back(grid);

  std::ofstream ofile("mygrids.vdb", std::ios_base::binary);
  openvdb::io::Stream(ofile).write(*grids);

  
}
