#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <memory>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <deque>
#include <climits>
#include <cassert>
#include <random>
#include <chrono>
#include <bits/extc++.h>

using namespace std;
using namespace __gnu_pbds;

static double VALUATION_POWER = 1.2;
static double BLINK_RATE = 0.01;
static bool ROTATION_ALLOWED = true;
static int AVG_NODES_REMOVED = 6;
static int MAX_RUN_TIME = 3;
static int HISTORY_LENGTH = 500;
static int MAX_RR_ITEATIONS = 1000000000;

//std::minstd_rand rng;
std::minstd_rand rng(time(nullptr));

// std::uniform_real_distribution<double> dist(0.0, 1.0);
enum class Direction { 
    Vertical, Horizontal, None 
};

Direction rotate(const Direction& direction)
{
    if(direction == Direction::Vertical) 
        return Direction::Horizontal;
    else if(direction == Direction::Horizontal)
        return Direction::Vertical;
    else 
        return Direction::None;
}

ostream& operator<<(ostream &os, Direction direction)
{
    switch (direction) {
        case Direction::Vertical: {
            os << "Vertical";
            break;
        }
        case Direction::Horizontal: {
            os << "Horizontal";
            break;
        }
        case Direction::None: {
            os << "None";
            break;
        }
    }
    return os;
}

enum class Rotation { Default, Rotated, None };
Rotation rotate(const Rotation& rotation)
{
    if(rotation == Rotation::Default) 
        return Rotation::Rotated;
    else if(rotation == Rotation::Rotated)
        return Rotation::Default;
    else 
        return Rotation::None;
}
ostream& operator<<(ostream &os, Rotation& rotation)
{
    switch (rotation) {
        case Rotation::Default: {
            os << "Default";
            break;
        }
        case Rotation::Rotated: {
            os << "Rotated";
            break;
        }
        case Rotation::None: {
            os << "None";
            break;
        }
    }
    return os;
}

enum class LayoutType { Existing, Empty };
ostream& operator<<(ostream &os, LayoutType layouttype)
{
    switch (layouttype) {
        case LayoutType::Existing: {
            os << "Existing";
            break;
        }
        case LayoutType::Empty: {
            os << "Empty";
            break;
        }
    }
    return os;
}

struct LayoutIndex {
    LayoutType type;
    int index;   

    //LayoutIndex() {}
    LayoutIndex(const LayoutType& type_, const int& index_) : type(type_), index(index_) {}

    bool operator<(const LayoutIndex& other) const {
    return type == LayoutType::Empty && other.type == LayoutType::Existing ? false :
            type == LayoutType::Existing && other.type == LayoutType::Empty ? true :
            index < other.index;
    }
    bool operator==(const LayoutIndex& other) const { return index == other.index && type == other.type; }
};
namespace std{
    template<> struct tr1::hash<pair<LayoutIndex, int>>{
        size_t operator()(const pair<LayoutIndex, int> &index) const{
            return index.first.index + (index.second << 10);
        }
    };
}

struct Cost {
    int material_cost;
    double leftover_value; 
    int excluded_item_area;
    int included_item_area;

    //Cost() {material_cost = 0; leftover_value = 0; excluded_item_area = 0; included_item_area = 0; }
    Cost(const int& material_cost, const double& leftover_value, const int& excluded_item_area, const int& included_item_area) :
    material_cost(material_cost), leftover_value(leftover_value), excluded_item_area(excluded_item_area), included_item_area(included_item_area) {}

    Cost operator+(const Cost &other) { 
        material_cost += other.material_cost, 
        leftover_value += other.leftover_value,
        excluded_item_area += other.excluded_item_area,
        included_item_area += other.included_item_area;
        return *this;
    }
    Cost operator+=(const Cost &other) { 
        material_cost += other.material_cost, 
        leftover_value += other.leftover_value,
        excluded_item_area += other.excluded_item_area,
        included_item_area += other.included_item_area;
        return *this;
    }


    Cost operator-(const Cost &other) { 
        material_cost -= other.material_cost, 
        leftover_value -= other.leftover_value,
        excluded_item_area -= other.excluded_item_area,
        included_item_area -= other.included_item_area;
        return *this;
    }
    void empty() {material_cost = 0; leftover_value = 0; excluded_item_area = 0; included_item_area = 0;}
    
    inline double item_area_fraction_included() const { return (double) included_item_area / (double) (included_item_area + excluded_item_area); }

    inline void add_leftover_value(const double& add_value) { leftover_value += add_value; }

    inline void add_material_cost(const int& add_material) { material_cost += add_material; }
};
ostream& operator<<(ostream &os, const Cost& cost)
{
    os << "material cost: "  << cost.material_cost
       << " leftover value: "  << cost.leftover_value
       << " excluded item area: "<< cost.excluded_item_area
       << " included item area: " << cost.included_item_area 
       ;
    return os;
}

struct Space{
    int index;
    int bin_index;
    int width;
    int height; 
    double x;
    double y;

    Space(const int& index, const int& bin_index, const int& width, const int& height, const double& x, const double& y):
    index(index), bin_index(bin_index), width(width), height(height), x(x), y(y) {}
    bool operator<(const Space& other) const { 
        if(bin_index < other.bin_index) return true;
        else if (bin_index > other.bin_index) return false;
        if(y < other.y) return true;
        else if (y > other.y) return false;
        if(x < other.x) return true;
        else if (x > other.x) return false;
        return false;
    }
};

double get_font_size(const int& a, const int& b)
{
    return min(max(a, b) * 0.005, min(a, b) * 0.02);
}

struct Rectangle {
    int width;
    int height;
    int area;

    //Rectangle() {}
    Rectangle(const int& width, const int& height) : width(width), height(height), area(width * height) {}   
    bool operator<(const Rectangle& other) const { 
        return area == other.area ? width > other.width : area > other.area; 
    } //按面积递减的顺序排序
    bool operator==(const Rectangle& other) const { return width == other.width && height == other.height; }
};

namespace std{
    template<> struct tr1::hash<Rectangle>{
        size_t operator()(const Rectangle &rectangle) const{
            return rectangle.width + rectangle.height << 10;
        }
    };
}

struct ItemType {
    int id;
    int width;
    int height;
    Rotation fixed_rotation;
    Rectangle rectangle;
    Rectangle rotated_rectangle;
    int area_;
    
    //ItemType() {}
    ItemType(const int& id, const int& width, const int& height, const Rotation& fixed_rotation) : 
        id(id), width(width), height(height), fixed_rotation(fixed_rotation), 
        rectangle(Rectangle(width, height)), rotated_rectangle(Rectangle(height, width)), area_(width * height) {}
    
    bool operator==(const ItemType &other) const { return id == other.id; }
    bool operator<(const ItemType &other) const { return rectangle < other.rectangle; }
    //int area() const { return area_; }
};
namespace std{
    template<> struct tr1::hash<ItemType>{
        size_t operator()(const ItemType &itemtype) const{
            return itemtype.id;
        }
    };
}

ostream& operator<<(ostream &os, const ItemType& item_type)
{
    os << "id: " << std::setw(3) << item_type.id
       << " width: " << std::setw(3) << item_type.width 
       << " height: " << std::setw(3) << item_type.height
       << " area: " << std::setw(3) << item_type.rectangle.area 
       << " rotated: " << std::setw(3) << item_type.rotated_rectangle.area
       ;
    return os;
}

