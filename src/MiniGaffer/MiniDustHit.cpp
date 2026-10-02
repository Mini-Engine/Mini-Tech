#define NOMINMAX

#include "Mini/MiniDustHit.h"

#include "GafferScene/SceneAlgo.h"
#include "GafferScene/ScenePlug.h"

#include "Gaffer/Context.h"

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"

#include "IECore/PathMatcher.h"
#include "IECore/StringAlgo.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <mutex>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace Gaffer;
using namespace GafferScene;
using namespace MiniGaffer;

namespace
{

    // The dust only depends on the last `lifetime` frames of the colliders, plus
    // the frames needed to apply `cooldown` and to detect when contact starts.
    struct FrameRange
    {
        int first; // First frame to look for hits.
        int last;  // Frame being evaluated, rounded down.
    };

    struct ColliderSample
    {
        vector<V3f> points;
        vector<array<int, 3>> triangles;
        vector<Box3f> triangleBounds;
        Box3f bound;
        V3f centroid = V3f( 0 );
    };

    struct Contact
    {
        bool touching = false;
        V3f position = V3f( 0 );
        V3f normal = V3f( 0, 1, 0 );
    };

    struct FrameSample
    {
        vector<ColliderSample> colliders;
        // Indexed by `i * colliders.size() + j`, only valid for `i < j`.
        vector<Contact> contacts;
    };

    struct Hit
    {
        int frame;
        int pair;
        V3f position;
        V3f normal;
        float speed;
    };

    FrameRange frameRange( float frame, int lifetime, int cooldown )
    {
        const int last = (int)floor( frame );
        return { last - max( lifetime, 0 ) - max( cooldown, 0 ) - 1, last };
    }

    vector<ScenePlug::ScenePath> findColliders( const ScenePlug *scene, const std::string &patterns )
    {
        vector<string> tokens;
        IECore::StringAlgo::tokenize<string>( patterns, ' ', back_inserter( tokens ) );

        PathMatcher matcher;
        for( const string &token : tokens )
        {
            if( !token.empty() )
            {
                matcher.addPath( token );
            }
        }

        vector<ScenePlug::ScenePath> result;
        if( matcher.isEmpty() )
        {
            return result;
        }

        const PathMatcher found = SceneAlgo::findAll(
            scene,
            [&matcher]( const ScenePlug *, const ScenePlug::ScenePath &path ) {
                return ( matcher.match( path ) & PathMatcher::ExactMatch ) != 0;
            }
        );

        for( PathMatcher::Iterator it = found.begin(); it != found.end(); ++it )
        {
            ConstPrimitivePtr primitive = runTimeCast<const Primitive>( scene->object( *it ) );
            if( primitive && primitive->variableData<V3fVectorData>( "P" ) )
            {
                result.push_back( *it );
            }
        }
        return result;
    }

