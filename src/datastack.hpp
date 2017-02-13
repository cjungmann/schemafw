// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_MAIN -o datastack datastack.cpp" -*-

#ifndef DATASTACK_HPP
#define DATASTACK_HPP

#include <string.h>
#include <assert.h>
#include <stdexcept>

template <class T> class StackBuilder;

typedef size_t      data_os;
typedef const char* line_link;
typedef const char* line_handle;

struct DMAP
{
   data_os   offset;  /**< Subtract this from &this to get pointer to data. */
   line_link link;
};


// forward declaration to allow being named a friend
class BaseStack;

/**
 * @brief Provides access to the type-independent parts of a line_handle.
 *
 * A line_handle, and thus the pointer to the line, is a pointer to char string.
 * The rest of the data comes before the char string.  The memory diagram looks
 * like this:
 * dddddddddddddMMMMssssssssss
 *                  ^ line_handle points here
 *
 * The d's represent the data based on the template used to instantiate the
 * pool class. The s's represent the string name of the line.  It's possible
 * that there could be only a single s=='\0' for an unnamed line.  The M's
 * represent the fixed-length DMAP structure that comes before the string.
 * From that strictly-defined structure we can find the beginning of the data.
*/
class base_handle
{
   friend class BaseStack;
   template <class U> friend class StackBuilder;
   
public:
   virtual ~base_handle()                     { }
   inline bool valid(void) const              { return _addr()!=NULL; }
   inline line_link link(void) const          { return _cp_map()->link; }
   inline data_os offset(void) const          { return _cp_map()->offset; }
   inline line_handle lhandle(void) const     { return static_cast<line_handle>(_addr()); }
   inline int calc_size(void) const           { return static_cast<int>(strlen(_addr()) + offset()); }

   inline operator const char *(void) const   { return _addr(); }
   inline const char* str(void) const         { return _addr(); }
   inline operator line_handle(void) const    { return lhandle(); }


   inline bool is_equal_to(const char *str) const          { return valid() && strcmp(str,static_cast<const char*>(_addr()))==0; }
   /** @brief Special version for comparing path substrings. */
   inline bool is_equal_to(const char *str, int len) const { return valid() && 0==strncmp(str,static_cast<const char*>(_addr()),len); }
   inline bool operator==(const char *str) const           { return is_equal_to(str); }
   inline bool operator!=(const char *str) const           { return !is_equal_to(str); }

   inline static const base_handle* convert(line_handle h) { return _cp_handle(h); }


   /**
    * @name Static utility functions for building line_handle lists.
    *
    * For situations where it is preferable to build a DataStack by hand,
    * these functions help allocate, initialize, and bundle raw line_handles.
    * @{
    */
   static inline DMAP* get_dmap_from_line_handle(line_handle lh)
   {
      return reinterpret_cast<DMAP*>(const_cast<char*>(lh)-sizeof(DMAP));
   }

   static size_t get_line_handle_size(const char *name,
                                      size_t size_object,
                                      size_t size_extra);
   static line_handle init_line_handle(char *buffer,
                                       const char *name,
                                       size_t size_object,
                                       size_t size_extra,
                                       line_handle parent = nullptr);

   static int count(line_handle bh);
   static void fill_array(line_handle *array, int count, line_handle top);
/**@}*/


   
protected:
   /** @name Functions for calculating fundamental addresses. @{ */

   inline const char* _addr(void) const    { return reinterpret_cast<const char*>(this); }
   inline const char *_origin(void) const  { return _addr() - _oset(); }
   inline const DMAP* _cp_map(void) const  { return reinterpret_cast<const DMAP*>(_addr()-sizeof(DMAP)); }
   inline size_t _oset(void) const         { return _cp_map()->offset; }
   /**@}*/
   
//   inline const DMAP* _map(void) const     { return _cp_map(); }
//   inline DMAP* _map(void)                 { return const_cast<DMAP*>(_cp_map()); }

   /** @name Functions to cast line_handle to base_handle addresses. @{*/
   static inline const base_handle* _cp_handle(line_handle h) { return reinterpret_cast<const base_handle*>(h); }
   static inline const DMAP* _cp_map(line_handle h)           { return _cp_handle(h)->_cp_map(); }
   static inline const void* _cp_data(line_handle h)          { return static_cast<const void*>(h - _cp_map(h)->offset); }
   /**@}*/