struct BinType {
    int id;
    int width;
    int height;
    int area;
    Direction fixed_first_cut_orientation;

    //BinType() {}
    BinType(const int& id, const int& width, const int& height, const int& area, const Direction& fixed_first_cut_orientation) :
        id(id), width(width), height(height), area(area), fixed_first_cut_orientation(fixed_first_cut_orientation) {}

};
ostream& operator<<(ostream &os, const BinType& bin_type)
{
    os  << "id: " << bin_type.id << " "
        << "width: " << bin_type.width << " "
        << "height: " << bin_type.height << " ";
    return os;
}


struct Instance {
    int total_item_area = 0;
    int total_item_qty = 0;
    vector<pair<ItemType, int>> items; // first中存type， second存每种type的数量
    vector<pair<BinType, int>> bins; 

    Instance(const Instance& other) :
        total_item_area(other.total_item_area),
        total_item_qty(other.total_item_qty),
        items(other.items),
        bins(other.bins) {}
    
    Instance(const char* classfile)
    {
        std::ifstream infile(classfile);
        if(!infile.is_open())
        {
            std::cerr << "cannot open instance file from " << classfile << std::endl;
            exit(1);
        }
        std::cerr << "Now starting optimize the instance from " << classfile << std::endl;
        int n; 
        int j, w, h, d, b, p;

        __gnu_pbds::gp_hash_table<Rectangle, int> cnt;

        infile >> n >> w >> h;
        total_item_qty = n;
        
        BinType fixed_bin(0, w, h, w * h, Direction::None); 
        bins.emplace_back(fixed_bin, n);
    
        while(n--) //pre-process 合并相同形状的item
        {
            infile >> j >> w >> h >> d >> b >> p;
            if(w < h) std::swap(w, h);
            Rectangle rec(w, h);
            if(cnt.find(rec) == cnt.end()) cnt[rec] = 1;
            else cnt[rec]++;
            total_item_area += rec.area;
        }

        int itemtypeid = 0;
        for(const auto& rec : cnt)
        {
            int w = rec.first.width, h = rec.first.height;
            ItemType item(itemtypeid, w, h, Rotation::None);
            items.emplace_back(item, rec.second);
            itemtypeid++;
        }

        infile.close();
    }

    int smallest_bin_area() {
        int result = bins[0].first.area;
        for(int i = 1; i < bins.size(); ++i)
            result = min(result, bins[i].first.area);
        return result; 
    }
};



struct IOCUpdates {
    vector<int> removed_nodes;
    vector<int> new_nodes;
    LayoutIndex layout_index;

    // IOCUpdates() {}
    IOCUpdates(const LayoutIndex& layout_index_) : layout_index(layout_index_) {}    
};

struct NodeBlueprint {
    int width;
    int height;
    vector<NodeBlueprint> children;
    int itemtype_id; // if no, item_id == -1
    Direction next_cut_orientation; 
    int area;    

    // NodeBlueprint() {}
    NodeBlueprint(const int& width_, const int& height_, const int& itemtype_id_, const Direction& next_cut_orientation_) :
    width(width_), height(height_), itemtype_id(itemtype_id_), next_cut_orientation(next_cut_orientation_), area(width_ * height_) {}
    
    const Cost calculate_cost() const {
        if(itemtype_id >= 0) 
            return Cost(0, 0.0, 0, 0); //这个结点是item结点，不用计算cost
        else if(children.empty())
            return Cost(0, pow((double) area, VALUATION_POWER), 0, 0); //这个结点是leftover，需要计算value
        else {
            Cost cost(0, 0.0, 0, 0);
            for(const auto& child : children)
                cost += child.calculate_cost();
            return cost;
        }
    }

    const bool is_empty() const { // 不含item和children就为空结点
        //assert(itemtype_id >= -1);
        return itemtype_id == -1 && children.empty(); 
    }
};

struct Node {
    int type; //-1 if no itemtype, else itemtype_id
    int width; 
    int height; 
    optional<int> parent; //if not existing, parent = -1
    vector<int> children;
    Direction next_cut_orientation;
    int area;

    //Node() {parent = nullopt; type = -1;}
    Node(const int& width_, const int& height_, const Direction& next_cut_orientation_, const int& type_) :
    width(width_), height(height_), next_cut_orientation(next_cut_orientation_), type(type_), parent(nullopt), area(width * height) {}

    bool operator<(const Node &other) const { return area < other.area; }
    
    const bool is_empty() const { 
        //assert(type >= -1);
        return type == -1 && children.empty(); 
    }
    
    const bool insertion_possible(const ItemType& itemtype, const Rotation& rotation) const {
        //assert(children.empty() && type == -1);
        
        const Rectangle& rect = (rotation == Rotation::Default ? itemtype.rectangle : itemtype.rotated_rectangle);
        return width >= rect.width && height >= rect.height;
    }

    const Cost calculate_cost() const {
        const bool& has_itemtype = (type > -1);
        const bool& is_empty = children.empty();
        if(has_itemtype && is_empty) return Cost(0, 0.0, 0, 0); //itemnode
        else if(!has_itemtype && !is_empty) return Cost(0, 0.0, 0, 0); //structure node
        else if(!has_itemtype && is_empty)  return Cost(0, pow((double) area, VALUATION_POWER), 0, 0); //leftover node
        else throw "Itemtype set on node with children";
    }