    V3f closestPointOnTriangle( const V3f &p, const V3f &a, const V3f &b, const V3f &c )
    {
        const V3f ab = b - a, ac = c - a, ap = p - a;
        const float d1 = ab.dot( ap ), d2 = ac.dot( ap );
        if( d1 <= 0 && d2 <= 0 ) return a;

        const V3f bp = p - b;
        const float d3 = ab.dot( bp ), d4 = ac.dot( bp );
        if( d3 >= 0 && d4 <= d3 ) return b;

        const float vc = d1 * d4 - d3 * d2;
        if( vc <= 0 && d1 >= 0 && d3 <= 0 ) return a + ab * ( d1 / ( d1 - d3 ) );

        const V3f cp = p - c;
        const float d5 = ab.dot( cp ), d6 = ac.dot( cp );
        if( d6 >= 0 && d5 <= d6 ) return c;

        const float vb = d5 * d2 - d1 * d6;
        if( vb <= 0 && d2 >= 0 && d6 <= 0 ) return a + ac * ( d2 / ( d2 - d6 ) );

        const float va = d3 * d6 - d5 * d4;
        if( va <= 0 && ( d4 - d3 ) >= 0 && ( d5 - d6 ) >= 0 )
        {
            return b + ( c - b ) * ( ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) ) );
        }

        const float denom = 1.0f / ( va + vb + vc );
        return a + ab * ( vb * denom ) + ac * ( vc * denom );
    }

    V3f triangleNormal( const ColliderSample &c, size_t t )
    {
        const auto &tri = c.triangles[t];
        V3f n = ( c.points[tri[1]] - c.points[tri[0]] ).cross( c.points[tri[2]] - c.points[tri[0]] );
        return n.length2() > 1e-20f ? n.normalized() : V3f( 0, 1, 0 );
    }

    ColliderSample sampleCollider( const ScenePlug *scene, const ScenePlug::ScenePath &path )
    {
        ColliderSample result;

        ConstPrimitivePtr primitive = runTimeCast<const Primitive>( scene->object( path ) );
        if( !primitive )
        {
            return result;
        }
        const V3fVectorData *p = primitive->variableData<V3fVectorData>( "P" );
        if( !p )
        {
            return result;
        }

        const M44f transform = scene->fullTransform( path );
        result.points.reserve( p->readable().size() );
        V3f sum( 0 );
        for( const V3f &point : p->readable() )
        {
            const V3f world = point * transform;
            result.points.push_back( world );
            result.bound.extendBy( world );
            sum += world;
        }
        if( !result.points.empty() )
        {
            result.centroid = sum / (float)result.points.size();
        }

        if( const MeshPrimitive *mesh = runTimeCast<const MeshPrimitive>( primitive.get() ) )
        {
            const vector<int> &counts = mesh->verticesPerFace()->readable();
            const vector<int> &ids = mesh->vertexIds()->readable();
            size_t offset = 0;
            for( int count : counts )
            {
                for( int i = 1; i + 1 < count; ++i )
                {
                    const array<int, 3> tri = { ids[offset], ids[offset + i], ids[offset + i + 1] };
                    result.triangles.push_back( tri );

                    Box3f b;
                    b.extendBy( result.points[tri[0]] );
                    b.extendBy( result.points[tri[1]] );
                    b.extendBy( result.points[tri[2]] );
                    result.triangleBounds.push_back( b );
                }
                offset += count;
            }
        }
        return result;
    }

    // Finds the points of `a` that are within `distance` of the surface of `b`,
    // accumulating where they touch, and the direction from `b` towards `a`
    // (or the reverse, if `flip` is true).
    void accumulateContacts( const ColliderSample &a, const ColliderSample &b, float distance, bool flip, V3f &position, V3f &normal, int &count )
    {
        Box3f search = b.bound;
        search.min -= V3f( distance );
        search.max += V3f( distance );
        const float distance2 = distance * distance;

        for( const V3f &p : a.points )
        {
            if( !search.intersects( p ) )
            {
                continue;
            }

            float bestDistance2 = distance2;
            V3f bestPoint( 0 );
            size_t bestTriangle = 0;
            bool found = false;

            if( b.triangles.empty() )
            {
                for( const V3f &q : b.points )
                {
                    const float d2 = ( p - q ).length2();
                    if( d2 <= bestDistance2 )
                    {
                        bestDistance2 = d2; bestPoint = q; found = true;
                    }
                }
            }
            else
            {
                for( size_t t = 0; t < b.triangles.size(); ++t )
                {
                    const Box3f &tb = b.triangleBounds[t];
                    if( p.x < tb.min.x - distance || p.x > tb.max.x + distance ||
                        p.y < tb.min.y - distance || p.y > tb.max.y + distance ||
                        p.z < tb.min.z - distance || p.z > tb.max.z + distance )
                    {
                        continue;
                    }
                    const auto &tri = b.triangles[t];
                    const V3f c = closestPointOnTriangle( p, b.points[tri[0]], b.points[tri[1]], b.points[tri[2]] );
                    const float d2 = ( p - c ).length2();
                    if( d2 <= bestDistance2 )
                    {
                        bestDistance2 = d2; bestPoint = c; bestTriangle = t; found = true;
                    }
                }
            }

            if( !found )
            {
                continue;
            }

            V3f n = p - bestPoint;
            if( n.length2() > 1e-12f )
            {
                n.normalize();
            }
            else if( !b.triangles.empty() )
            {
                n = triangleNormal( b, bestTriangle );
            }
            else
            {
                n = V3f( 0, 1, 0 );
            }

            position += ( p + bestPoint ) * 0.5f;
            normal += flip ? -n : n;
            count++;
        }
    }

    Contact findContact( const ColliderSample &a, const ColliderSample &b, float distance )
    {
        Contact result;

        if( a.bound.isEmpty() || b.bound.isEmpty() )
        {
            return result;
        }
        Box3f expanded = a.bound;
        expanded.min -= V3f( distance );
        expanded.max += V3f( distance );
        if( !expanded.intersects( b.bound ) )
        {
            return result;
        }

        V3f position( 0 ), normal( 0 );
        int count = 0;
        // Normal is defined as pointing from `b` towards `a`.
        accumulateContacts( a, b, distance, false, position, normal, count );
        accumulateContacts( b, a, distance, true, position, normal, count );

        if( count == 0 )
        {
            return result;
        }

        result.touching = true;
        result.position = position / (float)count;
        if( normal.length2() > 1e-12f )
        {
            result.normal = normal.normalized();
        }
        else
        {
            const V3f d = a.centroid - b.centroid;
            result.normal = d.length2() > 1e-12f ? d.normalized() : V3f( 0, 1, 0 );
        }
        return result;
    }

    // Caches the (expensive) per frame contact detection, so that consecutive
    // frames, which share most of their frame range, don't repeat it.
    shared_ptr<const FrameSample> sampleFrame( const ScenePlug *scene, const vector<ScenePlug::ScenePath> &paths, float distance, const Context *context, int frame )
    {
        static map<MurmurHash, shared_ptr<const FrameSample>> cache;
        static mutex cacheMutex;

        Context::EditableScope scope( context );
        scope.setFrame( (float)frame );

        MurmurHash key;
        key.append( distance );
        for( const auto &path : paths )
        {
            for( const InternedString &name : path )
            {
                key.append( name.string() );
            }
            key.append( scene->objectHash( path ) );
            key.append( scene->fullTransformHash( path ) );
        }

        {
            lock_guard<mutex> guard( cacheMutex );
            auto it = cache.find( key );
            if( it != cache.end() )
            {
                return it->second;
            }
        }

        auto sample = make_shared<FrameSample>();
        for( const auto &path : paths )
        {
            sample->colliders.push_back( sampleCollider( scene, path ) );
        }

        const size_t n = paths.size();
        sample->contacts.resize( n * n );
        for( size_t i = 0; i < n; ++i )
        {
            for( size_t j = i + 1; j < n; ++j )
            {
                sample->contacts[i * n + j] = findContact( sample->colliders[i], sample->colliders[j], distance );
            }
        }

        {
            lock_guard<mutex> guard( cacheMutex );
            if( cache.size() > 1024 )
            {
                cache.clear();
            }
            cache[key] = sample;
        }
        return sample;
    }

    vector<Hit> findHits( const ScenePlug *scene, const vector<ScenePlug::ScenePath> &paths, float distance, float minSpeed, int cooldown, const FrameRange &range, const Context *context )
    {
        vector<Hit> hits;
        const size_t n = paths.size();
        if( n < 2 )
        {
            return hits;
        }

        const float fps = context->getFramesPerSecond();
        vector<int> lastHit( n * n, numeric_limits<int>::lowest() / 2 );

        shared_ptr<const FrameSample> previous = sampleFrame( scene, paths, distance, context, range.first - 1 );
        for( int frame = range.first; frame <= range.last; ++frame )
        {
            shared_ptr<const FrameSample> current = sampleFrame( scene, paths, distance, context, frame );
            for( size_t i = 0; i < n; ++i )
            {
                for( size_t j = i + 1; j < n; ++j )
                {
                    const size_t pair = i * n + j;
                    const Contact &now = current->contacts[pair];
                    if( !now.touching || previous->contacts[pair].touching )
                    {
                        continue;
                    }

                    const V3f velocityI = ( current->colliders[i].centroid - previous->colliders[i].centroid ) * fps;
                    const V3f velocityJ = ( current->colliders[j].centroid - previous->colliders[j].centroid ) * fps;
                    const float speed = ( velocityI - velocityJ ).length();

                    if( speed < minSpeed || frame - lastHit[pair] < cooldown )
                    {
                        continue;
                    }
                    lastHit[pair] = frame;
                    hits.push_back( { frame, (int)pair, now.position, now.normal, speed } );
                }
            }
            previous = current;
        }
        return hits;
    }

    inline uint64_t mix( uint64_t x )
    {
        x += 0x9e3779b97f4a7c15ULL;
        x = ( x ^ ( x >> 30 ) ) * 0xbf58476d1ce4e5b9ULL;
        x = ( x ^ ( x >> 27 ) ) * 0x94d049bb133111ebULL;
        return x ^ ( x >> 31 );
    }

    struct Random
    {
        explicit Random( uint64_t seed ) : state( mix( seed ) ) {}

        float next()
        {
            state = mix( state );
            return ( state >> 40 ) * ( 1.0f / 16777216.0f );
        }

        V3f unitVector()
        {
            const float z = 2.0f * next() - 1.0f;
            const float phi = 2.0f * (float)M_PI * next();
            const float r = sqrt( max( 0.0f, 1.0f - z * z ) );
            return V3f( r * cos( phi ), z, r * sin( phi ) );
        }

        uint64_t state;
    };

}

