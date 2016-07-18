// Local Variables:
// compile-command: "g++ -o stacker -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb stacker.cpp -DINCLUDE_MAIN"
// End:
   
#ifndef STACKER_HPP
#define STACKER_HPP

#include <functional>
#include <stdarg.h>




/**
 * @brief Base class for StackerT template class.
 *
 * Provides as much generic operations as I can think off for the
 * template classes to use.  Set StackerT for more information.
 */
class Stacker
{
public:
   /**
    * @name Typedefs to hide implementation of info sizes.
    * @{ */
   typedef char string_count;
   typedef char* line_pointer;
   typedef char* line_handle;
   /**@}*/

   /** @brief Structure to help interpret data in line_handle */
   struct Overlay
   {
      line_pointer p;
      string_count c;
      char *       s;
   };

protected:
   line_handle m_head_line;

private:
   /**
    * @name This stack-based class should never be returned.
    * @{
    */
   Stacker(const Stacker&)            = delete;
   Stacker& operator=(const Stacker&) = delete;
   /**@}*/

public:
   /**
    * @brief Child class of Stacker for passing a set of strings.
    *
    * This class must be quickly created, used, and destroyed because it
    * will be put up and taken down twice for each line of the Stacker.
    * It uses a variadic constructor to access the strings, and either
    * provides the length of the strings or copies the strings to a
    * buffer.
    */
   class StringSet
   {
   private:
      
      /**
       * @name This stack-based class should never be returned.
       * @{
       */
      StringSet(const StringSet&)            = delete;
      StringSet& operator=(const StringSet&) = delete;
      /**@)*/
      
      int m_count;
      const char **m_pp_strings;

   public:
      StringSet(int count, const char** list)
         : m_count(count), m_pp_strings(list)  { }
      ~StringSet()                             { }

      inline int count(void) const             { return m_count; }
      size_t length(void) const;
      void copy(char *buffer) const;
   };


   /**
    * @name Non-class-based callbacks.
    *
    * Callback functions StringSet_Getter and StringSet_Nexter are passed to
    * StackerT<T>::build to give the build function access to the string or
    * strings to be associated with a Stacker line.  It's a bit contorted in
    * order to manage the stack pointer.
    * @{
    */
   /** @brief Callback for StringSet_Getter callback. */
   typedef std::function<void(StringSet&)> StringSet_User;
   /** @brief Callback for Stacker to get a list of strings. */
   typedef std::function<void(StringSet_User)> StringSet_Getter;
   /** @brief Callback for incrementing pointer. */
   typedef std::function<bool(void)> StringSet_Nexter;
   /**@}*/

   
public:
   Stacker(char *p_head_line) : m_head_line(p_head_line) { }
   virtual ~Stacker()                                    { }

   static inline size_t get_under_length(size_t size_of_data)                     { return size_of_data + sizeof(line_pointer) + sizeof(string_count); }
   static line_handle get_line_handle_from_raw(size_t size_of_data, char *buffer) { return buffer+get_under_length(size_of_data); }


   static inline Overlay* get_overlay(line_handle line)                           { return reinterpret_cast<Overlay*>(&line[-(sizeof(string_count)+sizeof(line_pointer))]); }
   static inline line_pointer& get_ref_to_next_pointer(line_handle line)          { return get_overlay(line)->p; }
   static inline line_pointer get_next_line(line_handle line)                     { return get_overlay(line)->p; }
   static inline string_count &get_ref_to_string_count(line_handle line)          { return get_overlay(line)->c; }
   static inline void *get_pointer_to_data(size_t size_of_data, line_handle line) { return static_cast<void*>(line-sizeof(string_count)-sizeof(line_pointer)-size_of_data); }
   static inline void set_string_count(line_handle line, int count)               { get_ref_to_string_count(line) = static_cast<string_count>(count); }

   static inline void set_next_pointer(size_t size_of_data,
                                       line_handle bottom,
                                       line_handle newline)                       { get_ref_to_next_pointer(bottom) = newline; }
   

   static void initialize_new_line(size_t size_of_data, line_handle bottom, line_handle newline, int count);

   line_handle find_line(const char *) const;
};





