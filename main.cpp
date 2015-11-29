#include <iostream>
#include <string>
#include <functional>
#include <cassert>
#include <sstream>

using namespace std;
using namespace std::placeholders;




////////////////////////////////////////////////////////// PLACEHOLDER //////////////////////////////////////////////////////////////////
///  easy_bind from xeo  () , fills remaining positions in a variadic call with placeholder ( will we have a standard implemeentation? in the future?)
///  http://stackoverflow.com/questions/15024223/how-to-implement-an-easy-bind-that-automagically-inserts-implied-placeholders

template <std::size_t... Is>
struct indices {};

template <std::size_t N, std::size_t... Is>
struct build_indices
	: build_indices<N - 1, N - 1, Is...> {};

template <std::size_t... Is>
struct build_indices<0, Is...> : indices<Is...>{};

template<int I> struct placeholder{};


namespace std{
	template<int I>
	struct is_placeholder< ::placeholder<I>> : std::integral_constant<int, I>{};
} // std::

namespace detail{
	template<std::size_t... Is, class F, class... Args>
	auto easy_bind(indices<Is...>, F const& f, Args&&... args)
		-> decltype(std::bind(f, std::forward<Args>(args)..., placeholder<1 + Is>{}...))
	{
		return std::bind(f, std::forward<Args>(args)..., placeholder<1 + Is>{}...);
	}
} // detail::

template<class R, class... FArgs, class... Args>
auto easy_bind(std::function<R(FArgs...)> const& f, Args&&... args)
-> decltype(detail::easy_bind(build_indices<sizeof...(FArgs)-sizeof...(Args)>{}, f, std::forward<Args>(args)...))
{
	return detail::easy_bind(build_indices<sizeof...(FArgs)-sizeof...(Args)>{}, f, std::forward<Args>(args)...);
}

///////////////////////////////////////////////////////// FBIND /////////////////////////////////////////////////////////////////////////




template<typename ...>class FBind;

template <typename D, typename T, typename ...Args>
class FBind<D, T, Args...>  //DestType, T , Rest
{	
public:
	typedef function<D(T, Args...) > 	Function0;   
private:
	Function0  _f;
public:
	FBind<D, T, Args...>( typename FBind<D, T, Args...>::Function0& f){
		_f = f;
	}
	FBind<D, T, Args...>(){
		_f = nullptr;
	}
	FBind<D, T, Args...>(FBind<D, T, Args...>&&) = default;
	FBind<D, T, Args...>& operator= (const FBind<D, T, Args...>& other){
		_f = other._f;
		return *this;
	}
///apply
	FBind<D, Args...>  operator () (T a){
		typename FBind<D, Args...>::Function0  ff = easy_bind(_f, a);
		FBind<D, Args...> binding(ff);
		return binding;
	}
	
	inline bool complete() {return false;}
};

template <typename D, typename T>
class FBind<D, T>
{
public:
	typedef function<D(T) > Function0;
	Function0 	_f;
	FBind<D, T>(Function0  f){
		_f = f;
	}
	FBind<D, T>(){
		_f = nullptr;
	}

	FBind<D, T>(FBind<D, T>&&) = default;
	FBind<D, T>& operator= (const FBind<D, T>& other){
		_f = other._f;
		return *this;
	}
///apply
	D operator ()(T a){
		return (_f)(a); 
	}
	inline bool complete() {return true;}
};

//////////////////////////////////////////////////// for Demo: Output from the Wrapper  /////////////////////////////////
//     an discriptiv output of the function
namespace std{
#ifndef __clang__      //problems with clang 3.5
	template <typename D, typename T>
	ostream& operator <<(ostream& ost, const class FBind<D, T>& bound){
		ost << " bound(2): " << typeid(T).name() << " => " << typeid(D).name() << endl;
			return ost;
	}
#endif	

//problems with clang 3.5
	template <typename D, typename T, typename ...Args>
	ostream& operator <<(ostream& ost, const class FBind<D, T, Args...>& bound){
		
		ost << " bound(2+): " << typeid(T).name() << " => " << typeid(D).name() << "\t -> ...\n\t" ;
#ifndef __clang__		
		FBind<D, Args...> b1;
		ost << b1 ;
#endif		
		return ost;
	}
}

//////////////////////////////////////////////////// makes the FBind from the function, to avoid redundances /////////////////////////////////


template <typename R, typename ...Args> 
auto mkFBound(function<R(Args...)> f) ->FBind<R,Args...>{
  return FBind<R,Args...>(f);
}

/////////////////////////////////////////////////////////////////////////////////////

int main()
{

	function< string(char, long, double)> fTest = [](  char c, long lvalue,double dvalue)->string{
			stringstream ist; ist << c << ":" << dvalue << "  " << dvalue*lvalue << " (" << lvalue << ")" << endl;
			return ist.str();
		};

	//FBind<string, char, long, double> _f(fTest);
	
	auto _f= mkFBound(fTest);  //makes the upcoming vs2014 c++-compiler crash

	/**/
	auto ab1 = _f(' R');
	cout << endl << "we have applied a char "<< endl<< ab1 << endl<<"======================================================="<<endl;
	
	auto ab2 = ab1(11);
	cout << endl << "we have applied a int to the first result "<< endl<< ab2 << endl<<"---------------------------------------------------------------"<<endl;
	auto ab2_1 = ab1(13);
	cout << endl << "we have applied another int to the first result "<< endl<< ab2_1 << endl<<"======================================================="<<endl;

	auto ab3 = ab2(0.333);
	auto ab3_1 = ab2_1(0.333);
	cout << endl << "we have applied the double, the function itself has been called " << endl<<"---------------------------------------------------------------"<<endl;
	cout << endl << ab3 << endl;
	cout << endl << ab3_1 << endl;

	return 0;
}

