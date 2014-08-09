// #include <boost/archive/impl/text_iarchive_impl.ipp>
// #include <boost/archive/impl/text_oarchive_impl.ipp>
// #include <boost/archive/impl/basic_text_iarchive.ipp>
#include "json_oarchive.hpp"
#include "json_iarchive.hpp"
#include <boost/archive/impl/archive_serializer_map.ipp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/format.hpp>

class Derived;
class Base {
	private:
		double	_valueD;
		float	_valueF;

		friend class boost::serialization::access;
		template <class Archive>
		friend void boost::serialization::save_construct_data(Archive&, const Base*, const unsigned int);
		template <class Archive>
		friend void boost::serialization::save_construct_data(Archive&, const Derived*, const unsigned int);
		template <class AR>
		void serialize(AR& ar, const unsigned int ver) {
			ar & BOOST_SERIALIZATION_NVP(_valueD)
				& BOOST_SERIALIZATION_NVP(_valueF);
		}
	public:
		// コンストラクタ引数が必要なクラスのテスト
		Base(double d, float f): _valueD(d), _valueF(f) {}
		void print_value() {
			std::cout << boost::format("valueD=%1%, valueF=%2%") % _valueD % _valueF;
		}
		virtual void print() {
			std::cout << "this is Base Class. ";
			print_value();
			std::cout << std::endl;
		}
};
class Derived : public Base {
	private:
		int _valueI;

		friend class boost::serialization::access;
		template <class Archive>
		friend void boost::serialization::save_construct_data(Archive&, const Derived*, const unsigned int);
		template <class AR>
		void serialize(AR& ar, const unsigned int ver) {
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base)
				& BOOST_SERIALIZATION_NVP(_valueI);
		}
	public:
		Derived(int i, double d, float f): Base(d, f), _valueI(i) {}
		void print_value() {
			std::cout << boost::format(", valueI=%1%") % _valueI;
		}
		void print() override {
			std::cout << "this is Derived Class. ";
			Base::print_value();
			print_value();
			std::cout << std::endl;
		}
};
class MyClass {
	private:
		Base*		_ptrBase[4] = {};
		Derived		_derived;
		friend class boost::serialization::access;
		template <class AR>
		void serialize(AR& ar, const unsigned int ver) {
			ar & BOOST_SERIALIZATION_NVP(_ptrBase)
				& BOOST_SERIALIZATION_NVP(_derived);
		}
	public:
		MyClass(): _derived(0,0,0) {}
		MyClass(std::true_type): _derived(32, 64, 128) {
			double valD = 256;
			float valF = 512;
			for(int i=0 ; i<4 ; i++) {
				if(i&1)
					_ptrBase[i] = new Derived(i, valD, valF);
				else
					_ptrBase[i] = new Base(valD, valF);

				valD *= 2;
				valF *= 2;
			}
		}
		~MyClass() {
			for(auto& p : _ptrBase)
				if(p) delete p;
		}
		void print() {
			int idx=0;
			for(auto& p : _ptrBase) {
				std::cout << idx++ << ": ";
				p->print();
			}
			_derived.print();
		}
};
BOOST_CLASS_EXPORT(Base)
BOOST_CLASS_EXPORT(Derived)
BOOST_CLASS_EXPORT(MyClass)

namespace boost {
	namespace serialization {
		template <class Archive>
		inline void save_construct_data(Archive& ar, const Derived* v, const unsigned int ver) {
// 			std::cout << "save_construct(Derived)" << std::endl;
			ar & boost::serialization::make_nvp("ctor_valueI", v->_valueI)
				& boost::serialization::make_nvp("ctor_valueD", v->_valueD)
				& boost::serialization::make_nvp("ctor_valueF", v->_valueF);
		}
		template <class Archive>
		inline void save_construct_data(Archive& ar, const Base* v, const unsigned int ver) {
// 			std::cout << "save_construct(Base)" << std::endl;
			ar & boost::serialization::make_nvp("ctor_valueD", v->_valueD)
				& boost::serialization::make_nvp("ctor_valueF", v->_valueF);
		}
		template <class Archive>
		inline void load_construct_data(Archive& ar, Derived* v, const unsigned int ver) {
// 			std::cout << "load_construct(Derived)" << std::endl;
			int iv;
			double dv;
			float fv;
			ar & boost::serialization::make_nvp("ctor_valueI", iv)
				& boost::serialization::make_nvp("ctor_valueD", dv)
				& boost::serialization::make_nvp("ctor_valueF", fv);
			new(v) Derived(iv, dv, fv);
		}
		template <class Archive>
		inline void load_construct_data(Archive& ar, Base* v, const unsigned int ver) {
// 			std::cout << "load_construct(Base)" << std::endl;
			double dv;
			float fv;
			ar & boost::serialization::make_nvp("ctor_valueD", dv)
				& boost::serialization::make_nvp("ctor_valueF", fv);
			new(v) Base(dv, fv);
		}
	}
}

int main() {
	MyClass myc{std::true_type()}, myc2;
	json_oarchive ja;
	ja << BOOST_SERIALIZATION_NVP(myc);
	std::cout << ja.getNode().toStyledString() << std::endl;

	json_iarchive jia(ja.getNode());
	jia >> boost::serialization::make_nvp("myc", myc2);
	myc2.print();

	return 0;
}

