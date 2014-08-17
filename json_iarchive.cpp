#include "json_iarchive.hpp"
#include "debugmsg.hpp"
#include <cassert>
#include <cstring>

using namespace boost::archive;
json_iarchive::json_iarchive(Json::Value& root):
	_stack(root),
	_arrayLevel(0)
{
	_clearIds();
	_loadClassMap();
}
//TODO: 例外クラスを定義
void json_iarchive::load_override(class_name_type& t, int) {
	if(!_classId)
		throw 0;
	auto itr = _classMap.find(*_classId);
	if(itr == _classMap.end())
		throw 0;
	std::strcpy((char*)t, itr->second.name.c_str());
}
Json::Value& json_iarchive::_getEntry() {
	Json::Value& e = _stack.getEnt();
	auto idx = _arrayStack.back().index;
	bool bArray = e.type() == Json::arrayValue;
	if(idx == 0) {
		if(bArray)
			return e[0];
	} else {
		if(bArray)
			return e[idx];
	}
	return e;
}
void json_iarchive::load(std::string& s) {
	s = _getEntry().asString();
}
void json_iarchive::load(boost::serialization::item_version_type& v) {
	uint32_t val;
	load(val);
	v = boost::serialization::item_version_type(val);
}
void json_iarchive::load_override(version_type& t, int) {
	if(!_classId)
		throw 0;
	uint32_t ver = 0;
	auto itr = _classMap.find(*_classId);
	if(itr != _classMap.end())
		ver = itr->second.version;
	t = version_type(ver);
}
void json_iarchive::_clearIds() {
	_classId = _objectId = boost::none;
}
void json_iarchive::_loadClassMap() {
	_stack.pushClassRepository();
	Json::Value& cm = _stack.getEnt();
	for(auto itr=cm.begin() ; itr!=cm.end() ; itr++) {
		auto& value = *itr;
		auto& dst = _classMap[std::stoi(itr.key().asString())];
		dst.name = value[cs_ClassNameType].asString();
		dst.track = value[cs_TrackingType].asInt();
		dst.version = value[cs_VersionType].asInt();
	}
	_stack.popToMark();
}
void json_iarchive::load_override(boost::serialization::collection_size_type& t, int) {
	auto& e = _stack.getEnt();
	uint32_t val;
	if(e.type() == Json::arrayValue)
		val = e.size();
	else
		val = e["count"].asInt();
	t = boost::serialization::collection_size_type(val);
}
void json_iarchive::load_override(boost::serialization::nvp<boost::serialization::collection_size_type> const& t, int) {
	load_override(t.value(), 0);
}
void json_iarchive::load_override(object_id_type& t, int) {
	auto& js = _stack.getEnt()[cs_ObjectIdType];
	if(!js.isIntegral()) {
		DebugMsgLn("%1% is not found", cs_ObjectIdType);
		object_reference_type r(object_id_type(0));
		load_override(r, 0);
		t = object_id_type(r);
		return;
	}
	_objectId = js.asInt();
	_objectType = ObjectIDType::Normal;
	DebugMsgLn("ObjectID: %1%", *_objectId);
	t = object_id_type(*_objectId);
}
void json_iarchive::load_override(object_reference_type& t, int) {
	auto& js = _stack.getEnt()[cs_ObjectReferenceType];
	if(!js.isIntegral()) {
		DebugMsgLn("%1% is not found", cs_ObjectReferenceType);
		return;
	}
	auto val = js.asInt();
	_objectId = val;
	_objectType = ObjectIDType::Reference;
	DebugMsgLn("ObjectRefID: %1%", *_objectId);
	t = object_reference_type(object_id_type(val));
}
void json_iarchive::load_override(class_id_type& t, int) {
	auto& js = _stack.getEnt()[cs_ClassIdType];
	if(!js.isIntegral()) {
		DebugMsgLn("%1% is not found", cs_ClassIdType);
		class_id_reference_type r(class_id_type(0));
		load_override(r, 0);
		t = class_id_type(int(r));
		return;
	}
	_classId = js.asInt();
	_classType = ClassIDType::Normal;
	_objectId = boost::none;
	DebugMsgLn("ClassId: %1%", *_classId);
	t = class_id_type(int(*_classId));
}
void json_iarchive::load_override(class_id_optional_type& t, int) {
	auto& js = _getEntry()[cs_ClassIdOptionalType];
	if(!js.isIntegral()) {
		DebugMsgLn("%1% is not found", cs_ClassIdOptionalType);
		return;
	}
	auto val = js.asInt();
	_classId = val;
	_classType = ClassIDType::Optional;
	_objectId = boost::none;
	t = class_id_optional_type(class_id_type(val));
}
void json_iarchive::load_override(class_id_reference_type& t, int) {
	auto& js = _stack.getEnt()[cs_ClassIdReferenceType];
	if(!js.isIntegral()) {
		DebugMsgLn("%1% is not found", cs_ClassIdReferenceType);
		return;
	}
	auto val = js.asInt();
	_classId = val;
	_classType = ClassIDType::Reference;
	_objectId = boost::none;
	DebugMsgLn("ClassRefId: %1%", *_classId);
	t = class_id_reference_type(class_id_type(val));
}
void json_iarchive::load_override(tracking_type& t, int) {
	if(!_classId)
		throw 0;
	uint32_t tr = 0;
	auto itr = _classMap.find(*_classId);
	if(itr != _classMap.end())
		tr = itr->second.track;
	t = tracking_type(tr);
}
void json_iarchive::load_start(const char* name) {
	_stack.pushEnt(name);
	auto sz = _arrayStack.size();
	if(sz < ++_arrayLevel) {
		assert(sz == _arrayLevel-1);
		_arrayStack.emplace_back(AItem{name, 0, false});
	} else {
		assert(sz == _arrayLevel);
		bool bArray = _stack.getEnt().type() == Json::arrayValue;
		auto& bks = _arrayStack.back();
		if(bks.key == name) {
			assert(bArray);
			++bks.index;
			bks.popFlag = true;
			_stack.pushEnt(bks.index);
		} else {
			bks.index = 0;
			bks.key = name;
			if(bArray) {
				bks.popFlag = true;
				_stack.pushEnt(bks.index);
			}
		}
	}
}
void json_iarchive::load_end(const char* name) {
	_stack.popEnt();
	if(_arrayStack[--_arrayLevel].popFlag)
		_stack.popEnt();
	_arrayStack.resize(_arrayLevel + 1);
}
void json_iarchive::load_preamble() {}
void json_iarchive::array_item(int n) {
	_stack.pushEnt(n);
	_arrayStack.resize(_arrayLevel);
}
void json_iarchive::array_end() {
	_stack.popEnt();
}

