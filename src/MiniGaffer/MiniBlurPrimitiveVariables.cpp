#include "Mini/MiniBlurPrimitiveVariables.h"

#include "Gaffer/StringPlug.h"

#include "IECore/DataAlgo.h"

#include "IECoreScene/Primitive.h"
#include "IECoreScene/MeshAlgo.h"

#include <IECore/VectorTypedData.h>
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

struct BlurFunctor
{

public:
	BlurFunctor( ConstMeshPrimitivePtr mesh, const size_t &iterations, const float &iterationScale, const std::vector<int> &neighbors, const std::vector<int> &offsets, const std::string weights ) :
	m_mesh(mesh), m_iterations(iterations), m_iterationScale(iterationScale), m_neighbors(neighbors), m_offsets(offsets), m_pointWeights(nullptr)
	{
		if( const FloatVectorData *w = m_mesh->variableData<FloatVectorData>( weights ) )
		{
			m_pointWeights = &w->readable();
		}
	}

	DataPtr operator()( const Color3fVectorData *data, const string &name, const PrimitiveVariable &primitiveVariable )
	{
		return doBlur<Color3fVectorData>( data, name, primitiveVariable );
	}

	DataPtr operator()( const V2fVectorData *data, const string &name, const PrimitiveVariable &primitiveVariable )
	{
		return doBlur<V2fVectorData>( data, name, primitiveVariable );
	}

	DataPtr operator()( const V3fVectorData *data, const string &name, const PrimitiveVariable &primitiveVariable )
	{
		return doBlur<V3fVectorData>( data, name, primitiveVariable );
	}

	DataPtr operator()( const FloatVectorData *data, const string &name, const PrimitiveVariable &primitiveVariable )
	{
		return doBlur<FloatVectorData>( data, name, primitiveVariable );
	}

	DataPtr operator()( const IntVectorData *data, const string &name, const PrimitiveVariable &primitiveVariable )
	{
		return doBlur<IntVectorData>( data, name, primitiveVariable );
	}

	DataPtr operator()( const Data *data, const string &name, const PrimitiveVariable &primitiveVariable )
	{
		throw IECore::Exception( fmt::format( "PrimitiveVariable \"{}\" has unsupported type \"{}\"", name, data->typeName() ) );
	}

	void calculateEdgeWeights( const std::string &position )
	{
		const V3fVectorData *positionData = m_mesh->variableData<V3fVectorData>( position );
		if( !positionData )
		{
			throw InvalidArgumentException( fmt::format( "MiniBlurPrimitiveVariables : MeshPrimitive has no Vertex \"{}\" primitive variable.", position ) );
		}

		m_edgeWeights.resize( m_neighbors.size(), 0.0f );

		const auto &points = positionData->readable();
		for( size_t i = 0; i < points.size(); ++i )
		{
			float totalEdgeWeights = 0.0f;
			const size_t firstIndex = i > 0 ? m_offsets[i - 1] : 0;
			const size_t lastIndex = m_offsets[i];
			for( size_t j = firstIndex; j < lastIndex; ++j )
			{
				float edgeWeight = (points[i] - points[m_neighbors[j]]).length();
				if( edgeWeight < 10e-5 )
				{
					edgeWeight = 0.0f;
				}
				else
				{
					edgeWeight = 1.0f / edgeWeight;
				}
				totalEdgeWeights += edgeWeight;
				m_edgeWeights[j] = edgeWeight;
			}

			for( size_t j = firstIndex; j < lastIndex; ++j )
			{
				m_edgeWeights[j] /= totalEdgeWeights;
			}
		}
	}

private:
	template<typename T>
	DataPtr doBlur( const Data *data, const string &name, const PrimitiveVariable &primitiveVariable )
	{
		using ResultType = typename T::ValueType::value_type;
		using DataView = PrimitiveVariable::IndexedView<ResultType>;
		DataView dataView;

		switch( primitiveVariable.interpolation )
		{
			case PrimitiveVariable::Vertex :
			case PrimitiveVariable::Varying :
				dataView = DataView( primitiveVariable );
				break;
			default :
				throw IECore::Exception( fmt::format( "Primitive variable \"{}\" must have Vertex or Varying interpolation", name ) );
		}

		size_t count = dataView.size();
		typename T::Ptr resultData = new T( dataView.data() );
		auto &result = resultData->writable();

		typename T::Ptr iterationData = new T();
		auto &iterationResult = iterationData->writable();
		iterationResult.reserve( count );

		for( size_t n = 0; n < m_iterations; ++n )
		{
			tbb::parallel_for(
				tbb::blocked_range<size_t>(0, count),
				[this, &result, &iterationResult](tbb::blocked_range<size_t> r) {
				for( size_t i = r.begin(); i != r.end(); ++i )
				{
					auto sum = ResultType(0);
					size_t firstIndex = i > 0 ? m_offsets[i - 1] : 0;
					size_t lastIndex = m_offsets[i];

					if( m_edgeWeights.size() > 0 )
					{
						for( size_t j = firstIndex; j < lastIndex; ++j )
						{
							sum += result[m_neighbors[j]] * m_edgeWeights[j];
						}

						sum = sum - result[i];
					}
					else
					{
						for( size_t j = firstIndex; j < lastIndex; ++j )
						{
							sum += result[m_neighbors[j]];
						}

						sum = sum / (lastIndex - firstIndex) - result[i];
					}

					if( m_pointWeights )
					{
						iterationResult[i] = result[i] + m_iterationScale * (*m_pointWeights)[i] * sum;
					}
					else
					{
						iterationResult[i] = result[i] + m_iterationScale * sum;
					}
				}
			});

			for( size_t i = 0; i < count; ++i )
			{
				result[i] = iterationResult[i];
			}
		}

		return resultData;
	}

