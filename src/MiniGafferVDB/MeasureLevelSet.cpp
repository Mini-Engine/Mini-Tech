#include "MiniGafferVDB/MeasureLevelSet.h"
#include "GafferVDB/Interrupter.h"
#include "MiniGafferVDB/Dispatcher.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"
#include "GafferScene/ScenePlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/LevelSetMeasure.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;

namespace
{
    //! results of measuring the level set
    struct Measurements
    {
        Measurements() : area(0.0), volume(0.0), averageMeanCurvature(0.0), valid(false), hasCurvature(false) {}

        openvdb::Real area;
        openvdb::Real volume;
        openvdb::Real averageMeanCurvature;

        bool valid;
        bool hasCurvature;
    };

    //! The kernel of the measurement operation as a template to support different grid types
    template<typename G>
    class Measurer
    {
    public:

        Measurements operator()(typename G::ConstPtr grid, const MeasureLevelSet* node, const Gaffer::Context* context) const
        {
            GafferVDB::Interrupter interrupter( context->canceller() );

            openvdb::tools::LevelSetMeasure<G, GafferVDB::Interrupter> measure ( *grid, &interrupter );

            Measurements measurements;
            measurements.valid = true;

            const bool calculateCurvature = node->curvaturePlug()->getValue();
            const bool worldUnits = node->worldUnitsPlug()->getValue();

        	measurements.area = measure.area(worldUnits);
        	measurements.volume = measure.volume(worldUnits);
        	if (calculateCurvature)
        	{
        		measurements.averageMeanCurvature = measure.avgMeanCurvature(worldUnits);
        		measurements.hasCurvature = true;

        	}

            if ( interrupter.wasInterrupted() )
            {
                throw IECore::Cancelled();
            }

            return measurements;
        }
    };

} // namespace

IE_CORE_DEFINERUNTIMETYPED( MeasureLevelSet );

size_t MeasureLevelSet::g_firstPlugIndex = 0;

MeasureLevelSet::MeasureLevelSet(const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild( g_firstPlugIndex );

	addChild ( new StringPlug( "grids", Plug::In, "*" ) );
    addChild ( new BoolPlug( "curvature", Plug::In, false ) );
    addChild ( new BoolPlug( "worldUnits", Plug::In, false ) );

	outPlug()->boundPlug()->setInput( inPlug()->boundPlug() );
	outPlug()->objectPlug()->setInput( inPlug()->objectPlug() );
}

MeasureLevelSet::~MeasureLevelSet()
{
}

Gaffer::StringPlug *MeasureLevelSet::gridsPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *MeasureLevelSet::gridsPlug() const
{
	return getChild<const StringPlug>( g_firstPlugIndex );
}

Gaffer::BoolPlug *MeasureLevelSet::curvaturePlug()
{
    return getChild<BoolPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::BoolPlug *MeasureLevelSet::curvaturePlug() const
{
    return getChild<const BoolPlug>( g_firstPlugIndex + 1 );
}

Gaffer::BoolPlug *MeasureLevelSet::worldUnitsPlug()
{
    return getChild<BoolPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::BoolPlug *MeasureLevelSet::worldUnitsPlug() const
{
    return getChild<const BoolPlug>( g_firstPlugIndex + 2 );
}

void MeasureLevelSet::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if ( input == gridsPlug() || input == curvaturePlug() || input == worldUnitsPlug() )
	{
		outputs.push_back( outPlug()->attributesPlug() );
	}
}

bool MeasureLevelSet::processesAttributes() const
{
	return true;
}

void MeasureLevelSet::hashProcessedAttributes(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedAttributes( path, context, h );

	h.append( inPlug()->objectHash( path ) );
	h.append( gridsPlug()->hash() );
	h.append( curvaturePlug()->hash() );
	h.append( worldUnitsPlug()->hash() );
}

IECore::ConstCompoundObjectPtr MeasureLevelSet::computeProcessedAttributes(const ScenePath &path, const Gaffer::Context *context, IECore::ConstCompoundObjectPtr inputAttributes ) const
{
	auto vdbObject = IECore::runTimeCast<const IECoreVDB::VDBObject> ( inPlug()->object( path ) );

	if ( !vdbObject )
	{
		return inputAttributes;
	}

	std::vector<std::string> gridNames = vdbObject->gridNames();
	std::string grids = gridsPlug()->getValue();

	IECore::CompoundObjectPtr newAttributes = inputAttributes->copy();

    ScalarGridDispatcher<Measurer, MeasureLevelSet, Measurements> measurer(this, context );

	for ( const auto &gridName : gridNames )
	{
		if ( !IECore::StringAlgo::matchMultiple( gridName, grids ) )
		{
            continue;
        }

        openvdb::GridBase::ConstPtr srcGrid = vdbObject->findGrid( gridName );

        Measurements measurements = measurer( srcGrid );

        if ( measurements.valid )
        {
            newAttributes->members().insert(std::make_pair( std::string("levelset:") + gridName + ":area", new IECore::DoubleData( measurements.area ) ) );
            newAttributes->members().insert(std::make_pair( std::string("levelset:") + gridName + ":volume", new IECore::DoubleData( measurements.volume ) ) );

            if ( measurements.hasCurvature )
            {
                newAttributes->members().insert(std::make_pair( std::string("levelset:") + gridName + ":averageMeanCurvature", new IECore::DoubleData( measurements.averageMeanCurvature ) ) );
            }
        }

	}

	return newAttributes;
}


