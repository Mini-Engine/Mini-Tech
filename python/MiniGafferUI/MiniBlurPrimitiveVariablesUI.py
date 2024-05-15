import IECore

import Gaffer
import MiniGaffer


Gaffer.Metadata.registerNode(

	MiniGaffer.MiniBlurPrimitiveVariables,

	"description",
	"""
	Blurs vertex V3f, Color3f, V2f, Float and Int primitive variables.
	""",

	"layout:activator:weightingIsEdgeLength", lambda parent : parent["weighting"].getValue() == MiniGaffer.MiniBlurPrimitiveVariables.Weighting.EdgeLength,
	"icon", "mini.png",
	plugs = {

		"weighting" : [

			"description",
			"""
			The weighting of the blur.

			- Uniform
			- Weighted by Edge Length

			""",

			"preset:Uniform", 0,
			"preset:Edge Length", 1,

			"plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",
		],

		"primitiveVariables" : [

			"description",
			"""
			The names of the primitive variables to be blurred. These
			should be separated by spaces and can use Gaffer's standard
			wildcards to match multiple primitive variables.
			""",

		],

		"iterations" : [

			"description",
			"""
			The number of blurring iterations.
			""",

		],

		"iterationScale" : [

			"description",
			"""
			The amount of blurring to apply at each iteration.
			""",

		],

		"position" : [

			"description",
			"""
			Name of the primitive variable containing position data used to calculate edge lengths.
			For example 'Pref' would compute edge lengths using the reference positions.
			""",

			"layout:activator", "weightingIsEdgeLength",

		],

		"weight" : [

			"description",
			"""
			The name of a float primitive variable containing weight data used as a multiplier.
			""",

		],

	},

)