    void generate_insertion_node_blueprints(const ItemType& item, const Rotation& rotation, vector<vector<NodeBlueprint>>& insertion_replacements)
    {
        //assert(rotation != Rotation::None);
        //assert(insertion_possible(item, rotation));
        const Rectangle& item_rect = rotation == Rotation::Default ? item.rectangle : item.rotated_rectangle;

        /*
             Scenario 1: Part fits exactly into Node
             ---*****          ---*****             *       ->      *
                *   *             *$$$*
                *   *     ->      *$$$*
                *   *             *$$$*
             ---*****          ---*****

             -> node gets replaced by one node on same level
             -> = Scenario 2
         */

        /*
            Scenario 2: Part has same dimensions in the direction of the current cut
             ---*****          ---*****             *       ->      $   *
                *   *             *$* *
                *   *     ->      *$* *
                *   *             *$* *
             ---*****          ---*****

             -> node splits into 2 new nodes on same level
         */
        if(next_cut_orientation == Direction::Horizontal && height == item_rect.height)
        {
            const int& remainder_width = width - item_rect.width;
            NodeBlueprint item_node(item_rect.width, height, item.id, next_cut_orientation);
            NodeBlueprint remainder_node(remainder_width, height, -1, next_cut_orientation);
            
            //insertion_replacements.reserve(insertion_replacements.size() + 1);
            insertion_replacements.emplace_back(vector<NodeBlueprint>{move(item_node), move(remainder_node)});

            return;
        }
        if(next_cut_orientation == Direction::Vertical && width == item_rect.width)
        {
            const int& remainder_height = height - item_rect.height;
            NodeBlueprint item_node(width, item_rect.height, item.id, next_cut_orientation);
            NodeBlueprint remainder_node(width, remainder_height, -1, next_cut_orientation);

            //insertion_replacements.reserve(insertion_replacements.size() + 1);
            insertion_replacements.emplace_back(vector<NodeBlueprint>{move(item_node), move(remainder_node)});

            return;
        }

        /*
             Scenario 3: Part fits exactly in opposite dimension of cut
             ---*****          ---*****             *       ->      *  
                *   *             *$$$*                            / \
                *   *     ->      *****                           $   *
                *   *             *   *
             ---*****          ---*****
         */

        if(next_cut_orientation == Direction::Horizontal && width == item_rect.width)
        {
            NodeBlueprint copy(width, height, -1, next_cut_orientation);

            const int& remainer_height = height - item_rect.height;

            NodeBlueprint item_node(width, item_rect.height, item.id, rotate(next_cut_orientation));
            NodeBlueprint remainder_node(width, remainer_height, -1, rotate(next_cut_orientation));

            copy.children.reserve(copy.children.size() + 2);
            copy.children.emplace_back(move(item_node));
            copy.children.emplace_back(move(remainder_node));
            
            //insertion_replacements.reserve(insertion_replacements.size() + 1);
            insertion_replacements.emplace_back(vector<NodeBlueprint>{move(copy)});
            
            return;
        }
        if(next_cut_orientation == Direction::Vertical && height == item_rect.height)
        {
            NodeBlueprint copy(width, height, -1, next_cut_orientation);

            const int& remainder_width = width - item_rect.width;

            NodeBlueprint item_node(item_rect.width, height, item.id, rotate(next_cut_orientation));
            NodeBlueprint remainder_node(remainder_width, height, -1, rotate(next_cut_orientation));

            copy.children.reserve(copy.children.size() + 2);
            copy.children.emplace_back(move(item_node));
            copy.children.emplace_back(move(remainder_node));

            //insertion_replacements.reserve(insertion_replacements.size() + 1);
            insertion_replacements.emplace_back(vector<NodeBlueprint>{move(copy)});

            return;
        }
        /*
             Scenario 4: Part doesn't fit exactly in any dimension

             Scenario 4.1: First cut in same direction as current orientation
             ---*****          ---*****             *       ->      *   *
                *   *             *$* *                            / \
                *   *     ->      *** *                           $   *
                *   *             * * *
             ---*****          ---*****

             This requires an extra available level
         */
        if(next_cut_orientation == Direction::Horizontal) 
        {
            const int& remainder_width_top = width - item_rect.width; 
            NodeBlueprint item_node_parent(item_rect.width, height, -1, next_cut_orientation);
            NodeBlueprint remainder_node_top(remainder_width_top, height, -1, next_cut_orientation);

            const int& remainder_height_bottom = height - item_rect.height;
            NodeBlueprint item_node(item_rect.width, item_rect.height, item.id, rotate(next_cut_orientation));
            NodeBlueprint remainder_node_bottom(item_rect.width, remainder_height_bottom, -1, rotate(next_cut_orientation));

            item_node_parent.children.reserve(item_node_parent.children.size() + 2);
            item_node_parent.children.emplace_back(move(item_node));
            item_node_parent.children.emplace_back(move(remainder_node_bottom));

            //std::vector<NodeBlueprint> tmp = {item_node_parent, remainder_node_top};
            //insertion_replacements.reserve(insertion_replacements.size() + 1);
            insertion_replacements.emplace_back(vector<NodeBlueprint>{move(item_node_parent), move(remainder_node_top)});
        }
        if(next_cut_orientation == Direction::Vertical)
        {
            const int& remainder_height_top = height - item_rect.height;
            NodeBlueprint item_node_parent(width, item_rect.height, -1, next_cut_orientation);
            NodeBlueprint remainder_node_top(width, remainder_height_top, -1, next_cut_orientation);

            const int& remainder_width_bottom = width - item_rect.width;
            NodeBlueprint item_node(item_rect.width, item_rect.height, item.id, rotate(next_cut_orientation));
            NodeBlueprint remainder_node_bottom(remainder_width_bottom, item_rect.height, -1, rotate(next_cut_orientation));

            item_node_parent.children.reserve(item_node_parent.children.size() + 2);
            item_node_parent.children.emplace_back(move(item_node));
            item_node_parent.children.emplace_back(move(remainder_node_bottom));
            
            // std::vector<NodeBlueprint> tmp = {item_node_parent, remainder_node_top};

            //insertion_replacements.reserve(insertion_replacements.size() + 1);
            insertion_replacements.emplace_back(vector<NodeBlueprint>{move(item_node_parent), move(remainder_node_top)});
        }

        /*
                Scenario 4.2: First cut in opposite of current orientation
                ---*****          ---*****             *       ->      *   *
                    *   *             *$* *                            / \
                    *   *     ->      *****                           *   *
                    *   *             *   *                          / \
                ---*****          ---*****                         $   *

            */
        
        if(next_cut_orientation == Direction::Horizontal)
        {
            NodeBlueprint copy(width, height, -1, next_cut_orientation);
            
            const int& remainder_height_top = height - item_rect.height; 
            NodeBlueprint item_node_parent(width, item_rect.height, -1, rotate(next_cut_orientation));
            NodeBlueprint remainder_node_top(width, remainder_height_top, -1, rotate(next_cut_orientation));

            const int& remainder_width_bottom = width - item_rect.width;
            NodeBlueprint item_node(item_rect.width, item_rect.height, item.id, next_cut_orientation);
            NodeBlueprint remainder_node_bottom(remainder_width_bottom, item_rect.height, -1, next_cut_orientation);
            
            item_node_parent.children.reserve(item_node_parent.children.size() + 2);
            item_node_parent.children.emplace_back(move(item_node));
            item_node_parent.children.emplace_back(move(remainder_node_bottom));
            
            copy.children.reserve(copy.children.size() + 2);
            copy.children.emplace_back(move(item_node_parent));
            copy.children.emplace_back(move(remainder_node_top));
            
            //insertion_replacements.reserve(insertion_replacements.size() + 1);
            insertion_replacements.emplace_back(vector<NodeBlueprint>{move(copy)});
        }
        if(next_cut_orientation == Direction::Vertical)
        {
            NodeBlueprint copy(width, height, -1, next_cut_orientation);
            
            const int& remainder_width_top = width - item_rect.width; 
            NodeBlueprint item_node_parent(item_rect.width, height, -1, rotate(next_cut_orientation));
            NodeBlueprint remainder_node_top(remainder_width_top, height, -1, rotate(next_cut_orientation));

            const int& remainder_height_bottom = height - item_rect.height;
            NodeBlueprint item_node(item_rect.width, item_rect.height, item.id, next_cut_orientation);
            NodeBlueprint remainder_node_bottom(item_rect.width, remainder_height_bottom, -1, next_cut_orientation);

            item_node_parent.children.reserve(item_node_parent.children.size() + 2);
            item_node_parent.children.emplace_back(move(item_node));
            item_node_parent.children.emplace_back(move(remainder_node_bottom));
            
            copy.children.reserve(copy.children.size() + 2);
            copy.children.emplace_back(move(item_node_parent));
            copy.children.emplace_back(move(remainder_node_top));

            //insertion_replacements.reserve(insertion_replacements.size() + 1);
            insertion_replacements.emplace_back(vector<NodeBlueprint>{move(copy)});
        }
        return;
    }
};
ostream& operator<<(ostream &os, Node node)
{
    os << "{Node type:" <<  node.type
        << "width: " << node.width
        << "height: " << node.height
        << "next_cut_orientation: " << node.next_cut_orientation
        << "}";
    return os;
}

