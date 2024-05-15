#include <Mini/MiniSubdivideMesh.h>

#include <IECore/DataAlgo.h>

#include <IECoreScene/MeshPrimitive.h>

#include <opensubdiv/far/topologyRefiner.h>
#include <opensubdiv/far/primvarRefiner.h>
#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/bfr/refinerSurfaceFactory.h>
#include <opensubdiv/bfr/surface.h>

namespace
{
    template<typename T>
    struct Refinable
    {
        Refinable() {}

        Refinable(T const & v) : data(v) {}

        Refinable(Refinable const & src)
        {
            data = src.data;
        }

        void Clear( void* = nullptr)
        {
            data = T(0);
        }

        void AddWithWeight(Refinable const & src, float weight)
        {
            data += weight * src.data;
        }

        T data;
    };

    struct PrimvarRefiner
    {
        OpenSubdiv::Far::PrimvarRefiner& refiner;
        const OpenSubdiv::Far::TopologyLevel& lastTopologyLevel;
        int maxLevel;

        PrimvarRefiner(OpenSubdiv::Far::PrimvarRefiner& refiner, const OpenSubdiv::Far::TopologyLevel& lastTopologyLevel, int maxLevel)
        : refiner(refiner), lastTopologyLevel(lastTopologyLevel), maxLevel(maxLevel)
        {}

        template<typename VectorDataType>
        boost::intrusive_ptr<VectorDataType> VertexInterpolate(const VectorDataType* data, IECoreScene::PrimitiveVariable primitiveVariable)
        {
            int NumVerts = lastTopologyLevel.GetNumVertices();
            int TotalNumVerts = refiner.GetTopologyRefiner().GetNumVerticesTotal();
            int firstOfLastVerts = TotalNumVerts - NumVerts;

            using ResultType = typename VectorDataType::ValueType::value_type;
            using DataView = IECoreScene::PrimitiveVariable::IndexedView<ResultType>;
            DataView dataView = DataView( primitiveVariable );

            std::vector<Refinable<ResultType>> refinedBuffer(TotalNumVerts);

            for ( int i = 0; i < data->readable().size(); ++i )
            {
                refinedBuffer[i] = dataView[i] ;
            }

            Refinable<ResultType>* src2 = refinedBuffer.data();
            for (int level = 1; level <= maxLevel; ++level)
            {
                Refinable<ResultType> * dst2 = src2 + refiner.GetTopologyRefiner().GetLevel(level-1).GetNumVertices();
                refiner.Interpolate(level, src2, dst2);
                src2 = dst2;
            }

            auto outputData = boost::intrusive_ptr<VectorDataType> { new VectorDataType };

            outputData->writable().resize(NumVerts);

            for (int vert = 0; vert < NumVerts; ++vert)
            {
                outputData->writable()[vert] = refinedBuffer[firstOfLastVerts + vert].data;
            }
            return outputData;
        }

        template<typename VectorDataType>
        boost::intrusive_ptr<VectorDataType> FaceVaryingInterpolate(const VectorDataType* data, IECoreScene::PrimitiveVariable primitiveVariable)
        {
            int NumFaces = lastTopologyLevel.GetNumFaces();
            int TotalFVar = refiner.GetTopologyRefiner().GetNumFVarValuesTotal();
            int NumFVarValues = lastTopologyLevel.GetNumFVarValues();
            int firstOfFVar = TotalFVar - NumFVarValues;

            using ResultType = typename VectorDataType::ValueType::value_type;
            using DataView = IECoreScene::PrimitiveVariable::IndexedView<ResultType>;
            DataView dataView = DataView( primitiveVariable );

            std::vector<Refinable<ResultType>> refinedBuffer(TotalFVar);

            for ( int i = 0; i < dataView.size(); ++i )
            {
                refinedBuffer[i] = dataView[i];
            }

            Refinable<ResultType>* src2 = refinedBuffer.data();
            for (int level = 1; level <= maxLevel; ++level)
            {
                int lastLevelSize = refiner.GetTopologyRefiner().GetLevel(level-1).GetNumFVarValues();
                Refinable<ResultType> * dst2 = src2 + lastLevelSize;
                refiner.InterpolateFaceVarying(level, src2, dst2);
                src2 = dst2;
            }

            auto outputData = boost::intrusive_ptr<VectorDataType> { new VectorDataType };

            outputData->writable().resize(lastTopologyLevel.GetNumFaceVertices());

            int index = 0;
            for (int face = 0; face < NumFaces; ++face)
            {
                OpenSubdiv::Far::ConstIndexArray fuvs = lastTopologyLevel.GetFaceFVarValues(face);
                for (int i = 0; i < fuvs.size();++i)
                {
                    outputData->writable()[index++] = refinedBuffer[firstOfFVar + fuvs[i]].data;
                }
            }
            return outputData;
        }

