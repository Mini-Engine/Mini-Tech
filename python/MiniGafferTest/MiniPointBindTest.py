import unittest
import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene

import GafferTest
import GafferSceneTest

import MiniGaffer

class MiniPointBindTest( GafferSceneTest.SceneTestCase ) :

    def testConstruct( self ) :

        node = MiniGaffer.MiniPointBind()
        self.assertEqual( node.getName(), "MiniPointBind" )

    def testCanBindHighResolutionGridToLowResolutionGrid(self):
        node = MiniGaffer.MiniPointBind()