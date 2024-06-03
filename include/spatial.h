#ifndef SPATIAL_H
#define SPATIAL_H

// start by implementing point-based quadtree
// follow up with an upgrade to a "loose" quadtree to store AABBs
// the result is spatial indexing that stores each AABB only once

// - use single vector for nodes
// - rebuild every time tree changes
// - required functions: build, query
// - potentially use std::partition???

#include <cstdint>
#include <vector>
#include <algorithm>

using id_t = uint32_t;

constexpr id_t no_id = (id_t)(-1);
constexpr uint32_t points_per_node = 8;

struct point {
    float x, y;
};

struct aabb {
    point min, max;
};

struct node {
    id_t child_nw;
    id_t child_ne;
    id_t child_sw;
    id_t child_se;
};

struct quadtree {
    aabb bounds;
    id_t root;
    std::vector<node> nodes;
    std::vector<std::pair<id_t, id_t>> point_range;
    std::vector<point> points;
    // std::vector<aabb> bbox;
};

template<class Iterator>
id_t build_helper(quadtree &qt, aabb const &bbox, Iterator begin, Iterator end) {
    // create node from range of points

    // if no points - return no_id i.e. no node was created
    if (begin == end) return no_id;

    // create node
    id_t node_id = qt.nodes.size();
    qt.nodes.emplace_back();

    // if n_points < split threshold - return id of leaf node with points
    if (end < begin + 8) {
        // todo: push points and set ranges
        return node_id;
    }

    // if n_points > split threshold - split into four by using recursive calls, return id
    // use std::partition to sort points in order; nw, ne, sw, se

    point center = {(bbox.min.x + bbox.max.x) / 2.f, (bbox.min.y + bbox.max.y) / 2.f};

    auto is_north = [center](point const &p){return (p.y < center.y)};
    auto is_west = [center](point const &p){return (p.x < center.x)};

    Iterator south_start = std::partition(begin, end, is_north);
    Iterator south_east_start = std::partition(south_start, end, is_west);
    Iterator north_east_start = std::partition(begin, south_start, is_west);

    // points should have been rearranged to nw, nw, nw ... ne, ne, ne ... sw, sw, sw ... se, se, se

    aabb nw_box = {bbox.min, center};
    aabb ne_box = {{center.x, bbox.min.y}, {bbox.max.x, center.y}};
    aabb sw_box = {{bbox.min.x, center.y}, {center.x, bbox.max.y}};
    aabb se_box = {center, bbox.max};

    qt.nodes[node_id].child_nw = build_helper(qt, nw_box, begin, north_east_start);
    qt.nodes[node_id].child_ne = build_helper(qt, ne_box, north_east_start, south_start);
    qt.nodes[node_id].child_sw = build_helper(qt, sw_box, south_start, south_east_start);
    qt.nodes[node_id].child_se = build_helper(qt, se_box, south_east_start, end);

    return node_id;
}

// build quadtree from a list of points
quadtree build_quadtree(std::vector<point> points) {
    quadtree qt;

    qt.points = std::move(points);
    // qt.root = build_helper(qt, );
    


}

// fetch leaf nodes that overlap with region
std::vector<node> query(aabb region);


#endif