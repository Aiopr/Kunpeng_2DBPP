#include <lahc.h>

using namespace std;

//clock_t start, finish;

ofstream result, outfile;

void GDRR::recreate(int mat_limit_budget, const int& max_item_area_excluded)
{
    //Collect all items that need to be considered
    
    // start = chrono::system_clock::now();

    vector<ItemType> itemtypes_to_consider;
    itemtypes_to_consider.reserve(formulation.itemtype_qtys.size());
    for(int i = 0; i < formulation.itemtype_qtys.size(); ++i) 
        if(formulation.itemtype_qtys[i] > 0)
            itemtypes_to_consider.emplace_back(instance.items[i].first);

    //Collect all layouts that should be considered
    vector<pair<LayoutIndex, Layout>> layouts_to_consider;
    layouts_to_consider.reserve(formulation.layouts.size() + formulation.empty_layouts.size());
    for(const auto& [_, layout] : formulation.layouts)
        layouts_to_consider.emplace_back(make_pair(LayoutIndex(LayoutType::Existing, layout.id), layout));
    for(const auto& layout : formulation.empty_layouts)
        if(formulation.bintype_qtys[layout.bintype.id] > 0) 
            layouts_to_consider.emplace_back(make_pair(LayoutIndex(LayoutType::Empty, layout.id), layout));
    
    int item_area_not_included = 0;
    InsertionOptionCache insertion_option_cache;
    // auto end = chrono::system_clock::now();
    // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
    // int cnt = 0;
    // for(const auto&[_, layout] : formulation.layouts)
    // {
    //     for(const auto& [_, node_index] : layout.sorted_empty_nodes)
    //     {
    //         const Node& node = layout.nodes.find(node_index)->second;
    //         cout << cnt << " " << node.type << " " << node.width << " " << node.height << endl; 
    //     }
    //     ++cnt;
    // }
    insertion_option_cache.add_for_itemtypes(itemtypes_to_consider, layouts_to_consider);
    // int sum = 0;
    // int counter = 0;
    // for(const auto &[_, layout] : formulation.layouts)
    // {
    //     layout.test_print(layout.top_node_id, 0, 0, counter);
    //     counter = counter + 1;
    // }
    // for(const auto& itemtype : itemtypes_to_consider)
    // {
    //     const vector<InsertionOption>& itemtype_options = insertion_option_cache.option_itemtype_map.find(itemtype)->second;
    //     cout << itemtype << endl;
    //     for(const auto& option : itemtype_options)
    //     {
    //         if(option.layout_index.type == LayoutType::Existing)
    //         {
    //             const Node& node = formulation.get_layout(option.layout_index).nodes.find(option.original_node_index)->second;
    //             cout << " " << node.type << " " << node.width << " " << node.height << endl;
    //         }
    //     }
    // }
    
    // for(const auto& [_, itemtype_options]:insertion_option_cache.option_itemtype_map)
    // sum += itemtype_options.size();
    // cout << sum << endl;
    // auto end = chrono::system_clock::now();
    // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);

    while(!itemtypes_to_consider.empty() && item_area_not_included <= max_item_area_excluded) {
        const ItemType& elected_itemtype = select_next_itemtype(itemtypes_to_consider, insertion_option_cache);
        // auto end = chrono::system_clock::now();
        // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
        // exit(0);
        
        const optional<InsertionBlueprint>& elected_blueprint_ = select_insertion_blueprint(elected_itemtype, insertion_option_cache, mat_limit_budget);

        // auto end = chrono::system_clock::now();
        // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
        // exit(0);

        
        
        if(elected_blueprint_.has_value()) {
            const InsertionBlueprint& elected_blueprint = *elected_blueprint_;
            // auto end = chrono::system_clock::now();
            // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
            // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
            // exit(0);

            const IOCUpdates& cache_updates = formulation.implement_insertion_blueprint(elected_blueprint);
            // auto end = chrono::system_clock::now();
            // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
            // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
            // exit(0);

            insertion_option_cache.update_cache(cache_updates, itemtypes_to_consider, formulation);
            
            // auto end = chrono::system_clock::now();
            // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
            // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
            // exit(0);

            if(elected_blueprint.layout_index.type == LayoutType::Empty) {
                const Layout& empty_layout = formulation.empty_layouts[elected_blueprint.layout_index.index];
                mat_limit_budget -= empty_layout.bintype.area;

                if(formulation.bintype_qtys[empty_layout.bintype.id] == 0)
                    insertion_option_cache.remove_all_for_layout(elected_blueprint.layout_index, empty_layout);
            }

            if(formulation.itemtype_qtys[elected_itemtype.id] == 0)
                {
                    remove(itemtypes_to_consider.begin(), itemtypes_to_consider.end(), elected_itemtype);
                    itemtypes_to_consider.pop_back();
                }
            
            if(insertion_option_cache.is_empty()) break;

        } else {
            //cout << elected_itemtype << endl;
            item_area_not_included += formulation.itemtype_qtys[elected_itemtype.id] * elected_itemtype.area_;
            // int counter = 0;
            // for(const auto &[_, layout] : formulation.layouts)
            // {
            //     layout.test_print(layout.top_node_id, 0, 0, counter);
            //     counter = counter + 1;
            // }
            // cout << elected_itemtype << endl;
            remove(itemtypes_to_consider.begin(), itemtypes_to_consider.end(), elected_itemtype);
            itemtypes_to_consider.pop_back();
        }
    }
    // cout << item_area_not_included << endl;
    // cout << endl;
    // for(const auto& itemtype : itemtypes_to_consider)
    // cout << itemtype << endl;
    // auto end = chrono::system_clock::now();
    // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    // result << double(duration.count()) * chrono::microseconds::period::num << " ";
}

