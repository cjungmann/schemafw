// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -pthread -ggdb -D_DEBUG -DINCLUDE_ADB_MAIN -o adbranch adbranch.cpp" -*-

/** @file adbranch.hpp */

#ifndef ADBRANCH_HPP
#define ADBRANCH_HPP

#include "genericuser.hpp"
#include "advisor.hpp"
#include "datastack.hpp"

class SpecsReader;

// I don't know what it is, but there's something in prandstr.hpp
// that makes it mandatory that it be the last include, at least here.
#include "prandstr.hpp"

class Advisor_Branch;

/**
 * @brief struct to hold child and sibling links
 *
 * When collecting the lines, m_sibling and m_child are set to nullptr
 * and m_level is set to the number of white space character preceeding
 * each line.
 *
 * When the lines are all collected, link_nodes will visit each line,
 * setting the links approriately using the m_level value as a guide to
 * the relationships between the lines.
 */
struct abnode
{
   /** @name link members @{ */
   line_handle m_sibling;
   line_handle m_child;
   /**@}*/
   /** @brief Saved level used to identify relationship from one line to the next. */
   int    m_level;

   int level(void) const                       { return m_level; }
   const t_handle<abnode>& sibling(void) const { return *t_handle<abnode>::cp_cast(m_sibling); }
   const t_handle<abnode>& child(void) const   { return *t_handle<abnode>::cp_cast(m_child); }
};

/**
 * @brief A stack-memory-resident data tree representing a section of a specs file (@ref Specs_File).
 *
 * This class provides the means to randomly access, with persistent values, a section
 * of an Advisor specs file.
 *
 * This class is derived from t_handle<abnode> and is built from an Advisor class
 * instance, using a BranchPool object, by reading all the lines of the
 * currently-selected section of the Advisor's specs file.  The entire section
 * is contained in memory with links between lines according to their relationships
 * with each other.  Since the entired contents of the section are in memory,
 * the tags and values returned from this class remain valid as long as the instance
 * of the class exists.
 *
 * There are several functions that make certain tests convenient.  In particular,
 * the ab_handle::dump(FILE* out, int indent=0) will print out the entire memory
 * representation of the branch, with shared references included.
 *
 * -# Boolean value @ref ab_handle_Bool_Testing
 */
class ab_handle : public t_handle<abnode>
{
protected:
   static const char *s_list_true_vals[];
   static const char *s_list_false_vals[];
public:
   /**
    * @name Stack building functions and class
    * @{
    */
   static void t_build(Advisor &adv,
                       const IGeneric_Callback_Const_Pointer<ab_handle> &user);


   /**
    * @brief Builder function that takes a function object argument.
    *
    * @tparam Func Functor, lambda, or callback to use to make ab_handle available.
    */
   template <class F>
   static void build(Advisor &adv, const F &f)
   {
      Generic_User_Const_Pointer<ab_handle, F> cb(f);
      t_build(adv, cb);
   }

   inline static void build(Advisor &adv,
                            const IGeneric_Callback_Const_Pointer<ab_handle> &cb)
   {
      t_build(adv, cb);
   }

   static ab_handle* link_nodes(ab_handle* ref);
   /** @} */

   // protected non-const for clear_value
   inline char* value(void)                    { return const_cast<char*>(static_cast<const char*>(_extra())); }

public:
   inline const char* tag(void) const          { return static_cast<const char*>(_addr()); }
   inline const char* value(void) const        { return static_cast<const char*>(_extra()); }
   inline const int intvalue(void) const       { return atoi(value()); }
   inline bool has_value(void) const           { return *value()!='\0'; }
   inline void clear_value(void)               { *(value()) = '\0'; }
   inline bool is_section(void) const          { return !has_value(); }
   inline bool is_setting(void) const          { return has_value(); }
   inline bool is_shared_ref(void) const       { return *value()=='$'; }
   inline bool is_siblings_branch(void) const  { return 0==strcmp(tag(),"siblings"); }

