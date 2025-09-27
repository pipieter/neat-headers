#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <neat/allocators.hpp>
#include <vector>

struct Node {
    Node* left;
    Node* right;
};

Node* build(size_t depth, neat::allocators::arena<Node>& arena) {
    if (depth == 0) {
        return nullptr;
    }

    Node* node  = arena.allocate();
    node->left  = build(depth - 1, arena);
    node->right = build(depth - 1, arena);
    return node;
}

void count(Node* node, size_t depth, std::vector<size_t>& counts) {
    if (node == nullptr) {
        return;
    }

    counts[depth]++;
    count(node->left, depth + 1, counts);
    count(node->right, depth + 1, counts);
}

int main(int argc, const char** argv) {
    size_t depth = 10;
    if (argc > 1)
        depth = std::atoll(argv[1]);

    if (depth > 63)
        depth = 63;

    size_t nodes = ((2ull << depth) - 1ull);
    std::cout << "Building tree with depth " << depth << " or " << nodes << " nodes." << std::endl;

    neat::allocators::arena<Node> arena(nodes);
    if (arena.failure()) {
        std::cerr << "Could not allocate for " << nodes << " nodes." << std::endl;
        return 1;
    }

    Node* tree = build(depth, arena);

    std::vector<size_t> counts(depth, 0);
    count(tree, 0, counts);

    for (size_t i = 0; i < counts.size(); i++) {
        std::cout << i << '\t' << counts[i] << std::endl;
    }

    return 0;
}