#pragma once
#include <string>
#include <json/json.h>
#include <boost/optional.hpp>

extern void* Enabler;
extern const std::string cs_ClassNameType,
							cs_VersionType,
							cs_ItemVersionType,
							cs_ObjectIdType,
							cs_ObjectReferenceType,
							cs_ClassIdType,
							cs_ClassIdOptionalType,
							cs_ClassIdReferenceType,
							cs_TrackingType,
							cs_ObjRepository,
							cs_ClassRepository;

class base_json_archive {
	private:
		using EntryStack = std::vector<Json::Value*>;
		EntryStack					_stack;		//!< データ全般が書き込まれるスタック
		std::vector<int>			_mark;
		Json::Value*				_objRep;
		std::vector<std::string>	_test;
	public:
		base_json_archive(Json::Value& r);
		void pushClassRepository();
		void pushObjRepository(uint32_t clsId, uint32_t objId);
		void popToMark();

		void pushEnt(const char* name);
		void pushEnt(int n);
		void popEnt();
		const Json::Value& getEnt() const;
		Json::Value& getEnt();
};

#include <unordered_map>
struct ClassInfo {
	std::string		name;
	int				track,
					version;
	void clear() {
		name.clear();
		track = version = 0;
	}
};
using ClassMap = std::unordered_map<uint32_t, ClassInfo>;
using OPId = boost::optional<uint32_t>;
enum class ClassIDType {
	Normal,
	Optional,
	Reference
};
enum class ObjectIDType {
	Normal,
	Reference
};

