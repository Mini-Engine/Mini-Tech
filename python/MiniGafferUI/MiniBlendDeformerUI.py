import functools

import imath

import IECore

import Gaffer
import GafferUI
import MiniGaffer

Gaffer.Metadata.registerNode(

    MiniGaffer.MiniBlendDeformer,

    "description",
    """
    Blends the points of the filtered geometry towards one or more target shapes,
    like Maya's blendShape.

    Each target adds `weight * ( target - base )` to the points. The sum is scaled
    by the envelope and the optional mask. The geometry and every target must have
    the same number of points in the same order.
    """,
    "icon", "mini.png",
    plugs = {

        "envelope" : [
            "description",
            """
            Overall strength of the deformer. 0 passes the geometry through unchanged.
            """,
            "plugValueWidget:type", "GafferUI.NumericPlugValueWidget",
        ],

        "baseLocation" : [
            "description",
            """
            Location of the geometry the targets were modelled from. Target offsets are
            measured from its points. Leave empty to measure from the points being
            deformed, which is correct when the targets were modelled from this geometry.
            """,
            "plugValueWidget:type", "GafferSceneUI.ScenePathPlugValueWidget",
        ],

        "maskPrimVar" : [
            "description",
            """
            Optional name of a float vertex primitive variable (one value per point)
            which scales the deformation per point, like painted blendShape weights.
            """,
        ],

        "targets" : [
            "description",
            """
            The shapes to blend towards. Add as many as you need with the + button.
            """,
            "plugValueWidget:type", "GafferUI.LayoutPlugValueWidget",
            "layout:customWidget:footer:widgetType", "MiniGafferUI.MiniBlendDeformerUI._TargetsFooter",
            "layout:customWidget:footer:index", -1,
            "nodule:type", "",
        ],

        "targets.*" : [
            "description",
            """
            A target shape, its weight, and whether it is used.
            """,
        ],

        "targets.*.enabled" : [
            "description",
            """
            Turn the target on or off without losing its weight.
            """,
        ],

        "targets.*.location" : [
            "description",
            """
            Location of the target shape. It must have the same point count as the filtered geometry.
            """,
            "plugValueWidget:type", "GafferSceneUI.ScenePathPlugValueWidget",
        ],

        "targets.*.weight" : [
            "description",
            """
            How much of the target to blend in. Values outside 0-1 extrapolate, as in Maya.
            """,
        ],

    }

)

##########################################################################
# Add and remove targets
##########################################################################

class _TargetsFooter( GafferUI.PlugValueWidget ) :

    def __init__( self, plug, **kw ) :

        row = GafferUI.ListContainer( GafferUI.ListContainer.Orientation.Horizontal )
        GafferUI.PlugValueWidget.__init__( self, row, plug, **kw )

        with row :
            GafferUI.Spacer( imath.V2i( GafferUI.PlugWidget.labelWidth(), 1 ) )
            button = GafferUI.Button( image = "plus.png", hasFrame = False, toolTip = "Add target" )
            button.clickedSignal().connect( Gaffer.WeakMethod( self.__addTarget ), scoped = False )
            GafferUI.Spacer( imath.V2i( 1 ), maximumSize = imath.V2i( 100000, 1 ), parenting = { "expand" : True } )

    def _updateFromEditable( self ) :

        self.setEnabled( self._editable() )

    def __addTarget( self, *unused ) :

        plug = self.getPlug()
        with Gaffer.UndoScope( plug.ancestor( Gaffer.ScriptNode ) ) :
            plug.resize( len( plug ) + 1 )

def __plugPopupMenu( menuDefinition, plugValueWidget ) :

    plug = plugValueWidget.getPlug()
    target = plug if isinstance( plug.parent(), Gaffer.ArrayPlug ) else plug.parent()
    if not isinstance( target.parent(), Gaffer.ArrayPlug ) or not isinstance( target.node(), MiniGaffer.MiniBlendDeformer ) :
        return

    if len( menuDefinition.items() ) :
        menuDefinition.append( "/DeleteDivider", { "divider" : True } )

    menuDefinition.append(
        "/Delete Target",
        {
            "command" : functools.partial( __deleteTarget, target ),
            "active" : not Gaffer.MetadataAlgo.readOnly( target ),
        }
    )

def __deleteTarget( target ) :

    with Gaffer.UndoScope( target.ancestor( Gaffer.ScriptNode ) ) :
        target.parent().removeChild( target )

GafferUI.PlugValueWidget.popupMenuSignal().connect( __plugPopupMenu, scoped = False )
