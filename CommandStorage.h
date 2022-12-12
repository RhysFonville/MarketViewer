#include <algorithm>
#include "ecmd/Command.h"
#include "gumbo.h"
#include "curl/curl.h"

const std::string CURRENT_VALUE_CLASS_NAME = "value";

static void base_callback();
static void optional_s(NArgument::ArgvLocation);
static void optional_i(NArgument::ArgvLocation);

bool get_stock_info = false;
std::string symbol;
bool stock = false;
double value = 0.0;

NArgument::ArgumentHandler argument_handler(base_callback,
std::vector<std::shared_ptr<NArgument::Argument>>({
	MAKE_ARG(NArgument, ExpansiveOptionalArgument(
		"s",
		"symbol",
		"Stock symbol.",
		optional_s	
	)),
	MAKE_ARG(NArgument, ExpansiveOptionalArgument(
		"i",
		"symbol",
		"Index symbol.",
		optional_i
	))
}));

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static std::string request(const std::string &url) {
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

static std::string get_element_text_value(GumboNode *node) {
	std::string ret;
	GumboNode* text = static_cast<GumboNode*>(node->v.element.children.data[0]);
	if (text->type == GUMBO_NODE_TEXT) {
		ret = text->v.text.text;
	}
	return ret;
}

static bool loop_recursively(GumboNode *node, const std::function<bool(GumboNode*)> &func) {
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

static std::string search_string_to_link(const std::string type, const std::string &symbol) {
	return "https://www.marketwatch.com/investing/" + type + "/" + symbol;
}

static double string_to_double_value(std::string str) {
	str.erase(std::remove(str.begin(), str.end(), ','), str.end());
	return stod(str);
}	

static bool get_current_value(GumboNode *node) {
	static bool in_intraday_price = false;
	if (node->type == GumboNodeType::GUMBO_NODE_ELEMENT) {
		GumboAttribute *attribute = gumbo_get_attribute(&node->v.element.attributes, "class");
		if (attribute != nullptr) {
			if (std::string(attribute->value) != "intraday__price ") {	
				if (std::string(attribute->value) == CURRENT_VALUE_CLASS_NAME && in_intraday_price) {
					in_intraday_price = false;
					std::string text = get_element_text_value(node);
					if (!text.empty()) {
						value = string_to_double_value(text);
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

static void base_callback() {
	if (get_stock_info) {
		std::string link = search_string_to_link((stock ? "stock" : "index"), symbol);
		std::string html = request(link);
		
		std::ofstream of("htmlsrc.html", std::ios::trunc);
		of << html;
		of.close();
	
		GumboOutput *output = gumbo_parse(html.c_str());
		loop_recursively(output->root, std::function<bool(GumboNode*)>(get_current_value));
		gumbo_destroy_output(&kGumboDefaultOptions, output);
		std::cout << value << std::endl;
	} else {
		std::cout << "No overview yet." << std::endl;
	}
}

static void optional_s(NArgument::ArgvLocation location) {
	get_stock_info = true;
	stock = true;
	symbol = *(location+1);
}

static void optional_i(NArgument::ArgvLocation location) {
	get_stock_info = true;
	stock = false;
	symbol = *(location+1);
}

