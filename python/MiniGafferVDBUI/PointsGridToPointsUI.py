
import GafferUI
import MiniGafferVDB

GafferUI.Metadata.registerNode(
	MiniGafferVDB.PointsGridToPoints,
	'description',
	"""Converts a points grid in a VDB object to a points primitive.""",
	plugs={
		'grid' : [
			'description',
			"""
			Name of the points grid in the VDB to create a points primitive from.
			"""
		],

		"names" : [
			"description",
			"""
			The names of the primitive variables to be extracted from VDB points grid.
			Names should be separated by spaces, and Gaffer's
			standard wildcard characters may be used.
			"""
		],

		"invertNames" : [
			"description",
			"""
			When on, the primitive variables matched by names
			are not extracted, and the non-matching primitive
			variables are extracted instead.
			"""
		],

	}
)
