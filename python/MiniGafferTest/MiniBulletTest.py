import unittest
import imath

import IECore
import IECoreScene

import Gaffer
import GafferScene

import GafferTest
import GafferSceneTest

import MiniGaffer

class MiniBulletTest( GafferSceneTest.SceneTestCase ) :

    def testConstruct( self ) :

        node = MiniGaffer.MiniBullet()
        self.assertEqual( node.getName(), "MiniBullet" )