   /** @name Non-const functions that have cast-away constness of related _cp_xxx functions. @{*/
   static inline base_handle* _p_handle(line_handle h)        { return const_cast<base_handle*>(_cp_handle(h)); }
   static inline DMAP* _p_map(line_handle h)                  { return const_cast<DMAP*>(_cp_map(h)); }
   static inline void* _p_data(line_handle h)                 { return const_cast<void*>(_cp_data(h)); }
   /**@}*/

   /** @brief Returns handle to next line of the linked line_handle set. */
   static inline line_link get_link(line_handle h)            { return _cp_map(h)->link; }
   static inline line_link* get_pp_link(line_handle h)        { return &_p_map(h)->link; }

   /** @brief Non-const set_link should only be used by StackPool::update_link_pointers. */
   static inline void set_link(line_handle h, line_link l)    { _p_map(h)->link = l; }


   /**
    * @name Prevent copy or assignment.
    *
    * Most important for this item type that it is never copied or
    * assigned, but that one only takes it address, either as a pointer
    * or a reference.
    *
    * It you use \a auto, make sure it's like <em>auto &v</em> or <em>auto *v</em>.
    * @{
    */
   base_handle(const base_handle &)             = delete;
   base_handle & operator=(const base_handle &) = delete;
   /**@}*/
   
};

/**
 * @brief Provides access to the type-dependent parts of a line_handle.
 *
 * The data section is laid out thus:
 * TTTTTTTTTxxxxx...
 * The T's represent the memory block for the template class and the x's
 * represent extra data, if requested.
 *
 * @sa @ref T_HANDLEs
 */
template <class T>
class t_handle : public base_handle
{
protected:
   static int s_fixed_size;
   inline const T* _object(void) const   { return reinterpret_cast<const T*>(_origin()); }
   
   // _extra() now using _origin() instead of _object() despite being the same
   // address. Being a T* affects the meaning of '+', jumping multiples of T
   // instead of number of bytes.
   
   /** @brief Returns pointer to address returned from alloca. */
   inline const void* _extra(void) const { return _origin() + sizeof(T); }

public:
   inline const T& object(void) const    { return *_object(); }
   inline T& object(void)                { return *const_cast<T*>(_object()); }
   inline const void *extra(void) const  { return _extra(); }
   inline void* extra(void)              { return const_cast<void*>(_extra()); }
   inline int sizeof_extra(void) const   { return static_cast<int>(_addr() - _origin() - s_fixed_size); }
   inline bool has_extra(void) const     { return sizeof_extra()>0; }

   inline const t_handle<T>* next(void) const { return cp_cast(link()); }
   inline t_handle<T>* next(void)             { return p_cast(link()); }

   inline t_handle<T>** pp_next(void)
   {
      char **p = const_cast<char**>(get_pp_link(_addr()));
      return reinterpret_cast<t_handle<T>**>(p);
   }

   /**
    * @brief Very late addition to support adding or abandoning nodes.
    *
    * @param h A t_handle node that will now serve as the next link.
    * @return The pointer value of the previous next link.
    */
   inline t_handle<T>* next(t_handle<T>* h)
   {
      t_handle<T>* s = p_cast(link());
      // Copied from init_line_handle().  May not be
      // clearest expression, but it should work.
      get_dmap_from_line_handle(*this)->link = reinterpret_cast<char*>(h);
      return s;
   }

   inline operator const char*(void) const    { return _addr(); }
   inline const char* str(void) const         { return _addr(); }

   inline static const t_handle<T>* cp_cast(line_handle h) { return reinterpret_cast<const t_handle<T>*>(h); } 
   inline static t_handle<T>* p_cast(line_handle h)        { return reinterpret_cast<t_handle<T>*>(const_cast<char*>(h)); } 
   inline static const t_handle<T>& cr_cast(line_handle h) { return *cp_cast(h); }
   inline static t_handle<T>& r_cast(line_handle h)        { return *p_cast(h); }

   /** @brief Returns size of T + sizeof DMAP. */
   inline static size_t fixed_size(void)                   { return s_fixed_size; }

