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
    rcConfig rcCfg{};
    rcCfg.cs = cfg.cellSize;
    rcCfg.ch = cfg.cellHeight;
    rcCfg.walkableSlopeAngle = cfg.agentMaxSlope;
    rcCfg.walkableHeight = (int)std::ceil(cfg.agentHeight / rcCfg.ch);
    rcCfg.walkableClimb  = (int)std::floor(cfg.agentMaxClimb / rcCfg.ch);
    rcCfg.walkableRadius = (int)std::ceil(cfg.agentRadius / rcCfg.cs);
    rcCfg.maxEdgeLen     = (int)(cfg.edgeMaxLen / cfg.cellSize);
    rcCfg.maxSimplificationError = cfg.edgeMaxError;
    rcCfg.minRegionArea  = (int)rcSqr(cfg.regionMinSize);
    rcCfg.mergeRegionArea = (int)rcSqr(cfg.regionMergeSize);
    rcCfg.maxVertsPerPoly = (int)cfg.vertsPerPoly;
    rcCfg.detailSampleDist = cfg.detailSampleDist < 0.9f ? 0.0f * rcCfg.cs : cfg.detailSampleDist * rcCfg.cs;
    rcCfg.detailSampleMaxError = cfg.detailSampleMaxError * rcCfg.ch;

    rcVcopy(rcCfg.bmin, bmin);
    rcVcopy(rcCfg.bmax, bmax);

    rcCalcGridSize(rcCfg.bmin, rcCfg.bmax, rcCfg.cs, &rcCfg.width, &rcCfg.height);

    //
    // 3. Allocate Recast structures
    //
    rcHeightfield* solid = rcAllocHeightfield();
    if (!solid) return false;

    if (!rcCreateHeightfield(m_ctx, *solid, rcCfg.width, rcCfg.height, rcCfg.bmin, rcCfg.bmax, rcCfg.cs, rcCfg.ch))
    {
        rcFreeHeightField(solid);
        return false;
    }

    // Walkable triangle flags
    std::vector<unsigned char> triAreas(triCount, 0);

    rcMarkWalkableTriangles(m_ctx, rcCfg.walkableSlopeAngle,
                            verts, vertCount, tris, triCount, triAreas.data());

    rcRasterizeTriangles(m_ctx,
                         verts, vertCount,
                         tris, triAreas.data(), triCount,
                         *solid, rcCfg.walkableClimb);

    // Filter
    rcFilterLowHangingWalkableObstacles(m_ctx, rcCfg.walkableClimb, *solid);
    rcFilterLedgeSpans(m_ctx, rcCfg.walkableHeight, rcCfg.walkableClimb, *solid);
    rcFilterWalkableLowHeightSpans(m_ctx, rcCfg.walkableHeight, *solid);

    //
    // 4. Compact heightfield
    //
    rcCompactHeightfield* chf = rcAllocCompactHeightfield();
    if (!chf) { rcFreeHeightField(solid); return false; }

    if (!rcBuildCompactHeightfield(m_ctx, rcCfg.walkableHeight, rcCfg.walkableClimb, *solid, *chf))
    {
        rcFreeCompactHeightfield(chf);
        rcFreeHeightField(solid);
        return false;
    }

    rcFreeHeightField(solid);

    // Erode walkable area by agent radius
    if (!rcErodeWalkableArea(m_ctx, rcCfg.walkableRadius, *chf))
    {
        rcFreeCompactHeightfield(chf);
        return false;
    }

    //
    // 5. Partition / regions
    //
    if (!rcBuildDistanceField(m_ctx, *chf) ||
        !rcBuildRegions(m_ctx, *chf, 0, rcCfg.minRegionArea, rcCfg.mergeRegionArea))
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

    if (!rcBuildContours(m_ctx, *chf, rcCfg.maxSimplificationError, rcCfg.maxEdgeLen, *cset))
    {
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    //
    // 7. Poly mesh
    //
    rcPolyMesh* pmesh = rcAllocPolyMesh();
    if (!pmesh)
    {
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    if (!rcBuildPolyMesh(m_ctx, *cset, rcCfg.maxVertsPerPoly, *pmesh))
    {
        rcFreePolyMesh(pmesh);
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
        rcFreePolyMesh(pmesh);
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    if (!rcBuildPolyMeshDetail(m_ctx, *pmesh, *chf,
                               rcCfg.detailSampleDist, rcCfg.detailSampleMaxError, *dmesh))
    {
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(pmesh);
        rcFreeContourSet(cset);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    rcFreeCompactHeightfield(chf);
    rcFreeContourSet(cset);

    // Mark walkable flags on polys
    unsigned short walkableFlag = 0x01;
    for (int i = 0; i < pmesh->npolys; ++i)
    {
        pmesh->flags[i] = walkableFlag;
    }

    //
    // 9. Detour data
    //
    unsigned char* navData = nullptr;
    int navDataSize = 0;

    dtNavMeshCreateParams params{};
    params.verts = pmesh->verts;
    params.vertCount = pmesh->nverts;
    params.polys = pmesh->polys;
    params.polyAreas = pmesh->areas;
    params.polyFlags = pmesh->flags;
    params.polyCount = pmesh->npolys;
    params.nvp = pmesh->nvp;

    params.detailMeshes = dmesh->meshes;
    params.detailVerts = dmesh->verts;
    params.detailVertsCount = dmesh->nverts;
    params.detailTris = dmesh->tris;
    params.detailTriCount = dmesh->ntris;

    params.walkableHeight = cfg.agentHeight;
    params.walkableRadius = cfg.agentRadius;
    params.walkableClimb = cfg.agentMaxClimb;

    rcVcopy(params.bmin, pmesh->bmin);
    rcVcopy(params.bmax, pmesh->bmax);
    params.cs = rcCfg.cs;
    params.ch = rcCfg.ch;
    params.buildBvTree = true;

    if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
    {
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(pmesh);
        return false;
    }

    rcFreePolyMeshDetail(dmesh);
    rcFreePolyMesh(pmesh);

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
