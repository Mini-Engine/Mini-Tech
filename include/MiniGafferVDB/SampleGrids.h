#pragma once

#include "MiniGafferVDB/TypeIds.h"

#include "GafferScene/SceneElementProcessor.h"

#include "GafferScene/ScenePlug.h"

#include "Gaffer/NumericPlug.h"
#include "Gaffer/StringPlug.h"

namespace MiniGafferVDB
{

class SampleGrids : public GafferScene::SceneElementProcessor
{
	public :

		SampleGrids(const std::string &name = defaultName<SampleGrids>() );
		~SampleGrids();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION(MiniGafferVDB::SampleGrids, TypeId::SampleGridsTypeId, GafferScene::SceneElementProcessor );

		GafferScene::ScenePlug *otherPlug();
		const GafferScene::ScenePlug *otherPlug() const;

		Gaffer::StringPlug *gridsPlug();
		const Gaffer::StringPlug *gridsPlug() const;

		Gaffer::StringPlug *vdbLocationPlug();
		const Gaffer::StringPlug *vdbLocationPlug() const;

		Gaffer::StringPlug *positionPlug();
		const Gaffer::StringPlug *positionPlug() const;

		Gaffer::IntPlug *interpolationPlug();
		const Gaffer::IntPlug *interpolationPlug() const;

		virtual void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		bool processesObject() const override;
		void hashProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		IECore::ConstObjectPtr computeProcessedObject( const ScenePath &path, const Gaffer::Context *context, IECore::ConstObjectPtr inputObject ) const override;

	private:

		static size_t g_firstPlugIndex;
};

IE_CORE_DECLAREPTR( SampleGrids )

} // namespace MiniGafferVDB