/**
 * @brief StackerT template class to create a name-indexed stack-resident list of items.
 *
 * This is kind of an experiment in programming, to generate data structures completely
 * on the stack using recursion, alloca, and state-capturing lambda functions as callbacks.
 *
 * The challenge is how to determine the size of the memory allocations for items that are
 * only known by a called function.  The called function moves the stack pointer, so the
 * calling function can't safely use alloca().
 *
 * The StackerY object is created by calling a static function, StackerT<T>::build(),
 * providing function pointers (using std::function wrapping state-saving lambda
 * functions) to iterate through a list.  I picture storing cookie values, file
 * indexes, and indexed MYSQL_BIND arrays using this method.
 *
 * The basic strategy is for the calling function to provide an iteration of the
 * list of names and values with the callbacks.  The StackerT callback requires
 * another callback with which StackerT will get a set of strings to associate with
 * the data line.  The first call will be to get the length of the strings.  The
 * callback function returns, restoring the stack pointer, before using alloca()
 * to create a buffer for the data.  StackerT will the use the callback a second
 * time to copy the strings into the data buffer.  Since the recursuve StackerT
 * function will have already alloca-ed the data, this function call will not
 * disturb the stack.
 *
 * I hope to provide more information about the process in the documentation of
 * the callback functions.
 */
template <class T>
class StackerT : public Stacker
{
public:
   typedef std::function<void(T*)> Stacker_Data_Setter;
   typedef std::function<void(StackerT&)> Stacker_User;
   
   class CBGroup
   {
   private:
      CBGroup(const CBGroup &)             = delete;
      CBGroup & operator=(const CBGroup &) = delete;
      
      StringSet_Getter    m_getter;
      StringSet_Nexter    m_nexter;
      Stacker_Data_Setter m_setter;
      Stacker_User        m_user;
      line_handle         m_top;
      line_handle         m_bottom;
   public:
      CBGroup(StringSet_Getter    getter,
              StringSet_Nexter    nexter,
              Stacker_Data_Setter setter,
              Stacker_User        user,
              line_handle         top=nullptr,
              line_handle         bottom=nullptr)
         : m_getter(getter), m_nexter(nexter),
           m_setter(setter), m_user(user),
           m_top(top), m_bottom(bottom)           { }
   };

public:
   /** Do-nothing destructor to satisfy compiler: */
   virtual ~StackerT()                  { }

private:
   /** @brief Private constructor because member data is result of recursion. */
   StackerT(char *list) : Stacker(list) { }
   
   static void recursive_build(StringSet_Getter getter,
                               StringSet_Nexter nexter,
                               Stacker_Data_Setter setter,
                               Stacker_User user,
                               line_handle top=nullptr,
                               line_handle bottom=nullptr)
   {
      size_t len_strings = 0;
      int num_strings = 0;
      line_handle newline = nullptr;

      // The clear way is the following commented lines, but to avoid adding to the stack
      // frame, the implementation is to call getter() with an anonymous function.
      // StringSet_User su_get_length = [&len_strings, &num_strings](StringSet &ss) { len_strings = ss.length(); num_strings = ss.count(); }
      // getter(su_get_length);
      // Use the following instead of the preceding:
      getter( [&len_strings, &num_strings](StringSet &ss) { len_strings = ss.length(); num_strings = ss.count(); } );
      
      if (len_strings>0)
      {
         // Figure full size, alloca the buffer and get handle to data:
         size_t len_line = len_strings + get_under_length(sizeof(T));
         newline = get_line_handle_from_raw(sizeof(T), static_cast<char*>(alloca(len_line)));

         initialize_new_line(sizeof(T), bottom, newline, num_strings);

         // Set string(s):
         // The folloinwg commented demonstrates the uncommented single line implementation that follows.
         // StringSet_User su_set_strings = [&newline](StringSet &ss){ ss.copy(newline); };
         // getter(su_set_strings);
         // Use the following instead of the preceding:
         getter( [&newline](StringSet &ss){ ss.copy(newline); } );

         setter(static_cast<T*>(get_pointer_to_data(sizeof(T), newline)));

         // If first time through, set top for subsequent calls:
         if (!top)
            top = newline;
      }


      if (nexter())
         recursive_build(getter, nexter, setter, user, top, newline);
      else
      {
         StackerT<T> sst(top);
         user(sst);
      }
   }
                              

public:
   static void build(StringSet_Getter    getter,
                     StringSet_Nexter    nexter,
                     Stacker_Data_Setter setter,
                     Stacker_User        user)
   {
      // To to: Modify recursive_build to use CBGroup,
      // then construct CBGroup with parameters to pass in.
      
      recursive_build(getter,nexter,setter,user,nullptr,nullptr);
   }

   T* find(const char *name)
   {
      Stacker::line_handle h = find_line(name);
      if (h)
         return static_cast<T*>(get_pointer_to_data(sizeof(T),h));
      else
         return nullptr;
   }

   
};


class IStringer
{
public:
   virtual bool atEnd(void) = 0;
   virtual bool next(void) = 0;
   virtual int string_count(void) = 0;
   virtual size_t strings_length(void) = 0;
   virtual void copy_strings(char *buffer) = 0;
};


#endif   // STACKER_HPP

