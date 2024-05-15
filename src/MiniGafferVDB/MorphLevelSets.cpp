
#include "MiniGafferVDB/MorphLevelSets.h"
#include "GafferVDB/Interrupter.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "GafferScene/ScenePlug.h"
#include "Gaffer/StringPlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/LevelSetMorph.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED(MorphLevelSets );

size_t MorphLevelSets::g_firstPlugIndex = 0;

MorphLevelSets::MorphLevelSets(const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild(g_firstPlugIndex);

    addChild( new ScenePlug( "in", Gaffer::Plug::In ) );

    addChild( new StringPlug( "vdbLocation", Plug::In, "/vdb" ) );
    addChild( new StringPlug( "grids", Plug::In, "surface" ) );
    addChild( new StringPlug( "outputGrid", Plug::In, "${grid}" ) );
    addChild( new FloatPlug( "time0", Plug::In, 0.0f ) );
    addChild( new FloatPlug( "time1", Plug::In, 1.0f ) );
}

MorphLevelSets::~MorphLevelSets()
{
}

const GafferScene::ScenePlug *MorphLevelSets::otherPlug() const
{
    return  getChild<ScenePlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *MorphLevelSets::vdbLocationPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

const Gaffer::StringPlug *MorphLevelSets::vdbLocationPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 1);
}

Gaffer::StringPlug *MorphLevelSets::gridsPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *MorphLevelSets::gridsPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::StringPlug *MorphLevelSets::outputGridPlug()
{
    return  getChild<StringPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::StringPlug *MorphLevelSets::outputGridPlug() const
{
    return  getChild<StringPlug>( g_firstPlugIndex + 3 );
}

Gaffer::FloatPlug *MorphLevelSets::time0Plug()
{
    return  getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::FloatPlug *MorphLevelSets::time0Plug() const
{
    return  getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

Gaffer::FloatPlug *MorphLevelSets::time1Plug()
{
    return  getChild<FloatPlug>( g_firstPlugIndex + 5 );
}

const Gaffer::FloatPlug *MorphLevelSets::time1Plug() const
{
    return  getChild<FloatPlug>( g_firstPlugIndex + 5 );
}

void MorphLevelSets::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if(
	input == vdbLocationPlug() ||
	input == gridsPlug() ||
	input == outputGridPlug() ||
	input->parent() == otherPlug() ||
	input == time0Plug() ||
	input == time1Plug() )
	{
		outputs.push_back( outPlug()->objectPlug() );
		outputs.push_back( outPlug()->boundPlug() );
	}
}

bool MorphLevelSets::processesObject() const
{
	return true;
}

void MorphLevelSets::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedObject( path, context, h );

    ScenePlug::ScenePath otherPath;
    ScenePlug::stringToPath( vdbLocationPlug()->getValue(), otherPath );

    h.append( vdbLocationPlug()->hash() );
	h.append( gridsPlug()->hash() );
    h.append( outputGridPlug()->hash() );
    h.append( otherPlug()->fullTransformHash( otherPath ) );
    h.append( otherPlug()->objectHash( otherPath ) );
    h.append( time0Plug()->hash() );
    h.append( time1Plug()->hash() );

}

IECore::ConstObjectPtr MorphLevelSets::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const
{
    const IECoreVDB::VDBObject *vdbObject = runTimeCast<const IECoreVDB::VDBObject>( inputObject.get() );
    if ( !vdbObject )
    {
        return inputObject;
    }

    std::vector<std::string> grids = vdbObject->gridNames();

    std::string gridsToProcess = gridsPlug()->getValue();

    IECoreVDB::VDBObjectPtr newVDBObject = vdbObject->copy();

    GafferVDB::Interrupter interrupter( context->canceller() );

    ScenePlug::ScenePath p ;
    ScenePlug::stringToPath( vdbLocationPlug()->getValue(), p );

    ConstVDBObjectPtr vdbObjectB = runTimeCast<const VDBObject>( otherPlug()->object( p ) );

    for (const auto &gridName : grids)
    {
        if ( !IECore::StringAlgo::matchMultiple( gridName, gridsToProcess ) )
        {
            continue;
        }

        openvdb::GridBase::ConstPtr grid = vdbObject->findGrid( gridName );
        openvdb::FloatGrid::ConstPtr floatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( grid );

        if ( !floatGrid )
        {
            continue;
        }

        openvdb::GridBase::ConstPtr otherGrid = vdbObjectB->findGrid( gridName );

        if ( !otherGrid )
        {
            continue;
        }

        openvdb::FloatGrid::ConstPtr otherFloatGrid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( otherGrid );
        if ( !otherFloatGrid )
        {
            continue;
        }

        openvdb::FloatGrid::Ptr outputGrid = floatGrid->deepCopy();

        Context::EditableScope scope( context );
        scope.set( IECore::InternedString("grid"), &gridName );
        const std::string outGridName = context->substitute( outputGridPlug()->getValue() );

        openvdb::tools::LevelSetMorphing<openvdb::FloatGrid, GafferVDB::Interrupter> morpher( *outputGrid, *otherFloatGrid,  &interrupter );

        morpher.advect( time0Plug()->getValue(), time1Plug()->getValue() );

        if ( interrupter.wasInterrupted() )
        {
            throw IECore::Cancelled();
        }

        outputGrid->setName( outGridName );
        newVDBObject->insertGrid( outputGrid );
    }
    return newVDBObject;
}



bool MorphLevelSets::processesBound() const
{
	return true;
}

void MorphLevelSets::hashProcessedBound(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedBound( path, context, h );

	hashProcessedObject( path, context, h);
}

Imath::Box3f MorphLevelSets::computeProcessedBound(const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const
{
	// todo calculate bounds from vdb grids
	return inputBound;
}

