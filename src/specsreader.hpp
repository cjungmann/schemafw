// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -fno-inline -pedantic -ggdb -DINCLUDE_SPECSREADER_MAIN -o specsreader specsreader.cpp" -*-

#ifndef SPECSREADER_HPP_SOURCE
#define SPECSREADER_HPP_SOURCE

#include "adbranch.hpp"
#include "genericuser.hpp"

void print_xml_attribute(FILE *out,
                         const char *tag,
                         const char *value);

void print_xml_attribute(FILE *out,
                         const char *name,
                         unsigned long value);

/**
 * @brief Index of mode lines from an Advisor/Specs file.
 *
 * This class keeps both the position and the value of the mode lines.
 * This allows immediate access to the value of single-line global values
 * (the old version required reading from the Advisor file using callbacks),
 * and the ability to search for a mode by name + value instead of just
 * by name.
 *
 * The name + value search is used for searching for $shared modes.
 *
 * @sa @ref Specs_Shared_Mode
 * @sa SpecsReader
 * @ref Specs_Shared_Mode.
 */
class Advisor_Index
{
public:
   struct info
   {
      long int   m_position;
      const char *m_value;

      inline bool is_equal_to(const char *v) const { return 0==strcmp(v,m_value); }
      inline bool operator==(const char* v) const  { return 0==strcmp(v,m_value); }
      inline bool operator!=(const char* v) const  { return 0!=strcmp(v,m_value); }
      inline operator const char*(void) const      { return m_value; }
      inline operator long int(void) const         { return m_position; }

      inline const char *value(void) const         { return m_value; }
      inline long position(void) const             { return m_position; }

      inline bool is_setting(void) const           { return m_value!=nullptr; }
   };

protected:
   DataStack<info> &m_ds;
   Advisor_Index(DataStack<info> &ds)
      : m_ds(ds)   { }

public:
   static void t_build(Advisor &advisor, IGeneric_Callback<Advisor_Index> &callback);
   
   template <class Func>
   inline static void build(Advisor &advisor, const Func &f)
   {
      Generic_User<Advisor_Index, Func> user(f);
      t_build(advisor, user);
   }

public:
   /**
    * @brief Returns the first t_handle for iteration.
    * @sa @ref T_HANDLE_Iteration
    **/
   inline const t_handle<info>* start(void) const { return m_ds.start(); }
   const t_handle<info>* seek(const char *name, const char *value=nullptr) const;

   template <class Func>
   inline void scan(const Func &f) const
   {
      for (const auto* ptr = m_ds.start(); ptr; ptr = ptr->next())
      {
         f(ptr->str(), ptr->object().value());
      }
   }
     
   
   long int position(const char *name, const char *value=nullptr) const;

   void print_modes(FILE *f, bool include_shared=false) const;
      
};

/**
 * @brief This class speeds access to the contents of a Specs file by making
 * an index of it modes.
 *
 * This class scans the entire Advisor file to note the position and values
 * of every mode contained in the file.  Using the index, the modes can be
 * accessed by directly reading from the indicated position.  Perhaps more
 * importantly, it prevents the futile full-file scan when looking for a
 * mode that might not exist.
 *
 * Most functions have a callback parameter through which the results of a
 * search are returned.  For member functions that begin with seek_, the
 * callback is always called with a pointer that is null if the item is not
 * found.  Functions that begin with get_ will only invoke the callback
 * function if the mode is found.  Use seek_ functions if you must act on
 * failure as well as success, and use get_ functions if nothing is done
 * in case of failure.  Compare seek_mode(const char *name, const Func &callback)
 * with get_mode(const char *name, const Func &callback).
 *
 * @sa @ref Specs_File
 * @sa @ref Using_SpecsReader
 * @sa Advisor_Index
 */
