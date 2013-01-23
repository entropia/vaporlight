#include "colors.hpp"

#include <sstream>
#include <set>

using namespace vlpp;

std::vector<rgba_color> str_to_cols(const std::string& str){
	using std::string;
	using std::set;
	set<rgba_color> colors;
	std::istringstream data(str);
	string tmp;
	while(getline(data, tmp, ',')){
		auto key_it = COLOR_SETS_MAP.find(tmp);
		if(key_it != COLOR_SETS_MAP.end()){
			for(auto& col: key_it->second){
				colors.insert(col);
			}
		}
		else{
			colors.insert(str_to_col(tmp));
		}
	}
	return {colors.begin(), colors.end()};
}

vlpp::rgba_color str_to_col(const std::string& str){
	auto col_map_it = COLOR_MAP.find(str);
	if(col_map_it != COLOR_MAP.end()){
		return col_map_it->second;
	}
	else{
		return {str};
	}
}
