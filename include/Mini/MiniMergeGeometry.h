#pragma once

#include "TypeIds.h"

#include "Gaffer/StringPlug.h"
#include "Gaffer/ArrayPlug.h"
#include "Gaffer/TypedObjectPlug.h"

#include "GafferScene/SceneProcessor.h"
#include "GafferScene/ScenePlug.h"
#include "GafferScene/FilterPlug.h"

#include "IECoreScene/SceneInterface.h"

namespace MiniGaffer
{
    class MiniMergeGeometry : public GafferScene::SceneProcessor
    {
    public:

        explicit MiniMergeGeometry( const std::string &name = defaultName<MiniMergeGeometry>() );
        ~MiniMergeGeometry() = default;

        GafferScene::FilterPlug *filterPlug();
        const GafferScene::FilterPlug *filterPlug() const;

        Gaffer::StringPlug *mergedNamePlug();
        const Gaffer::StringPlug *mergedNamePlug() const;

        Gaffer::BoolPlug *pedanticPlug();
        const Gaffer::BoolPlug *pedanticPlug() const;

        void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

        GAFFER_NODE_DECLARE_TYPE( MiniGaffer::MiniMergeGeometry, TypeId::MiniMergeGeometryTypeId, GafferScene::SceneProcessor );

    protected:

        void hashBound( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const override;
        void hashTransform( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const override;
        void hashAttributes( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const override;
        void hashObject( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const override;
        void hashChildNames( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const override;
        void hashSetNames( const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const override;
        void hashSet( const IECore::InternedString &setName, const Gaffer::Context *context, const GafferScene::ScenePlug *parent, IECore::MurmurHash &h ) const override;

        Imath::Box3f computeBound( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const override;
        Imath::M44f computeTransform( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const override;
        IECore::ConstCompoundObjectPtr computeAttributes( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const override;
        IECore::ConstObjectPtr computeObject( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const override;
        IECore::ConstInternedStringVectorDataPtr computeChildNames( const ScenePath &path, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const override;
        IECore::ConstInternedStringVectorDataPtr computeSetNames( const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const override;
        IECore::ConstPathMatcherDataPtr computeSet( const IECore::InternedString &setName, const Gaffer::Context *context, const GafferScene::ScenePlug *parent ) const;
    private:
        static size_t g_firstPlugIndex;

    };

    IE_CORE_DECLAREPTR(MiniMergeGeometry)
} // namespace MiniGaffer