const ItemType& GDRR::select_next_itemtype(const vector<ItemType>& itemtypes, const InsertionOptionCache& insertion_option_cache)
{
    const int& len = itemtypes.size();
    vector<int> indices(len);
    iota(indices.begin(), indices.end(), 0); // fill with [0, 1, ..., n-1]
    shuffle(indices.begin(), indices.end(), rng);
    // for (int i = len - 1; i > 0; --i) {
    //     const int j = rng() % i;
    //     std::swap(indices[i], indices[j]);
    // }
    // auto end = chrono::system_clock::now();
    // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
    // exit(0);

    vector<int> n_options;
    n_options.reserve(len);
    for(const int& i : indices)
    if(insertion_option_cache.option_itemtype_map.find(itemtypes[i]) != insertion_option_cache.option_itemtype_map.end())
        n_options.emplace_back(insertion_option_cache.option_itemtype_map.find(itemtypes[i])->second.size());
    else 
        n_options.emplace_back(0);
    // for(const auto& option : n_options)
    //     cout << option << " ";
    // cout << endl;
    int blink = select_lowest_entry(n_options);

    return itemtypes[indices[blink]];
}

const optional<InsertionBlueprint> GDRR::select_insertion_blueprint(const ItemType& itemtype, const InsertionOptionCache& insertion_option_cache, const int& mat_limit_budget)
{
    const vector<InsertionOption>& insertion_options = insertion_option_cache.option_itemtype_map.find(itemtype)->second;
    if(!insertion_options.empty()) {
        vector<InsertionBlueprint> existing_layout_blueprints; 
        existing_layout_blueprints.reserve(10);
        vector<InsertionBlueprint> new_layout_blueprints; 
        new_layout_blueprints.reserve(2);
        for(const auto& option : insertion_options) {
            if(existing_layout_blueprints.size() > 10) break;
            
            if(option.layout_index.type == LayoutType::Existing) 
                {
                    const vector<InsertionBlueprint>& all_blueprints = generate_blueprints(option, formulation);
                    for(const auto& blueprint : all_blueprints)
                    {   
                        //existing_layout_blueprints.reserve(existing_layout_blueprints.size() + 1);
                        existing_layout_blueprints.emplace_back(blueprint);
                    }
                }
            else if(option.layout_index.type == LayoutType::Empty && mat_limit_budget >= formulation.empty_layouts[option.layout_index.index].bintype.area)
                {
                    const vector<InsertionBlueprint>& all_blueprints = generate_blueprints(option, formulation);

                    for(const auto& blueprint : all_blueprints)
                    {
                        //new_layout_blueprints.reserve(new_layout_blueprints.size() + 1);
                        new_layout_blueprints.emplace_back(blueprint);
                    }
                }
        }
        // auto end = chrono::system_clock::now();
        // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
        // exit(0);

        if(existing_layout_blueprints.empty()) {
            if(new_layout_blueprints.empty()) {
                return nullopt;
            } else {
                //int selected_index = static_cast<double>(rand()) / RAND_MAX * new_layout_blueprints.size();
                int selected_index = static_cast<double>(rng()) / RAND_MAX * new_layout_blueprints.size();
                return new_layout_blueprints[selected_index];
            }
        } else {
            stable_sort(existing_layout_blueprints.begin(), existing_layout_blueprints.end());

            int blink = select_lowest_range(existing_layout_blueprints.size());
            return existing_layout_blueprints[blink];
        }
    } else {
        return nullopt;
    }
}

