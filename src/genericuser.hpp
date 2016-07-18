// -*- compile-command: "g++ -x c++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_GENERICUSER_MAIN -o genericuser genericuser.hpp" -*-

#ifndef GENERICUSER_HPP_SOURCE
#define GENERICUSER_HPP_SOURCE

/**
 * @file
 *
 * Templates for creating callback object.
 */



/**
 * @brief Simple interface for callback.
 * @sa Generic_User
 * @sa @ref Templated_Callbacks
 */
template <class T>
class IGeneric_Callback
{
public:
   virtual void operator()(T &t) const = 0;
};

/**
 * @brief Generic User class for callbacks with constructed objects.
 *
 * @tparam T      Class of the object that will be constructed.
 * @tparam Func   Function object that takes a reference to a class C object.
 *
 * @sa IGeneric_Callback
 * @sa @ref Templated_Callbacks
 */
template <class T, class Func>
class Generic_User : public IGeneric_Callback<T>
{
protected:
   const Func &m_f;
public:
   Generic_User(const Func &f) : m_f(f) { }
   virtual void operator()(T &t) const  { m_f(t); }
};


/**
 * @name Pointer version of Generic user templates.
 * @{
 */
template <class T>
class IGeneric_Callback_Pointer
{
public:
   virtual void operator()(T *t) const = 0;
};

template <class T, class Func>
class Generic_User_Pointer : public IGeneric_Callback_Pointer<T>
{
protected:
   const Func &m_f;
public:
   Generic_User_Pointer(const Func &f) : m_f(f) { }
   virtual void operator()(T *t) const          { m_f(t); }
};


/** @brief Const Pointer version */
template <class T>
class IGeneric_Callback_Const_Pointer
{
public:
   virtual void operator()(const T *t) const = 0;
};

template <class T, class Func>
class Generic_User_Const_Pointer : public IGeneric_Callback_Const_Pointer<T>
{
protected:
   const Func &m_f;
public:
   Generic_User_Const_Pointer(const Func &f) : m_f(f) { }
   virtual void operator()(const T *t) const          { m_f(t); }
};

/**@}*/

//[Generic_String_Definitions]
using IGeneric_String_Callback = IGeneric_Callback_Const_Pointer<char>;
template <class F> using Generic_String_User = Generic_User_Const_Pointer<char,F>;
//[Generic_String_Definitions]


#endif

#ifdef INCLUDE_GENERICUSER_MAIN

#include <istdio.h>

struct bogus
{
   int    m_int;
   double m_double;
};


class class_test_generic
{
public:
   class_test_generic(void) { }
   static void build(IGeneric_Callback<bogus> &cb)
   {
      bogus b = { 10, 10.0 };
      cb(b);
   }
   
};

void show_bogus(bogus &val)
{
   printf("test_generic received a bogus %d, %g\n",
          val.m_int,
          val.m_double);
}

void test_generic(void)
{
   auto f = [](bogus &val)
   {
      show_bogus(val);
   };

   Generic_User<bogus, decltype(f)> cb(f);
   class_test_generic::build(cb);
}


template <class F>
void template_function(F f)
{
   bogus b2 = { 20, 20.0 };
   f(b2);
}


void test_template_function(void)
{
   auto f = [](bogus &b)
   {
      show_bogus(b);
   };

   template_function(f);
}

int main(int argc, char** argv)
{
   printf("Hello, mom!\n");
   return 0;
}
#endif

/**
 * @page Templated_Callbacks Templated Callbacks
 *
 * This project uses a design pattern where, with the exception of
 * some C library functions, all objects are created on the stack.
 * This is discussed further in @ref Discuss_Stack.
 *
 * Objects that have been created in a function's stack frame cannot be
 * safely returned because the memory they occupy has been returned to the
 * stack and may be overwritten before subsequent usage.  SchemaFW avoids
 * this by giving the calling function to the object through a callback
 * rather than the return value.
 *
 * The IGeneric_Callback and Generic_User templates, and their derivatives,
 * aim to make declaring and implementing the callback easier.  IGeneric_Callback
 * declares an unspecified class that is an argument to a function, and the
 * Generic_User instantiation will contain the code that executes when an
 * object of the template parameter is returned.
 *
 * The following example will better explain how to use IGeneric_Callback:
 *
 * @code
class MyFancyClass
{
protected:
   obj *m_array;
   int m_count;
   MyFancyClass(obj *array, int count) : m_array(array), m_count(count) { }
   
public:
   static void build(objpool &pool, IGeneric_Callback<MyFancyClass> &cb)
   {
      obj *array = nullptr;
      int count = 0;
      .
      . use pool to build array and count the elements.
      .
      MyFancyClass mfc(array, count);
      cb(mfc);
   }
};
 * @endcode
 *
 * Things to note in this class:
 * - A protected constructor prevents direct instantiation.
 * - The build function uses a template to declare the callback parameter.
 * - Finally an object is created with the protected constructor and passed
 * on through the cb argument.
 *
 * An example of how to use this class:
 * @code
void Use_A_FancyClass(objpool &pool)
{
   auto f = [](MyFancyClass &mfc)
   {
      mfc.do_something();
   }
   Generic_User<MyFancyClass, decltype(f)> gu(f);
   MyFancyClass::build(pool, gu);
}
 * @endcode
 *
 * Another important use of IGeneric_Callback is to make it easier to put
 * some longer member functions of a template class into a source file so they
 * won't be copied for each instantiation of the class.  Look at the source
 * code for SimpleProcedure::build_query_string(const char*, int, const IGeneric_String_Callback &) for an example.
 *
 * @section Using_Alias Making an Alias
 *
 * It is a common practice to provide an alias for template classes, especially
 * when there are multiple template parameters.  One example of such an alias is
 * the paired IGeneric_String_Callback and IGeneric_String_User.  This type of
 * callback is often needed for returning a constructed string on the stack.
 *
 * @snippet genericuser.hpp Generic_String_Definitions
 *
 * To use the above:
 *
 * @code
 * template <class F>
 *
 * @code
 * template <class Func> using AliasClassName = Generic_User_Pointer<ab_handle,Func>;
 * @endcode
 * and use it like this:
 * @code
 * AliasClassName<decltype(lambdafunc)> acn(lambdafunc);
 * @endcode 
 * 
 * @todo Make link to SchemaReader::build source code.
 *
 * @note The lambda function could have used a capture.  See
 * @ref NonRecursive_DataStack for an example of using a capturing lambda.
 *
 * @note The declaration of the Generic_User class uses a decltype() for its second
 * template parameter.  The decltype returns the type of the argument so the template
 * can properly generate the code.
 */
