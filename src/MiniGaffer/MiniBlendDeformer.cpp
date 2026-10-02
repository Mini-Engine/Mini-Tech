#define NOMINMAX

#include "Mini/MiniBlendDeformer.h"

#include "IECoreScene/Primitive.h"

#include "IECore/StringAlgo.h"

#include "Gaffer/ArrayPlug.h"
#include "Gaffer/StringPlug.h"

#include "fmt/format.h"

#include <limits>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGaffer;

namespace
{
	// Children of a target element : enabled, location, weight.
	const BoolPlug *enabledPlug( const ValuePlug *target ) { return target->getChild<BoolPlug>( 0 ); }
	const StringPlug *locationPlug( const ValuePlug *target ) { return target->getChild<StringPlug>( 1 ); }
	const FloatPlug *weightPlug( const ValuePlug *target ) { return target->getChild<FloatPlug>( 2 ); }

	ScenePlug::ScenePath makeScenePath( const std::string &p )
	{
		ScenePlug::ScenePath output;
		IECore::StringAlgo::tokenize<IECore::InternedString>( p, '/', back_inserter( output ) );
		return output;
	}

	// A target only contributes if it is enabled, has a location and a non zero weight.
	bool isActive( const ValuePlug *target )
	{
		return
			enabledPlug( target )->getValue() &&
			weightPlug( target )->getValue() != 0.0f &&
			!locationPlug( target )->getValue().empty()
		;
	}

	const V3fVectorData *pointsOf( const IECore::Object *object, const std::string &description, size_t expectedSize )
	{
		const auto primitive = IECore::runTimeCast<const Primitive>( object );
		if( !primitive )
		{
			throw IECore::Exception( fmt::format( "{} is not a primitive", description ) );
		}

		const auto it = primitive->variables.find( "P" );
		const auto points = it != primitive->variables.end() ? IECore::runTimeCast<const V3fVectorData>( it->second.data.get() ) : nullptr;
		if( !points )
		{
			throw IECore::Exception( fmt::format( "{} has no V3f \"P\" primitive variable", description ) );
		}

		if( points->readable().size() != expectedSize )
		{
			throw IECore::Exception( fmt::format( "{} has {} points but {} are required", description, points->readable().size(), expectedSize ) );
		}

		return points;
	}

	// `holder` keeps the object alive for as long as the returned points are used.
	const V3fVectorData *pointsAt( const ScenePlug *scene, const std::string &location, const std::string &description, size_t expectedSize, IECore::ConstObjectPtr &holder )
	{
		const ScenePlug::ScenePath path = makeScenePath( location );
		if( !scene->exists( path ) )
		{
			throw IECore::Exception( fmt::format( "{} location \"{}\" does not exist", description, location ) );
		}
		holder = scene->object( path );
		return pointsOf( holder.get(), fmt::format( "{} \"{}\"", description, location ), expectedSize );
	}
}

IE_CORE_DEFINERUNTIMETYPED( MiniBlendDeformer );

size_t MiniBlendDeformer::g_firstPlugIndex = 0;

MiniBlendDeformer::MiniBlendDeformer( const std::string &name )
	: ObjectProcessor( name )
{
	storeIndexOfNextChild( g_firstPlugIndex );

	addChild( new FloatPlug( "envelope", Plug::In, 1.0f ) );
	addChild( new StringPlug( "baseLocation" ) );
	addChild( new StringPlug( "maskPrimVar" ) );

	ValuePlugPtr target = new ValuePlug( "target" );
	target->addChild( new BoolPlug( "enabled", Plug::In, true ) );
	target->addChild( new StringPlug( "location" ) );
	target->addChild( new FloatPlug( "weight", Plug::In, 0.0f ) );
	addChild( new ArrayPlug( "targets", Plug::In, target, 0, std::numeric_limits<size_t>::max(), Plug::Default, false ) );
}

Gaffer::FloatPlug *MiniBlendDeformer::envelopePlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex );
}

const Gaffer::FloatPlug *MiniBlendDeformer::envelopePlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *MiniBlendDeformer::baseLocationPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *MiniBlendDeformer::baseLocationPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