struct InsertionBlueprint {
    LayoutIndex layout_index; 
    int original_node_index;
    vector<NodeBlueprint> replacements;
    ItemType itemtype; 
    Cost cost;

    InsertionBlueprint(const LayoutIndex& layout_index_, const int& original_node_index_, const vector<NodeBlueprint>& replacements_, const ItemType& itemtype_, const Cost& cost_) :
    layout_index(layout_index_), original_node_index(original_node_index_), replacements(replacements_), itemtype(itemtype_), cost(cost_) {}
    bool operator<(const InsertionBlueprint& other) const { return cost.leftover_value > other.cost.leftover_value; }
};

struct Layout {
    int id; 
    BinType bintype; 
    __gnu_pbds::gp_hash_table<int, Node> nodes;
    int top_node_id;
    optional<Cost> cached_cost;
    optional<double> cached_usage;
    //tree<pair<int, Index>, null_type, less<pair<int, Index>>, rb_tree_tag> sorted_empty_nodes;
    set<pair<int, int>, greater<pair<int, int>>> sorted_empty_nodes; //第一个int里面存面积，第二个int里面存index
    int node_counter; 

    
    //Layout() {}
    Layout(const int& id_, const BinType& bintype_, const Direction& first_cut_orientation_) : 
        id(id_) , 
        bintype(bintype_), 
        cached_cost(nullopt), 
        cached_usage(nullopt),
        node_counter(0), 
        top_node_id(0)  {
        //assert(first_cut_orientation_ != Direction::None);

        Node top_node(bintype.width, bintype.height, first_cut_orientation_, -1);
        nodes.insert(make_pair(top_node_id, top_node));

        Node placeholder_node(bintype.width, bintype.height, rotate(first_cut_orientation_), -1);
        register_node(placeholder_node, top_node_id, true);
    }

    void implement_insertion_blueprint(const InsertionBlueprint& blueprint, const Instance& instance, IOCUpdates& updates)
    {
        const int& original = blueprint.original_node_index;

        //assert(nodes[original].parent.has_value());
        
        const int parent = *nodes.find(original)->second.parent;
        

        optional<vector<int>> empty_vector = nullopt;
        unregister_node(original, empty_vector); //把原来的结点删除
        updates.removed_nodes.reserve(updates.removed_nodes.size() + 1);
        updates.removed_nodes.emplace_back(original);

        vector<int> all_created_nodes;
        for(const auto& replacement : blueprint.replacements)
            implement_node_blueprint(parent, replacement, instance, all_created_nodes); //插入新的结点
        
        updates.new_nodes.reserve(updates.new_nodes.size() + all_created_nodes.size());
        for(const auto& node : all_created_nodes)
        {
            updates.new_nodes.emplace_back(node);
        }    
    }

    void implement_node_blueprint(const int& parent, const NodeBlueprint& blueprint, const Instance& instance, vector<int>& new_nodes)
    {

        Node node(blueprint.width, blueprint.height, blueprint.next_cut_orientation, blueprint.itemtype_id);
        const int& node_index = register_node(node, parent, blueprint.is_empty());
        
        new_nodes.reserve(new_nodes.size() + 1);
        new_nodes.emplace_back(node_index);

        for(const auto& child : blueprint.children) {
            implement_node_blueprint(node_index, child, instance, new_nodes);
        }
            
    }

    vector<int> remove_node(const int& node_index)
    {
        /*®
           Scenario 1: Empty node present + other child(ren)
            -> expand existing waste piece

             ---******               ---******
                *$$$$*                  *$$$$*
                ******                  ******
                *XXXX*     ----->       *    *
                ******                  *    *
                *    *                  *    *
             ---******               ---******

             Scenario 2: No waste piece present
                -> convert Node to be removed into waste Node

             ---******               ---******
                *$$$$*                  *$$$$*
                ******    ----->        ******
                *XXXX*                  *    *
             ---******               ---******

             Scenario 3: No other children present besides waste piece
                -> convert parent into waste piece

             ---******               ---******
                *XXXX*                  *    *
                ******    ----->        *    *
                *    *                  *    *
             ---******               ---******

         */
        
        const int parent_node_index = *nodes.find(node_index)->second.parent;

        const Node& parent_node = nodes.find(parent_node_index)->second;

        const auto empty_node_index = find_if(parent_node.children.begin(), parent_node.children.end(), [=](int child){ return nodes.find(child)->second.is_empty(); });
        
        optional<vector<int>> removed_items = vector<int>{};
        
        if(empty_node_index != parent_node.children.end()) {
            //Scenario 1 and 3
            
            //assert(nodes.find(*empty_node_index)->second.is_empty());
            if(parent_node.children.size() > 1 || parent_node.parent == nullopt) {
                //Sceario 1 (also do this when the parent node is the root) 
                //Two children are merged into one
                assert(nodes.find(*empty_node_index)->second.is_empty());
                const Node& node = nodes.find(node_index)->second; 
                const Node& empty_node = nodes.find(*empty_node_index)->second;
                const Node replacement_node = parent_node.next_cut_orientation == Direction::Horizontal ? 
                    Node(node.width, empty_node.height + node.height, node.next_cut_orientation, -1) :
                    Node(empty_node.width + node.width, node.height, node.next_cut_orientation, -1);
                
                unregister_node(*empty_node_index, removed_items);
                unregister_node(node_index, removed_items);
                
                register_node(replacement_node, parent_node_index, true);
            } else {
                //Scenerio 3: replace the parent with an empty node
                const int grandparent_index = *parent_node.parent;
                
                const Node empty_parent_node(parent_node.width, parent_node.height, parent_node.next_cut_orientation, -1);

                unregister_node(parent_node_index, removed_items);
                register_node(empty_parent_node, grandparent_index, true);
            }

        } else {
            const Node node = nodes.find(node_index)->second; 
            const Node replacement_node(node.width, node.height, node.next_cut_orientation, -1);
            unregister_node(node_index, removed_items);
            register_node(replacement_node, parent_node_index, true);
        }
        return *removed_items;
    }

    void invalidate_caches() {
        cached_cost = nullopt;
        cached_usage = nullopt;
    }

    const int register_node(const Node& node, const int& parent, const bool& is_empty)
    {
        invalidate_caches();
        const int& node_index = ++node_counter;
        nodes.insert(make_pair(node_index, node));
        if(is_empty) {
            //assert(nodes[node_index].is_empty());
            const int& node_area = nodes.find(node_index)->second.area;
            sorted_empty_nodes.insert(make_pair(node_area, node_index));
        }    

        nodes.find(node_index)->second.parent = parent;

        //nodes[parent].children.reserve(nodes[parent].children.size() + 1);
        nodes.find(parent)->second.children.emplace_back(node_index);
        return node_index;
    }

    void unregister_node(const int& node_index, optional<vector<int>>& removed_item_ids)
    {
        invalidate_caches();
        const Node node = nodes.find(node_index)->second;
        if(node.is_empty()) {
            const pair<int, int>& empty_node = make_pair(node.area, node_index);
            sorted_empty_nodes.erase(empty_node);
        }

        for(const auto& child : node.children) 
        unregister_node(child, removed_item_ids);

        nodes.erase(node_index);

        if(node.type >= 0) {
            if(removed_item_ids.has_value()){
                (*removed_item_ids).emplace_back(node.type);
            }
            invalidate_caches();
        }

        if(node.parent.has_value()) {
            vector<int>& children = nodes.find(*node.parent)->second.children;
            remove(children.begin(), children.end(), node_index);
            children.pop_back();
        }
    }

