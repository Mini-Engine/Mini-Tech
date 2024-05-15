#pragma once

#include "MiniGafferVDB/Export.h"
#include "GafferScene/ObjectSource.h"

#include "Gaffer/CompoundNumericPlug.h"

#include "MiniGafferVDB/TypeIds.h"

namespace MiniGafferVDB
{

    class MINIGAFFERVDB_API PlatonicLevelSet : public GafferScene::ObjectSource
    {

    public :

        IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGafferVDB::PlatonicLevelSet, TypeId::PlatonicLevelSetTypeId, ObjectSource );

        PlatonicLevelSet( const std::string &name=defaultName<PlatonicLevelSet>() );
        ~PlatonicLevelSet() override;

        Gaffer::StringPlug *gridPlug();
        const Gaffer::StringPlug *gridPlug() const;

        Gaffer::IntPlug *facesPlug();
        const Gaffer::IntPlug *facesPlug() const;

        Gaffer::FloatPlug *scalePlug();
        const Gaffer::FloatPlug *scalePlug() const;

        Gaffer::V3fPlug *centerPlug();
        const Gaffer::V3fPlug *centerPlug() const;

        Gaffer::FloatPlug *voxelSizePlug();
        const Gaffer::FloatPlug *voxelSizePlug() const;

        Gaffer::FloatPlug *halfWidthPlug();
        const Gaffer::FloatPlug *halfWidthPlug() const;

        void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

    protected :

        void hashSource( const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
        IECore::ConstObjectPtr computeSource( const Gaffer::Context *context ) const override;

    private :

        static size_t g_firstPlugIndex;

    };

    IE_CORE_DECLAREPTR( PlatonicLevelSet )

}