   /**
    * @name These functions make it easier to build a DataStack without using an IStackPool.
    * @{
    */
   inline static size_t line_size(size_t len_name, size_t len_extra) { return s_fixed_size + len_name + 1 + len_extra; }
   inline static size_t line_size(const char *name, size_t len_extra) { return line_size(strlen(name), len_extra); }
   /**
    * @brief Initialize the handle's name and DMAP map.
    *
    * Note that it is necessary to initialize the T object and the extra data
    * in a separate step from init_handle.
    */
   inline static t_handle<T>* init_handle(void* buffer,
                                          const char *name,
                                          size_t size_extra,
                                          t_handle<T>* parent=nullptr)
   {
      return p_cast(base_handle::init_line_handle(static_cast<char*>(buffer),
                                                  name,
                                                  sizeof(T),
                                                  size_extra,
                                                  parent->lhandle()));
   }
   /**@}*/

   /**
    * @name Prevent copy or assignment.
    *
    * Most important for this item type that it is never copied or
    * assigned, but that one only takes it address, either as a pointer
    * or a reference.
    *
    * It you use \a auto, make sure it's like <em>auto &v</em> or <em>auto *v</em>.
    * @{
    */
   t_handle(const t_handle &)             = delete;
   t_handle & operator=(const t_handle &) = delete;
   /**@}*/
};

template <class T>
int t_handle<T>::s_fixed_size = static_cast<int>(sizeof(T)+sizeof(DMAP));

/**
 * @brief Provides functions to access non-type-specific elements of a DataStack.
 */
class BaseStack
{
public:
   typedef DMAP Overlay;

protected:
   template <class U> friend class StackBuilder;
   
   line_handle  *m_line_array;
   unsigned      m_line_count;

public:   
   /** @brief Protected constructor to limit creation. */
   BaseStack(line_handle *line_array, unsigned line_count)
      : m_line_array(line_array), m_line_count(line_count)   { }

private:
   /** @name Class must never be returned. @{ */
   BaseStack(const BaseStack&)            = delete;
   BaseStack& operator=(const BaseStack&) = delete;
   /**@}*/

   /** @name Internal data access functions that can be used by public member functions. @{ */
   const void* _data_by_index(unsigned index) const;
   const void* _data_by_name(const char *name) const;
   /**@}*/

public:
   virtual ~BaseStack()                           { }

   unsigned count(void) const                     { return m_line_count; }

   int index_by_name(const char *name) const;

   line_handle line_by_index(unsigned index) const;
   line_handle line_by_name(const char *name) const;
   line_handle array_start(void) const;

   const char *name_by_index(unsigned index) const  { return line_by_index(index); }
   const void* data_by_index(unsigned index) const  { return _data_by_index(index); };
   void* data_by_index(unsigned index)              { return const_cast<void*>(_data_by_index(index)); }
   const void* data_by_name(const char *name) const { return _data_by_name(name); }
   void* data_by_name(const char *name)             { return const_cast<void*>(_data_by_name(name)); }
};

/** @brief Type-specific functions of DataStack class. */
template <class T>
class DataStack : public BaseStack
{
public:
   DataStack(line_handle *line_array, int line_count)
      : BaseStack(line_array, line_count)      { }

   // line_handle find(const char *name) const { return line_by_name(name); }
   // line_handle operator[](int index) const  { return line_by_index(index); }
//   T& operator[](int index)              { return *static_cast<T*>(data_by_index(index)); }

   inline const T* _cp_object(int index) const { return static_cast<const T*>(data_by_index(index)); }
   inline T* _p_object(int index)              { return const_cast<T*>(_cp_object(index)); }
   
