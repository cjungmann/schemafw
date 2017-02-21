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

class Path_List;


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
      const char *m_filepath;
      const char *m_value;
      bool       m_has_children;

      inline bool is_equal_to(const char *v) const { return 0==strcmp(v,m_value); }
      inline bool operator==(const char* v) const  { return 0==strcmp(v,m_value); }
      inline bool operator!=(const char* v) const  { return 0!=strcmp(v,m_value); }
      inline operator const char*(void) const      { return m_value; }
      inline operator long int(void) const         { return m_position; }

      inline long position(void) const             { return m_position; }
      inline const char *filepath(void) const      { return m_filepath; }
      inline const char *value(void) const         { return m_value; }
      inline bool has_children(void) const         { return m_has_children; }
      inline bool is_external(void) const          { return m_filepath!=nullptr; }

      inline bool is_setting(void) const           { return m_value!=nullptr; }
   };

   typedef t_handle<info> ninfo;

   friend class Path_List;

   struct BEnds
   {
      ninfo *m_head;
      ninfo *m_tail;
      int   m_includes;
      BEnds(ninfo* mode=nullptr) : m_head(mode), m_tail(mode), m_includes(0) {}

      void append(ninfo *node);
      void scan_for_includes(Path_List &pl, IGeneric_Callback<ninfo*> &callback);
      bool is_empty(void) const { return 0==m_head; }
   };

   static void t_collect_modes(Advisor &advisor,
                               IGeneric_Callback<BEnds&> &callback,
                               const char *filepath = nullptr);
   
   static void t_collect_include_modes(const char *filename,
                                       IGeneric_Callback<BEnds&> &callback);

protected:
   DataStack<info> &m_ds;
   Advisor_Index(DataStack<info> &ds)
      : m_ds(ds)   { }

public:
   static void t_build_new(Advisor &advisor,
                           IGeneric_Callback<Advisor_Index> &callback);
   
   static void t_build_old(Advisor &advisor, IGeneric_Callback<Advisor_Index> &callback);
   
   template <class Func>
   inline static void build(Advisor &advisor, const Func &f)
   {
      Generic_User<Advisor_Index, Func> user(f);
      t_build_new(advisor, user);
   }

public:
   /**
    * @brief Returns the first t_handle for iteration.
    * @sa @ref T_HANDLE_Iteration
    **/
   inline const t_handle<info>* start(void) const { return m_ds.start(); }
   const t_handle<info>* seek(const char *name, const char *value=nullptr) const;

   /**
    * @brief Calls the template function with each instruction.
    *
    * The template function type should look like this:
    *
    * `[](const char *tag, const char *value) -> bool`
    *
    * returning `true` to continue and `false` to terminate.
    */
   template <class Func>
   inline void scan(const Func &f) const
   {
      for (const auto* ptr = m_ds.start();
           ptr && f(ptr->str(), ptr->object().value());
           ptr = ptr->next())
      {
      }
   }
     
   
   long int position(const char *name, const char *value=nullptr) const;

   void print_modes(FILE *f, bool include_shared=false) const;
      
};

/**
 * @brief Class to manage a stack-allocated list of advisor file names.
 */
class Path_List
{
   const char           *m_path;      /**< path to include file **/
   Advisor_Index::ninfo *m_precedent; /**< t_handle that points to the include mode
                                       *   Will have its next pointer replaced with
                                       *   the head of the contents of the include file.
                                       */
   Path_List  *m_next;      /**< Pointer to the next include file, if any.*/

public:
   Path_List(const char *npath=nullptr, Advisor_Index::ninfo *precedent=nullptr)
      : m_path(npath), m_precedent(precedent), m_next(nullptr)
   {}

   inline bool is_not_equal(const char *cpath) const { return !m_path || strcmp(cpath,m_path)!=0; }
   inline bool is_equal(const char* cpath) const     { return m_path && strcmp(cpath,m_path)==0; }
   inline bool operator==(const char* cpath) const   { return is_equal(cpath); }

   inline Path_List* attach_next(Path_List &npl)
   {
      Path_List *save = m_next;
      m_next = &npl;
      return save;
   }

   /**
    * @brief Scans list for matching name.
    *
    * @param cname Candidate name for which to search
    * @return Matching node if found, last element if not found.
    *         It will be necessary to check the return value for
    *         a match to distinguish between a successful or failed
    *         search.
    */
   Path_List *scan(const char *cpath)
   {
      if (m_next && !is_equal(cpath))
         return m_next->scan(cpath);
      else
         return this;
   }

   /**
    * @brief Add, if unique, a stack-allocated Path_List element to the end of the list.
    *
    * Must be called from the root of the Path_List in order that the new path
    * name is checked against existing inventory.  If that rule is followed, each
    * path string will only be represented once in the list.
    */
   void append(Path_List &pl)
   {
      if (pl.m_path)
      {
         Path_List *tail = scan(pl.m_path);
         if (tail && tail->is_not_equal(pl.m_path))
            tail->attach_next(pl);
      }
   }

   void process(Advisor_Index::ninfo* head,
                int count,
                IGeneric_Callback<Advisor_Index> &callback);

   void send_index(Advisor_Index::ninfo* root,
                   IGeneric_Callback<Advisor_Index> &callback);
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

   /**
    * @brief Build a SpecsReader from the path to an SRM file.
    *
    * This function uses AFile_Handle::build() and Advisor_Index::t_build()
    * and their callbacks to conveniently construct a SpecsReader in one
    * function call.
    *
    * ~~~c++
    * auto f_sreader [](SpecsReader &sr)
    * {
    *    // use the SpecsReader object...
    * };
    *
    * SpecsReader::build(path_to_srm, f_sreader);
    * ~~~
    */
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

         Advisor_Index::t_build_new(advisor, gu);
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
   
   const t_handle<Advisor_Index::info>* seek_advisor_mode(const char *tag,
                                                          const char *value) const;
   
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

   const t_handle<Advisor_Index::info>* get_shared_advisor_mode(const char *name) const
   {
      return seek_advisor_mode("$shared", ++name);
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
