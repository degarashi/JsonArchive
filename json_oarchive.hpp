#pragma once
#include "json_archive.hpp"
#include <type_traits>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>
#include <boost/archive/detail/archive_serializer_map.hpp>
#include <boost/archive/text_oarchive.hpp>

class json_oarchive : public boost::archive::detail::common_oarchive<json_oarchive> {
	private:
		Json::Value			_root;
		base_json_archive	_stack;
		OPId				_classId,
							_objectId;
		ClassIDType			_classType;
		ObjectIDType		_objectType;
		// 同じキー名を連続アクセスした時は配列とみなす
		using StrVec = std::vector<std::pair<std::string,bool>>;
		StrVec				_arrayStack;
		int					_arrayLevel;

		void _clearIds() {
			_classId = _objectId = boost::none;
		}
		ClassInfo			_classInfo;
		ClassMap			_classMap;

		// permit serialization system privileged access to permit
		// implementation of inline templates for maximum speed.
		friend class boost::archive::save_access;
		friend class boost::archive::detail::interface_oarchive<json_oarchive>;
		using base_t = boost::archive::detail::common_oarchive<json_oarchive>;

		// 1. direct
		static const char* _ConvType(const char* t);
		static const std::string& _ConvType(const std::string& t);
		static bool _ConvType(bool t);
		static std::nullptr_t _ConvType(std::nullptr_t);
		static long _ConvType(boost::serialization::item_version_type);
		// 2. floatingpoint
		template <class T,
				  std::enable_if_t<std::is_floating_point<T>::value>*& = Enabler>
		static double _ConvType(T t);
		// 3. integral
		template <class T,
				std::enable_if_t<std::is_integral<T>::value>*& = Enabler>
		static long _ConvType(T t);
		// 4. convertible to long
		template <class T,
				std::enable_if_t<!std::is_integral<T>::value>*& = Enabler,
				std::enable_if_t<!std::is_floating_point<T>::value>*& = Enabler,
				std::enable_if_t<std::is_convertible<T,long>::value>*& = Enabler>
		static long _ConvType(T t);
		// 5. error (not found)
		struct UnknownType {};
		static UnknownType _ConvType(...);

		template <class T>
		void save(const T& t) {
			using Typ = decltype(_ConvType(t));
			_save(static_cast<Typ>(t));
		}
		// member template for saving primitive types.
		// Specialize for any types/templates that special treatment
		template <class T>
		void _save(const T&);
		void _save(const char* s);
		void _save(const std::string& s);
		void _save(double v);
		void _save(long v);
		void _save(bool b);
		void _save(std::nullptr_t);
		void _save(const boost::serialization::item_version_type&);

		void save_start(const char* name);
		void save_end(const char* name);
		void array_item(int n);
		void array_end();
		void end_preamble();

		void save_override(char *const t, int) {
			save(t);
		}
		// write primitive
		template <class T>
		void save_override(T& t, int) {
			base_t::save_override(t, 0);
		}

		// NVP(class)
		template <class T,
					std::enable_if_t<std::is_class<T>::value>*& = Enabler>
		void save_override(const boost::serialization::nvp<T>& t, int) {
			if(_classId && _classType != ClassIDType::Reference) {
				// ClassInfo書き込み
				if(_classMap.count(*_classId) == 0)
					_classMap[*_classId] = _classInfo;
				_classInfo.clear();
			}
			if(!t.name()) {
				if(_classId && _classType != ClassIDType::Optional) {
					if(_objectId) {
						if(_objectType == ObjectIDType::Normal) {
							// ClassID + ObjID
							// 共有オブジェクト定義
							_stack.pushObjRepository(*_classId, *_objectId);
							base_t::save_override(t.const_value(), 0);
							_stack.popToMark();
						} else {
							// ClassID + ObjIDRef
							// (共有オブジェクト参照)
						}
					} else {
						// ClassID + none
						// ポインタ経由での書き込み
						base_t::save_override(t.const_value(), 0);
					}
					_clearIds();
					return;
				}
			}
			// 参照からの書き込み
			save_start(t.name());
			base_t::save_override(t.const_value(), 0);
			save_end(t.name());

			_clearIds();
		}
		// NVP(non-class)
		template <class T,
					std::enable_if_t<!std::is_class<T>::value>*& = Enabler>
		void save_override(const boost::serialization::nvp<T>& t, int) {
			save_start(t.name());
			base_t::save_override<const T&>(t.const_value(), 0);
			save_end(t.name());
		}
		// JSONのArrayで書き込む
		template <class T>
		void save_override(const boost::serialization::array<T>& t, int) {
			std::size_t sz = t.count();
			for(std::size_t i=0 ; i<sz ; i++) {
				array_item(i);
				base_t::save_override(t.address()[i], 0);
				array_end();
			}
		}
		void save_override(const boost::archive::class_name_type& t, int);
		void save_override(const boost::archive::version_type& t, int);
		void save_override(const boost::archive::object_id_type& t, int);
		void save_override(const boost::archive::object_reference_type& t, int);
		void save_override(const boost::archive::class_id_type& t, int);
		void save_override(const boost::archive::class_id_optional_type& t, int);
		void save_override(const boost::archive::class_id_reference_type& t, int);
		void save_override(const boost::archive::tracking_type& t, int);

	public:
		json_oarchive();
		void save_binary(void* address, std::size_t count);
		void finalize();
		Json::Value& getNode() {
			finalize();
			return _root;
		}
};
BOOST_SERIALIZATION_REGISTER_ARCHIVE(json_oarchive);