    const vector<int> get_included_items() const
    {
        //cout << valid_nodes.size() << endl;
        vector<int> result;
        result.reserve(nodes.size());
        for(const auto& [_, node] : nodes)
        {
            if(node.type >= 0) 
                result.emplace_back(node.type);
        }
        return result;
    }

    const Cost calculate_cost() const
    {
        Cost result(bintype.area, 0.0, 0, 0);
        for(const auto& [_, node] : nodes)
        {
            result += node.calculate_cost();
        }
        return result;
    }
    
    const Cost& cost()
    {
        if(cached_cost.has_value()) return *cached_cost; 
        cached_cost = calculate_cost();
        return *cached_cost;
    }

    const double calculate_usage() const
    {
        int used_area = 0;
        for(const auto& [_, node] : nodes)
            used_area += (node.type >= 0 ? node.area : 0);
        return (double) used_area / (double) bintype.area;
    }

    const double& usage()
    {
        if(cached_usage.has_value()) return *cached_usage;
        cached_usage = calculate_usage();
        return *cached_usage;
    }

    const bool is_empty() const
    {
        return nodes.find(top_node_id)->second.is_empty();
        //return nodes.find(top_node_id)->second.children.size() == 1 && nodes.find(nodes.find(top_node_id)->second.children[0])->second.type == -1;
    }

    const vector<int> get_removable_nodes() const
    {
        vector<int> result;
        result.reserve(nodes.size());
        for(const auto& [id, node] : nodes)
            if(!node.is_empty()) result.emplace_back(id);
        return result;
    }

    void print(const int& node_index) const
    {
        const Node& node = nodes.find(node_index)->second;
        cout << node.type << " " << node.width << " " << node.height << endl;
        for(const auto& child : node.children)
            print(child);
    }

    void test_print(const int& node_index, int x, int y, int bin_id, vector<Space>& spaces) const
    {
        const Node& node = nodes.find(node_index)->second;
        if(node.type > -1) spaces.push_back(Space(node.type + 1, bin_id, node.width, node.height, x, y));
        else if(node.children.empty()) spaces.push_back(Space(-1, bin_id, node.width, node.height, x, y));
        else {
            for(const auto& child : node.children)
                if(node.next_cut_orientation == Direction::Vertical) test_print(child, x, y, bin_id, spaces), x += nodes.find(child)->second.width;
                else test_print(child, x, y, bin_id, spaces), y += nodes.find(child)->second.height;
        }
    }

};

const bool cost_compare_le(const Cost& a, const Cost& b)
{
    if(a.excluded_item_area == b.excluded_item_area) return a.leftover_value >= b.leftover_value;
    return a.excluded_item_area < b.excluded_item_area;
}

const bool cost_compare_less(const Cost& a, const Cost& b)
{
    if(a.excluded_item_area == b.excluded_item_area) return a.leftover_value > b.leftover_value;
    return a.excluded_item_area < b.excluded_item_area;
}

namespace std{
    template<> struct tr1::hash<Layout>{
        size_t operator()(const Layout &layout) const{
            return layout.id;
        }
    };
}

struct InsertionOption{
    LayoutIndex layout_index; 
    int original_node_index; 
    ItemType itemtype;
    Rotation rotation;  

    //InsertionOption() {}
    InsertionOption(const LayoutIndex& layout_index_, const int& original_node_index_, const ItemType& itemtype_, const Rotation& rotation_) :
    layout_index(layout_index_), original_node_index(original_node_index_), itemtype(itemtype_), rotation(rotation_) {}

    bool operator==(const InsertionOption& other) const { return (layout_index == other.layout_index && original_node_index == other.original_node_index && itemtype == other.itemtype && rotation == other.rotation); }
};

struct Formulation{
    Instance instance; 
    vector<int> itemtype_qtys; 
    vector<int> bintype_qtys; 
    __gnu_pbds::gp_hash_table<int, Layout> layouts;
    //GenerationalIndexArray<Layout> layouts;
    vector<Layout> empty_layouts;
    vector<int> changed_layouts;
    optional<int> solution_id_changed_layouts;
    int layout_id_counter;
    int empty_layout_counter;
    int solution_id_counter; 

    //Formulation() {}
    Formulation(const Instance &instance_) : 
        layout_id_counter(-1), 
        solution_id_counter(-1), 
        empty_layout_counter(-1), 
        instance(instance_)
    {
        itemtype_qtys.reserve(instance.items.size());
        bintype_qtys.reserve(instance.bins.size());
        for(const auto& [_, qty] : instance.items) 
            itemtype_qtys.emplace_back(qty);
        for(const auto& [_, qty] : instance.bins)
            bintype_qtys.emplace_back(qty);
        // layouts.reserve(instance.items.size());
        empty_layouts.reserve(instance.bins.size() << 1);
        
        for(const auto& [bintype, _] : instance.bins) {
            if(bintype.fixed_first_cut_orientation == Direction::None) {
                Layout empty_layout_h(++empty_layout_counter, bintype, Direction::Horizontal);
                Layout empty_layout_v(++empty_layout_counter, bintype, Direction::Vertical);
                empty_layouts.emplace_back(empty_layout_h), empty_layouts.emplace_back(empty_layout_v);
            }
            else {
                Layout empty_layout(++empty_layout_counter, bintype, bintype.fixed_first_cut_orientation);
                empty_layouts.emplace_back(empty_layout);
            }
        }
    }

    IOCUpdates implement_insertion_blueprint(const InsertionBlueprint& blueprint)
    {
        --itemtype_qtys[blueprint.itemtype.id];

        if(blueprint.layout_index.type == LayoutType::Existing) {
            Layout& blueprint_layout = layouts.find(blueprint.layout_index.index)->second;
            IOCUpdates cache_updates(blueprint.layout_index);
            blueprint_layout.implement_insertion_blueprint(blueprint, instance, cache_updates);
            
            changed_layouts.reserve(changed_layouts.size() + 1);
            changed_layouts.emplace_back(blueprint_layout.id);

            return cache_updates;
        } else { // if(blueprint.layout_index.type == LayoutType::Empty)
            //这里和Rust不太一样，rust需要clone进行深复制，而C++只需要等号
            Layout empty_layout = empty_layouts[blueprint.layout_index.index];
            const int& layout_id = register_layout(empty_layout);
            LayoutIndex layout_index(LayoutType::Existing, layout_id);
            IOCUpdates cache_updates(layout_index);
            layouts.find(layout_id)->second.implement_insertion_blueprint(blueprint, instance, cache_updates);
            
            return cache_updates;
        }
    }

    const optional<Layout> remove_node(const int& node_index, const LayoutIndex& layout_index)
    {
        //assert(layout_index.type == LayoutType::Existing);
        changed_layouts.emplace_back(get_layout(layout_index).id);
        Layout& layout = layouts.find(layout_index.index)->second;
        if(node_index == layout.top_node_id) {
            return unregister_layout(layout_index);
        } 
        else {
            const vector<int>& removed_item_ids = layout.remove_node(node_index);
            for(const auto& item_id : removed_item_ids)
                ++itemtype_qtys[item_id];

            if(get_layout(layout_index).is_empty()) {
                return unregister_layout(layout_index);    
            } 
            return nullopt;
        }
    }

