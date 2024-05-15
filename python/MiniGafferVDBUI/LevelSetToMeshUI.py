import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
	MiniGafferVDB.LevelSetToMesh,
	'description',
	"""Converts a level set VDB object to a mesh primitive .""",

	plugs={
		'grid' : [
			'description',
			"""
			Name of the level set grid to create a mesh primitive from.
			"""
		],
		'isoValue' : [
			'description',
			"""
			Value which defines the isosurface to convert to a mesh primitive. 
			Usually this is set to zero but setting a small positive number will generate a dilated mesh and negative will create an eroded mesh. 
			"""
		],
		'adaptivity' : [
			'description',
			"""
			Adaptively generate fewer polygons from level set. 0 - uniform meshing, 1 - maximum level of adaptivity.
			"""
		]
	}
)
