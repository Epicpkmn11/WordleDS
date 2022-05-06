#ifndef JSON_HPP
#define JSON_HPP

#include "cJSON.h"
#include <memory>
#include <string>
#include <vector>

class Json {
	std::shared_ptr<cJSON> _json = nullptr;

public:
	class Iterator {
		cJSON *_json;

	public:
		Iterator(cJSON *json) : _json(json) {}

		inline const cJSON *get(void) const { return _json; }

		bool operator==(const Iterator &rhs) const { return get() == rhs.get(); }
		bool operator!=(const Iterator &rhs) const { return get() != rhs.get(); }
		const void operator++(void) { _json = _json->next; }
		const void operator--(void) { _json = _json->prev; }
		Json operator*(void) { return Json(_json); }
	};

	explicit Json(void);
	explicit Json(const char *path);
	Json(cJSON *json) : _json(std::shared_ptr<cJSON>(json, [](cJSON *p) {})) {}

	const Json operator[](int index) const { return cJSON_GetArrayItem(_json.get(), index); }
	const Json operator[](const char *string) const { return cJSON_GetObjectItemCaseSensitive(_json.get(), string); }

	const Iterator begin(void) const { return _json->child; }
	const Iterator end(void) const { return nullptr; }

	std::string dump(void) const;

	inline const cJSON *get(void) const { return _json.get(); }
	inline int size(void) const { return cJSON_GetArraySize(_json.get()); }
	inline bool contains(const char *string) const { return cJSON_GetObjectItemCaseSensitive(_json.get(), string) != nullptr; }
	inline bool isArray(void) const { return cJSON_IsArray(_json.get()); }
	inline bool isBool(void) const { return cJSON_IsBool(_json.get()); }
	inline bool isFalse(void) const { return cJSON_IsFalse(_json.get()); }
	inline bool isNull(void) const { return cJSON_IsNull(_json.get()); }
	inline bool isNumber(void) const { return cJSON_IsNumber(_json.get()); }
	inline bool isObject(void) const { return cJSON_IsObject(_json.get()); }
	inline bool isString(void) const { return cJSON_IsString(_json.get()); }
	inline bool isTrue(void) const { return cJSON_IsTrue(_json.get()); }

	Json create(bool object, const char *name = nullptr) { return set(object ? cJSON_CreateObject() : cJSON_CreateArray(), name); }

	Json set(cJSON *item, const char *name = nullptr);
	Json set(const std::vector<std::string> &val, const char *name = nullptr);
	inline Json set(bool val, const char *name = nullptr) { return set(cJSON_CreateBool(val), name); }
	inline Json set(int val, const char *name = nullptr) { return set(cJSON_CreateNumber(val), name); }
	inline Json set(double val, const char *name = nullptr) { return set(cJSON_CreateNumber(val), name); }
	inline Json set(const char *val, const char *name = nullptr) { return set(cJSON_CreateString(val), name); }
	inline Json set(const std::vector<int> &val, const char *name = nullptr) { return set(cJSON_CreateIntArray(val.data(), val.size()), name); }
};

#endif // JSON_HPP
