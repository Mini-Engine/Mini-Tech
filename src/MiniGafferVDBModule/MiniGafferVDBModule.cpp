
#include <MiniGafferVDB/AdvectPoints.h>

#include "boost/python.hpp"

#include "IECorePython/RunTimeTypedBinding.h"

#include "GafferBindings/DependencyNodeBinding.h"
#include "IECore/MessageHandler.h"

#include "MiniGafferVDB/SampleGrids.h"

#include "MiniGafferVDB/OffsetLevelSet.h"

#include "MiniGafferVDB/PointsGridToPoints.h"
#include "MiniGafferVDB/DeleteGrids.h"
#include "MiniGafferVDB/AdvectGrids.h"
#include "MiniGafferVDB/AdvectPoints.h"
#include "MiniGafferVDB/MathOpGrids.h"
#include "MiniGafferVDB/StatisticsGrids.h"
#include "MiniGafferVDB/CSGLevelSets.h"
#include "MiniGafferVDB/TransformGrids.h"
#include "MiniGafferVDB/PointsToLevelSet.h"
#include "MiniGafferVDB/VDBObject.h"
#include "MiniGafferVDB/SampleGrids.h"
#include "MiniGafferVDB/FilterGrids.h"
#include "MiniGafferVDB/MeasureLevelSet.h"
#include "MiniGafferVDB/FilterLevelSet.h"
#include "MiniGafferVDB/VolumeToSpheres.h"
#include "MiniGafferVDB/ClipGrids.h"
#include "MiniGafferVDB/LevelSetToFog.h"
#include "MiniGafferVDB/SegmentLevelSets.h"
#include "MiniGafferVDB/IntersectLevelSet.h"
#include "MiniGafferVDB/MorphLevelSets.h"
#include "MiniGafferVDB/CompositeGrids.h"
#include "MiniGafferVDB/PlatonicLevelSet.h"
#include "MiniGafferVDB/SphereLevelSet.h"
#include "MiniGafferVDB/MaskGrids.h"


using namespace boost::python;

BOOST_PYTHON_MODULE( _MiniGafferVDB )
{

    GafferBindings::DependencyNodeClass<MiniGafferVDB::OffsetLevelSet>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::PointsGridToPoints>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::DeleteGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::AdvectGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::MathOpGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::StatisticsGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::CSGLevelSets>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::TransformGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::PointsToLevelSet>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::VDBObject>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::SampleGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::FilterGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::MeasureLevelSet>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::FilterLevelSet>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::VolumeToSpheres>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::ClipGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::LevelSetToFog>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::SegmentLevelSets>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::IntersectLevelSet>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::MorphLevelSets>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::CompositeGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::PlatonicLevelSet>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::SphereLevelSet>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::MaskGrids>();
    GafferBindings::DependencyNodeClass<MiniGafferVDB::AdvectPoints>();
}



