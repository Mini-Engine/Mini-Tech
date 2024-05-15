import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
	MiniGafferVDB.MeshToLevelSet,
	'description',
	"""Converts a mesh primitive to a level set VDB object.""",
	plugs={
		'grid' : [
			'description',
			"""
			Name of the level set grid to create in the VDB object.
			"""
		],
		'voxelSize' : [
			'description',
			"""
			Size of the voxel in the level set grid. Smaller voxel sizes will increase resolution, take more memory & longer to process. 
			"""
		],
		'exteriorBandwidth' : [
			'description',
			"""
			Defines the exterior width of the level set in voxel units. 
			"""
		],
		'interiorBandwidth' : [
			'description',
			"""
			Defines the interior width of the level set in voxel units.
			"""
		],
	}
)
