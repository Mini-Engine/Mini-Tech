import unittest
import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene
import GafferOSL

import GafferTest
import GafferSceneTest

import MiniGaffer

class MiniMergeGeometryTest( GafferSceneTest.SceneTestCase ) :

    def testConstruct( self ) :
        node = MiniGaffer.MiniMergeGeometry()
        self.assertEqual( node.getName(), "MiniMergeGeometry" )

    def testCanMergePoints(self):
        pass

    def testCanMergeCurves(self):
        pass

    def testCanMergeMeshes(self):
        pass

    def testErrorIfDifferingGeometryTypesFound(self):
        pass

    def testPendanticMode(self):
        pass
