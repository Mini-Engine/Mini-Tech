#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"

namespace MiniGafferVDB
{

class MINIGAFFERVDB_API LevelSetToMesh : public GafferScene::SceneElementProcessor
{

	public :

		LevelSetToMesh( const std::string &name=defaultName<LevelSetToMesh>() );
		~LevelSetToMesh() override;

		GAFFER_GRAPHCOMPONENT_DECLARE_TYPE( MiniGafferVDB::LevelSetToMesh, TypeId::LevelSetToMeshTypeId, GafferScene::SceneElementProcessor );

		Gaffer::StringPlug *gridPlug();
		const Gaffer::StringPlug *gridPlug() const;

		Gaffer::FloatPlug *isoValuePlug();
		const Gaffer::FloatPlug *isoValuePlug() const;

		Gaffer::FloatPlug *adaptivityPlug();
		const Gaffer::FloatPlug *adaptivityPlug() const;

		void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		bool processesObject() const override;
		void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const override;

		bool processesBound() const override;
		void hashProcessedBound( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		Imath::Box3f computeProcessedBound( const ScenePath &path, const Gaffer::Context *context, const Imath::Box3f &inputBound ) const override;

	private :

		static size_t g_firstPlugIndex;

};

IE_CORE_DECLAREPTR( LevelSetToMesh )

}