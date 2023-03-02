#include "json.hpp"

#include <nds.h>
#include <stdio.h>

// sassert but it fixes the brightness
#undef sassert
#define sassert(e,...) ((e) ? (void)0 : (setBrightness(2, 0), __sassert(__FILE__, __LINE__, #e, __VA_ARGS__)))

Json::Json(void) {
	_json = std::shared_ptr<cJSON>(cJSON_CreateObject(), [](cJSON *p) { cJSON_Delete(p); });
}

Json::Json(const char *str, bool fromFile) {
	if(fromFile) {
		FILE *file = fopen(str, "r");
		if(!file)
			return;

		fseek(file, 0, SEEK_END);
		size_t fsize = ftell(file);
		if(fsize == 0) {
			fclose(file);
			return;
		}
		fseek(file, 0, SEEK_SET);

		char *buffer = new char[fsize];
		if(fread(buffer, 1, fsize, file) != fsize) {
			fclose(file);
			delete[] buffer;
			return;
		}
		fclose(file);

		_json = std::shared_ptr<cJSON>(cJSON_ParseWithLength(buffer, fsize), [](cJSON *p) { cJSON_Delete(p); });

		delete[] buffer;
	} else {
		_json = std::shared_ptr<cJSON>(cJSON_Parse(str), [](cJSON *p) { cJSON_Delete(p); });
	}
}

std::string Json::dump() const {
	char *charStr = cJSON_PrintUnformatted(_json.get());
	std::string str(charStr);
	free(charStr);

	return str;
}

Json Json::set(cJSON *item, const char *name) {
	if(isArray()) {
		sassert(name == nullptr, "name must be nullptr for arrays");
		cJSON_AddItemToArray(_json.get(), item);
	} else if(isObject()) {
		sassert(name != nullptr, "name must NOT be nullptr for\nobjects");
		if(contains(name))
			cJSON_ReplaceItemInObjectCaseSensitive(_json.get(), name, item);
		else
			cJSON_AddItemToObject(_json.get(), name, item);
	} else {
		sassert(false, "set can only be used on arrays\nand objects");
	}

	return item;
}

Json Json::set(const std::vector<std::string> &val, const char *name) {
	Json array = create(false, name);
	for(const std::string &str : val) {
		array.set(str.c_str());
	}

	return array;
}