   /**
    * @brief Returns a pointer to the last character in value.
    *
    * @return Pointer to the last character in the value string.  NULL if there is no value.
    *
    * Use this function for continuation strings and/or printing.  The pointer can
    * be used as a termination condition on a printing loop.
    */
   inline const char* last_char(void) const
   {
      // size_of_string includes '\0', so subtract 2 to get last character
      int size_of_string = sizeof_extra();
      const char *str = static_cast<const char*>(_extra());
      if (size_of_string>1)
         return &str[size_of_string-2];
      else
         return nullptr;
   }
   /**
    * @brief Informs if a string continues to the next response mode line.
    *
    * If a value string terminates with a backslash, its contents are continued
    * on the next line in the SRM file.
    */
   inline bool is_continuation(void) const
   {
      const char *ptr=last_char();
      return (ptr && *ptr=='\\');
   }

   /**
    * @defgroup ab_handle_Bool_Testing Boolean Testing for ab_handle
    *
    * These function test if a handle's value is included in a pre-defined list
    * of boolean values.  The @em true values are represented in the string array
    * #s_list_true_vals and the @em false values are represented in the string array
    * #s_list_false_vals.
    *
    * Note that
    * @code myhand->is_false() @endcode
    * is not the same as
    * @code !myhand->is_true() @endcode
    *
    * These functions are intended to test if the instructions are explicitly
    * true or false.  For example, if a @em field branch contains an @em omit
    * instruction, I assume that including @em omit indicates that the author wants
    * to omit the field from the schema.  I check omit->is_false() in case
    * they're including the instruction as a reminder to set it, but temporarily
    * want to NOT omit the field.  That is, if an @em omit is present, I want to
    * judge it @em true unless it is explicitly marked @em false.
    *
    * @{
    */
   /**
    * @brief Helper function for is_true and is_false.
    * @param list Null-terminated list of strings to compare against value.
    *
    * Returns true if the handle has a value and the value is in the list.
    */
   inline bool tag_in_list(const char** list) const { return string_in_list(tag(), list); }
   inline bool val_in_list(const char** list) const { return has_value() && string_in_list(value(), list); }
   /** @brief Returns @em true if branch has_value and matches #s_list_true_vals. */
   inline bool is_true(void) const                  { return val_in_list(s_list_true_vals); }
   /** @brief Returns @em true if branch has_value and matches #s_list_false_vals. */
   inline bool is_false(void) const                 { return val_in_list(s_list_false_vals); }
   /** @} */

   /** @name Functions for BranchPool::link_nodes @{ */
   inline int level(void) const                { return object().m_level; }
   inline void set_level(int lev)              { object().m_level = lev; }
   inline void inc_level(void)                 { ++(object().m_level); }
   inline void set_sibling(line_handle s)      { object().m_sibling = s; }
   inline void set_sibling(ab_handle *s)       { object().m_sibling = s->lhandle(); }
   inline void set_child(line_handle c)        { object().m_child = c; }
   inline void set_child(ab_handle *c)         { object().m_child = c->lhandle(); }
   /**@}*/

private:
   friend class SpecsReader;
   inline const ab_handle* next(void) const    { return cp_cast(link()); }
   inline ab_handle* next(void)                { return p_cast(link()); }
   
public:   
   inline static ab_handle* init_handle(void *buffer,
                                        const char *name,
                                        size_t size_extra,
                                        ab_handle *parent)
   {
      ab_handle *h = p_cast(base_handle::init_line_handle(static_cast<char*>(buffer),
                                                          name,
                                                          sizeof(abnode),
                                                          size_extra,
                                                          parent->lhandle()));
      abnode &obj = h->object();
      // Write one extra 0 for value-less handles:
      memset(&obj,0,sizeof(abnode)+1);
      return h;
   }

   inline static int len_handle(const Advisor &adv)
   {
      return t_handle<abnode>::line_size(adv.len_tag(), 1+adv.len_value());
   }
   
