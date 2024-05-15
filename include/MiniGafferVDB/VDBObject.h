#pragma once

#include "MiniGafferVDB/Export.h"
#include "GafferScene/ObjectSource.h"

#include "Gaffer/CompoundNumericPlug.h"

#include "MiniGafferVDB/TypeIds.h"

namespace MiniGafferVDB
{

	class MINIGAFFERVDB_API VDBObject : public GafferScene::ObjectSource
	{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MiniGafferVDB::VDBObject, TypeId::VDBObjectTypeId, ObjectSource );

		VDBObject( const std::string &name=defaultName<VDBObject>() );
		~VDBObject() override;

		Gaffer::V3iPlug *dimensionsPlug();
		const Gaffer::V3iPlug *dimensionsPlug() const;

        Gaffer::IntPlug *gridTypePlug();
        const Gaffer::IntPlug *gridTypePlug() const;

        Gaffer::StringPlug *gridNamePlug();
        const Gaffer::StringPlug *gridNamePlug() const;

        Gaffer::FloatPlug *voxelSizePlug();
        const Gaffer::FloatPlug *voxelSizePlug() const;

		void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		void hashSource( const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstObjectPtr computeSource( const Gaffer::Context *context ) const override;

	private :

		static size_t g_firstPlugIndex;

	};

	IE_CORE_DECLAREPTR( VDBObject )

}