    const int& register_layout(Layout& layout) 
    {
        --bintype_qtys[layout.bintype.id];
        const vector<int>& all_item_ids = layout.get_included_items();
        for(const auto& id : all_item_ids)
            itemtype_qtys[id] -= 1;
        layout.id = ++layout_id_counter;

        changed_layouts.reserve(changed_layouts.size() + 1);
        changed_layouts.emplace_back(layout.id);
        //layouts.insert(make_pair(layout.id, layout));
        layouts.insert(make_pair(layout.id, layout));
        return layout.id;
    }

    const Layout unregister_layout(const LayoutIndex& layout_index)
    {
        //assert(layout_index.type == LayoutType::Existing);
        const Layout layout = get_layout(layout_index);

        layouts.erase(layout_index.index);

        ++bintype_qtys[layout.bintype.id];
        const vector<int>& included_items = layout.get_included_items();
        //cout << included_items.size() << endl;
        for(const auto& item : included_items)
            ++itemtype_qtys[item];

        return layout;
    }

    const Layout& get_layout(const LayoutIndex& layout_index) const
    {
        return layout_index.type == LayoutType::Existing ? layouts.find(layout_index.index)->second : empty_layouts[layout_index.index];
        // if(layout_index.type == LayoutType::Existing) return layouts[layout_index.index];
        // else return empty_layouts[layout_index.index];
    }

    const Cost cost()
    {
        Cost result(0, 0.0, 0, 0);
        for(auto& [_, layout] : layouts) {
            result += layout.cost();
        }
        for(int i = 0; i < instance.items.size(); ++i)
        {
            result.excluded_item_area += instance.items[i].first.area_ * itemtype_qtys[i];
        }
        result.included_item_area = instance.total_item_area - result.excluded_item_area;
        return result; 
    }

    const double usage() const
    {
        int total_included_item_area = 0;
        for(int i = 0; i < itemtype_qtys.size(); ++i) 
            total_included_item_area += instance.items[i].first.area_ * (instance.items[i].second - itemtype_qtys[i]);
        int total_used_bin_area = 0; 
        for(const auto& [_, layout] : layouts) 
            total_used_bin_area += layout.bintype.area;
        
        return (double) total_included_item_area / (double) total_used_bin_area;
    }
}; 



struct InsertionOptionCache{
    //multimap<pair<LayoutIndex, int>, InsertionOption, std::less<std::pair<LayoutIndex, int>>> option_node_map;
    __gnu_pbds::gp_hash_table<pair<LayoutIndex, int>, vector<InsertionOption>> option_node_map;
    __gnu_pbds::gp_hash_table<ItemType, vector<InsertionOption>> option_itemtype_map;
    
    void update_cache(const IOCUpdates& cache_updates, const vector<ItemType>& itemtypes, const Formulation& formulation)
    {
        const LayoutIndex& layout_index = cache_updates.layout_index;
        for(const auto& node_index : cache_updates.removed_nodes)
            remove_for_node(layout_index, node_index);
        const Layout& layout = formulation.get_layout(layout_index);
        for(const auto& node_index : cache_updates.new_nodes)
        {
            const Node& node = layout.nodes.find(node_index)->second;
            add_for_node(node_index, node, layout_index, itemtypes);
        }
    }//更新所有的选择

    void add_for_itemtypes(vector<ItemType> itemtypes, const vector<pair<LayoutIndex, Layout>>& layouts)
    {
        // auto start = chrono::system_clock::now();
        if(itemtypes.empty()) return;

        stable_sort(itemtypes.begin(), itemtypes.end());
        // for(const auto& item : itemtypes) cout << item.area_ << " ";
        // auto end = chrono::system_clock::now();
        // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);

        const unsigned int& itemtypes_len = itemtypes.size();
        vector<InsertionOption> generated_insertion_options;
        generated_insertion_options.reserve(itemtypes_len);
        // cout << layouts.size() << endl;
        // for(const auto& [layout_index, layout] : layouts)
        // {
        //     cout << layout_index.type << " ";
        //     for(const auto& [area, node_index] : layout.sorted_empty_nodes)
        //     {
        //         cout << area << " ";
        //     }
        //     cout << endl;
        // }
        for(const auto& [layout_index, layout] : layouts) {
            
            const auto sorted_empty_nodes = layout.sorted_empty_nodes;
            // for(const auto& node : sorted_empty_nodes) cout << node.first << " ";
            int starting_index = 0;
            
            for(const auto& [_, empty_node_index] : sorted_empty_nodes) {
                const Node& empty_node = layout.nodes.find(empty_node_index)->second;

                if(itemtypes.back().area_ > empty_node.area) break;

                generated_insertion_options.clear();
                for(int i = starting_index; i < itemtypes_len; ++i) {
                    const ItemType& itemtype = itemtypes[i];
                    if(empty_node.area < itemtype.area_)
                        ++starting_index;
                    else {
                        const optional<InsertionOption>& insertion_option = generate_insertion_option(empty_node, itemtype, layout_index, empty_node_index);
                        if(insertion_option.has_value())
                            generated_insertion_options.emplace_back(*insertion_option);
                    }
                }
                

                const pair<LayoutIndex, int>& node_key = make_pair(layout_index, empty_node_index); 
                auto& option_nodes = option_node_map[node_key];
                option_nodes.reserve(option_nodes.size() + generated_insertion_options.size());

                for(const auto& insertion_option : generated_insertion_options) {
                    const ItemType& itemtype = insertion_option.itemtype;
                    option_itemtype_map[itemtype].emplace_back(insertion_option);
                    option_nodes.emplace_back(insertion_option);
                }
            }
        }
    }

    const optional<InsertionOption> generate_insertion_option(const Node& node, const ItemType& itemtype, const LayoutIndex& layout_index, const int& node_index)
    {
        const bool& default_possible = node.insertion_possible(itemtype, Rotation::Default);
        const bool& rotated_possible = node.insertion_possible(itemtype, Rotation::Rotated);
        if (itemtype.fixed_rotation != Rotation::None) {
            if (itemtype.fixed_rotation == Rotation::Default && default_possible)
                return InsertionOption(layout_index, node_index, itemtype, Rotation::Default);
            else if (itemtype.fixed_rotation == Rotation::Rotated && rotated_possible)
                return InsertionOption(layout_index, node_index, itemtype, Rotation::Rotated);
            else
                return nullopt;
        }
        else {
            if (default_possible && rotated_possible)
                return InsertionOption(layout_index, node_index, itemtype, Rotation::None);
            else if (default_possible)
                return InsertionOption(layout_index, node_index, itemtype, Rotation::Default);
            else if (rotated_possible)
                return InsertionOption(layout_index, node_index, itemtype, Rotation::Rotated);
            else
                return nullopt;
        }
    }
    

