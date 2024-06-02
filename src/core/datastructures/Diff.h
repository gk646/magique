#include <vector>
#include <iostream>
#include <optional>
#include <algorithm>

// Element Node
struct ElementNode {
    int next; // Index of the next element in the leaf node (-1 for end)
    int elementIndex; // Index of the element in the elements vector
};

// Element
struct Element {
    int left, top, right, bottom; // Bounding box of the element
    int id; // ID of the element
};

// Node
struct Node {
    int firstChild; // Index of the first child or the first element
    int numElements; // Number of elements in the node or -1 if it's not a leaf
    int mx, my, sx, sy; // Centered rectangle and half-size
    int depth; // Depth of the node

    Node(int fc, int ne, int _mx, int _my, int _sx, int _sy, int d)
        : firstChild(fc), numElements(ne), mx(_mx), my(_my), sx(_sx), sy(_sy), depth(d) {}
};

// QuadTree
template <typename T>
class QuadTree {
public:
    QuadTree(int worldSizeX, int worldSizeY, int maxElementsPerNode = 4, int maxDepth = 5)
        : maxElementsPerNode(maxElementsPerNode), maxDepth(maxDepth) {
        nodes.push_back(Node(-1, 0, worldSizeX / 2, worldSizeY / 2, worldSizeX / 2, worldSizeY / 2, 0)); // Root node
    }

    void insert(int x, int y, int width, int height, int id) {
        elements.push_back({x, y, x + width, y + height, id});
        insertElement(0, elements.size() - 1, 0);
    }

    void query(int x1, int y1, int x2, int y2, std::vector<int>& results) const {
        queryNode(0, x1, y1, x2, y2, results);
    }

private:
    std::vector<Node> nodes;
    std::vector<Element> elements;
    std::vector<ElementNode> elementNodes;
    int maxElementsPerNode;
    int maxDepth;

    void insertElement(int nodeIndex, int elementIndex, int depth) {
        Node& node = nodes[nodeIndex];

        if (node.numElements != -1 && (depth >= maxDepth || node.numElements < maxElementsPerNode)) {
            addElementToNode(nodeIndex, elementIndex);
            return;
        }

        if (node.numElements != -1) {
            subdivideNode(nodeIndex);
        }

        for (int i = 0; i < 4; ++i) {
            int childIndex = nodes[nodeIndex].firstChild + i;
            if (nodes[childIndex].mx - nodes[childIndex].sx <= elements[elementIndex].left &&
                nodes[childIndex].mx + nodes[childIndex].sx >= elements[elementIndex].right &&
                nodes[childIndex].my - nodes[childIndex].sy <= elements[elementIndex].top &&
                nodes[childIndex].my + nodes[childIndex].sy >= elements[elementIndex].bottom) {
                insertElement(childIndex, elementIndex, depth + 1);
                return;
            }
        }
    }

    void subdivideNode(int nodeIndex) {
        Node& node = nodes[nodeIndex];
        int midX = node.mx;
        int midY = node.my;
        int halfWidth = node.sx / 2;
        int halfHeight = node.sy / 2;

        node.firstChild = nodes.size();
        node.numElements = -1;

        nodes.push_back(Node(-1, 0, midX - halfWidth, midY - halfHeight, halfWidth, halfHeight, node.depth + 1));
        nodes.push_back(Node(-1, 0, midX + halfWidth, midY - halfHeight, halfWidth, halfHeight, node.depth + 1));
        nodes.push_back(Node(-1, 0, midX - halfWidth, midY + halfHeight, halfWidth, halfHeight, node.depth + 1));
        nodes.push_back(Node(-1, 0, midX + halfWidth, midY + halfHeight, halfWidth, halfHeight, node.depth + 1));

        // Redistribute elements into child nodes
        int currentIndex = node.firstChild;
        while (currentIndex != -1) {
            const ElementNode& elementNode = elementNodes[currentIndex];
            const Element& element = elements[elementNode.elementIndex];

            node.firstChild = elementNode.next;

            for (int i = 0; i < 4; ++i) {
                int childIndex = node.firstChild + i;
                if (nodes[childIndex].mx - nodes[childIndex].sx <= element.left &&
                    nodes[childIndex].mx + nodes[childIndex].sx >= element.right &&
                    nodes[childIndex].my - nodes[childIndex].sy <= element.top &&
                    nodes[childIndex].my + nodes[childIndex].sy >= element.bottom) {
                    addElementToNode(childIndex, elementNode.elementIndex);
                    break;
                }
            }

            currentIndex = elementNode.next;
        }
    }

    void addElementToNode(int nodeIndex, int elementIndex) {
        Node& node = nodes[nodeIndex];
        elementNodes.push_back({node.firstChild, elementIndex});
        node.firstChild = elementNodes.size() - 1;
        node.numElements++;
    }

    void queryNode(int nodeIndex, int x1, int y1, int x2, int y2, std::vector<int>& results) const {
        const Node& node = nodes[nodeIndex];
        if (node.mx + node.sx < x1 || node.mx - node.sx > x2 || node.my + node.sy < y1 || node.my - node.sy > y2) {
            return;
        }

        if (node.numElements != -1) {
            int currentIndex = node.firstChild;
            while (currentIndex != -1) {
                const ElementNode& elementNode = elementNodes[currentIndex];
                const Element& element = elements[elementNode.elementIndex];
                if (element.left < x2 && element.right > x1 && element.top < y2 && element.bottom > y1) {
                    results.push_back(element.id);
                }
                currentIndex = elementNode.next;
            }
        } else {
            for (int i = 0; i < 4; ++i) {
                queryNode(node.firstChild + i, x1, y1, x2, y2, results);
            }
        }
    }
};

int main() {
    QuadTree<int> qt(100, 100);

    qt.insert(10, 10, 5, 5, 1);
    qt.insert(20, 20, 5, 5, 2);
    qt.insert(30, 30, 5, 5, 3);
    qt.insert(40, 40, 5, 5, 4);
    qt.insert(50, 50, 5, 5, 5);

    std::vector<int> results;
    qt.query(15, 15, 35, 35, results);

    std::cout << "Query results: ";
    for (int id : results) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    return 0;
}