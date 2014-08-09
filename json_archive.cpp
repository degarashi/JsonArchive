#include "json_archive.hpp"

const std::string cs_ClassNameType("class_name"),
					cs_VersionType("version"),
					cs_ObjectIdType("object_id"),
					cs_ObjectReferenceType("object_ref"),
					cs_ClassIdType("class_id"),
					cs_ClassIdOptionalType("class_id_opt"),
					cs_ClassIdReferenceType("class_id_ref"),
					cs_TrackingType("tracking"),
					cs_ObjRepository("__obj_repository"),
					cs_ClassRepository("__class_repository");
void* Enabler;

base_json_archive::base_json_archive(Json::Value& r):
	_objRep(&r[cs_ObjRepository])
{
	_stack.push_back(&r);
}
void base_json_archive::pushClassRepository() {
	_mark.push_back(_stack.size());
	pushEnt(cs_ClassRepository.c_str());
}
void base_json_archive::pushObjRepository(uint32_t clsId, uint32_t objId) {
// 	std::cout << "PUSH: clsId=" << clsId << " objId=" << objId << std::endl;
	_mark.push_back(_stack.size());
	_stack.push_back(_objRep);
	pushEnt(std::to_string(clsId).c_str());
	pushEnt(std::to_string(objId).c_str());
}
void base_json_archive::popToMark() {
	_stack.resize(_mark.back());
	_mark.pop_back();
}

// #include <iostream>
void base_json_archive::pushEnt(const char* name) {
	_test.push_back(name);
// 	std::cout << "PUSH: " << name << std::endl;
	_stack.push_back(&getEnt()[name]);
}
void base_json_archive::pushEnt(int n) {
	_test.push_back(std::to_string(n));
// 	std::cout << "PUSH: " << n << std::endl;
	auto& target = getEnt();
	if(!target.isArray())
		target = Json::Value();
	_stack.push_back(&target[n]);
}
void base_json_archive::popEnt() {
	_stack.pop_back();
// 	std::cout << "POP: " << _test.back() << std::endl;
	_test.pop_back();
}
const Json::Value& base_json_archive::getEnt() const {
	return *_stack.back();
}
Json::Value& base_json_archive::getEnt() {
	return *_stack.back();
}