Gaffer::StringPlug *MiniBlendDeformer::maskPrimVarPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *MiniBlendDeformer::maskPrimVarPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::ArrayPlug *MiniBlendDeformer::targetsPlug()
{
	return getChild<ArrayPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::ArrayPlug *MiniBlendDeformer::targetsPlug() const
{
	return getChild<ArrayPlug>( g_firstPlugIndex + 3 );
}

bool MiniBlendDeformer::affectsProcessedObject( const Gaffer::Plug *input ) const
{
	if(
		input == envelopePlug() ||
		input == baseLocationPlug() ||
		input == maskPrimVarPlug() ||
		targetsPlug()->isAncestorOf( input )
	)
	{
		return true;
	}

	return ObjectProcessor::affectsProcessedObject( input );
}

void MiniBlendDeformer::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	ObjectProcessor::hashProcessedObject( path, context, h );

	envelopePlug()->hash( h );
	maskPrimVarPlug()->hash( h );
	baseLocationPlug()->hash( h );

	const std::string baseLocation = baseLocationPlug()->getValue();
	if( !baseLocation.empty() )
	{
		h.append( inPlug()->objectHash( makeScenePath( baseLocation ) ) );
	}

	for( const auto &child : Plug::InputRange( *targetsPlug() ) )
	{
		const auto target = static_cast<const ValuePlug *>( child.get() );
		if( !isActive( target ) )
		{
			continue;
		}

		locationPlug( target )->hash( h );
		weightPlug( target )->hash( h );
		h.append( inPlug()->objectHash( makeScenePath( locationPlug( target )->getValue() ) ) );
	}
}

IECore::ConstObjectPtr MiniBlendDeformer::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
{
	const auto inputPrimitive = IECore::runTimeCast<const Primitive>( inputObject );
	if( !inputPrimitive )
	{
		return inputObject;
	}

	const auto inputPointsIt = inputPrimitive->variables.find( "P" );
	if( inputPointsIt == inputPrimitive->variables.end() )
	{
		return inputObject;
	}
	const auto inputPoints = IECore::runTimeCast<const V3fVectorData>( inputPointsIt->second.data.get() );
	if( !inputPoints )
	{
		return inputObject;
	}
	const size_t numPoints = inputPoints->readable().size();

	// Gather the active targets, validating them against the points being deformed.

	struct Target
	{
		ConstObjectPtr holder;
		const V3fVectorData *points;
		float weight;
	};
	std::vector<Target> targets;

	for( const auto &child : Plug::InputRange( *targetsPlug() ) )
	{
		const auto target = static_cast<const ValuePlug *>( child.get() );
		if( !isActive( target ) )
		{
			continue;
		}

		Target t;
		t.weight = weightPlug( target )->getValue();
		t.points = pointsAt( inPlug(), locationPlug( target )->getValue(), "Blend target", numPoints, t.holder );
		targets.push_back( std::move( t ) );
	}

	const float envelope = envelopePlug()->getValue();
	if( targets.empty() || envelope == 0.0f )
	{
		return inputObject;
	}

	// Per point weights, if requested.

	const FloatVectorData *mask = nullptr;
	const std::string maskName = maskPrimVarPlug()->getValue();
	if( !maskName.empty() )
	{
		const auto maskIt = inputPrimitive->variables.find( maskName );
		if( maskIt != inputPrimitive->variables.end() )
		{
			mask = IECore::runTimeCast<const FloatVectorData>( maskIt->second.data.get() );
		}
		if( !mask || mask->readable().size() != numPoints )
		{
			throw IECore::Exception( fmt::format( "Mask primitive variable \"{}\" must be a float vector with one value per point", maskName ) );
		}
	}

	// The base the target deltas are measured from.

	ConstObjectPtr baseHolder;
	const V3fVectorData *basePoints = inputPoints;
	const std::string baseLocation = baseLocationPlug()->getValue();
	if( !baseLocation.empty() )
	{
		basePoints = pointsAt( inPlug(), baseLocation, "Base", numPoints, baseHolder );
	}

	// Accumulate the weighted deltas.

	V3fVectorDataPtr outputPoints = new V3fVectorData( inputPoints->readable() );
	outputPoints->setInterpretation( inputPoints->getInterpretation() );
	auto &output = outputPoints->writable();
	const auto &base = basePoints->readable();

	for( size_t i = 0; i < numPoints; ++i )
	{
		V3f delta( 0.0f );
		for( const Target &t : targets )
		{
			delta += t.weight * ( t.points->readable()[i] - base[i] );
		}

		output[i] += ( mask ? envelope * mask->readable()[i] : envelope ) * delta;
	}

	PrimitivePtr outputPrimitive = inputPrimitive->copy();
	outputPrimitive->variables["P"] = PrimitiveVariable( inputPointsIt->second.interpolation, outputPoints );
	return outputPrimitive;
}

Gaffer::ValuePlug::CachePolicy MiniBlendDeformer::processedObjectComputeCachePolicy() const
{
	return ValuePlug::CachePolicy::TaskCollaboration;
}
