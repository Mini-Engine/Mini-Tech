#pragma once

#include "MiniGafferVDB/Export.h"
#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "Gaffer/NumericPlug.h"

namespace Gaffer
{
class StringPlug;
}

namespace MiniGafferVDB
{

class MINIGAFFERVDB_API PointsGridToPoints : public GafferScene::SceneElementProcessor
{

	public :

		PointsGridToPoints( const std::string &name=defaultName<PointsGridToPoints>() );
		~PointsGridToPoints() override;

		GAFFER_GRAPHCOMPONENT_DECLARE_TYPE( MiniGafferVDB::PointsGridToPoints, TypeId::PointsGridToPointsId, GafferScene::SceneElementProcessor );

		Gaffer::StringPlug *gridPlug();
		const Gaffer::StringPlug *gridPlug() const;

		Gaffer::StringPlug *namesPlug();
		const Gaffer::StringPlug *namesPlug() const;

		Gaffer::BoolPlug *invertNamesPlug();
		const Gaffer::BoolPlug *invertNamesPlug() const;

		void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		bool processesObject() const override;
		void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const override;

	private :

		static size_t g_firstPlugIndex;

};

IE_CORE_DECLAREPTR( PointsGridToPoints )

}