class SpecsReader
{
public:
   struct payload
   {
      long int    m_position;
      const char *m_value;
   };
protected:
   Advisor             &m_advisor; /**< Advisor file reader. */
   const Advisor_Index &m_index;   /**< Advisor_Index for getting global mode information. */

protected:
   /** @brief Protected constructor to prevent direct instantiation. */
   SpecsReader(Advisor &advisor, const Advisor_Index &index)
      : m_advisor(advisor), m_index(index)     {  }


public:
   /**
    * @defgroup SpecsReader building functions
    * @{
    */
   static void t_build(Advisor &advisor, IGeneric_Callback<SpecsReader> &cb);
   static void t_build(const char *path, IGeneric_Callback<SpecsReader> &cb);
   
   /** @brief Template build function that takes a function object that can be a lambda. */
   template <class Func>
   static void build(Advisor &advisor, Func &f)
   {
      Generic_User<SpecsReader, Func> user(f);
      t_build(advisor,user);
   }
   
   // template <class Func>
   // static void build(const char *name, Func &f)
   // {
   //    Generic_User<SpecsReader, Func> user(f);
   //    t_build(name, user);
   // }

   template <class Func>
   static void build(const char *path, Func f)
   {
      auto fGotFile = [&f](AFile_Handle &afh)
      {
         Advisor advisor(afh);
         auto fGotIndex = [&f, &advisor](Advisor_Index &ai)
         {
            SpecsReader sr(advisor, ai);
            f(sr);
         };
         Generic_User<Advisor_Index,decltype(fGotIndex)> gu(fGotIndex);

         Advisor_Index::t_build(advisor, gu);
      };

      AFile_Handle::build(path, fGotFile);
   }


   /**@}*/


   void print_as_xml(FILE *out, const ab_handle *handle);
   void print_sub_elements(FILE *out, const ab_handle *branch, const char **skip=nullptr);

   inline void print_modes(FILE *out, bool include_shared=false) const
   {
      m_index.print_modes(out, include_shared);
   }

   template <class Func>
   inline void scan_modes(const Func &f) const { m_index.scan(f); }

   long int get_advisor_position(void) const { return m_advisor.get_position(); }
   void restore_advisor_position(long int pos) { m_advisor.restore_state(pos); }

   /**
    * @brief Gets a branch at position, if position>=0.
    *
    * @tparam Func Callback function whose parameter is const ab_handle *.
    * @param position  The position of the branch.  Nothing is done if position<0.
    * @param callback The function object to call with the branch.
    *
    * This function invokes callback only if the position>=0.  The reason for this
    * function is to avoid the multiplication of instantiations of several template
    * functions with a single lambda.  Every template get_ function should call
    * this function with the results of the appropriate get_xxxxx_position function.
    */
   void get_branch(long int position,
                   const IGeneric_Callback_Const_Pointer<ab_handle> &callback) const
   {
      if (position>=0)
      {
         m_advisor.restore_state(position);
         ab_handle::t_build(m_advisor, callback);
      }
   }

   using abh_callback = IGeneric_Callback_Const_Pointer<ab_handle>;
   template <class F> using abh_user = Generic_User_Const_Pointer<ab_handle, F>;
      
   inline static int len_handle(const Advisor &adv) { return ab_handle::len_handle(adv); }
   inline static ab_handle* make_handle(void *buff,
                                               const Advisor &adv,
                                               ab_handle *parent=nullptr)
   {
      return ab_handle::make_handle(buff,adv,parent);
   }
   
   inline static int len_shared_handle(const Advisor &adv) { return ab_handle::len_value_handle(adv); }
   inline static ab_handle *make_shared_handle(void *buff,
                                               const Advisor &adv,
                                               ab_handle *parent=nullptr)
   {
      return ab_handle::make_value_handle(buff, adv, parent);
   }