   static ab_handle* make_handle(void *buff,
                                 const Advisor &adv,
                                 ab_handle *parent=nullptr);

   inline static int len_value_handle(const Advisor &adv)
   {
      return t_handle<abnode>::line_size(adv.len_value(), 1);
   }

   static ab_handle* make_value_handle(void *buff,
                                       const Advisor &adv,
                                       ab_handle *parent=nullptr);
   
   
   /** @name Conversion functions @{ */
   inline static const ab_handle* cp_cast(line_handle h) { return reinterpret_cast<const ab_handle*>(h); } 
   inline static ab_handle* p_cast(line_handle h)        { return reinterpret_cast<ab_handle*>(const_cast<char*>(h)); } 
   inline static const ab_handle& cr_cast(line_handle h) { return *cp_cast(h); }
   inline static ab_handle& r_cast(line_handle h)        { return *p_cast(h); }
   /** @brief Convenient specialization for BranchPool::link_nodes. */
   inline static ab_handle* p_cast(t_handle<abnode> *h)  { return reinterpret_cast<ab_handle*>(const_cast<char*>(h->lhandle())); }
   /**@}*/

   /** @name Functions for traversing the nodes tree. @{ */
   inline ab_handle* next_sibling(void) const            { return p_cast(object().m_sibling); }
   inline ab_handle* first_child(void) const             { return p_cast(object().m_child); }
   inline bool has_sibling(void) const                   { return object().m_sibling!=nullptr; }
   inline bool has_children(void) const                  { return object().m_child!=nullptr; }

   int count_children(const char *tag=nullptr) const;
   /**@}*/

   inline bool is_tag(const char *str) const      { return 0==strcmp(str,tag()); }
   inline bool is_value(const char *str) const    { return has_value() && 0==strcmp(str,value()); }
   inline bool operator==(const char *str) const  { return is_tag(str); }
   inline bool operator!=(const char *str) const  { return !is_tag(str); }

   const ab_handle* seek(const char *path, const char *value=nullptr) const;
   const ab_handle* seek(const char *path, int value) const;

   const char *seek_value(const char *path) const;

   const ab_handle* child_by_position(const char *name, unsigned position) const;
   /**
    * @brief Prevent making copies.
    *
    * Since ab_handle objects are simply overlaid on a memory location,
    * making a copy of the object will create an unusable object.
    * @{
    */
   ab_handle(const ab_handle&)            = delete;
   ab_handle& operator=(const ab_handle&) = delete;
   /**@}*/

   void print_value(FILE* out, bool xml) const;

   void print_as_xml_attribute(FILE* out, const char *name=nullptr) const;

   void dump(FILE* out,
             bool include_refs=false,
             int indent=0,
             bool is_parent=true) const;

};

/**
 * @brief Class to simplify a search of siblings.
 *
 * This class makes it easier to get a visit and test each of a list of siblings.
 * There are several test methods to test characteristics of the currently-held
 * node.  When using SiblingWalker::has_children() and SiblingWalker::child(),
 * it is easy to traverse an entire node tree.
 *
 * The class uses the increment operator to traverse the siblings, and when the
 * end of the sibling list is reached, most of the test functions will cause an
 * error.  Careful checking of the status will prevent this from being a problem,
 * but a few _safe_ functions have also been provided that check for a valid
 * handle before running the test.
 *
 * Use a simple boolean test to determine if the object is valid.  See the code
 * sample for a example.
 * 
 ~~~c
 int found_fields(const ab_handle *schema)
 {
    int count = 0;
    SiblingWalker sw(schema->first_child());
    while (sw)
    {
       // The == operator test equivalence to the _tag_ value.
       if (sw == "field")
          ++count;

       ++sw;             
    }
    return count;
 }
 ~~~
 */
class SiblingWalker
{
   const ab_handle *m_cur;
   const ab_handle *m_saved_next;

   void insert_siblings_branch(const ab_handle *sib_branch);
   void move_next(void);

