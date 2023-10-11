#include "TriangleMesh.h"

#include "glm/glm.hpp"
#include <vector>


using QuadTreeNodeIndex = std::size_t;

struct QuadTreeNode
{
    AABB aabb;
    std::vector<glm::vec3> modelPositions;
    bool visible;
    unsigned int lastVisited;
};

struct QuadTree
{
    std::vector<QuadTreeNode> nodes;

    QuadTreeNodeIndex root()
    {
        return 0;
    }

    QuadTreeNodeIndex parent (QuadTreeNodeIndex i)
    {
        return (i-1)/4;
    }

    bool hasParent (QuadTreeNodeIndex i)
    {
        return i > 0;
    }

    bool isLeaf (QuadTreeNodeIndex i)
    {
        QuadTreeNodeIndex child = 4*i + 1;
        return child >= nodes.size();
    }

};