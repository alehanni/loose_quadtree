#ifndef QUADTREE_H
#define QUADTREE_H

#include <limits>
#include <algorithm>
#include <cstdint>
#include <cassert>

struct point {
    float x, y;
};

static constexpr float inf = std::numeric_limits<float>::infinity();

struct box {
    point min{inf, inf};
    point max{-inf, -inf};

    box &operator |= (point const &p) {
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        return *this;
    }
};

point middle(point const &p1, point const &p2) {
    return {(p1.x + p2.x) / 2.f, (p1.y + p2.y) / 2.f};
}

box bbox(point *begin, point *end) {
    box result;
    for (auto it = begin; it != end; ++it)
        result |= *it;
    return result;
}

using node_id = std::uint32_t;

static constexpr node_id null = node_id(-1);

struct node {
    node_id children[2][2] {
        {null, null},
        {null, null}
    };
};

struct quadtree {
    box bbox;
    node_id root;
    std::vector<node> nodes;
    std::vector<point> points; // variable size
    std::vector<std::uint32_t> node_points_begin; // same size as nodes
};

node_id build_impl(quadtree &tree, box const &bbox, point *begin, point *end, std::size_t depth_limit) {
    
    if (begin == end) return null;

    node_id result = tree.nodes.size();
    tree.nodes.emplace_back();
    tree.node_points_begin.push_back(begin - &tree.points.front());

    if (begin + 1 == end || 0 == depth_limit) return result;

    point center = middle(bbox.min, bbox.max);

    auto bottom = [center](point const &p){return p.y < center.y;};
    auto left = [center](point const &p){return p.x < center.x;};

    point *split_y = std::partition(begin, end, bottom);
    point *split_x_lower = std::partition(begin, split_y, left);
    point *split_x_upper = std::partition(split_y, end, left);

    tree.nodes[result].children[0][0] = build_impl(tree, {bbox.min, center}, begin, split_x_lower, depth_limit - 1);
    tree.nodes[result].children[0][1] = build_impl(tree, {{center.x, bbox.min.y}, {bbox.max.x, center.y}}, split_x_lower, split_y, depth_limit - 1);
    tree.nodes[result].children[1][0] = build_impl(tree, {{bbox.min.x, center.y}, {center.x, bbox.max.y}}, split_y, split_x_upper, depth_limit - 1);
    tree.nodes[result].children[1][1] = build_impl(tree, {center, bbox.max}, split_x_upper, end, depth_limit - 1);

    return result;
}

static constexpr std::size_t MAX_QUADTREE_DETPH = 5;

quadtree build(std::vector<point> points) {
    assert(points.size() > 0);
    quadtree result;
    result.points = std::move(points);
    result.root = build_impl(
        result,
        bbox(&result.points.front(), &result.points.back()),
        &result.points.front(),
        &result.points.back(),
        MAX_QUADTREE_DETPH
    );
    result.node_points_begin.push_back(result.points.size());
    return result;
}

#endif