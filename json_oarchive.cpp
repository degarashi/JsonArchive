#include "json_oarchive.hpp"

json_oarchive::json_oarchive():
	_stack(_root),
	_arrayLevel(0)
{
	_clearIds();
}
namespace {
	auto fnWrite = [](Json::Value& jv, const Json::Value& v) {
		auto typ = jv.type();
		if(typ == Json::nullValue)
			jv = v;
		else if(typ == Json::arrayValue)
			jv.append(v);
		else {
			Json::Value tmp;
			tmp.swap(jv);
			jv.append(tmp);
			jv.append(v);
		}
	};
}
void json_oarchive::_save(const char* s) {
	fnWrite(_stack.getEnt(), Json::Value(s));
}
void json_oarchive::_save(double v) {
	fnWrite(_stack.getEnt(), Json::Value(v));
}
void json_oarchive::_save(long v) {
	fnWrite(_stack.getEnt(), Json::LargestInt(v));
}
void json_oarchive::_save(bool b) {
	fnWrite(_stack.getEnt(), Json::Value(b));
}
void json_oarchive::_save(std::nullptr_t) {
	fnWrite(_stack.getEnt(), Json::Value());
}
void json_oarchive::_save(const std::string& s) {
	fnWrite(_stack.getEnt(), Json::Value(s));
}
void json_oarchive::save_start(const char* name) {
	_stack.pushEnt(name);

	auto& bk = _stack.getEnt();
	auto sz = _arrayStack.size();
	if(sz < ++_arrayLevel) {
		assert(sz == _arrayLevel-1);
		_arrayStack.emplace_back(name,false);
		bk = Json::Value();
	} else {
		assert(sz == _arrayLevel);
		if(_arrayStack.back().first != name) {
			// エントリの上書き
			_arrayStack.back().first = name;
			bk = Json::Value();
		} else {
			// 同じ名前で再度書き込んだ場合は配列として扱う
			if(bk.type() != Json::arrayValue) {
				_arrayStack.back().second = true;

				Json::Value tmp;
				tmp.swap(bk);
				bk.append(tmp);
			}
			_stack.pushEnt(bk.size());
		}
	}
}
void json_oarchive::save_end(const char* name) {
	_stack.popEnt();
	if(_arrayStack[--_arrayLevel].second)
		_stack.popEnt();
	_arrayStack.resize(_arrayLevel + 1);
}
void json_oarchive::array_item(int n) {
	_stack.pushEnt(n);
	_arrayStack.resize(_arrayLevel);
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

