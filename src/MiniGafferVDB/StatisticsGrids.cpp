
#include "MiniGafferVDB/StatisticsGrids.h"

#include "IECore/StringAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECoreVDB/VDBObject.h"

#include "Gaffer/StringPlug.h"
#include "GafferScene/ScenePlug.h"

#include "openvdb/openvdb.h"
#include "openvdb/tools/GridOperators.h"
#include "openvdb/tools/Statistics.h"


using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreVDB;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGafferVDB;

IE_CORE_DEFINERUNTIMETYPED(StatisticsGrids );

size_t StatisticsGrids::g_firstPlugIndex = 0;

StatisticsGrids::StatisticsGrids(const std::string &name )
		: SceneElementProcessor( name, IECore::PathMatcher::NoMatch )
{
	storeIndexOfNextChild( g_firstPlugIndex );

	addChild ( new StringPlug( "grids", Plug::In, "*" ) );
    addChild ( new BoolPlug( "histogram", Plug::In, false ) );
    addChild ( new IntPlug( "bins", Plug::In, 8 ) );

	outPlug()->boundPlug()->setInput( inPlug()->boundPlug() );
	outPlug()->objectPlug()->setInput( inPlug()->objectPlug() );
}

StatisticsGrids::~StatisticsGrids()
{
}

Gaffer::StringPlug *StatisticsGrids::gridsPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *StatisticsGrids::gridsPlug() const
{
	return getChild<const StringPlug>( g_firstPlugIndex );
}

Gaffer::BoolPlug *StatisticsGrids::histogramPlug()
{
    return getChild<BoolPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::BoolPlug *StatisticsGrids::histogramPlug() const
{
    return getChild<BoolPlug>( g_firstPlugIndex + 1 );
}

Gaffer::IntPlug *StatisticsGrids::binsPlug()
{
    return getChild<IntPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::IntPlug *StatisticsGrids::binsPlug() const
{
    return getChild<IntPlug>( g_firstPlugIndex + 2 );
}

void StatisticsGrids::affects(const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneElementProcessor::affects( input, outputs );

	if (
	    input == gridsPlug() ||
	    input == histogramPlug() ||
        input == binsPlug()
	)
	{
		outputs.push_back( outPlug()->attributesPlug() );
	}
}

bool StatisticsGrids::processesAttributes() const
{
	return true;
}

void StatisticsGrids::hashProcessedAttributes(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	SceneElementProcessor::hashProcessedAttributes( path, context, h );

	h.append( inPlug()->objectHash( path ) );
	h.append( gridsPlug()->hash() );
    h.append( histogramPlug()->hash() );
    h.append( binsPlug()->hash() );
}

IECore::ConstCompoundObjectPtr StatisticsGrids::computeProcessedAttributes(const ScenePath &path, const Gaffer::Context *context, IECore::ConstCompoundObjectPtr inputAttributes ) const
{

	auto vdbObject = IECore::runTimeCast<const IECoreVDB::VDBObject> ( inPlug()->object( path ) );

	if ( !vdbObject )
	{
		return inputAttributes;
	}

	std::vector<std::string> gridNames = vdbObject->gridNames();
	std::string grids = gridsPlug()->getValue();

	IECore::CompoundObjectPtr newAttributes = inputAttributes->copy();

	for ( const auto &gridName : gridNames )
	{
		if ( !IECore::StringAlgo::matchMultiple( gridName, grids ) )
		{
		    continue;
        }
		openvdb::GridBase::ConstPtr srcGrid = vdbObject->findGrid( gridName );

        openvdb::FloatGrid::ConstPtr grid = openvdb::GridBase::constGrid<openvdb::FloatGrid>( srcGrid );

        if ( !grid )
        {
            continue;
        }

        openvdb::math::Stats stats = openvdb::tools::statistics( grid->cbeginValueOn() ); //, openvdb::Local::addIfPositive, /*threaded=*/true

        newAttributes->members().insert(std::make_pair( std::string("stats:") + gridName + ":average", new IECore::FloatData( stats.avg() ) ) );
        newAttributes->members().insert(std::make_pair( std::string("stats:") + gridName + ":min", new IECore::FloatData( stats.min() ) ) );
        newAttributes->members().insert(std::make_pair( std::string("stats:") + gridName + ":max", new IECore::FloatData( stats.max() ) ) );
        newAttributes->members().insert(std::make_pair( std::string("stats:") + gridName + ":stddev", new IECore::FloatData( stats.stdDev() ) ) );

        const int numBins = binsPlug()->getValue();
        if ( histogramPlug()->getValue() && numBins > 0 )
        {
            openvdb::math::Histogram histogram = openvdb::tools::histogram( grid->cbeginValueOn(), stats.min(), stats.max(), numBins);

            std::vector<float> binValues (histogram.numBins());

            for (size_t i = 0; i < histogram.numBins(); ++i)
            {
                binValues[i] = (float) ( histogram.count( i ) / ((double) histogram.size()));
            }

            newAttributes->members().insert(std::make_pair( std::string("stats:") + gridName + ":histogram", new IECore::FloatVectorData( binValues ) ) );

        }

	}

	return newAttributes;
}