        template<typename VectorDataType>
        boost::intrusive_ptr<VectorDataType> UniformInterpolate(const VectorDataType* data, IECoreScene::PrimitiveVariable primitiveVariable)
        {
            int NumFaces = lastTopologyLevel.GetNumFaces();
            int TotalFaces = refiner.GetTopologyRefiner().GetNumFacesTotal();
            int firstOfLastFaces = TotalFaces - NumFaces;

            using ResultType = typename VectorDataType::ValueType::value_type;
            using DataView = IECoreScene::PrimitiveVariable::IndexedView<ResultType>;
            DataView dataView = DataView( primitiveVariable );

            std::vector<Refinable<ResultType>> refinedBuffer(TotalFaces);

            for ( int i = 0; i < data->readable().size(); ++i )
            {
                refinedBuffer[i] = dataView[i] ;
            }

            Refinable<ResultType>* src2 = refinedBuffer.data();
            for (int level = 1; level <= maxLevel; ++level)
            {
                Refinable<ResultType> * dst2 = src2 + refiner.GetTopologyRefiner().GetLevel(level-1).GetNumFaces();
                refiner.InterpolateFaceUniform(level, src2, dst2);
                src2 = dst2;
            }

            auto outputData = boost::intrusive_ptr<VectorDataType> { new VectorDataType };

            outputData->writable().resize(NumFaces);

            for (int vert = 0; vert < NumFaces; ++vert)
            {
                outputData->writable()[vert] = refinedBuffer[firstOfLastFaces + vert].data;
            }
            return outputData;
        }

        template<typename VectorDataType>
        boost::intrusive_ptr<VectorDataType> Interpolate(const VectorDataType* data, IECoreScene::PrimitiveVariable primitiveVariable)
        {
            if ( primitiveVariable.interpolation == IECoreScene::PrimitiveVariable::Interpolation::Vertex )
            {
                return VertexInterpolate(data, primitiveVariable);
            }
            else if (primitiveVariable.interpolation == IECoreScene::PrimitiveVariable::Interpolation::Uniform)
            {
                return UniformInterpolate(data, primitiveVariable);
            }
            else if (primitiveVariable.interpolation == IECoreScene::PrimitiveVariable::Interpolation::FaceVarying)
            {
                return FaceVaryingInterpolate(data, primitiveVariable);
            }
            return nullptr;
        }

        IECore::DataPtr operator()( const IECore::Color3fVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            return Interpolate(data, primitiveVariable);
        }

        IECore::DataPtr operator()( const IECore::V2fVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            auto refinedData = Interpolate(data, primitiveVariable);
            refinedData->setInterpretation(data->getInterpretation());
            return refinedData;
        }

        IECore::DataPtr operator()( const IECore::V3fVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            auto refinedData = Interpolate(data, primitiveVariable);
            refinedData->setInterpretation(data->getInterpretation());
            return refinedData;
        }

        IECore::DataPtr operator()( const IECore::FloatVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            return Interpolate(data, primitiveVariable);
        }

        IECore::DataPtr operator()( const IECore::IntVectorData *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            return Interpolate(data, primitiveVariable);
        }

        IECore::DataPtr operator()( const IECore::Data *data, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable )
        {
            return nullptr;
            //throw IECore::Exception( fmt::format( "PrimitiveVariable \"{}\" has unsupported type \"{}\"", name, data->typeName() ) );
        }
    };

}
namespace MiniGaffer
{
    size_t MiniSubdivideMesh::g_firstPlugIndex = 0;

    IE_CORE_DEFINERUNTIMETYPED( MiniGaffer::MiniSubdivideMesh );

    MiniSubdivideMesh::MiniSubdivideMesh ( const std::string &name ) : GafferScene::ObjectProcessor( name )
    {
        storeIndexOfNextChild(g_firstPlugIndex);
        addChild(new Gaffer::IntPlug ("level", Gaffer::Plug::In, 0, 0, 8));
    }

    Gaffer::IntPlug *MiniSubdivideMesh::levelPlug()
    {
        return getChild<Gaffer::IntPlug>( g_firstPlugIndex + 0 );
    }

    const Gaffer::IntPlug *MiniSubdivideMesh::levelPlug() const
    {
        return getChild<Gaffer::IntPlug>( g_firstPlugIndex + 0 );
    }


    bool MiniSubdivideMesh::affectsProcessedObject( const Gaffer::Plug *input ) const
    {
        if (input == inPlug()->objectPlug() || input == levelPlug())
        {
            return true;
        }
        return false;
    }

    void MiniSubdivideMesh::hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const
    {
        ObjectProcessor::hashProcessedObject(path, context, h);
        levelPlug()->hash(h);
    }