	IECoreScene::ConstMeshPrimitivePtr m_mesh;
	const size_t m_iterations;
	const float m_iterationScale;

	const std::vector<int> &m_neighbors;
	const std::vector<int> &m_offsets;

	const std::vector<float> *m_pointWeights;
	std::vector<float> m_edgeWeights;

};

} // namespace

IE_CORE_DEFINERUNTIMETYPED(MiniBlurPrimitiveVariables );

size_t MiniBlurPrimitiveVariables::g_firstPlugIndex = 0;

MiniBlurPrimitiveVariables::MiniBlurPrimitiveVariables(const std::string &name )
	: Deformer( name )
{
	storeIndexOfNextChild( g_firstPlugIndex );

	addChild( new IntPlug( "weighting", Plug::In, Weighting::Uniform, Weighting::Uniform, Weighting::EdgeLength ) );
	addChild( new StringPlug( "primitiveVariables", Plug::In, "" ) );
	addChild( new StringPlug( "position", Plug::In, "P" ) );
	addChild( new IntPlug( "iterations", Plug::In, 1, 0 ) );
	addChild( new FloatPlug( "iterationScale", Plug::In, 0.5, 0.0, 1.0 ) );
	addChild( new StringPlug( "weight", Plug::In, "" ) );
}

MiniBlurPrimitiveVariables::~MiniBlurPrimitiveVariables()
{
}

Gaffer::IntPlug *MiniBlurPrimitiveVariables::weightingPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 0 );
}

const Gaffer::IntPlug *MiniBlurPrimitiveVariables::weightingPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 0 );
}

Gaffer::StringPlug *MiniBlurPrimitiveVariables::primitiveVariablesPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *MiniBlurPrimitiveVariables::primitiveVariablesPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

Gaffer::StringPlug *MiniBlurPrimitiveVariables::positionPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *MiniBlurPrimitiveVariables::positionPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::IntPlug *MiniBlurPrimitiveVariables::iterationsPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::IntPlug *MiniBlurPrimitiveVariables::iterationsPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 3 );
}

Gaffer::FloatPlug *MiniBlurPrimitiveVariables::iterationScalePlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::FloatPlug *MiniBlurPrimitiveVariables::iterationScalePlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex + 4 );
}

Gaffer::StringPlug *MiniBlurPrimitiveVariables::weightPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 5 );
}

const Gaffer::StringPlug *MiniBlurPrimitiveVariables::weightPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 5 );
}

bool MiniBlurPrimitiveVariables::affectsProcessedObject(const Gaffer::Plug *input ) const
{
	return Deformer::affectsProcessedObject( input ) ||
		input == weightingPlug() ||
		input == primitiveVariablesPlug() ||
		input == positionPlug() ||
		input == iterationsPlug() ||
		input == iterationScalePlug() ||
		input == weightPlug()
	;
}

void MiniBlurPrimitiveVariables::hashProcessedObject(const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	Deformer::hashProcessedObject( path, context, h );

	weightingPlug()->hash(h);
	primitiveVariablesPlug()->hash(h);
	positionPlug()->hash(h);
	iterationsPlug()->hash(h);
	iterationScalePlug()->hash(h);
	weightPlug()->hash(h);
}

IECore::ConstObjectPtr MiniBlurPrimitiveVariables::computeProcessedObject(const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
{
	const MeshPrimitive *mesh = runTimeCast<const MeshPrimitive>( inputObject );
	if( !mesh )
	{
		return inputObject;
	}

	const std::string primitiveVariables = primitiveVariablesPlug()->getValue();
	if( primitiveVariables.empty() )
	{
		return inputObject;
	}

	const int iterations = iterationsPlug()->getValue();
	if( iterations == 0 )
	{
		return inputObject;
	}

	const float iterationScale = iterationScalePlug()->getValue();
	if( iterationScale == 0.0f )
	{
		return inputObject;
	}

	const bool useEdgeWeights = (MiniBlurPrimitiveVariables::Weighting)weightingPlug()->getValue() == MiniBlurPrimitiveVariables::EdgeLength;
	if( useEdgeWeights )
	{
		if( positionPlug()->getValue().empty() )
		{
			IECore::msg( IECore::Msg::Warning, "MiniBlurPrimitiveVariables", "A position primitive variable is required to calculate edge weights." );
			return inputObject;
		}
	}

	MeshPrimitivePtr result = mesh->copy();

	std::pair<IntVectorDataPtr, IntVectorDataPtr> connectedVerts = MeshAlgo::connectedVertices( result.get() );
	ConstIntVectorDataPtr neighborData = connectedVerts.first;
	ConstIntVectorDataPtr offsetData = connectedVerts.second;

	BlurFunctor fn( result.get(), iterations, iterationScale, neighborData->readable(), offsetData->readable(), weightPlug()->getValue() );

	if( useEdgeWeights )
	{
		fn.calculateEdgeWeights( positionPlug()->getValue() );
	}

	for( const auto &primVar : result->variables )
	{
		if( StringAlgo::matchMultiple( primVar.first, primitiveVariables ) )
		{
			const DataPtr blurredData = IECore::dispatch( primVar.second.data.get(), fn, primVar.first, primVar.second );
			result->variables[primVar.first] = PrimitiveVariable( primVar.second.interpolation, blurredData );
		}
	}

	return result;
}

bool MiniBlurPrimitiveVariables::adjustBounds() const
{
	if( !Deformer::adjustBounds() )
	{
		return false;
	}

	return StringAlgo::matchMultiple( "P", primitiveVariablesPlug()->getValue() );
}