GAFFER_NODE_DEFINE_TYPE( MiniDustHit );

size_t MiniDustHit::g_firstPlugIndex = 0;

MiniDustHit::MiniDustHit( const std::string &name )
    : BranchCreator( name )
{
    storeIndexOfNextChild( g_firstPlugIndex );

    addChild( new StringPlug( "name", Plug::In, "dust" ) );
    addChild( new StringPlug( "colliders", Plug::In, "" ) );
    addChild( new FloatPlug( "contactDistance", Plug::In, 0.01f, 0.0f ) );
    addChild( new FloatPlug( "minSpeed", Plug::In, 0.0f, 0.0f ) );
    addChild( new IntPlug( "cooldown", Plug::In, 5, 0 ) );
    addChild( new IntPlug( "countPerHit", Plug::In, 200, 0 ) );
    addChild( new FloatPlug( "countBySpeed", Plug::In, 0.0f, 0.0f ) );
    addChild( new IntPlug( "seed", Plug::In, 0 ) );
    addChild( new IntPlug( "lifetime", Plug::In, 24, 1 ) );
    addChild( new FloatPlug( "speed", Plug::In, 1.0f, 0.0f ) );
    addChild( new FloatPlug( "spread", Plug::In, 0.5f, 0.0f ) );
    addChild( new FloatPlug( "normalBias", Plug::In, 0.5f, 0.0f ) );
    addChild( new FloatPlug( "drag", Plug::In, 1.5f, 0.0f ) );
    addChild( new V3fPlug( "gravity", Plug::In, V3f( 0, -0.5f, 0 ) ) );
    addChild( new FloatPlug( "startWidth", Plug::In, 0.05f, 0.0f ) );
    addChild( new FloatPlug( "endWidth", Plug::In, 0.3f, 0.0f ) );
    addChild( new Color3fPlug( "color", Plug::In, Color3f( 0.6f, 0.55f, 0.45f ) ) );
    addChild( new FloatPlug( "fadeOut", Plug::In, 1.0f, 0.0f ) );
}

