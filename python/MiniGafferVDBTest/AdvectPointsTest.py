import unittest
import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene

import GafferTest
import GafferSceneTest

import MiniGafferVDB

class AdvectPointsTest( GafferSceneTest.SceneTestCase ) :
    def testConstruct( self ) :

        node = MiniGafferVDB.AdvectPoints()
        self.assertEqual( node.getName(), "AdvectPoints" )