   void first_good_line(const ab_handle *candidate);
   
public:
   SiblingWalker(const ab_handle *node)
      : m_cur(nullptr), m_saved_next(nullptr)    { first_good_line(node); }

   inline const char *tag(void) const            { return m_cur->tag(); }
   inline const char *value(void) const          { return m_cur->value(); }
   inline const ab_handle* child(void) const     { return m_cur->first_child(); }

   inline bool has_value(void) const             { return m_cur->has_value(); }
   inline bool has_children(void) const          { return m_cur->has_children(); }
   inline bool is_siblings_branch(void) const    { return m_cur->is_siblings_branch(); }
   inline bool is_section(void) const            { return m_cur->is_section(); }
   inline bool is_setting(void) const            { return m_cur->is_setting(); }
   inline bool is_tag(const char *str) const     { return m_cur->is_tag(str); }
   inline bool is_value(const char *str) const   { return m_cur->is_value(str); }
   inline bool is_true(void) const               { return m_cur->is_true(); }
   inline bool is_false(void) const              { return m_cur->is_false(); }
   inline bool tag_in_list(const char **list) const { return m_cur->tag_in_list(list); }
   inline bool val_in_list(const char **list) const { return m_cur->val_in_list(list); }
   inline bool is_equal_to(const char *str) const   { return m_cur->is_equal_to(str); }
   inline bool is_equal_to(const char *str, int len=-1) const
   {
      return m_cur->is_equal_to(str,len);
   }

   inline const ab_handle* seek(const char *str, const char *value=nullptr) const
   {
      return m_cur->seek(str,value);
   }
   
   inline void print_value(FILE* out, bool xml) const { m_cur->print_value(out,xml); }

   void print_as_xml_attribute(FILE* out, const char *name=nullptr)
                                                    { m_cur->print_as_xml_attribute(out,name); }

   inline const char *safe_tag(void) const          { return m_cur ? m_cur->tag() : nullptr; }
   inline const char *safe_value(void) const        { return m_cur ? m_cur->value() : nullptr; }
   inline const ab_handle* safe_child(void) const   { return m_cur ? m_cur->first_child() : nullptr; }
   inline bool safe_is_tag(const char *str) const   { return m_cur && m_cur->is_tag(str); }
   inline bool safe_is_value(const char *str) const { return m_cur && m_cur->is_value(str); }

   inline bool safe_is_true(void) const          { return m_cur && m_cur->is_true(); }
   inline bool safe_is_false(void) const         { return m_cur && m_cur->is_false(); }
   inline bool safe_tag_in_list(const char **list) const
   {
      return m_cur && m_cur->tag_in_list(list);
   }
   inline bool safe_val_in_list(const char **list) const
   {
      return m_cur && m_cur->val_in_list(list);
   }
   
   inline bool safe_is_equal_to(const char *str) const
   {
      return m_cur && m_cur->is_equal_to(str);
   }
   inline bool safe_is_equal_to(const char *str, int len=-1) const
   {
      return m_cur && m_cur->is_equal_to(str,len);
   }

   inline const ab_handle* safe_seek(const char *str, const char *value=nullptr) const
   {
      return m_cur ? m_cur->seek(str,value) : nullptr;
   }

   inline operator const ab_handle *(void) const { return m_cur; }
   inline const ab_handle& obj(void) const       { return *m_cur; }
   inline const ab_handle* ptr(void) const       { return m_cur; }

   
   inline SiblingWalker& operator++(void)        { move_next(); return *this; }
   inline operator bool(void) const              { return m_cur!=nullptr; }
   inline bool operator==(const char *val) const { return  m_cur->is_equal_to(val); }
};



/** @brief Alias/shortcut ad_handle-specific TDataStack_User. */
//template <class Function> using AdvisorIndex_User = TDataStack_User<long int, Function>;

void build_index(Advisor &advisor, IDataStack_User<long int> &user);

#endif  // ADBRANCH_HPP

