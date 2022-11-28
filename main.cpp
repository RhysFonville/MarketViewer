#include <iostream>
#include <string>
#include <string.h>
#include <functional>
#include <fstream>
#include <algorithm>
#include "curl/curl.h"
#include "gumbo.h"

const std::string CURRENT_VALUE_CLASS_NAME = "value";

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string request(const std::string &url) {
	CURL *curl;
	CURLcode res;
	std::string read_buffer;
	
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}	
	return read_buffer;
}

std::string get_element_text_value(GumboNode *node) {
	std::string ret;
	GumboNode* text = static_cast<GumboNode*>(node->v.element.children.data[0]);
	if (text->type == GUMBO_NODE_TEXT) {
		ret = text->v.text.text;
	}
	return ret;
}

bool loop_recursively(GumboNode *node, const std::function<bool(GumboNode*)> &func) {
	if (node != nullptr) {
		if (node->type == GumboNodeType::GUMBO_NODE_ELEMENT) {
			for (int i = 0; i < node->v.element.children.length; i++) {
				if (!func(node)) return false;
				if (!loop_recursively((GumboNode*)node->v.element.children.data[i], func)) return false;
			}
		}
	}
	return true;
}

bool get_current_value(GumboNode *node) {
	static bool in_intraday_price = false;
	if (node->type == GumboNodeType::GUMBO_NODE_ELEMENT) {
		GumboAttribute *attribute = gumbo_get_attribute(&node->v.element.attributes, "class");
		if (attribute != nullptr) {
			if (std::string(attribute->value) != "intraday__price ") {	
				if (std::string(attribute->value) == CURRENT_VALUE_CLASS_NAME && in_intraday_price) {
					in_intraday_price = false;
					std::string text = get_element_text_value(node);
					if (!text.empty()) {
						std::cout << "Stock current value: " << text << std::endl;
						return false;
					}
				}
			} else {
				in_intraday_price = true;
			}
		}
	}
	return true;
}

std::string search_string_to_link(const std::string arg1, const std::string &symbol) {
	return "https://www.marketwatch.com/investing/" + arg1 + "/" + symbol;
}

int main(int argc, char *argv[]) {
	if (argc > 2) {
		std::string link = search_string_to_link(argv[1], argv[2]);
		std::cout << "Link: " << link << std::endl;
		std::string html = request(link);
		
		std::ofstream of("htmlsrc.html", std::ios::trunc);
		of << html;
		of.close();
	
		GumboOutput *output = gumbo_parse(html.c_str());
		
		loop_recursively(output->root, std::function<bool(GumboNode*)>(get_current_value));
		
		gumbo_destroy_output(&kGumboDefaultOptions, output);
	
		return 0;
	} else {
		std::cerr << "Specify the stock you want info on.\n";
	}
}
