
#include "MiniGafferVDB/VolumeToSpheres.h"
#include "GafferVDB/Interrupter.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/VolumeToSpheres.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED( VolumeToSpheres );

size_t VolumeToSpheres::g_firstPlugIndex = 0;

VolumeToSpheres::VolumeToSpheres( const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild(g_firstPlugIndex);

	addChild( new StringPlug( "grids", Plug::In, "surface" ) );
    addChild( new IntPlug( "minSpheres", Plug::In, 0, 0 ) );
    addChild( new IntPlug( "maxSpheres", Plug::In, 100, 0 ) );
    addChild( new BoolPlug( "overlapping", Plug::In, false ) );
    addChild( new FloatPlug( "minRadius", Plug::In, 0.0f, 0.0f ) );
    addChild( new FloatPlug( "maxRadius", Plug::In, 10.0f, 0.0f ) );
    addChild( new FloatPlug( "isoValue", Plug::In, 0.0f, 0.0f ) );
}

VolumeToSpheres::~VolumeToSpheres()
{
}

Gaffer::StringPlug *VolumeToSpheres::gridsPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *VolumeToSpheres::gridsPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::IntPlug *VolumeToSpheres::minSpheresPlug()
{
    return getChild<IntPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::IntPlug *VolumeToSpheres::minSpheresPlug() const
{
    return getChild<IntPlug>( g_firstPlugIndex + 1 );
}

Gaffer::IntPlug *VolumeToSpheres::maxSpheresPlug()
{
    return getChild<IntPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::IntPlug *VolumeToSpheres::maxSpheresPlug() const
{
    return getChild<IntPlug>( g_firstPlugIndex + 2 );
}

Gaffer::BoolPlug *VolumeToSpheres::overlappingPlug()
{
    return getChild<BoolPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::BoolPlug *VolumeToSpheres::overlappingPlug() const
{
    return getChild<BoolPlug>( g_firstPlugIndex + 3 );
}

Gaffer::FloatPlug *VolumeToSpheres::minRadiusPlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::FloatPlug *VolumeToSpheres::minRadiusPlug() const
{
    return getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

Gaffer::FloatPlug *VolumeToSpheres::maxRadiusPlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 5 );
}

const Gaffer::FloatPlug *VolumeToSpheres::maxRadiusPlug() const
{
    return getChild<FloatPlug>( g_firstPlugIndex + 5 );
}

Gaffer::FloatPlug *VolumeToSpheres::isoValuePlug()
{
    return getChild<FloatPlug>( g_firstPlugIndex + 6 );
}

const Gaffer::FloatPlug *VolumeToSpheres::isoValuePlug() const
{
    return getChild<FloatPlug>( g_firstPlugIndex + 6 );
}

void VolumeToSpheres::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if( input == gridsPlug() ||
        input == minSpheresPlug() ||
        input == maxSpheresPlug() ||
        input == overlappingPlug() ||
        input == minRadiusPlug() ||
        input == maxRadiusPlug() ||
        input == isoValuePlug() )
	{
		outputs.push_back( outPlug()->objectPlug() );
		outputs.push_back( outPlug()->boundPlug() );
	}
}

bool VolumeToSpheres::processesObject() const
{
	return true;
}

void VolumeToSpheres::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedObject( path, context, h );

	h.append( gridsPlug()->hash() );
    h.append( minSpheresPlug()->hash() );
	h.append( maxSpheresPlug()->hash() );
	h.append( overlappingPlug()->hash() );
	h.append( minRadiusPlug()->hash() );
	h.append( maxRadiusPlug()->hash() );
	h.append( isoValuePlug()->hash() );

}

IECore::ConstObjectPtr VolumeToSpheres::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
	const IECoreVDB::VDBObject *vdbObject = runTimeCast<const IECoreVDB::VDBObject>(inputObject.get());
	if( !vdbObject )
	{
		return inputObject;
	}

	std::vector<std::string> grids = vdbObject->gridNames();

	std::string gridsToProcess = gridsPlug()->getValue();

    GafferVDB::Interrupter interrupter( context->canceller() );

    const int instanceCount = 10000;

	for (const auto &gridName : grids )
	{
		if ( !IECore::StringAlgo::matchMultiple(gridName, gridsToProcess ) )
		{
            continue;
        }

        openvdb::GridBase::ConstPtr grid = vdbObject->findGrid( gridName );
        openvdb::FloatGrid::ConstPtr floatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( grid );

        if ( !floatGrid )
        {
            continue;
        }

        std::vector<openvdb::Vec4f> spheres;
        openvdb::tools::fillWithSpheres<openvdb::FloatGrid, GafferVDB::Interrupter>( *floatGrid,
                spheres,
                openvdb::Vec2i( minSpheresPlug()->getValue(), maxSpheresPlug()->getValue() ),
                overlappingPlug()->getValue(),
                minRadiusPlug()->getValue(),
                maxRadiusPlug()->getValue(),
                isoValuePlug()->getValue(),
                instanceCount,
                &interrupter);

        IECore::V3fVectorDataPtr positions = new IECore::V3fVectorData();
        IECore::FloatVectorDataPtr radii = new IECore::FloatVectorData();

        auto &wposArray = positions->writable();
        auto &sizeArray = radii->writable();

        for (const auto s : spheres)
        {
            wposArray.push_back(Imath::V3f(s[0], s[1], s[2]));
            sizeArray.push_back(s[3] * 2.0f);
        }

        IECoreScene::PointsPrimitivePtr pointsPrimitive = new IECoreScene::PointsPrimitive( positions );
        pointsPrimitive->variables["width"] = IECoreScene::PrimitiveVariable(IECoreScene::PrimitiveVariable::Interpolation::Vertex,
                radii);

        return pointsPrimitive;
	}

	return vdbObject;
}

bool VolumeToSpheres::processesBound() const
{
	return true;
}

void VolumeToSpheres::hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedBound( path, context, h );

	hashProcessedObject( path, context, h);
}

Imath::Box3f VolumeToSpheres::computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
	// todo calculate bounds from vdb grids
	return inputBound;
}

