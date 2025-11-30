    // NavMeshBuilder.cpp
#include <AI/NavMeshBuilder.hpp>
#include <cstring> // memset, memcpy
#include <vector>
#include <cmath>   // std::ceil, std::floor
#include <cfloat> 
#include <Recast.h>

NavMeshBuilder::NavMeshBuilder()
{
    m_ctx = new rcContext();
}

NavMeshBuilder::~NavMeshBuilder()
{
    if (m_navQuery)
    {
        dtFreeNavMeshQuery(m_navQuery);
        m_navQuery = nullptr;
    }

    if (m_navMesh)
    {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = nullptr;
    }

    delete m_ctx;
    m_ctx = nullptr;
}

bool NavMeshBuilder::build(
    const float* verts, int vertCount,
    const int* tris, int triCount,
    const NavMeshConfig& cfg)
{
    // Cleanup previous
    if (m_navQuery) {
        dtFreeNavMeshQuery(m_navQuery);
        m_navQuery = nullptr;
    }
    if (m_navMesh) {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = nullptr;
    }
    //
    // 1. Compute bounding box of geometry
    //
    float bmin[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
    float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (int i = 0; i < vertCount; ++i)
    {
        const float* v = &verts[i*3];
        bmin[0] = std::min(bmin[0], v[0]);
        bmin[1] = std::min(bmin[1], v[1]);
        bmin[2] = std::min(bmin[2], v[2]);
        bmax[0] = std::max(bmax[0], v[0]);
        bmax[1] = std::max(bmax[1], v[1]);
        bmax[2] = std::max(bmax[2], v[2]);
    }

    //
    // 2. Recast config
    //
    m_cfg.cs = cfg.cellSize;
    m_cfg.ch = cfg.cellHeight;
    m_cfg.walkableSlopeAngle = cfg.agentMaxSlope;
    m_cfg.walkableHeight = (int)std::ceil(cfg.agentHeight / m_cfg.ch);
    m_cfg.walkableClimb  = (int)std::floor(cfg.agentMaxClimb / m_cfg.ch);
    m_cfg.walkableRadius = (int)std::ceil(cfg.agentRadius / m_cfg.cs);
    m_cfg.maxEdgeLen     = (int)(cfg.edgeMaxLen / cfg.cellSize);
    m_cfg.maxSimplificationError = cfg.edgeMaxError;
    m_cfg.minRegionArea  = (int)rcSqr(cfg.regionMinSize);
    m_cfg.mergeRegionArea = (int)rcSqr(cfg.regionMergeSize);
    m_cfg.maxVertsPerPoly = (int)cfg.vertsPerPoly;
    m_cfg.detailSampleDist = cfg.detailSampleDist < 0.9f ? 0.0f * m_cfg.cs : cfg.detailSampleDist * m_cfg.cs;
    m_cfg.detailSampleMaxError = cfg.detailSampleMaxError * m_cfg.ch;

    rcVcopy(m_cfg.bmin, bmin);
    rcVcopy(m_cfg.bmax, bmax);

    rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

    //
    // 3. Allocate Recast structures
    //
    rcHeightfield* solid = rcAllocHeightfield();
    if (!solid) return false;

    if (!rcCreateHeightfield(m_ctx, *solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
    {
        rcFreeHeightField(solid);
        return false;
    }

    // Walkable triangle flags
    std::vector<unsigned char> triAreas(triCount, 0);

    rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle,
                            verts, vertCount, tris, triCount, triAreas.data());

    rcRasterizeTriangles(m_ctx,
                         verts, vertCount,
                         tris, triAreas.data(), triCount,
                         *solid, m_cfg.walkableClimb);

    // Filter
    rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *solid);
    rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *solid);
    rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *solid);

    //
    // 4. Compact heightfield
    //
    rcCompactHeightfield* chf = rcAllocCompactHeightfield();
    if (!chf) { rcFreeHeightField(solid); return false; }

    if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *solid, *chf))
    {
        rcFreeCompactHeightfield(chf);
        rcFreeHeightField(solid);
        return false;
    }

    rcFreeHeightField(solid);

    // Erode walkable area by agent radius
    if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *chf))
    {
        rcFreeCompactHeightfield(chf);
        return false;
    }

    //
    // 5. Partition / regions
    //
    if (!rcBuildDistanceField(m_ctx, *chf) ||
        !rcBuildRegions(m_ctx, *chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
    {
        rcFreeCompactHeightfield(chf);
        return false;
    }

    //
    // 6. Contours
    //
    rcContourSet* cset = rcAllocContourSet();
    if (!cset)
    {
        rcFreeCompactHeightfield(chf);
        return false;
    }

    if (!rcBuildContours(m_ctx, *chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *cset))
    {
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    //
    // 7. Poly mesh
    //
    m_pmesh = rcAllocPolyMesh();
    if (!m_pmesh)
    {
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    if (!rcBuildPolyMesh(m_ctx, *cset, m_cfg.maxVertsPerPoly, *m_pmesh))
    {
        rcFreePolyMesh(m_pmesh);
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    //
    // 8. Detail mesh
    //
    rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
    if (!dmesh)
    {
        rcFreePolyMesh(m_pmesh);
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *chf,
                               m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *dmesh))
    {
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(m_pmesh);
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    rcFreeCompactHeightfield(chf);
    rcFreeContourSet(cset);

    // Mark walkable flags on polys
    unsigned short walkableFlag = 0x01;
    for (int i = 0; i < m_pmesh->npolys; ++i)
    {
        m_pmesh->flags[i] = walkableFlag;
    }

    //
    // 9. Detour data
    //
    unsigned char* navData = nullptr;
    int navDataSize = 0;

    dtNavMeshCreateParams params{};
    params.verts = m_pmesh->verts;
    params.vertCount = m_pmesh->nverts;
    params.polys = m_pmesh->polys;
    params.polyAreas = m_pmesh->areas;
    params.polyFlags = m_pmesh->flags;
    params.polyCount = m_pmesh->npolys;
    params.nvp = m_pmesh->nvp;

    params.detailMeshes = dmesh->meshes;
    params.detailVerts = dmesh->verts;
    params.detailVertsCount = dmesh->nverts;
    params.detailTris = dmesh->tris;
    params.detailTriCount = dmesh->ntris;

    params.walkableHeight = cfg.agentHeight;
    params.walkableRadius = cfg.agentRadius;
    params.walkableClimb = cfg.agentMaxClimb;

    rcVcopy(params.bmin, m_pmesh->bmin);
    rcVcopy(params.bmax, m_pmesh->bmax);
    params.cs = m_cfg.cs;
    params.ch = m_cfg.ch;
    params.buildBvTree = true;

    if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
    {
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(m_pmesh);
        return false;
    }

    rcFreePolyMeshDetail(dmesh);
    // rcFreePolyMesh(m_pmesh);

    //
    // 10. Create Detour navmesh + query
    //
    m_navMesh = dtAllocNavMesh();
    if (!m_navMesh) {
        dtFree(navData);
        return false;
    }

    dtStatus status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
    if (dtStatusFailed(status))
    {
        dtFree(navData);
        return false;
    }

    m_navQuery = dtAllocNavMeshQuery();
    status = m_navQuery->init(m_navMesh, 2048); // max nodes in search
    if (dtStatusFailed(status))
    {
        return false;
    }

    return true;
}

void NavMeshBuilder::getDebugNavmesh(std::vector<float>& outVerts,
                                     std::vector<unsigned>& outTris) const
{
    outVerts.clear();
    outTris.clear();

    if (!m_pmesh) return;

    const rcPolyMesh* pmesh = m_pmesh;

    const float cs = m_cfg.cs; // cell size (x,z)
    const float ch = m_cfg.ch; // cell height (y)

    // 1) Convert compact verts (ushort) to world-space float verts
    // pmesh->verts: [x0,y0,z0, x1,y1,z1, ...] in grid coords
    for (int i = 0; i < pmesh->nverts; ++i)
    {
        const unsigned short* v = &pmesh->verts[i * 3];

        float x = pmesh->bmin[0] + v[0] * cs;
        float y = pmesh->bmin[1] + v[1] * ch;
        float z = pmesh->bmin[2] + v[2] * cs;

        outVerts.push_back(x);
        outVerts.push_back(y);
        outVerts.push_back(z);
    }

    // 2) Build triangle list from polygons
    const int nvp = pmesh->nvp; // max verts per poly

    for (int i = 0; i < pmesh->npolys; ++i)
    {
        const unsigned short* p = &pmesh->polys[i * 2 * nvp];

        // Count valid verts in this poly (terminated by RC_MESH_NULL_IDX)
        int nv = 0;
        for (; nv < nvp; ++nv)
        {
            if (p[nv] == RC_MESH_NULL_IDX) break;
        }
        if (nv < 3) continue; // not a valid polygon

        // Fan triangulation: (0, j-1, j)
        for (int j = 2; j < nv; ++j)
        {
            outTris.push_back(p[0]);
            outTris.push_back(p[j - 1]);
            outTris.push_back(p[j]);
        }
    }
}