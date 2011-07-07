#include <vector>
#include <string>

typedef char* string;

using namespace std;
class SampleClass
{
public:
    SampleClass();
    /*!
     * Method 1
     */
    string meth1();

    ///
    /// Method 2 description
    ///
    /// @param v1 Variable 1
    ///
    int meth2(int v1);

    /**
     * Method 3 description
     *
     * \param v1 Variable 1
     * \param v2 Variable 2
     */
    void meth3(const string & v1, vector<string> & v2);

    /**********************************
     * Method 4 description
     *
     * @return Return value
     *********************************/
    unsigned int meth4();
private:
    void * meth5(){return NULL};	// invalid c++, fixing parser anyways.

    /// prop1 description
    string prop1;
    //! prop5 description
    int prop5;
};
namespace Alpha
{
    class AlphaClass
    {
    public:
        AlphaClass();

        void alphaMethod();

        string alphaString;
    };

    namespace Omega
    {
        class OmegaClass
        {
        public:
            OmegaClass();

            string omegaString;
        };
    };
}

// tests by hart //
namespace Gamma {
	typedef std::string mystring;
}
typedef std::string _StringBase;
typedef _StringBase String;

namespace Theta {
	class ThetaClass {
		public:
			ThetaClass();
			~ThetaClass();
		protected:
			struct mystruct {
				bool xxx;
			};
			void this_method_protected();
	}

	struct ThetaStruct {
		bool test1;
	};

	class ThetaClass2 {
		public:
			ThetaClass2();
			static inline void static_inlined_method();
			inline friend void myfriendmethod();
			static std::vector<float> returns_std_vector();
			float operator + ();
			template<typename AAA> std::vector<float> template_method( const AAA & aaa );

	}

	inline int ThetaClass::method_defined_outside() {
		return 1;
	}
	inline ThetaClass::operator ThetaClass() const
	{
		return ThetaClass();
	}


}

struct FreeStruct {
	bool freestructs_boolvar;
	float freestructs_floatvar;
};

namespace A {

	class FixMe {
		virtual ~__forced_unwind() throw();
		virtual void purevirt() = 0;
		int realMethod() {
			return 1
		}
	};	// legal to end class with }; ?

	class SubClassFixMe :  public FixMe {
		public:
			void purevirt();
	}

	namespace B {

	}
	class StillAbstract :  public FixMe {
		public:
			void somemethod();
			pointer operator->() const { return &(operator*()); }	// example of operator used in method def

	}

}
namespace C __attribute__ ((__visibility__ ("default"))) {
}

