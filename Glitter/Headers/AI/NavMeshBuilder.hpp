// NavMeshBuilder.h
#pragma once
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>

struct NavMeshConfig {
    float cellSize      = 0.3f;   // XZ resolution
    float cellHeight    = 0.2f;   // Y resolution
    float agentHeight   = 2.0f;
    float agentRadius   = 0.6f;
    float agentMaxClimb = 0.9f;
    float agentMaxSlope = 45.0f;

    float regionMinSize = 8.0f;
    float regionMergeSize = 20.0f;
    float edgeMaxLen    = 12.0f;
    float edgeMaxError  = 1.3f;
    float vertsPerPoly  = 6.0f;
    float detailSampleDist = 6.0f;
    float detailSampleMaxError = 1.0f;
};

class NavMeshBuilder {
public:
    NavMeshBuilder();
    ~NavMeshBuilder();

    bool build(
        const float* verts, int vertCount,
        const int* tris, int triCount,
        const NavMeshConfig& cfg
    );

    dtNavMesh*       getNavMesh()       { return m_navMesh; }
    dtNavMeshQuery*  getNavMeshQuery()  { return m_navQuery; }

private:
    rcContext*       m_ctx = nullptr;
    dtNavMesh*       m_navMesh = nullptr;
    dtNavMeshQuery*  m_navQuery = nullptr;
};