    void add_for_node(const int& node_index, const Node& node, const LayoutIndex& layout_index, const vector<ItemType>& itemtypes)
    {
        //assert(node.type >= -1);
        if(node.type == -1 && node.children.empty()) {
            const pair<LayoutIndex, int> node_key(layout_index, node_index);
            auto& option_nodes = option_node_map[node_key];
            option_nodes.reserve(option_nodes.size() + itemtypes.size());
            for(const auto& itemtype : itemtypes) {
                const optional<InsertionOption>& insertion_option = generate_insertion_option(node, itemtype, layout_index, node_index);
                if(!insertion_option.has_value()) continue;
                option_nodes.emplace_back(*insertion_option);
                //option_itemtype_map[itemtype].reserve(option_itemtype_map[itemtype].size() + 1);
                option_itemtype_map.find(itemtype)->second.emplace_back(*insertion_option);
            }
        }
    }

    void remove_for_node(const LayoutIndex& layout_index, const int& node_index)
    {
        const pair<LayoutIndex, int> node_key(layout_index, node_index);
        const auto& insertion_options = option_node_map[node_key];
        for(const auto& insertion_option : insertion_options)
        {
            const ItemType& itemtype = insertion_option.itemtype;

            auto& itemtype_options = option_itemtype_map.find(itemtype)->second;
            
            remove(itemtype_options.begin(), itemtype_options.end(), insertion_option);
            itemtype_options.pop_back();

            if(itemtype_options.empty()) option_itemtype_map.erase(itemtype);
        }
        option_node_map.erase(node_key);
    }

    void remove_all_for_layout(const LayoutIndex& layout_index, const Layout& layout)
    {
        for(const auto& [_, empty_node_index] : layout.sorted_empty_nodes)
            remove_for_node(layout_index, empty_node_index);
    }

    const bool is_empty() const
    {
        return option_itemtype_map.empty() && option_node_map.empty();
    }
};

const vector<InsertionBlueprint> generate_blueprints(const InsertionOption& insertion_option, const Formulation& formulation)
{
    const Layout& layout = formulation.get_layout(insertion_option.layout_index);
    //assert(layout.nodes.is_live(insertion_option.original_node_index));
    Node original_node = layout.nodes.find(insertion_option.original_node_index)->second;
    
    vector<vector<NodeBlueprint>> node_blueprints;
    node_blueprints.reserve(4);
    if(insertion_option.rotation == Rotation::None)  {
        original_node.generate_insertion_node_blueprints(insertion_option.itemtype, Rotation::Default, node_blueprints);
        original_node.generate_insertion_node_blueprints(insertion_option.itemtype, Rotation::Rotated, node_blueprints);
    }
    else original_node.generate_insertion_node_blueprints(insertion_option.itemtype, insertion_option.rotation, node_blueprints);

    const Cost& original_cost = original_node.calculate_cost();
    
    vector<InsertionBlueprint> result;
    result.reserve(node_blueprints.size());
    for(const auto& vec_blueprint : node_blueprints)
    {
        Cost new_cost(0, 0.0, 0, 0);
        for(const auto& blueprint : vec_blueprint)
            new_cost = new_cost + blueprint.calculate_cost();
        const Cost& insertion_cost = new_cost - original_cost;
        result.emplace_back(InsertionBlueprint(insertion_option.layout_index, insertion_option.original_node_index, vec_blueprint, insertion_option.itemtype, insertion_cost));
    }
    return result;
}

struct ProblemSolution{
    Instance instance;
    __gnu_pbds::gp_hash_table<int, Layout> layouts;
    Cost cost;
    int id; 
    vector<int> itemtype_qtys; 
    vector<int> bintype_qtys;
    double usage; 

    ProblemSolution(const Formulation& formulation, const Cost& cost, const int& id, const ProblemSolution& prev_solution) :
    instance(formulation.instance), cost(cost), id(id), itemtype_qtys(formulation.itemtype_qtys), bintype_qtys(formulation.bintype_qtys), usage(formulation.usage())
    {
        for(const auto& [_, layout] : formulation.layouts) {
            const int& layout_id = layout.id;
            if(find(formulation.changed_layouts.begin(), formulation.changed_layouts.end(), layout_id) != formulation.changed_layouts.end()) {
                layouts.insert(make_pair(layout_id, layout));
            } else {
                const Layout& prev_solution_layout = prev_solution.layouts.find(layout_id)->second;
                layouts.insert(make_pair(layout_id, prev_solution_layout));
            }
        }
    }

    ProblemSolution(const Formulation& formulation, const Cost& cost, const int& id) :
    instance(formulation.instance), cost(cost), id(id), itemtype_qtys(formulation.itemtype_qtys), bintype_qtys(formulation.bintype_qtys), usage(formulation.usage())
    {
        for(const auto& [_, layout] : formulation.layouts) {
            const int& layout_id = layout.id;
                layouts.insert(make_pair(layout_id, layout));
        }
    }
    
    const bool is_complete() const 
    {
        return cost.excluded_item_area == 0;
    }
};

const ProblemSolution create_solution(Formulation& formulation, const optional<ProblemSolution>& old_solution, const optional<Cost> cached_cost)
{
    const int& id = ++formulation.solution_id_counter;
    const Cost& cost = cached_cost.has_value() ? *cached_cost : formulation.cost();
    const ProblemSolution& solution = old_solution.has_value() ? ProblemSolution(formulation, cost, id, *old_solution) : ProblemSolution(formulation, cost, id); 
    
    formulation.changed_layouts.clear();
    formulation.solution_id_changed_layouts = solution.id;
   
    return solution; 
}

void restore_from_problem_solution(Formulation& formulation, const ProblemSolution& solution)
{
    // if(formulation.solution_id_changed_layouts.has_value()) {
    //     vector<int> present_layout_ids;
    //     vector<int> changed_layout_indices;
    //     vector<int> absent_layout_indices; 

    //     for(const auto& [index, layout] : formulation.layouts)
    //     {
    //         const int& layout_id = layout.id;
    //         cout << index << " " << layout_id <<endl;
    //         if(solution.layouts.find(layout_id) != solution.layouts.end()) {
    //             if(find(formulation.changed_layouts.begin(), formulation.changed_layouts.end(), layout_id) != formulation.changed_layouts.end())
    //                 changed_layout_indices.emplace_back(index);
    //             present_layout_ids.emplace_back(layout_id);
    //         } else {
    //             absent_layout_indices.emplace_back(layout_id);
    //         }
    //     }

    //     for(const auto& index : absent_layout_indices) 
    //         formulation.layouts.erase(index);

    //     for(const auto& index : changed_layout_indices) {
    //         const Layout& layout = formulation.layouts.find(index)->second;
    //         formulation.layouts.erase(index);
    //         const Layout& copy = solution.layouts.find(layout.id)->second;
    //         const int& layout_id = ++formulation.layout_id_counter;
    //         cout << layout_id << endl;
    //         formulation.layouts.insert(make_pair(layout_id, copy));
    //     }
        
    //     for(const auto& [id, _] : solution.layouts) {
    //         if(find(present_layout_ids.begin(), present_layout_ids.end(), id) == present_layout_ids.end()) {
    //             const Layout& copy = solution.layouts.find(id)->second;
    //             const int& layout_id = ++formulation.layout_id_counter;
    //             formulation.layouts.insert(make_pair(layout_id, copy));
    //         }
    //     }

    // } else {
        formulation.layouts.clear();
        for(const auto& [layout_id, layout] : solution.layouts) {
            // const Layout& copy = layout;
            // const int& layout_id = ++formulation.layout_id_counter;
            formulation.layouts.insert(make_pair(layout_id, layout));
        }
    //}

    formulation.itemtype_qtys = solution.itemtype_qtys;
    formulation.bintype_qtys = solution.bintype_qtys;

    formulation.changed_layouts.clear();
    formulation.solution_id_changed_layouts = solution.id;
}

