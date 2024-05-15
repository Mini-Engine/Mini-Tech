import unittest
import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene

import GafferTest
import GafferSceneTest

import MiniGaffer

class MiniPointDeformerTest( GafferSceneTest.SceneTestCase ) :

    def testConstruct( self ) :

        node = MiniGaffer.MiniPointDeformer()
        self.assertEqual( node.getName(), "MiniPointDeformer" )

    # bound to s single point with transform
    def testCanDeformUsingASinglePointCage(self):
        pass

    # bound to two points
    def testCanDeformUsingTwoPoints(self):
        pass

