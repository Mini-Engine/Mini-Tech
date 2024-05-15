import IECore

import Gaffer
import MiniGaffer

Gaffer.Metadata.registerNode(

	MiniGaffer.MiniMeshCurvature,

	"description",
	"""
	Creates a curvature primitive variable on a mesh, based on the vertex normal and positions of adjacent vertices.
	""",
	"icon", "mini.png",
	plugs = {

		"curvature" : [

			"description",
			"""
			The name of the curvature primitive variable to output.
			""",
			"layout:section", "Settings.Output",

		],

		"normalize" : [

			"description",
			"""
			Normalize curvature within range of -1 to 1.
			""",

		],

		"scale" : [

			"description",
			"""
			A scaling factor applied to the curvature.
			""",

		],

		"position" : [

			"description",
			"""
			Name of the primitive variable containing position data used to calculate curvature.
			For example 'Pref' would compute curvature using the reference positions.
			""",
			"layout:section", "Settings.Input",

		],

		"normal" : [

			"description",
			"""
			Name of the primitive variable containing normal data used to calculate curvature.
			""",
			"layout:section", "Settings.Input",

		],
	},

)
