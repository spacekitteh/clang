// RUN: %clang_cc1 -fsyntax-only -verify -std=c++11 %s

struct X0 { // expected-note 8{{candidate}}
  X0(int*, float*); // expected-note 4{{candidate}}
};

template<typename T, typename U>
X0 f0(T t, U u) {
  X0 x0(t, u); // expected-error{{no matching}}
  return X0(t, u); // expected-error{{no matching}}
}

void test_f0(int *ip, float *fp, double *dp) {
  f0(ip, fp);
  f0(ip, dp); // expected-note{{instantiation}}
}

template<typename Ret, typename T, typename U>
Ret f1(Ret *retty, T t, U u) {
  Ret r0(t, u); // expected-error{{no matching}}
  return Ret(t, u); // expected-error{{no matching}}
}

void test_f1(X0 *x0, int *ip, float *fp, double *dp) {
  f1(x0, ip, fp);
  f1(x0, ip, dp); // expected-note{{instantiation}}
}

namespace PR6457 {
  template <typename T> struct X { explicit X(T* p = 0) { }; };
  template <typename T> struct Y { Y(int, const T& x); };
  struct A { };
  template <typename T>
  struct B {
    B() : y(0, X<A>()) { }
    Y<X<A> > y;
  };
  B<int> b;
}

namespace PR6657 {
  struct X
  {
    X (int, int) { }
  };

  template <typename>
  void f0()
  {
    X x = X(0, 0);
  }

  void f1()
  {
    f0<int>();
  }
}

// Instantiate out-of-line definitions of static data members which complete
// types through an initializer even when the only use of the member that would
// cause instantiation is in an unevaluated context, but one requiring its
// complete type.
namespace PR10001 {
  template <typename T> struct S {
    static const int arr[];
    static const int x;
    static int f();
  };

  template <typename T> const int S<T>::arr[] = { 1, 2, 3 };
  template <typename T> const int S<T>::x = sizeof(arr) / sizeof(arr[0]);
  template <typename T> int S<T>::f() { return x; }

  int x = S<int>::f();
}

namespace PR7985 {
  template<int N> struct integral_c { };

  template <typename T, int N>
  integral_c<N> array_lengthof(T (&x)[N]) { return integral_c<N>(); } // expected-note 2{{candidate template ignored: could not match 'T [N]' against 'const Data<}}

  template<typename T>
  struct Data {
    T x;
  };

  template<typename T>
  struct Description {
    static const Data<T> data[];
  };

  template<typename T>
  const Data<T> Description<T>::data[] = {{ 1 }}; // expected-error{{cannot initialize a member subobject of type 'int *' with an rvalue of type 'int'}}

  template<>
  const Data<float*> Description<float*>::data[];

  void test() {
    integral_c<1> ic1 = array_lengthof(Description<int>::data);
    (void)sizeof(array_lengthof(Description<float>::data));

    sizeof(array_lengthof( // expected-error{{no matching function for call to 'array_lengthof'}}
                          Description<int*>::data // expected-note{{in instantiation of static data member 'PR7985::Description<int *>::data' requested here}}
                          ));

    array_lengthof(Description<float*>::data); // expected-error{{no matching function for call to 'array_lengthof'}}
  }
}

namespace PR13064 {
  // Ensure that in-class direct-initialization is instantiated as
  // direct-initialization and likewise copy-initialization is instantiated as
  // copy-initialization.
  struct A { explicit A(int); }; // expected-note{{here}}
  template<typename T> struct B { T a { 0 }; };
  B<A> b;
  // expected-note@+1 {{in instantiation of default member initializer}}
  template<typename T> struct C { T a = { 0 }; }; // expected-error{{explicit}}
  C<A> c; // expected-note{{here}}
}

namespace PR16903 {
  // Make sure we properly instantiate list-initialization.
  template<typename T>
  void fun (T it) {
  	int m = 0;
  	for (int i = 0; i < 4; ++i, ++it){
  		m |= long{char{*it}};
  	}
  }
  int test() {
  	char in[4] = {0,0,0,0};
  	fun(in);
  }
}

namespace ReturnStmtIsInitialization {
  struct X {
    X() {}
    X(const X &) = delete;
  };
  template<typename T> X f() { return {}; }
  auto &&x = f<void>();
}
