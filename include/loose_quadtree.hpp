#ifndef ALH_LOOSE_QUADTREE_H
#define ALH_LOOSE_QUADTREE_H

#include <cassert>
#include <cstdint>
#include <vector>
#include <limits>
#include <algorithm>

namespace alh {

namespace loose_quadtree {
    struct point_t {
        float x, y;
    };

    struct aabb_t {
        point_t min, max;

        static bool intersect(aabb_t const& a, aabb_t const& b) { return a.intersect(b); }
        
        bool intersect(aabb_t const& other) const {
            return (max.x >= other.min.x && min.x < other.max.x
                && max.y >= other.min.y && min.y < other.max.y);
        }
    };
};

template<typename T=void*, uint64_t MAX_DEPTH=4>
struct loose_quadtree_t {

    using id_t = uint64_t;
    using point_t = typename loose_quadtree::point_t;
    using aabb_t = typename loose_quadtree::aabb_t;

    static constexpr id_t empty = id_t(-1);
    static constexpr float inf = std::numeric_limits<float>::infinity();

    loose_quadtree_t(std::vector<aabb_t> const& in, std::vector<T> const& data) { build(in, data); }

    void build(std::vector<aabb_t> const& in, std::vector<T> const& data) {
        assert(in.size() > 0);
        assert(in.size() == data.size());

        nodes.clear(); // note: maybe it's fine to just stomp the memory?
        node_bbs.clear();
        node_points_begin.clear();
        boxes.clear();

        // get box centers (from implicit conversion)
        boxes.reserve(in.size());
        boxes.insert(boxes.begin(), in.begin(), in.end());
        
        auto it_data = data.begin();
        for (aabb_entry_t &box : boxes) box.data = *it_data++;

        query_list.insert(query_list.begin(), in.size(), empty);

        // get bounding box of all centers
        aabb = {{inf, inf}, {-inf, -inf}};
        for (aabb_entry_t bb : boxes) {
            aabb.min.x = std::min(aabb.min.x, bb.center.x);
            aabb.min.y = std::min(aabb.min.y, bb.center.y);
            aabb.max.x = std::max(aabb.max.x, bb.center.x);
            aabb.max.y = std::max(aabb.max.y, bb.center.y);
        }

        root = build_recursive(aabb,
                               &boxes.front(),
                               &boxes.back()+1,
                               MAX_DEPTH);
        node_points_begin.push_back(boxes.size());
    }

    struct query_iter_t {
        query_iter_t(loose_quadtree_t &tree, id_t head) : tree(tree), head(head) {}
        query_iter_t &operator++() { head = tree.query_list[head]; return *this; }
        //query_iter_t operator++(int);
        
        friend bool operator==(query_iter_t const& lhs, query_iter_t const& rhs) {
            return (lhs.head == rhs.head); 
        }
        
        friend bool operator!=(query_iter_t const& lhs, query_iter_t const& rhs) {
            return !(lhs == rhs);
        }
        
        T operator*() const { assert(head != empty); return tree.boxes[head].data; }
    private:
        loose_quadtree_t const& tree;
        id_t head;
    };

    // create linked-list with indices for query results
    query_iter_t query_start(aabb_t query_bb) {
        query_head = empty;
        query_start_recursive(query_bb, root);
        return query_iter_t(*this, query_head);
    }

    // return sentinel value (placed at end of query by query_start)
    query_iter_t query_end() { return query_iter_t(*this, empty); }

private:
    struct node_t {
        id_t nw = empty;
        id_t ne = empty;
        id_t sw = empty;
        id_t se = empty;
    };

    struct aabb_entry_t {
        aabb_entry_t(aabb_t bb) {
            aabb = bb;
            center.x = (bb.min.x + bb.max.x) / 2.f;
            center.y = (bb.min.y + bb.max.y) / 2.f;
        }
        aabb_t aabb;
        point_t center;
        T data;
    };