int GDRR::ruin(int mat_limit_budget)
{
    //auto start = chrono::system_clock::now();
    const int& n_nodes_to_remove = static_cast<double>(rng()) / RAND_MAX * (2 * AVG_NODES_REMOVED - 5) + 4;
    if(mat_limit_budget >= 0) {
        for(int i = 0; i < n_nodes_to_remove; ++i) {
            vector<pair<double, int>> entries;
            entries.reserve(formulation.layouts.size());
            for(auto &[id, layout] : formulation.layouts) {
                entries.emplace_back(make_pair(layout.usage(), id));
                //cout << layout.usage() << " ";
            }
            //cout << endl;
            const optional<int>& selected_layout = biased_sample(entries);
            
            if(!selected_layout.has_value()) break;
            //cout << formulation.layouts.find(*selected_layout)->second.usage() << endl;
            const vector<int> removable_nodes = formulation.layouts.find(*selected_layout)->second.get_removable_nodes();
            // for(const auto& node_index : removable_nodes)
            // {
            //     const Node& node = formulation.layouts.find(*selected_layout)->second.nodes.find(node_index)->second;
            //     cout << node.type << " " << node.width << " " << node.height << endl;
            // }
            // cout << endl;
            //if(removable_nodes.empty()) continue;
            const int& selected_node = removable_nodes[static_cast<double>(rng()) / RAND_MAX * removable_nodes.size()];

            const Node& node = formulation.layouts.find(*selected_layout)->second.nodes.find(selected_node)->second;
            //cout << node.type << " " << node.width << " " << node.height << endl;
            //cout << rng() % removable_nodes.size() << endl;
            //cout << "debug1" << endl;
            const optional<Layout>& removed_layout = formulation.remove_node(selected_node, LayoutIndex(LayoutType::Existing, *selected_layout));
            //cout << "debug2" << endl;
            if(removed_layout.has_value())
                mat_limit_budget += (*removed_layout).bintype.area;
        }
    } else {
        while(mat_limit_budget < 0) {
            
            int min_usage_layout_index = -1;
            double min_usage = 1;
            for(auto& [layout_index, layout] : formulation.layouts)
            {
                if(layout.usage() < min_usage) min_usage = layout.usage(), min_usage_layout_index = layout.id;
            }
            if(min_usage_layout_index == -1) break;
            const int& top_node_index = formulation.layouts.find(min_usage_layout_index)->second.top_node_id;
            
            const optional<Layout>& removed_layout = formulation.remove_node(top_node_index, LayoutIndex(LayoutType::Existing, min_usage_layout_index));
            // auto end = chrono::system_clock::now();
            // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
            // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
            

            if(removed_layout.has_value())
                mat_limit_budget += (*removed_layout).bintype.area;
            else
                throw "Top node should remove entire layout!";
        }
    }

    // auto end = chrono::system_clock::now();
    // auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    // printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);

    return mat_limit_budget;
}

