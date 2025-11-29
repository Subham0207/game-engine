#include <vector>
#include <glm/glm.hpp>

namespace AI
{
    // Represents a single polygon in the Nav Mesh
    class NavMeshPolygon {
        public:
            std::vector<glm::vec3> vertices;
            std::vector<int> neighborEdges; // Index of shared edges with neighbors
            // ... possibly area cost/type data
        };

        // Represents a shared edge (portal) between two polygons
        class NavMeshEdge {
        public:
            glm::vec3 p1, p2; // The endpoints of the edge
            int polyA, polyB; // Indices of the two adjacent polygons
        };

        // Main class for holding the Nav Mesh data and providing pathfinding
        class NavMesh {
        public:
            std::vector<NavMeshPolygon> polygons;
            std::vector<NavMeshEdge> edges;

            // Public API for pathfinding
            std::vector<glm::vec3> FindPath(const glm::vec3& start, const glm::vec3& end);

        private:
            // Helper methods for the pathfinding process
            NavMeshPolygon* FindPolygonContainingPoint(const glm::vec3& point);
            std::vector<int> AStarSearch(int startPolyIndex, int endPolyIndex);
            std::vector<glm::vec3> FunnelAlgorithm(const std::vector<int>& polyPath, const glm::vec3& start, const glm::vec3& end);
    };
}