#define MINI_DUST_PLUG( Type, accessor, index ) \
    Gaffer::Type *MiniDustHit::accessor() { return getChild<Type>( g_firstPlugIndex + index ); } \
    const Gaffer::Type *MiniDustHit::accessor() const { return getChild<Type>( g_firstPlugIndex + index ); }

MINI_DUST_PLUG( StringPlug, namePlug, 0 )
MINI_DUST_PLUG( StringPlug, collidersPlug, 1 )
MINI_DUST_PLUG( FloatPlug, contactDistancePlug, 2 )
MINI_DUST_PLUG( FloatPlug, minSpeedPlug, 3 )
MINI_DUST_PLUG( IntPlug, cooldownPlug, 4 )
MINI_DUST_PLUG( IntPlug, countPerHitPlug, 5 )
MINI_DUST_PLUG( FloatPlug, countBySpeedPlug, 6 )
MINI_DUST_PLUG( IntPlug, seedPlug, 7 )
MINI_DUST_PLUG( IntPlug, lifetimePlug, 8 )
MINI_DUST_PLUG( FloatPlug, speedPlug, 9 )
MINI_DUST_PLUG( FloatPlug, spreadPlug, 10 )
MINI_DUST_PLUG( FloatPlug, normalBiasPlug, 11 )
MINI_DUST_PLUG( FloatPlug, dragPlug, 12 )
MINI_DUST_PLUG( V3fPlug, gravityPlug, 13 )
MINI_DUST_PLUG( FloatPlug, startWidthPlug, 14 )
MINI_DUST_PLUG( FloatPlug, endWidthPlug, 15 )
MINI_DUST_PLUG( Color3fPlug, colorPlug, 16 )
MINI_DUST_PLUG( FloatPlug, fadeOutPlug, 17 )

#undef MINI_DUST_PLUG

bool MiniDustHit::affectsDust( const Gaffer::Plug *input ) const
{
    // Everything from `colliders` onwards. `name` only affects the child names.
    for( size_t i = 1; i < 18; ++i )
    {
        const Plug *plug = getChild<Plug>( g_firstPlugIndex + i );
        if( input == plug || plug->isAncestorOf( input ) )
        {
            return true;
        }
    }
    return input->parent() == inPlug();
}