void GDRR::lahc()
{
    auto start = chrono::system_clock::now();
    
    const int& max_rr_iterations = MAX_RR_ITEATIONS;
    
    const Cost empty_problem_cost(0, 0.0, instance.total_item_area, 0);

    deque<Cost> lahc_history; 
    
    lahc_history.push_back(empty_problem_cost);

    int n_iterations = 0;
    int n_accepted = 0;
    int n_improved = 0;
    int mat_limit = INT_MAX;
    optional<ProblemSolution> local_optimal = nullopt;
    

    auto end = chrono::system_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end - start);

    while(n_iterations < max_rr_iterations && double(duration.count()) * chrono::seconds::period::num < MAX_RUN_TIME) {
    //while(n_iterations < max_rr_iterations) {
        int mat_limit_budget = 0;

        //cout << "Iterations: " << n_iterations << endl;
        
        if(local_optimal.has_value()) mat_limit_budget = mat_limit - 1 - (*local_optimal).cost.material_cost;
        else mat_limit_budget = mat_limit - 1 - formulation.cost().material_cost;
        
        mat_limit_budget = ruin(mat_limit_budget);

        //cout << mat_limit_budget << endl;
        // for(const auto& itemtype_qty : formulation.itemtype_qtys)
        // cout << itemtype_qty << " ";
        // cout << endl;
        //cout << "After ruin:" << formulation.layouts.size() << endl;
        int max_item_area_excluded = lahc_history.front().excluded_item_area;
        if(local_optimal.has_value())
        {
            max_item_area_excluded = max(max_item_area_excluded, (*local_optimal).cost.excluded_item_area);
        }
        
        //cout << mat_limit_budget << " " << max_item_area_excluded << endl;
        //cout << "Before recreate: " << formulation.cost() << endl;
        
        recreate(mat_limit_budget, max_item_area_excluded);
        //if(n_iterations == 2) {
            // int counter = 0;
            // for(const auto &[_, layout] : formulation.layouts)
            // {
            //     layout.test_print(layout.top_node_id, 0, 0, counter);
            //     counter = counter + 1;
            // }
            // for(int i = 0; i < formulation.itemtype_qtys.size(); ++i)
            // {
            //     const int& itemtype = formulation.itemtype_qtys[i];
            //     if(itemtype > 0) cout << itemtype << " " << instance.items[i].first << endl;
            // }
            //exit(0);
        //}
        
        //cout << "After recreate:  " << formulation.cost() << endl << endl;
        //cout << "After recreate:" << formulation.layouts.size() << endl;
        const Cost& cost = formulation.cost();
        // result << cost << endl;
        // result << lahc_history.front() << endl;
        // result << endl;
        // cout << formulation.layouts.size() << endl;
        // int sum = 0;
        // for(const auto& itemtype_qty : formulation.itemtype_qtys)
        // sum += itemtype_qty;
        // int cnt = 0;
        // for(const auto& [_, layout] : formulation.layouts)
        // {
        //     cnt += layout.get_included_items().size();
        // }
        // cout << sum << " " << cnt << endl;

        // if(local_optimal.has_value()) 
        // {
        //     cout << "Line1: " << cost << endl;
        //     cout << "Line2: " << (*local_optimal).cost << endl;
        // }
        if(cost_compare_le(cost, lahc_history.front()) || (local_optimal.has_value() && cost_compare_le(cost, (*local_optimal).cost))) 
        {
            // if(local_optimal.has_value()) 
            // {
            //     cout << "Line3: " << cost << endl;
            //     cout << "Line4: " << (*local_optimal).cost << endl;
            // }
            local_optimal = create_solution(formulation, local_optimal, cost);
            
            lahc_history.pop_front();

            if(cost_compare_less(cost, (lahc_history.empty() ? empty_problem_cost : lahc_history.back()))) {
                const int& len = lahc_history.size();
                for(int i = 0; i < HISTORY_LENGTH - len; ++i) 
                    lahc_history.emplace_back(cost);
                //cout << (*local_optimal).layouts.size() << endl;
                solution_collector.report_problem_solution(*local_optimal);
                n_improved += 1;
            } else {
                const Cost& best = lahc_history.empty() ? empty_problem_cost : lahc_history.back();
                const int& len = lahc_history.size();
                for(int i = 0; i < HISTORY_LENGTH - len; ++i) {
                    lahc_history.emplace_back(best);
                }
            }
            n_accepted += 1;
        } 
        else {
            // cout << formulation.cost() << endl;
            // cout << (*local_optimal).cost << endl;
            restore_from_problem_solution(formulation, *local_optimal);
        }
        if(solution_collector.material_limit < mat_limit) {
            mat_limit = *solution_collector.material_limit;
            local_optimal = nullopt;
            lahc_history.clear();
            lahc_history.emplace_back(empty_problem_cost);
        }
        n_iterations += 1;
        end = chrono::system_clock::now();
        duration = chrono::duration_cast<chrono::seconds>(end - start);
    }
    cout << "iterations: " << n_iterations << " " << (double)n_iterations / MAX_RUN_TIME << endl;
    cout << "accept: " << n_accepted << " " << (double)n_accepted / MAX_RUN_TIME << endl;
    cout << "improve: " << n_improved << " " << (double)n_improved / MAX_RUN_TIME << endl;
}


int main(int argc, char **argv)
{
    srand(time(nullptr));
    Instance instance(argv[1]);
    

    result.open("Solution.csv", std::ios::app);

    auto start = chrono::system_clock::now();
    // for(const auto& item : instance.items)
    // {
    //     result << item.first << endl;
    // }
    GDRR gdrr(instance);
    gdrr.lahc();
    result << (*gdrr.solution_collector.best_complete_solution).layouts.size() << endl;
    result.close();

    auto end = chrono::system_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    printf("%.0lf us\n",double(duration.count()) * chrono::microseconds::period::num);
    int counter = 0;

    
    vector<Space> spaces;
    for(const auto &[_, layout] : (*gdrr.solution_collector.best_complete_solution).layouts)
    {
        layout.test_print(layout.top_node_id, 0, 0, counter, spaces);
        ++counter;
    }

    convert_into_html(argv[1], instance.bins[0].first.width, instance.bins[0].first.height,  (*gdrr.solution_collector.best_complete_solution).layouts.size(), spaces);
    outfile.close();

}