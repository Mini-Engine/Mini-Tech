#define NOMINMAX

#include "Mini/MiniBendDeformer.h"

#include "IECoreScene/Primitive.h"

#include <tbb/parallel_for.h>

#include <algorithm>
#include <cmath>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGaffer;

namespace
{
	// Bends a point in handle space. Returns the rotation angle applied at the point,
	// which is also the angle to rotate its normal by.
	struct Bend
	{
		float curvature;
		float low;
		float high;

		float angle( float y ) const
		{
			return curvature * std::clamp( y, low, high );
		}

		V3f point( const V3f &p ) const
		{
			const float yc = std::clamp( p.y, low, high );
			const float theta = curvature * yc;
			const float s = std::sin( theta );
			const float c = std::cos( theta );
			const float r = 1.0f / curvature;

			// Point on the arc, curving towards +X.
			V3f result( r + ( p.x - r ) * c, ( r - p.x ) * s, p.z );

			// Beyond the bounds carry on straight along the tangent at the end of the arc.
			const float extra = p.y - yc;
			result.x += extra * s;
			result.y += extra * c;
			return result;
		}

		V3f normal( const V3f &n, float theta ) const
		{
			const float s = std::sin( theta );
			const float c = std::cos( theta );
			return V3f( n.x * c + n.y * s, -n.x * s + n.y * c, n.z );
		}
	};
}

IE_CORE_DEFINERUNTIMETYPED( MiniBendDeformer );

size_t MiniBendDeformer::g_firstPlugIndex = 0;

MiniBendDeformer::MiniBendDeformer( const std::string &name )
	: ObjectProcessor( name )
{
	storeIndexOfNextChild( g_firstPlugIndex );

	addChild( new FloatPlug( "envelope", Plug::In, 1.0f ) );
	addChild( new FloatPlug( "curvature", Plug::In, 0.0f ) );
	addChild( new FloatPlug( "lowBound", Plug::In, -1.0f, std::numeric_limits<float>::lowest(), 0.0f ) );
	addChild( new FloatPlug( "highBound", Plug::In, 1.0f, 0.0f ) );
	addChild( new TransformPlug( "handle" ) );
}

Gaffer::FloatPlug *MiniBendDeformer::envelopePlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex );
}

const Gaffer::FloatPlug *MiniBendDeformer::envelopePlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex );
}

Gaffer::FloatPlug *MiniBendDeformer::curvaturePlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::FloatPlug *MiniBendDeformer::curvaturePlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex + 1 );
}

Gaffer::FloatPlug *MiniBendDeformer::lowBoundPlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::FloatPlug *MiniBendDeformer::lowBoundPlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex + 2 );
}

Gaffer::FloatPlug *MiniBendDeformer::highBoundPlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::FloatPlug *MiniBendDeformer::highBoundPlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex + 3 );
}

Gaffer::TransformPlug *MiniBendDeformer::handlePlug()
{
	return getChild<TransformPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::TransformPlug *MiniBendDeformer::handlePlug() const
{
	return getChild<TransformPlug>( g_firstPlugIndex + 4 );
}

bool MiniBendDeformer::affectsProcessedObject( const Gaffer::Plug *input ) const
{
	if(
		input == envelopePlug() ||
		input == curvaturePlug() ||
		input == lowBoundPlug() ||
		input == highBoundPlug() ||
		handlePlug()->isAncestorOf( input )
	)
	{
		return true;
	}

	return ObjectProcessor::affectsProcessedObject( input );
}

void MiniBendDeformer::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	ObjectProcessor::hashProcessedObject( path, context, h );

	envelopePlug()->hash( h );
	curvaturePlug()->hash( h );
	lowBoundPlug()->hash( h );
	highBoundPlug()->hash( h );
	handlePlug()->hash( h );
}

IECore::ConstObjectPtr MiniBendDeformer::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
{
	const auto inputPrimitive = IECore::runTimeCast<const Primitive>( inputObject );
	if( !inputPrimitive )
	{
		return inputObject;
	}

	const auto pIt = inputPrimitive->variables.find( "P" );
	if( pIt == inputPrimitive->variables.end() )
	{
		return inputObject;
	}
	const auto inputPoints = IECore::runTimeCast<const V3fVectorData>( pIt->second.data.get() );
	if( !inputPoints )
	{
		return inputObject;
	}

	Bend bend;
	bend.curvature = curvaturePlug()->getValue();
	bend.low = std::min( lowBoundPlug()->getValue(), 0.0f );
	bend.high = std::max( highBoundPlug()->getValue(), 0.0f );
	const float envelope = envelopePlug()->getValue();
	if( std::abs( bend.curvature ) < 1e-6f || envelope == 0.0f )
	{
		return inputObject;
	}

	const M44f handle = handlePlug()->matrix();
	const M44f toHandle = handle.inverse();
	const M44f normalToHandle = handle.transposed();
	const M44f normalFromHandle = toHandle.transposed();

	V3fVectorDataPtr outputPoints = new V3fVectorData( inputPoints->readable() );
	outputPoints->setInterpretation( inputPoints->getInterpretation() );
	auto &points = outputPoints->writable();

	// Normals are rotated by the bend at each point, if the primitive has them.
	V3fVectorDataPtr outputNormals;
	const auto nIt = inputPrimitive->variables.find( "N" );
	if( nIt != inputPrimitive->variables.end() )
	{
		if( const auto inputNormals = IECore::runTimeCast<const V3fVectorData>( nIt->second.data.get() ) )
		{
			if( inputNormals->readable().size() == points.size() )
			{
				outputNormals = new V3fVectorData( inputNormals->readable() );
				outputNormals->setInterpretation( inputNormals->getInterpretation() );
			}
		}
	}
	V3f *normals = outputNormals ? outputNormals->writable().data() : nullptr;

	tbb::parallel_for(
		tbb::blocked_range<size_t>( 0, points.size(), 4096 ),
		[&]( const tbb::blocked_range<size_t> &range )
		{
			for( size_t i = range.begin(); i != range.end(); ++i )
			{
				const V3f p = points[i] * toHandle;
				const float theta = bend.angle( p.y );
				const V3f bentPoint = bend.point( p ) * handle;
				points[i] += envelope * ( bentPoint - points[i] );

				if( normals )
				{
					V3f n, bentNormal;
					normalToHandle.multDirMatrix( normals[i], n );
					normalFromHandle.multDirMatrix( bend.normal( n, theta ), bentNormal );
					const V3f blended = normals[i] + envelope * ( bentNormal.normalized() - normals[i] );
					normals[i] = blended.length2() > 0.0f ? blended.normalized() : normals[i];
				}
			}
		}
	);

	PrimitivePtr outputPrimitive = inputPrimitive->copy();
	outputPrimitive->variables["P"] = PrimitiveVariable( pIt->second.interpolation, outputPoints );
	if( outputNormals )
	{
		outputPrimitive->variables["N"] = PrimitiveVariable( nIt->second.interpolation, outputNormals );
	}
	return outputPrimitive;
}