bool MiniDustHit::affectsBranchBound( const Gaffer::Plug *input ) const
{
    return affectsDust( input );
}

void MiniDustHit::hashBranchBound( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    if( branchPath.size() > 1 )
    {
        h = MurmurHash();
        h.append( Box3f() );
        return;
    }
    BranchCreator::hashBranchBound( sourcePath, branchPath, context, h );
    hashDust( context, h );
}

Imath::Box3f MiniDustHit::computeBranchBound( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const
{
    if( branchPath.size() > 1 )
    {
        return Box3f();
    }

    ConstPointsPrimitivePtr dust = runTimeCast<const PointsPrimitive>( generateDust( context ) );
    if( !dust || dust->getNumPoints() == 0 )
    {
        return Box3f();
    }

    Box3f result = dust->bound();
    const float pad = 0.5f * max( startWidthPlug()->getValue(), endWidthPlug()->getValue() );
    result.min -= V3f( pad );
    result.max += V3f( pad );
    return result;
}

bool MiniDustHit::affectsBranchTransform( const Gaffer::Plug *input ) const
{
    return false;
}

void MiniDustHit::hashBranchTransform( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    h = MurmurHash();
    h.append( M44f() );
}

Imath::M44f MiniDustHit::computeBranchTransform( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const
{
    return M44f();
}

bool MiniDustHit::affectsBranchAttributes( const Gaffer::Plug *input ) const
{
    return false;
}

void MiniDustHit::hashBranchAttributes( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    h = outPlug()->attributesPlug()->defaultValue()->Object::hash();
}

IECore::ConstCompoundObjectPtr MiniDustHit::computeBranchAttributes( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const
{
    return outPlug()->attributesPlug()->defaultValue();
}

bool MiniDustHit::affectsBranchObject( const Gaffer::Plug *input ) const
{
    return affectsDust( input );
}

void MiniDustHit::hashBranchObject( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    if( branchPath.size() != 1 )
    {
        h = outPlug()->objectPlug()->defaultValue()->Object::hash();
        return;
    }
    BranchCreator::hashBranchObject( sourcePath, branchPath, context, h );
    hashDust( context, h );
}

IECore::ConstObjectPtr MiniDustHit::computeBranchObject( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const
{
    if( branchPath.size() != 1 )
    {
        return outPlug()->objectPlug()->defaultValue();
    }
    return generateDust( context );
}

bool MiniDustHit::affectsBranchChildNames( const Gaffer::Plug *input ) const
{
    return input == namePlug();
}

void MiniDustHit::hashBranchChildNames( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    if( branchPath.size() == 0 )
    {
        BranchCreator::hashBranchChildNames( sourcePath, branchPath, context, h );
        namePlug()->hash( h );
    }
    else
    {
        h = outPlug()->childNamesPlug()->defaultValue()->Object::hash();
    }
}

IECore::ConstInternedStringVectorDataPtr MiniDustHit::computeBranchChildNames( const ScenePath &sourcePath, const ScenePath &branchPath, const Gaffer::Context *context ) const
{
    if( branchPath.size() == 0 )
    {
        InternedStringVectorDataPtr result = new InternedStringVectorData;
        result->writable().push_back( namePlug()->getValue() );
        return result;
    }
    return outPlug()->childNamesPlug()->defaultValue();
}

void MiniDustHit::hashDust( const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
    h.append( context->getFrame() );
    h.append( context->getFramesPerSecond() );

    for( size_t i = 1; i < 18; ++i )
    {
        getChild<ValuePlug>( g_firstPlugIndex + i )->hash( h );
    }

    const vector<ScenePlug::ScenePath> paths = findColliders( inPlug(), collidersPlug()->getValue() );
    const FrameRange range = frameRange( context->getFrame(), lifetimePlug()->getValue(), cooldownPlug()->getValue() );

    for( int frame = range.first - 1; frame <= range.last; ++frame )
    {
        Context::EditableScope scope( context );
        scope.setFrame( (float)frame );
        for( const auto &path : paths )
        {
            h.append( inPlug()->objectHash( path ) );
            h.append( inPlug()->fullTransformHash( path ) );
        }
    }
    for( const auto &path : paths )
    {
        for( const InternedString &name : path )
        {
            h.append( name.string() );
        }
    }
}

IECore::ConstObjectPtr MiniDustHit::generateDust( const Gaffer::Context *context ) const
{
    const float frame = context->getFrame();
    const float fps = context->getFramesPerSecond();
    const int lifetime = lifetimePlug()->getValue();
    const float lifeSeconds = lifetime / fps;
    const int cooldown = cooldownPlug()->getValue();

    const vector<ScenePlug::ScenePath> paths = findColliders( inPlug(), collidersPlug()->getValue() );
    const FrameRange range = frameRange( frame, lifetime, cooldown );
    const vector<Hit> hits = findHits(
        inPlug(), paths, max( contactDistancePlug()->getValue(), 1e-5f ), minSpeedPlug()->getValue(), cooldown, range, context
    );

    const int countPerHit = countPerHitPlug()->getValue();
    const float countBySpeed = countBySpeedPlug()->getValue();
    const int seed = seedPlug()->getValue();
    const float speed = speedPlug()->getValue();
    const float spread = spreadPlug()->getValue();
    const float normalBias = normalBiasPlug()->getValue();
    const float drag = dragPlug()->getValue();
    const V3f gravity = gravityPlug()->getValue();
    const float startWidth = startWidthPlug()->getValue();
    const float endWidth = endWidthPlug()->getValue();
    const Color3f color = colorPlug()->getValue();
    const float fadeOut = fadeOutPlug()->getValue();
    const float jitter = 0.5f * contactDistancePlug()->getValue();

    V3fVectorDataPtr pData = new V3fVectorData;
    V3fVectorDataPtr vData = new V3fVectorData;
    FloatVectorDataPtr widthData = new FloatVectorData;
    FloatVectorDataPtr opacityData = new FloatVectorData;
    FloatVectorDataPtr ageData = new FloatVectorData;
    Color3fVectorDataPtr colorData = new Color3fVectorData;
    IntVectorDataPtr idData = new IntVectorData;

    auto &P = pData->writable();
    auto &V = vData->writable();
    auto &W = widthData->writable();
    auto &O = opacityData->writable();
    auto &A = ageData->writable();
    auto &C = colorData->writable();
    auto &I = idData->writable();

    for( const Hit &hit : hits )
    {
        const float age = ( frame - hit.frame ) / fps;
        if( age < 0.0f || age > lifeSeconds )
        {
            continue;
        }

        const float u = lifeSeconds > 0.0f ? age / lifeSeconds : 1.0f;
        const int count = (int)round( countPerHit + countBySpeed * hit.speed );
        for( int i = 0; i < count; ++i )
        {
            Random random( mix( (uint64_t)(uint32_t)seed ^ mix( (uint64_t)hit.pair * 7919u + (uint64_t)(int64_t)hit.frame * 104729u ) ) + i );

            const V3f start = hit.position + random.unitVector() * ( jitter * random.next() );
            V3f direction = hit.normal * normalBias + random.unitVector() * spread;
            direction = direction.length2() > 1e-12f ? direction.normalized() : hit.normal;
            const V3f v0 = direction * ( speed * ( 0.5f + random.next() ) );

            V3f position, velocity;
            if( drag > 1e-4f )
            {
                const V3f terminal = gravity / drag;
                const float decay = exp( -drag * age );
                velocity = ( v0 - terminal ) * decay + terminal;
                position = start + terminal * age + ( v0 - terminal ) * ( ( 1.0f - decay ) / drag );
            }
            else
            {
                velocity = v0 + gravity * age;
                position = start + v0 * age + gravity * ( 0.5f * age * age );
            }

            P.push_back( position );
            V.push_back( velocity );
            W.push_back( startWidth + ( endWidth - startWidth ) * u );
            O.push_back( pow( max( 0.0f, 1.0f - u ), fadeOut ) );
            A.push_back( age );
            C.push_back( color );
            I.push_back( (int)I.size() );
        }
    }

    PointsPrimitivePtr result = new PointsPrimitive( pData );
    result->variables["v"] = PrimitiveVariable( PrimitiveVariable::Vertex, vData );
    result->variables["width"] = PrimitiveVariable( PrimitiveVariable::Vertex, widthData );
    result->variables["opacity"] = PrimitiveVariable( PrimitiveVariable::Vertex, opacityData );
    result->variables["age"] = PrimitiveVariable( PrimitiveVariable::Vertex, ageData );
    result->variables["Cd"] = PrimitiveVariable( PrimitiveVariable::Vertex, colorData );
    result->variables["id"] = PrimitiveVariable( PrimitiveVariable::Vertex, idData );
    return result;
}
