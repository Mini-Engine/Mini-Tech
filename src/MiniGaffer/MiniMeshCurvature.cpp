#include "Mini/MiniMeshCurvature.h"

#include "Gaffer/StringPlug.h"

#include "IECoreScene/Primitive.h"
#include "IECoreScene/PrimitiveEvaluator.h"
#include "IECoreScene/MeshAlgo.h"

#include <tbb/tbb.h>

#include "fmt/format.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGaffer;

namespace
{

PrimitiveVariable calculateCurvature( const MeshPrimitive *mesh, const std::string &position, const std::string &normal, const float scale, const bool normalize, const Canceller *canceller )
{
	const V3fVectorData *positionData = mesh->variableData<V3fVectorData>( position, PrimitiveVariable::Vertex );
	if( !positionData )
	{
		throw InvalidArgumentException( fmt::format( "MiniMeshCurvature : MeshPrimitive has no Vertex \"{}\" primitive variable.", position ) );
	}

	const V3fVectorData *normalData = mesh->variableData<V3fVectorData>( normal, PrimitiveVariable::Vertex );
	if( !normalData )
	{
		// \TODO: Handle faceVarying interpolated normals
		throw InvalidArgumentException( fmt::format( "MiniMeshCurvature : MeshPrimitive has no Vertex \"{}\" primitive variable.", normal ) );
	}

	const auto &points = positionData->readable();
	const auto &normals = normalData->readable();

	std::pair<IntVectorDataPtr, IntVectorDataPtr> connectivityPtr = MeshAlgo::connectedVertices( mesh );
	const auto &neighbors = connectivityPtr.first->readable();
	const auto &offsets = connectivityPtr.second->readable();

	IECore::FloatVectorDataPtr curvatureData = new IECore::FloatVectorData();
	auto &curvature = curvatureData->writable();
	curvature.resize( points.size(), 0.0f );

	tbb::parallel_for(
		tbb::blocked_range<size_t>( 0, points.size() ),
		[ scale, &points, &normals, &neighbors, &offsets, &curvature, canceller ](tbb::blocked_range<size_t> r) {
			for( size_t i = r.begin(); i != r.end(); ++i )
			{
				if( i % 10000 == 0 )
				{
					Canceller::check( canceller );
				}

				const int firstIndex = i > 0 ? offsets[i - 1] : 0;
				const int lastIndex = offsets[i];

				float sumEdgeLengths = 0.0f;
				for( size_t j = firstIndex; j < lastIndex; ++j )
				{
					sumEdgeLengths += (points[neighbors[j]] - points[i]).length();
				}

				const V3f normal = normals[i].normalized();
				float c = 0.0f;
				for( size_t j = firstIndex; j < lastIndex; ++j )
				{
					const V3f edge = points[neighbors[j]] - points[i];
					const float angle = acos( normal.dot( edge.normalized() ) );
					c += ( angle / M_PI - 0.5f ) * ( edge.length() / sumEdgeLengths );
				}

				curvature[i] = c;
			}
		});

	if( normalize )
	{
		const auto [minCurvature, maxCurvature] = std::minmax_element( begin( curvature ), end( curvature ) );
		const float minScale = -1.0 / *minCurvature;
		const float maxScale = 1.0 / *maxCurvature;
		for( size_t i = 0; i < curvature.size(); ++i )
		{
			if( curvature[i] > 0 )
			{
				curvature[i] *= maxScale;
			}
			else if( curvature[i] < 0 )
			{
				curvature[i] *= minScale;
			}
		}
	}

	if( scale != 1.0f )
	{
		for( size_t i = 0; i < curvature.size(); ++i )
		{
			curvature[i] *= scale;
		}
	}

	return PrimitiveVariable( IECoreScene::PrimitiveVariable::Interpolation::Vertex, curvatureData );
}

} // namespace

IE_CORE_DEFINERUNTIMETYPED(MiniMeshCurvature );

size_t MiniMeshCurvature::g_firstPlugIndex = 0;

MiniMeshCurvature::MiniMeshCurvature(const std::string &name )
	: ObjectProcessor( name )
{
	storeIndexOfNextChild(g_firstPlugIndex);
	addChild( new StringPlug( "position", Plug::In, "P" ) );
	addChild( new StringPlug( "normal", Plug::In, "N" ) );
	addChild( new StringPlug( "curvature", Plug::In, "curvature" ) );
	addChild( new FloatPlug( "scale", Plug::In, 1.0 ) );
	addChild( new BoolPlug( "normalize", Plug::In, false ) );
}

MiniMeshCurvature::~MiniMeshCurvature()
{
}

Gaffer::StringPlug *MiniMeshCurvature::positionPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *MiniMeshCurvature::positionPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *MiniMeshCurvature::normalPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *MiniMeshCurvature::normalPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

Gaffer::StringPlug *MiniMeshCurvature::curvaturePlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *MiniMeshCurvature::curvaturePlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::FloatPlug *MiniMeshCurvature::scalePlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::FloatPlug *MiniMeshCurvature::scalePlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex + 3 );
}

Gaffer::BoolPlug *MiniMeshCurvature::normalizePlug()
{
	return getChild<BoolPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::BoolPlug *MiniMeshCurvature::normalizePlug() const
{
	return getChild<BoolPlug>( g_firstPlugIndex + 4 );
}

bool MiniMeshCurvature::affectsProcessedObject(const Gaffer::Plug *input ) const
{
	return
		ObjectProcessor::affectsProcessedObject( input ) ||
		input == positionPlug() ||
		input == normalPlug() ||
		input == curvaturePlug() ||
		input == scalePlug() ||
		input == normalizePlug()
	;
}

void MiniMeshCurvature::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	ObjectProcessor::hashProcessedObject( path, context, h );
	positionPlug()->hash(h);
	normalPlug()->hash(h);
	curvaturePlug()->hash(h);
	scalePlug()->hash(h);
	normalizePlug()->hash(h);
}

IECore::ConstObjectPtr MiniMeshCurvature::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
{
	const MeshPrimitive *mesh = runTimeCast<const MeshPrimitive>( inputObject );
	if( !mesh )
	{
		return inputObject;
	}

	const string curvature = curvaturePlug()->getValue();
	if( curvature.empty() )
	{
		IECore::msg(IECore::Msg::Warning, "MiniMeshCurvature", "No curvature primvar specified!");
		return inputObject;
	}

	MeshPrimitivePtr meshWithCurvature = mesh->copy();

	const string position = positionPlug()->getValue();
	const string normal = normalPlug()->getValue();
	const float scale = scalePlug()->getValue();
	const bool normalize = normalizePlug()->getValue();

	meshWithCurvature->variables[ curvature ] = calculateCurvature( meshWithCurvature.get(), position, normal, scale, normalize, context->canceller() );

	return meshWithCurvature;
}

Gaffer::ValuePlug::CachePolicy MiniGaffer::MiniMeshCurvature::processedObjectComputeCachePolicy() const
{
	return ValuePlug::CachePolicy::TaskCollaboration;
}