   /**
    * @brief Seeks a branch at position, if position>=0.
    *
    * @tparam Func Callback function whose parameter is const ab_handle *.
    * @param position  The position of the branch.  Nothing is done if position<0.
    * @param callback The function object to call with the branch.
    *
    * This function always invokes the callback, but it will be invoked with a
    * null if the branch doesn't exist..  The reason for this function is to
    * avoid the multiplication of instantiations of several template functions
    * with a single lambda.  Every template seek_ function should call this
    * function with the results of the appropriate get_xxxxx_position function.
    */
   void seek_branch(long int position,
                    const IGeneric_Callback_Const_Pointer<ab_handle> &callback) const
   {
      if (position>=0)
      {
         m_advisor.restore_state(position);
         ab_handle::t_build(m_advisor, callback);
      }
      else
         callback(nullptr);
   }
   
   const Advisor_Index::info* seek_mode_info(const char *name,
                                             const char *value=nullptr) const;
   
   long int get_mode_position(const char *name, const char *value=nullptr) const;
   long int get_global_mode_position(const char *name, const char *value=nullptr) const;

   /**
    * @brief Gets the position of a shared global branch or -1 if not found.
    *
    * Since the signal to retrieve a global shared mode is a section setting value
    * that begins with '$', we will assume that the name begins with '$'.  The
    * shared global node value does not have a '$', so always skip the first
    * character of the name before searching for the mode.
    */
   long int get_shared_mode_position(const char *name) const
   {
      return get_mode_position("$shared", ++name);
   }

