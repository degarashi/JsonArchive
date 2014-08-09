#include "json_oarchive.hpp"

json_oarchive::json_oarchive(): _stack(_root) {
	_clearIds();
}
void json_oarchive::_save(const char* s) {
	_stack.getEnt() = s;
}
void json_oarchive::_save(double v) {
	_stack.getEnt() = v;
}
void json_oarchive::_save(long v) {
	_stack.getEnt() = Json::LargestInt(v);
}
void json_oarchive::_save(bool b) {
	_stack.getEnt() = Json::Value(b);
}
void json_oarchive::_save(std::nullptr_t) {
	_stack.getEnt() = Json::Value();
}
void json_oarchive::_save(const std::string& s) {
	_stack.getEnt() = s;
}
void json_oarchive::save_start(const char* name) {
	_stack.pushEnt(name);
}
void json_oarchive::save_end(const char* name) {
	_stack.popEnt();
}
void json_oarchive::array_item(int n) {
	_stack.pushEnt(n);
}
void json_oarchive::array_end() {
	_stack.popEnt();
}
void json_oarchive::end_preamble() {}
void json_oarchive::save_override(const boost::archive::class_name_type& t, int) {
	std::string name(t.t);
	_classInfo.name = name;
// 	save_override(boost::serialization::make_nvp(cs_ClassNameType.c_str(), name), 0);
}
void json_oarchive::save_override(const boost::archive::version_type& t, int) {
	uint32_t val(t);
	_classInfo.version = val;
// 	save_override(boost::serialization::make_nvp(cs_VersionType.c_str(), val), 0);
}
void json_oarchive::save_override(const boost::archive::object_id_type& t, int) {
	uint32_t val(t);
	_objectId = val;
	_objectType = ObjectIDType::Normal;
	save_override(boost::serialization::make_nvp(cs_ObjectIdType.c_str(), val), 0);
}
void json_oarchive::save_override(const boost::archive::object_reference_type& t, int) {
	uint32_t val(t);
	_objectId = val;
	_objectType = ObjectIDType::Reference;
	save_override(boost::serialization::make_nvp(cs_ObjectReferenceType.c_str(), val), 0);
}
void json_oarchive::save_override(const boost::archive::class_id_type& t, int) {
	uint32_t val(t);
	_classId = val;
	_objectId = boost::none;
	_classType = ClassIDType::Normal;
	save_override(boost::serialization::make_nvp(cs_ClassIdType.c_str(), val), 0);
}
void json_oarchive::save_override(const boost::archive::class_id_optional_type& t, int) {
	uint32_t val(t);
	_classId = val;
	_classType = ClassIDType::Optional;
	save_override(boost::serialization::make_nvp(cs_ClassIdOptionalType.c_str(), val), 0);
}
void json_oarchive::save_override(const boost::archive::class_id_reference_type& t, int) {
	uint32_t val(t);
	_classId = val;
	_classType = ClassIDType::Reference;
	save_override(boost::serialization::make_nvp(cs_ClassIdReferenceType.c_str(), val), 0);
}
void json_oarchive::save_override(const boost::archive::tracking_type& t, int) {
	uint32_t val(t);
	_classInfo.track = val;
// 	save_override(boost::serialization::make_nvp(cs_TrackingType.c_str(), val), 0);
}
void json_oarchive::save_binary(void* address, std::size_t count) {}
void json_oarchive::finalize() {
	_stack.pushEnt(cs_ClassRepository.c_str());
	for(auto& p : _classMap) {
		_stack.pushEnt(std::to_string(p.first).c_str());
		auto& ent = _stack.getEnt();
		ent[cs_ClassNameType] = p.second.name;
		ent[cs_VersionType] = p.second.version;
		ent[cs_TrackingType] = p.second.track;
		_stack.popEnt();
	}
	_stack.popEnt();
}

