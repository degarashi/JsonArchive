#pragma once
#include "json_archive.hpp"
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>
#include <boost/archive/detail/archive_serializer_map.hpp>
#include <boost/archive/text_iarchive.hpp>

class json_iarchive : public boost::archive::detail::common_iarchive<json_iarchive> {
	private:
		base_json_archive	_stack;
		OPId				_classId,
							_objectId;
		ClassIDType			_classType;
		ObjectIDType		_objectType;
		ClassMap			_classMap;
		// 同じキー名を連続アクセスした時は配列とみなす
		struct AItem {
			std::string	key;
			int			index;
			bool		popFlag;
		};
		using ArrayVec = std::vector<AItem>;
		ArrayVec			_arrayStack;
		int					_arrayLevel;

		void _clearIds();
		void _loadClassMap();
		Json::Value& _getEntry();

		friend class boost::archive::load_access;
		friend class boost::archive::detail::interface_iarchive<json_iarchive>;
		using base_t = boost::archive::detail::common_iarchive<json_iarchive>;

		// 1. direct
		void load(std::string& s);
		void load(boost::serialization::item_version_type& v);
		// 2. floatingpoint
		template <class T,
					std::enable_if_t<std::is_floating_point<T>::value>*& = Enabler>
		void load(T& t) {
			t = _getEntry().asDouble();
		}
		// 3. integral
		template <class T,
					std::enable_if_t<std::is_integral<T>::value>*& = Enabler>
		void load(T& t) {
			t = _getEntry().asLargestInt();
		}
		// 4. convertible to long
		template <class T,
				std::enable_if_t<!std::is_integral<T>::value>*& = Enabler,
				std::enable_if_t<!std::is_floating_point<T>::value>*& = Enabler,
				std::enable_if_t<std::is_convertible<T,long>::value>*& = Enabler>
		void load(T& t) {
			t = _getEntry().asLargestInt();
		}

		// load primitive
		template <class T>
		void load_override(T& t, int);
		template <class T>
		void load_override(T* t, int) {
			base_t::load_override(t, 0);
		}

		// NVP(class)
		template <class T,
				std::enable_if_t<std::is_class<T>::value>*& = Enabler>
		void load_override(boost::serialization::nvp<T> const& t, int) {
			if(!t.name()) {
				if(_classId && _classType != ClassIDType::Optional) {
					if(_objectId) {
						// リポジトリ参照
						_stack.pushObjRepository(*_classId, *_objectId);
						base_t::load_override(t.value(), 0);
						_stack.popToMark();
						_clearIds();
						return;
					} else {
						// ClassID + none
						// ポインタ経由でのクラス読み込み
						base_t::load_override(t.value(), 0);
						_clearIds();
						return;
					}
				}
				base_t::load_override(t.value(), 0);
				_clearIds();
			}
			load_start(t.name());
			if(_stack.getEnt().type() == Json::nullValue)
				throw 0;
			base_t::load_override(t.value(), 0);
			load_end(t.name());
			_clearIds();
		}
		// NVP(non-class)
		template <class T,
				std::enable_if_t<!std::is_class<T>::value>*& = Enabler>
		void load_override(boost::serialization::nvp<T> const& t, int) {
			load_start(t.name());
			if(_stack.getEnt().type() == Json::nullValue)
				throw 0;
			base_t::load_override(t.value(), 0);
			load_end(t.name());
		}

		// 配列のサイズはJSONが持っているので別途["count"]に記録していない
		void load_override(boost::serialization::collection_size_type& t, int);
		void load_override(boost::serialization::nvp<boost::serialization::collection_size_type> const& t, int);

		// JSONのArrayから読み込む
		template <class T>
		void load_override(boost::serialization::array<T> const& t, int) {
			auto sz = t.count();
			for(decltype(sz) i=0 ; i<sz ; i++) {
				array_item(i);
				base_t::load_override(t.address()[i], 0);
				array_end();
			}
		}

		void load_override(boost::archive::class_name_type& t, int);
		void load_override(boost::archive::version_type& t, int);
		void load_override(boost::archive::object_id_type& t, int);
		void load_override(boost::archive::object_reference_type& t, int);
		void load_override(boost::archive::class_id_type& t, int);
		void load_override(boost::archive::class_id_optional_type& t, int);
		void load_override(boost::archive::class_id_reference_type& t, int);
		void load_override(boost::archive::tracking_type& t, int);

		void load_start(const char* name);
		void load_end(const char* name);
		void load_preamble();
		void array_item(int n);
		void array_end();
	public:
		json_iarchive(Json::Value& root);
};
BOOST_SERIALIZATION_REGISTER_ARCHIVE(json_iarchive);

