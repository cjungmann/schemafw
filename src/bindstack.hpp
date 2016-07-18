// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_BINDSTACK_MAIN `mysql_config --cflags` -o bindstack bindstack.cpp `mysql_config --libs`" -*-

#ifndef BINDSTACK_HPP_SOURCE
#define BINDSTACK_HPP_SOURCE

#include "genericuser.hpp"
#include "bindc.hpp"

/**
 * @brief Wrapper class around an array of MYSQL_BIND elements for binding input
 *        parameters to a MYSQL_STMT.
 *
 * The BindStack class, derived from DataStack<BindC>, adds features
 * to support IParam_Setter.  The additional features are;
 * - find() functions to access the appropriate input parameter when the
 *   data for the parameter is available.
 * - build(MYSQL_RES*, const Func&) to build an input DataStack<BindC> from the
 *   parameters of a stored procedure.
 * - build(const char*, const Func&) to build an input DataStack<BindC> from a
 *   typestr.
 *
 * The BindStack is particularly important for input bound parameters due to the
 * search ability.  MySQL receives (as bound parameters), and sends data to prepared statements
 * using arrays of MYSQL_BIND elements.  BindStack, and its parent DataStack<BindC>,
 * are wrappers around arrays of MYSQL_BIND, providing the means to initialize,
 * and to set values to and read values from MYSQL_STMT objects.
 *
 * A _typestr_ is used to define the members of a BindStack.  See @ref _CType for
 * instructions on how to create a typestr.
 *
 * There are several ways to create a BindStack to use for input parameters:
 * -# Calling BindStack::build() with a typestr and a callback function.
 * -# With a typestr value in as the third argument of
 *    SimpleProcedure::build(MYSQL* const char* query, const char* typestr, const IParam_Setter*, IResult_User*);
 * -# Using a StoredProc object that queries MySQL for the parameters info, passing
 *    the information as the third argument of
 *    SimpleProcedure::build(MYSQL *conn, const char *query, MYSQL_RES *result, const IParam_Setter *ps, IResult_User *ru).
 *
 * After creating the BindStack, it is necessary to set the values of the BindStack.
 * This is done with a class derived from IParam_Setter, of which there are several.
 */
class BindStack : public DataStack<BindC>
{
public:
   BindStack(line_handle *line_array, int line_count)
      : DataStack(line_array, line_count)        { }


public:
   /** @brief Returns pointer to array of MYSQL_BINDs held in this BindStack. */
   MYSQL_BIND *binds(void) { return start()->object().m_bind; }

   void set_null_all_binds(void);

   /**
    * @name BindStack building functions.
    *
    * @tparam Func A function object that takes a BindStack reference for its
    * only parameter.  The function object can be a lambda, function pointer, or
    * a class that defines an operator()(BindStack &) member function.
    * @{
    */
   /** @brief Use a procedure result to Build a BindStack */
   template <class Func>
   static void build(MYSQL_RES *res, const Func &f)
   {
      Generic_User<BindStack, Func> user(f);
      t_build(res, user);
   }

   /**
    * @brief Use a typestring to build a BindStack.
    *
    * Using a string constructed from the types found in _CType,
    * build a BindStack object for setting a procedure's parameters.
    */
   template <class Func>
   static void build(const char *typestr, const Func &f)
   {
      Generic_User<BindStack, Func> user(f);
      t_build(typestr, user);
   }
   /**@}*/

   const BindC* find(const char *name) const { return static_cast<const BindC*>(data(name)); }
   BindC* find(const char *name)             { return static_cast<BindC*>(data(name)); }

protected:
   static const int s_max_buffer_length = 1024;

   static size_t calc_buffer_size(const _CType *ctype, size_t requested);
   static unsigned count_types(const char *typestr);

   static void t_build(MYSQL_RES *res, IGeneric_Callback<BindStack> &user);
   static void t_build(const char *typestr, IGeneric_Callback<BindStack> &user);
};



#endif

