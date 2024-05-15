import unittest

import Gaffer
import GafferUI
import GafferScene
import GafferSceneUI

import MiniGaffer
import MiniGafferUI

import GafferUITest

class DocumentationTest( GafferUITest.TestCase ) :

	def test( self ) :

		self.maxDiff = None
		self.assertNodesAreDocumented(
			MiniGaffer,
			additionalTerminalPlugTypes = ( GafferScene.ScenePlug, )
		)

if __name__ == "__main__":
	unittest.main()