   /**
    * @brief Returns t_handle<T>&.  DON'T USE auto, use auto&
    *
    * This convenience function is confusing, or I should say it confused me
    * for a hour or so.  If you use auto handle = ds[0], it will create a
    * new t_handle<T> object, not a reference, as I expected (that's the
    * return type).
    */
   inline t_handle<T>& operator[](int index)             { return *const_cast<t_handle<T>*>(reinterpret_cast<const t_handle<T>*>(line_by_index(index))); }
   inline const t_handle<T>& operator[](int index) const { return *const_cast<t_handle<T>*>(reinterpret_cast<const t_handle<T>*>(line_by_index(index))); }
   inline t_handle<T>& handle(int index)                 { return *const_cast<t_handle<T>*>(reinterpret_cast<const t_handle<T>*>(line_by_index(index))); }
   inline const t_handle<T>& handle(int index) const     { return *const_cast<t_handle<T>*>(reinterpret_cast<const t_handle<T>*>(line_by_index(index))); }

//   inline const t_handle<T>* start(void) const  { return reinterpret_cast<const t_handle<T>*>(array_start()); }
   inline const t_handle<T>* start(void) const  { return reinterpret_cast<const t_handle<T>*>(line_by_index(0)); }
   inline t_handle<T>* start(void)              { return const_cast<t_handle<T>*>(reinterpret_cast<const t_handle<T>*>(line_by_index(0))); }

   inline const T& object(int index) const      { return *_cp_object(index); }
   inline T& object(int index)                  { return *_p_object(index); }

   inline const T* object_ptr(int index) const        { return static_cast<const T*>(data_by_index(index)); }
   inline T* object_ptr(int index)                    { return static_cast<T*>(data_by_index(index)); }
   inline const T* object_ptr(const char *name) const { return static_cast<const T*>(data_by_name(name)); }
   inline T* object_ptr(const char *name)             { return static_cast<T*>(data_by_name(name)); }

   inline const T* data(int index) const        { return static_cast<const T*>(data_by_index(index)); }
   inline T* data(int index)                    { return static_cast<T*>(data_by_index(index)); }
   inline const T* data(const char *name) const { return static_cast<const T*>(data_by_name(name)); }
   inline T* data(const char *name)             { return static_cast<T*>(data_by_name(name)); }
};


/**
 * @name Set of classes supporting a new DataStack callback strategy.
 *
 * In the interest of clarity and efficiency, I am adding another method for
 * "returning" a DataStack object via a callback.
 *
 * Up until now, processes that needed the DataStack had to create a class
 * that declared itself a descendent of IStackPool and implement the use_datastack
 * function.
 *
 * With this new interface with a single member function, it will be easier to receive
 * a new DataStack in the same function that requested it.
 * @{
 */
/** @brief Simple interface class for making a DataStack available */
template <class T>
class IDataStack_User
{
public:
   virtual void use(DataStack<T> &ds) = 0;
};


/**
 * @brief Implementation of IDataStack_User that works with a lambda callback.
 *
 * @tparam T The class used to instantiate the desired DataStack object.
 * @tparam F A function object or lambda function that has
 a single DataStack<T> reference parameter.

 * This is an evolution of my original design that gets away from having to
 * use a separate class member function to use a DataStack.  Using a lambda
 * function for F allows the programmer to put the code that uses the DataStack
 * in the same function that asks for it.
 *
 * @sa @link build_index(Advisor &, IDataStack_User<long int> &) @endlink
*/
template <class T, class F>
class TDataStack_User : public IDataStack_User<T>
{
protected:
   F    &m_function_object;
public:
   TDataStack_User(F &fo)
      : m_function_object(fo)         { }
   virtual void use(DataStack<T> &ds) { m_function_object(ds); }
};
/**@}*/



/**
 * @brief Interface class for setting strings in the DataStack.
 *
 * Implement and instantiate this class to pass on the StackBuilder.
 */
class IStringSet
{
public:
   virtual ~IStringSet() { }
   virtual void next(void) = 0;               /**< @brief Move to next item. */
   virtual bool end(void) = 0;                /**< @brief True if past last item. */
   virtual size_t string_length(void) = 0;
   virtual void copy_string(char* buffer) = 0;
};

// Forward declaration so IStackPool can follow StackBuilder and
// thus use StackBuilder in its own build() function.
template <class T> class IStackPool;

/** @brief Template class from which to derive to create custom data stacks. */
template <class T>
class StackBuilder
{
   IStackPool<T>& m_pool;
   int            m_count;  /**< Track line count for creating array at end. */
   line_handle    m_base;   /**< First line of list */
   line_handle    m_bottom; /**< Keep track of last complete line. */