    IECore::ConstObjectPtr MiniSubdivideMesh::computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, const IECore::Object *inputObject ) const
    {
        auto meshPrimitive = IECore::runTimeCast<const IECoreScene::MeshPrimitive>(inputObject);

        if (!meshPrimitive)
        {
            return inputObject;
        }

        auto meshCopy = meshPrimitive->copy();

        OpenSubdiv::Far::TopologyRefiner * meshTopology = nullptr;
        OpenSubdiv::Sdc::SchemeType type = OpenSubdiv::Sdc::SCHEME_CATMARK;
        OpenSubdiv::Sdc::Options options;
        options.SetVtxBoundaryInterpolation(OpenSubdiv::Sdc::Options::VTX_BOUNDARY_EDGE_AND_CORNER);

        OpenSubdiv::Far::TopologyDescriptor::FVarChannel channels[1];
        channels[0].numValues = meshPrimitive->variableSize(IECoreScene::PrimitiveVariable::Interpolation::FaceVarying );

        std::vector<int> sequence(channels[0].numValues);
        for (int i = 0; i < sequence.size(); ++i) { sequence[i] = i; }
        channels[0].valueIndices = sequence.data() ;

        OpenSubdiv::Far::TopologyDescriptor desc;

        // Add the channel topology to the main descriptor
        desc.numFVarChannels = 1;
        desc.fvarChannels = channels;

        auto srcPositionData = IECore::runTimeCast<IECore::V3fVectorData >(meshCopy->variables["P"].data);

        desc.numVertices = (int) srcPositionData->readable().size();
        desc.numFaces = (int) meshPrimitive->numFaces() ;
        desc.numVertsPerFace = meshCopy->verticesPerFace()->readable().data();
        desc.vertIndicesPerFace = meshCopy->vertexIds()->readable().data();

        //  Instantiate a Far::TopologyRefiner from the descriptor.
        OpenSubdiv::Far::TopologyRefiner * refiner = OpenSubdiv::Far::TopologyRefinerFactory<OpenSubdiv::Far::TopologyDescriptor>::Create(desc,
                                                                OpenSubdiv::Far::TopologyRefinerFactory<OpenSubdiv::Far::TopologyDescriptor>::Options(type,options));

        int maxLevel = levelPlug()->getValue();
        refiner->RefineUniform(OpenSubdiv::Far::TopologyRefiner::UniformOptions(maxLevel));
        // Allocate a buffer for vertex primvar data. The buffer length is set to
        // be the sum of all children vertices up to the highest level of refinement.

        std::vector<Refinable<Imath::V3f>> positionBuffer(refiner->GetNumVerticesTotal());

        // Initialize coarse mesh positions
        for (int i=0; i< srcPositionData->readable().size(); ++i)
        {
            positionBuffer[i] =  srcPositionData->readable()[i] ;
        }

        // todo look at indexed primvars ( for all nodes )
        // todo settings for UV edges

        // Interpolate vertex primvar data
        OpenSubdiv::Far::PrimvarRefiner primvarRefiner(*refiner);

        Refinable<Imath::V3f>* src2 = positionBuffer.data();
        for (int level = 1; level <= maxLevel; ++level) {
            Refinable<Imath::V3f> * dst2 = src2 + refiner->GetLevel(level-1).GetNumVertices();
            primvarRefiner.Interpolate(level, src2, dst2);
            src2 = dst2;
        }

        auto refinedMesh = IECoreScene::MeshPrimitivePtr { new IECoreScene::MeshPrimitive };
        OpenSubdiv::Far::TopologyLevel const & refLastLevel = refiner->GetLevel(maxLevel);

        int NumFaces = refLastLevel.GetNumFaces();

        auto verticesPerFace = IECore::IntVectorDataPtr { new IECore::IntVectorData };
        auto vertexIds = IECore::IntVectorDataPtr { new IECore::IntVectorData };

        verticesPerFace->writable().resize(NumFaces);
        vertexIds->writable().resize(NumFaces * 4);

        // Print faces
        int index = 0 ;
        for (int face = 0; face < NumFaces; ++face)
        {
            verticesPerFace->writable()[face] = 4;

            OpenSubdiv::Far::ConstIndexArray fverts = refLastLevel.GetFaceVertices(face);

            // all refined Catmark faces should be quads
            assert(fverts.size()==4);

            for (int vert=0; vert<fverts.size(); ++vert)
            {
                vertexIds->writable()[index++] = fverts[vert];
            }
        }

        refinedMesh->setTopology(verticesPerFace, vertexIds);

        ::PrimvarRefiner fn(primvarRefiner, refLastLevel, maxLevel);

        for (const auto& primvar : meshPrimitive->variables)
        {
            if (primvar.second.interpolation == IECoreScene::PrimitiveVariable::Constant)
            {
                refinedMesh->variables[primvar.first] = IECoreScene::PrimitiveVariable(primvar.second.interpolation,
                                                                                       primvar.second.data->copy());
                continue;
            }

            const IECore::DataPtr refinedData = IECore::dispatch( primvar.second.data.get(), fn, primvar.first, primvar.second );
            if (refinedData)
            {
                refinedMesh->variables[primvar.first] = IECoreScene::PrimitiveVariable(primvar.second.interpolation,
                                                                                       refinedData);
            }
        }

        delete refiner;

        return refinedMesh;
    }

    Gaffer::ValuePlug::CachePolicy MiniSubdivideMesh::processedObjectComputeCachePolicy() const
    {
        return Gaffer::ValuePlug::CachePolicy::Standard;
    }

}