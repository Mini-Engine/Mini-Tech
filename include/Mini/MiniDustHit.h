#pragma once

#include "TypeIds.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"
#include "Gaffer/TypedPlug.h"

#include "GafferScene/BranchCreator.h"

namespace MiniGaffer
{

    // Emits a puff of dust particles (a PointsPrimitive) whenever two of the
    // collider locations start touching. Collisions are detected by comparing
    // the world space vertices of the colliders, and the particles are a pure
    // function of the hits that happened in the last `lifetime` frames, so the
    // node is deterministic and can be evaluated at any frame.
    class MiniDustHit : public GafferScene::BranchCreator
    {
    public:
        explicit MiniDustHit( const std::string &name = defaultName<MiniDustHit>() );
        ~MiniDustHit() override = default;

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGaffer::MiniDustHit, TypeId::MiniDustHitTypeId, GafferScene::BranchCreator );

        Gaffer::StringPlug *namePlug();
        const Gaffer::StringPlug *namePlug() const;
        Gaffer::StringPlug *collidersPlug();
        const Gaffer::StringPlug *collidersPlug() const;
        Gaffer::FloatPlug *contactDistancePlug();
        const Gaffer::FloatPlug *contactDistancePlug() const;
        Gaffer::FloatPlug *minSpeedPlug();
        const Gaffer::FloatPlug *minSpeedPlug() const;
        Gaffer::IntPlug *cooldownPlug();
        const Gaffer::IntPlug *cooldownPlug() const;
        Gaffer::IntPlug *countPerHitPlug();
        const Gaffer::IntPlug *countPerHitPlug() const;
        Gaffer::FloatPlug *countBySpeedPlug();
        const Gaffer::FloatPlug *countBySpeedPlug() const;
        Gaffer::IntPlug *seedPlug();
        const Gaffer::IntPlug *seedPlug() const;
        Gaffer::IntPlug *lifetimePlug();
        const Gaffer::IntPlug *lifetimePlug() const;
        Gaffer::FloatPlug *speedPlug();
        const Gaffer::FloatPlug *speedPlug() const;
        Gaffer::FloatPlug *spreadPlug();
        const Gaffer::FloatPlug *spreadPlug() const;
        Gaffer::FloatPlug *normalBiasPlug();
        const Gaffer::FloatPlug *normalBiasPlug() const;
        Gaffer::FloatPlug *dragPlug();
        const Gaffer::FloatPlug *dragPlug() const;
        Gaffer::V3fPlug *gravityPlug();
        const Gaffer::V3fPlug *gravityPlug() const;
        Gaffer::FloatPlug *startWidthPlug();
        const Gaffer::FloatPlug *startWidthPlug() const;
        Gaffer::FloatPlug *endWidthPlug();
        const Gaffer::FloatPlug *endWidthPlug() const;
        Gaffer::Color3fPlug *colorPlug();
        const Gaffer::Color3fPlug *colorPlug() const;
        Gaffer::FloatPlug *fadeOutPlug();
        const Gaffer::FloatPlug *fadeOutPlug() const;

    protected:

        bool affectsBranchBound( const Gaffer::Plug *input ) const override;
        void hashBranchBound( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        Imath::Box3f computeBranchBound( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const override;

        bool affectsBranchTransform( const Gaffer::Plug *input ) const override;
        void hashBranchTransform( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        Imath::M44f computeBranchTransform( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const override;

        bool affectsBranchAttributes( const Gaffer::Plug *input ) const override;
        void hashBranchAttributes( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstCompoundObjectPtr computeBranchAttributes( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const override;

        bool affectsBranchObject( const Gaffer::Plug *input ) const override;
        void hashBranchObject( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeBranchObject( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const override;

        bool affectsBranchChildNames( const Gaffer::Plug *input ) const override;
        void hashBranchChildNames( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstInternedStringVectorDataPtr computeBranchChildNames( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const override;

    private:

        // Hashes everything the dust depends on : the plugs and the colliders
        // over the frame range that can contribute particles.
        bool affectsDust( const Gaffer::Plug *input ) const;
        void hashDust( const Gaffer::Context *context, IECore::MurmurHash &h ) const;
        IECore::ConstObjectPtr generateDust( const Gaffer::Context *context ) const;

        static size_t g_firstPlugIndex;
    };

    IE_CORE_DECLAREPTR( MiniDustHit )

} // MiniGaffer