   /**
    * @name Data members to use as work variables.
    *
    * To minimize the size of the stack frame for the
    * recursive function build, I am putting the working
    * variables in the class definition.
    * @{
    */
   line_handle w_line;       /**< Incomplete line before replacing m_bottom. */
   size_t      w_line_size;  /**< Hold memory length required for alloca. */
   size_t      w_extra_size; /**< Keep size of extra because it's needed several times. */
   /**@}*/

   /** @name Don't allow copies or returns. @{ */
   StackBuilder(const StackBuilder&)            = delete;
   StackBuilder& operator=(const StackBuilder&) = delete;
   /**@}*/
   
public:
   StackBuilder(IStackPool<T> &pool)
      : m_pool(pool), m_count(0),
        m_base(nullptr), m_bottom(nullptr),
        w_line(nullptr), w_line_size(0),
        w_extra_size(0)    { }

   ~StackBuilder()         { }

   void construct_and_use(void)
   {
      line_handle *array = static_cast<line_handle*>(alloca(m_count*sizeof(line_handle*)));
      base_handle::fill_array(array, m_count, m_base);
      
      DataStack<T> ds(array,m_count);
      
      m_pool.use_datastack(ds);
   }

   void update_link_pointers(void)
   {
      base_handle::set_link(m_bottom, w_line);
      m_bottom = w_line;
      w_line = nullptr;
   }
   
   void build(void)
   {
      if (m_pool.end() && m_count)
         construct_and_use();
      else
      {
         ++m_count;

         w_extra_size = m_pool.get_extra_size();
         w_line_size = m_pool.get_line_size(w_extra_size);

         w_line = m_pool.get_line_handle(alloca(w_line_size), w_extra_size);

         m_pool.save_string_value(w_line);
         
         if (m_base==nullptr)
            m_base = m_bottom = w_line;
         else
            update_link_pointers();

         m_pool.next();
         build();
      }
   }
};
   
/**
 * @brief Class providing data to the StackBuilder.
 *
 * This class derives from IStringSet because the index value
 * one uses to access the strings is likely the same value
 * used to access the data.  It should be in the same class.
 *
 * I separated the classes because IStringSet doesn't depend
 * on what T is.
 *
 * @sa @ref NonRecursive_DataStack
 */
template <class T>
class IStackPool : public IStringSet
{
protected:
   typedef DMAP Overlay;
   
public:
   virtual ~IStackPool()   { }

   virtual size_t get_extra_size(void) const        { return 0; }  /**< @brief For extra memory into which members of T can point. */
   virtual void   set_data(T*, void* extra, size_t len_extra) = 0; /**< @brief Used to allow calling function set the data. */
   virtual void   use_datastack(DataStack<T> &ds)             = 0; /**< @brief Basically a callback for when the construction is complete. */

   /** @brief Simplifies process by creating the appropriate StackBuilder and building the stack. */
   void build(void)
   {
      StackBuilder<T> sb(*this);
      sb.build();
   }

   /** @name This group of functions calculates offsets for building the buffer. @{ */
   size_t      get_data_size(void)              { return sizeof(T); }
   size_t      get_line_size(size_t size_extra) { return sizeof(T) + sizeof(Overlay) + string_length() + size_extra; }
   line_handle get_line_handle(void* raw, size_t size_extra)
   {
      char *p = static_cast<char*>(raw);

      // initialize underwater stuff:
      memset(p,0,sizeof(T)+size_extra);
      
      T* pdata = static_cast<T*>(static_cast<void*>(p));
      p += sizeof(T);

      void *extra = nullptr;
      if (size_extra)
      {
         extra = const_cast<void*>(static_cast<const void*>(p));
         p += size_extra;
      }

      // Call virtual function so derived class can set the data
      set_data(pdata, extra, size_extra);
      
      DMAP *map = const_cast<DMAP*>(reinterpret_cast<const DMAP*>(p));
      p += sizeof(DMAP);

      map->offset = p - static_cast<char*>(raw);
      map->link   = nullptr;
      
      return static_cast<line_handle>(p);
   }
   /**@}*/
   
   void save_string_value(line_handle line)
   {
      // Set string, if any:
      if (string_length())
         copy_string(const_cast<char*>(line));
   }
};




#endif