    static void split_4(aabb_t const& bb, aabb_t &bb1, aabb_t &bb2, aabb_t &bb3, aabb_t &bb4) {
        point_t mid;
        mid.x = (bb.min.x + bb.max.x) / 2.f;
        mid.y = (bb.min.y + bb.max.y) / 2.f;

        bb1.min.x = bb.min.x;
        bb1.min.y = bb.min.y;
        bb1.max.x = mid.x;
        bb1.max.y = mid.y;

        bb2.min.x = mid.x;
        bb2.min.y = bb.min.y;
        bb2.max.x = bb.max.x;
        bb2.max.y = mid.y;

        bb3.min.x = bb.min.x;
        bb3.min.y = mid.y;
        bb3.max.x = mid.x;
        bb3.max.y = bb.max.y;

        bb4.min.x = mid.x;
        bb4.min.y = mid.y;
        bb4.max.x = bb.max.x;
        bb4.max.y = bb.max.y;
    }

    loose_quadtree_t() {}

    id_t build_recursive(aabb_t const& bb, aabb_entry_t *begin, aabb_entry_t *end, uint32_t depth) {
        if (begin == end) return empty;

        id_t nid = nodes.size();
        nodes.emplace_back();

        id_t idx = begin - &boxes.front();
        assert(idx >= 0 && idx < boxes.size());
        node_points_begin.push_back(idx);

        // compute bounding box for this node
        aabb_t node_bb{{inf, inf}, {-inf, -inf}};
        for (aabb_entry_t *it = begin; it != end; ++it) {
            node_bb.min.x = std::min(node_bb.min.x, it->aabb.min.x);
            node_bb.min.y = std::min(node_bb.min.y, it->aabb.min.y);
            node_bb.max.x = std::max(node_bb.max.x, it->aabb.max.x);
            node_bb.max.y = std::max(node_bb.max.y, it->aabb.max.y);
        }
        node_bbs.push_back(node_bb);

        if (begin+1 == end || 0 == depth) return nid;

        aabb_t bb_nw, bb_ne, bb_sw, bb_se;
        split_4(bb, bb_nw, bb_ne, bb_sw, bb_se);

        point_t mid = bb_nw.max;
        auto is_top = [mid](aabb_entry_t const& b){ return b.center.y < mid.y; };
        auto is_left = [mid](aabb_entry_t const& b){ return b.center.x < mid.x; };

        aabb_entry_t *split_y = std::partition(begin, end, is_top);
        aabb_entry_t *split_x_upper = std::partition(begin, split_y, is_left);
        aabb_entry_t *split_x_lower = std::partition(split_y, end, is_left);

        nodes[nid].nw = build_recursive(bb_nw, begin, split_x_upper, depth - 1);
        nodes[nid].ne = build_recursive(bb_ne, split_x_upper, split_y, depth - 1);
        nodes[nid].sw = build_recursive(bb_sw, split_y, split_x_lower, depth - 1);
        nodes[nid].se = build_recursive(bb_se, split_x_lower, end, depth - 1);

        return nid;
    }

    void query_start_recursive(aabb_t query_bb, id_t nid) {
        if (query_bb.intersect(node_bbs[nid])) {

            bool is_not_leaf = false;
            if (empty != nodes[nid].nw && (is_not_leaf=true)) query_start_recursive(query_bb, nodes[nid].nw);
            if (empty != nodes[nid].ne && (is_not_leaf=true)) query_start_recursive(query_bb, nodes[nid].ne);
            if (empty != nodes[nid].sw && (is_not_leaf=true)) query_start_recursive(query_bb, nodes[nid].sw);
            if (empty != nodes[nid].se && (is_not_leaf=true)) query_start_recursive(query_bb, nodes[nid].se);

            if (!is_not_leaf) {
                id_t i_front = node_points_begin[nid];
                id_t i_back = node_points_begin[nid+1];
                for (id_t i=i_front; i!=i_back; i++) {
                    if (query_bb.intersect(boxes[i].aabb)) {
                        query_list[i] = query_head;
                        query_head = i;
                    }
                }
            }
        }
    }

    id_t root;
    aabb_t aabb;
    id_t query_head;

    // per-node data
    std::vector<node_t> nodes;
    std::vector<aabb_t> node_bbs;
    std::vector<id_t> node_points_begin;

    // per-point data
    std::vector<aabb_entry_t> boxes;
    std::vector<id_t> query_list;
};

};

#endif