struct SolutionCollector {
    Instance instance; 
    optional<ProblemSolution> best_complete_solution; 
    optional<ProblemSolution> best_incomplete_solution;
    optional<int> material_limit;
    
    SolutionCollector(const Instance& I) : instance(I), best_complete_solution(nullopt), best_incomplete_solution(nullopt), material_limit(nullopt) {}

    void report_problem_solution(const ProblemSolution& solution) 
    {
        if(best_incomplete_solution.has_value()) {
            if(cost_compare_less(solution.cost, (*best_incomplete_solution).cost))
                accept_solution(solution);
        } else {
            if(solution.cost.material_cost < (material_limit.has_value() ? *material_limit : INT_MAX))
                accept_solution(solution);
        }
    }

    void accept_solution(const ProblemSolution& solution)
    {
        if(solution.is_complete()) {
            lower_mat_limit(solution.cost.material_cost);
            best_complete_solution = solution;
        } else {
            best_incomplete_solution = solution;
        }
    }

    void lower_mat_limit(const int& mat_limit)
    {
        material_limit = mat_limit;
        best_incomplete_solution = nullopt;
    }
};

struct GDRR{
    Instance instance;
    Formulation formulation;
    SolutionCollector solution_collector;

    GDRR(const Instance& instance_) : 
        instance(instance_), 
        formulation(Formulation(instance)),
        solution_collector(instance) {}

    void recreate(int mat_limit_budget, const int& max_item_area_excluded);
    const ItemType& select_next_itemtype(const vector<ItemType>& itemtypes, const InsertionOptionCache& insertion_option_cache);
    const optional<InsertionBlueprint> select_insertion_blueprint(const ItemType& itemtype, const InsertionOptionCache& insertion_option_cache, const int& mat_limit_budget);

    int ruin(int mat_limit_budget);

    void lahc();
};

const int select_lowest_entry(const vector<int>& entries)
{
    int lowest_value = entries.front();
    int selected_index = 0;
    
    for(const auto& entry : entries) {
        if(entry < lowest_value && static_cast<double>(rng()) / RAND_MAX > BLINK_RATE) {
            lowest_value = entry;
            selected_index = &entry - &entries[0];
        }
    }
    return selected_index;
}

const int select_lowest_range(const int& n)
{
    for(int i = 0; i < n; ++i)
    {
        if(static_cast<double>(rng()) / RAND_MAX > BLINK_RATE)
            return i;
    }
    return n - 1;
    //return selected_index;
}

const int N = 3; 

const optional<int> biased_sample(const vector<pair<double, int>> entries)
{
    if(entries.empty()) return nullopt;

    vector<pair<double, int>> samples;
    samples.reserve(N);
    for(int i = 0; i < N; ++i) 
        samples.emplace_back(entries[rng() % entries.size()]);
    stable_sort(samples.begin(), samples.end());
    double sample_rate = static_cast<double>(rng()) / RAND_MAX;
    if(sample_rate <= 0.625) return samples[0].second;
    if(sample_rate <= 0.875) return samples[1].second;
    return samples[2].second;
}

void convert_into_html(const char* classfile, const int& bin_width, const int& bin_height, const int& bin_num, vector<Space>& spaces)
{
    string outfilename = classfile;
    int st = outfilename.find_last_of("/") >= outfilename.size() ? 0 : outfilename.find_last_of("/");
    outfilename = outfilename.substr(st, outfilename.find_last_of("."));
    outfilename = outfilename + ".html";
    std::ofstream outfile(outfilename);

    const int total_area = bin_width * bin_height * bin_num;

    int leftover = 0;
    for(int i = 0; i < spaces.size(); ++i)
    {
        spaces[i].y = bin_height - spaces[i].height - spaces[i].y;
        if(spaces[i].index == -1) leftover += spaces[i].width * spaces[i].height;
    }

    sort(spaces.begin(), spaces.end());

    double Dicxy = bin_width * 0.002;
    string binbox="<div style=\"width:1000px;\"><svg height=\"100%\" viewBox=\""+to_string(-1*Dicxy)+' '+to_string(-1*Dicxy)+' '+to_string(bin_width+2*Dicxy)+' '+to_string(bin_height+2*Dicxy)+"\" width=\"100%\" xmlns=\"http://www.w3.org/2000/svg\">\n<g>\n";
    double Usage=(total_area-leftover) * 1.0 / total_area;

    outfile << "<!DOCTYPE html><html style=\"font-family:Arial\"><head><title>Solution large_example</title></head><body><h1>Solution</h1><h2>Statistics</h2><table><tr><th style=\"text-align:left\">Usage</th><td>"+to_string(100*Usage)+"%</td></tr><tr><th style=\"text-align:left\">Part area included</th><td>100.000%</td></tr><tr><th style=\"text-align:left\"># Objects used</th><td>"+to_string(bin_num)+"</td></tr><tr><th style=\"text-align:left\">Material cost</th><td>"+to_string(total_area)+"</td></tr></table>\n";
    outfile << "<h2>Cutting Patterns</h2>\n";
    int curspace = -1;
    for(auto space : spaces)
    {
        if(curspace < space.bin_index)
        {
            curspace = curspace + 1;
            if(curspace > 0)
                outfile << "</g></svg></div>\n";
            outfile << "<h3>Pattern "+ to_string(curspace) + "</h3>\n";
            outfile << binbox;
        }
        outfile<< "<g>\n";
        
        if(space.index > 0)
        {
            outfile<<"<rect fill=\"#BFBFBF\" height=\""+to_string(space.height)+"\" stroke=\"black\" stroke-width=\""+to_string(Dicxy)+"\" width=\""+to_string(space.width)+"\" x=\""+to_string(space.x)+"\" y=\""+to_string(space.y)+"\"/>\n";
            if(space.width>=space.height)
                outfile << "<text dominant-baseline=\"middle\" fill=\"black\" font-size=\"" + to_string(get_font_size(space.width,space.height)) + "em\" text-anchor=\"middle\" x=\""+to_string(space.width/2.0+space.x)+"\" y=\""+to_string(space.height/2.0+space.y)+"\">\n";
            else
                outfile << "<text dominant-baseline=\"middle\" fill=\"black\" font-size=\"" + to_string(get_font_size(space.width,space.height)) + "em\" text-anchor=\"middle\" transform=\"rotate(-90 "+to_string(space.width/2.0+space.x)+' '+to_string(space.height/2.0+space.y)+")\" x=\"" + to_string(space.width/2.0+space.x)+"\" y=\"" + to_string(space.height/2.0+space.y)+"\">\n";
            outfile << to_string(space.index)+": ["+to_string(space.width)+"x"+to_string(space.height)+"]\n";
            outfile << "</text>\n";
        }else
            outfile << "<rect fill=\"#A9D18E\" height=\""+to_string(space.height)+"\" stroke=\"black\" stroke-width=\""+to_string(Dicxy)+"\" width=\""+to_string(space.width)+"\" x=\""+to_string(space.x)+"\" y=\""+to_string(space.y)+"\"/>\n";
        outfile << "</g>\n";
    }
    outfile << "</g></svg></div>\n";
    outfile << "</svg></div></body></html>";
}