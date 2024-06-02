#ifndef QUADTREE_H
#define QUADTREE_H

// This is not really working
// Its a quadtree but with all the data at the top level
// Its basically 3 lookup tables
// Inspired by https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
#include <vector>


// Element Node
struct ElementNode
{
    int next;         // Index of the next element in the leaf node (-1 for end)
    int elementIndex; // Index of the element in the elements vector
    bool operator==(const ElementNode& o) const { return this == &o; }
};

// Element
template <typename T>
struct Element
{
    int left, top, right, bottom; // Bounding box of the element
    T id;                         // ID of the element
};

// Node
struct Node
{
    int firstChild;     // Index of the first child or the first element
    int numElements;    // Number of elements in the node or -1 if it's not a leaf
    int mx, my, sx, sy; // Centered rectangle and half-size
    int depth;          // Depth of the node
};


// QuadTree
template <typename T>
    requires std::is_trivially_constructible_v<T> && (sizeof(T) == 4)
class QuadTree
{
public:
    QuadTree(int worldSizeX, int worldSizeY, int maxElementsPerNode = 4, int maxDepth = 8) :
        maxElementsPerNode(maxElementsPerNode), maxDepth(maxDepth), worldSizeX(worldSizeX), worldSizeY(worldSizeY)
    {
        nodes.push_back(Node(-1, 0, worldSizeX / 2, worldSizeY / 2, worldSizeX / 2, worldSizeY / 2, 0)); // Root node
    }

    void insert(T id, int x, int y, int width, int height)
    {
        elements.push_back({x, y, x + width, y + height, id});
        insertElement(0, elements.size() - 1, 0);
    }

    void query(std::vector<T>& results, int x1, int y1, int x2, int y2) const { queryNode(0, x1, y1, x2, y2, results); }

    void clear()
    {
        nodes.clear();
        elements.clear();
        elementNodes.clear();
        nodes.push_back(Node(-1, 0, worldSizeX / 2, worldSizeY / 2, worldSizeX / 2, worldSizeY / 2, 0)); // Root node
    }

private:
    std::vector<Node> nodes;
    std::vector<Element<T>> elements;
    std::vector<ElementNode> elementNodes;
    int maxElementsPerNode;
    int maxDepth;
    int worldSizeX;
    int worldSizeY;

    static int intersect(int l1, int t1, int r1, int b1, int l2, int t2, int r2, int b2)
    {
        return l2 <= r1 && r2 >= l1 && t2 <= b1 && b2 >= t1;
    }

    void insertElement(int nodeIndex, int elementIndex, int depth)
    {
        Node& node = nodes[nodeIndex];

        if (node.numElements != -1 && (depth >= maxDepth || node.numElements < maxElementsPerNode))
        {
            addElementToNode(nodeIndex, elementIndex);
            return;
        }

        if (node.numElements != -1)
        {
            subdivideNode(nodeIndex);
        }

        for (int i = 0; i < 4; ++i)
        {
            int childIndex = nodes[nodeIndex].firstChild + i;
            if (nodes[childIndex].mx - nodes[childIndex].sx <= elements[elementIndex].left &&
                nodes[childIndex].mx + nodes[childIndex].sx >= elements[elementIndex].right &&
                nodes[childIndex].my - nodes[childIndex].sy <= elements[elementIndex].top &&
                nodes[childIndex].my + nodes[childIndex].sy >= elements[elementIndex].bottom)
            {
                insertElement(childIndex, elementIndex, depth + 1);
                return;
            }
        }
    }

    void subdivideNode(int nodeIndex)
    {
        Node& node = nodes[nodeIndex];
        int midX = node.mx;
        int midY = node.my;
        int halfWidth = node.sx / 2;
        int halfHeight = node.sy / 2;


        int currentIndex = nodes[nodeIndex].firstChild;
        nodes[nodeIndex].firstChild = nodes.size();
        const int currDepth = node.depth + 1;

        nodes.push_back({-1, 0, midX - halfWidth, midY - halfHeight, halfWidth, halfHeight, currDepth});
        nodes.push_back({-1, 0, midX + halfWidth, midY - halfHeight, halfWidth, halfHeight, currDepth});
        nodes.push_back(Node(-1, 0, midX - halfWidth, midY + halfHeight, halfWidth, halfHeight, currDepth));
        nodes.push_back(Node(-1, 0, midX + halfWidth, midY + halfHeight, halfWidth, halfHeight, currDepth));

        // Redistribute elements into child nodes
        while (currentIndex != -1)
        {
            const int nextIndex = elementNodes[currentIndex].next;
            const int elemIndex = elementNodes[currentIndex].elementIndex;
            const Element<T>& element = elements[elemIndex];
            //std::erase(elementNodes, elementNodes[currentIndex]);

            for (int i = 0; i < 4; ++i)
            {
                int childIndex = nodes[nodeIndex].firstChild + i;
                if (nodes[childIndex].mx - nodes[childIndex].sx <= element.left &&
                    nodes[childIndex].mx + nodes[childIndex].sx >= element.right &&
                    nodes[childIndex].my - nodes[childIndex].sy <= element.top &&
                    nodes[childIndex].my + nodes[childIndex].sy >= element.bottom)
                {
                    addElementToNode(childIndex, elemIndex);
                    break;
                }
            }
            currentIndex = nextIndex;
        }
        nodes[nodeIndex].numElements = -1;
    }

    void addElementToNode(int nodeIndex, int elementIndex)
    {
        Node& node = nodes[nodeIndex];
        elementNodes.push_back({node.firstChild, elementIndex});
        node.firstChild = elementNodes.size() - 1;
        node.numElements++;
    }

    void queryNode(int nodeIndex, int x1, int y1, int x2, int y2, std::vector<T>& results) const
    {
        const Node& node = nodes[nodeIndex];

        // Check if the query rectangle intersects the node's bounding box
        if (node.mx + node.sx < x1 && node.mx - node.sx > x2 && node.my + node.sy < y1 && node.my - node.sy > y2)
        {
            return;
        }

        if (node.numElements != -1)
        {
            int currentIndex = node.firstChild;
            while (currentIndex != -1)
            {
                const ElementNode& elementNode = elementNodes[currentIndex];
                const Element<T>& element = elements[elementNode.elementIndex];
                const int lft = element.left;
                const int top = element.top;
                const int rgt = element.right;
                const int btm = element.bottom;
                if (intersect(x1, y1, x2, y2, lft, top, rgt, btm))
                {
                    results.push_back(element.id);
                }
                currentIndex = elementNode.next;
            }
        }
        else
        {
            for (int i = 0; i < 4; ++i)
            {
                queryNode(node.firstChild + i, x1, y1, x2, y2, results);
            }
        }
    }
};


#endif //QUADTREE_H