   /**
    * @brief Gets a branch built from a global mode.  Adds '$' prefix to name.
    *
    * This function invokes callback only if the global mode is found.
    *
    * @tparam Func Callback function whose parameter is const ab_handle *.
    * @param name Name of global mode.  Adds '$' so make sure name doesn't begin with '$'.
    * @param value Optional value search filter.  If null, invokes callback for first found mode whose tag is name.
    */
   template <class Func>
   void get_global_mode(const char *name,
                          const char *value,
                          const Func &callback) const
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(callback);
      get_branch(get_global_mode_position(name,value), user);
   }

   /**
    * @brief Seeks a branch built from a global mode.  Adds '$' prefix to name.
    *
    * This function invokes callback with global mode if found, otherwise with null.
    *
    * @tparam Func Callback function whose parameter is const ab_handle *.
    * @param name Name of global mode.  Adds '$' so make sure name doesn't begin with '$'.
    * @param value Optional value search filter.  If null, invokes callback for first found mode whose tag is name.
    */
   template <class Func>
   void seek_global_mode(const char *name,
                         const char *value,
                         const Func &callback) const
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(callback);
      seek_branch(get_global_mode_position(name, value), user);
   }

   template <class Func>
   void seek_shared_mode(const char *name, const Func &callback) const
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(callback);
      seek_branch(get_shared_mode_position(name), user);
   }

   /**
    * @brief Use this function to iterate through modes.
    * @sa @ref T_HANDLE_Iteration
    */
   const t_handle<Advisor_Index::info>* start(void) const { return m_index.start(); }

   /**
    * @brief Get a mode by position.
    *
    * @tparam Func Function object, can be a callback, a lambda, or a Functor
    * with a operator()(const ab_handle*) member.
    *
    * This function is a bit different from other get_ functions in that
    * it may return nonsense if the position used to access the Advisor file
    * is not valid.
    */
   template <class Func>
   void get_mode(long position, const Func &callback)
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(callback);
      get_branch(position,user);
   }

   /**
    * @brief Get a mode by name.
    *
    * @tparam Func Function object, can be a callback, a lambda, or a Functor
    * with a operator()(const ab_handle *) member.
    *
    * @param name Name of the desired mode.
    * @param callback The callback function object.
    *
    * Searches for the position of the name.  The callback function will be
    * called only if the name is found.
    *
    * If your code requires a response regardless of the success finding
    * the mode, use SpecsReader::seek_mode instead.
    *
    * @code
    void use_get_mode(SpecsReader &sr, const char *name)
    {
       auto f = [](const ab_handle *mode)
       {
          // This code is only executed if name is found,
          // and mode will never be null.
          const ab_handle *proc = mode->seek("procedure");
          printf("For mode %s, the procedure is %s\n", mode->str(), proc->value());
       };
       sr.get_mode(name, f);
    }
    * @endcode
    */
   template <class Func>
   void get_mode(const char *name, const Func &callback)
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(callback);
      get_branch(m_index.position(name), user);
   }

   /**
    * @brief Get a handle to a mode.
    *
    * @tparam Func A function object that has a const ab_handle* parameter.
    *
    * Use to get global modes, this does not make a new SpecsReader,
    * but rather it calls f() with a const ab_handle*, the specs-file
    * mode if found, or nullptr if not.
    *
    * Use this function if you need to know when the seek fails
    *
    * @tparam Func Function object that takes const ab_handle* value.
    * @param name Name of mode to retrieve.
    *
    * @code
    void use_seek_mode(SpecsReader &sr, const char *name)
    {
       auto f = [&name](const ab_handle *mode)
       {
          // This function will be called if name is found or not.
          // You must check mode for a null value in case name is
          // not found.
          if (mode)
          {
             const ab_handle *proc = mode->seek("procedure");
             printf("For mode %s, the procedure is %s\n", mode->str(), proc->value());
          }
          else
             printf("Sorry, no procedure name \"%s\" found.\n", name);
       };
       sr.seek_mode(name, f);
    }
    * @endcode
    */
   template <class Func>
   void seek_mode(const char *name, const Func &callback)
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(callback);
      seek_branch(m_index.position(name), user);
   }

   /**
    * @brief Retrieve a shared global mode.
    *
    * Shared modes are global modes named $shared whose tag is a name
    * that is used to identify the mode for searching.  If the shared mode
    * exists, this function will invoke the callback with a branch constructed
    * from appropriate lines from the Specs file.
    *
    * The name parameter should still be prefixed with a '$' since the motivation
    * for searching for a shared mode is setting mode with a value prefixed with '$'.
    * If you need your name, you'll have to build the string yourself (at least until
    * I decide I need to do it, in which case I'll probably add function to do it).
    *
    * @tparam Func A function object (usually a lambda) whose sole parameter is const ab_handle*.
    * @param name The name of the shared object.  The name should still be prefixed with '$'.
    * @param callback A reference to the Func function object.  It's const so it can be a temporary argument.
    */
   template <class Func>
   void get_shared_mode(const char *name, const Func &callback)
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(callback);
      get_branch(get_shared_mode_position(name), user);
   }

   /** @brief Same as get_shared_mode except that it always invokes callback. */
   template <class Func>
   void seek_shared_mode(const char *name, const Func &callback)
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(callback);
      seek_branch(get_shared_mode_position(name), user);
   }
   

   void seek_specs_handle(const char *tag,
                          const IGeneric_Callback_Const_Pointer<ab_handle> &cb,
                          const ab_handle *mode_first,
                          const ab_handle *mode_second = nullptr,
                          const ab_handle *mode_third = nullptr) const;


   /**
    * @build Calls callback() with the handle if found or null if not.
    */
   template <class Func>
   void seek_handle_or_global(const ab_handle *node,
                             const char *globalname,
                             const Func &callback)
   {
      if (node)
         callback(node);
      else if (m_advisor.seek_tag(globalname, 0, true))
         ab_handle::build(m_advisor, callback);
   }

   
   static ab_handle *find_shared_link(ab_handle *mode_chain_head, const char *name);
   
   static void replace_shared_ref(ab_handle *head, ab_handle *ref);
   static void replace_all_shared_refs(ab_handle *head, ab_handle *branch, bool skip_siblings=false);
   static void reconcile_shares(ab_handle *head);

   void t_build_branch(long int position, abh_callback &callback) const;
   /** @brief Template function calling t_build_branch() with appropriate callback class. */
   template <class Func>
   void build_branch(long int position, const Func &func) const
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(func);
      t_build_branch(position, user);
   }


protected:
   bool is_valid_position(long int) const;

};


#endif
