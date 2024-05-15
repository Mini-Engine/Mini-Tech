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

class MiniSubdivideMeshTest( GafferSceneTest.SceneTestCase ) :
    def testConstruct( self ) :
        node = MiniGaffer.MiniSubdivideMesh()
        self.assertEqual( node.getName(), "MiniSubdivideMesh" )

