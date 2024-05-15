import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
	MiniGafferVDB.OffsetLevelSet,
	'description',
	"""Erodes or dilates a level set VDB.""",
	plugs={
		'grids' : [
			'description',
			"""
			Names of the level set grids to offset in the VDB object.
			"""
		],
		'outputGrid' : [
			'description',
			"""
            Name of the output grid to create. The name of the input grid is stored in the context variable `${grid}`
            """
		],
		'offset' : [
			'description',
			"""
			Amount to offset the level set by in voxel units. A positive number will erode the surface and negative will dilate.
			"""
		],
	}
)
