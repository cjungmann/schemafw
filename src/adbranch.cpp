// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -pthread -ggdb -D_DEBUG -DINCLUDE_ADB_MAIN -o adbranch adbranch.cpp" -*-

/** @file adbranch.cpp */

#include "adbranch.hpp"
#include "datastack.hpp"

/**
 * @brief List of values what will be judged as @em true.
 * @ingroup ab_handle_Bool_Testing
 */
const char *ab_handle::s_list_true_vals[] = { "true", "yes", "1", nullptr };
/**
 * @brief List of values what will be judged as @em false.
 * @ingroup ab_handle_Bool_Testing
 */
const char *ab_handle::s_list_false_vals[] = { "false", "no", "0", nullptr };

/**
 * @brief Recursive path search.
 *
 * The function only looks for siblings of the first child.
 * If a match is found, the *path pointer will move to the
 * next level and recursively call seek() for its children.
 * It keeps seeking until it finds a match, restarting with
 * siblings if a branch fails to match due to missing children.
 *
 * @todo This could be more xpath like, except much simpler.  How about
 * a search path like schema/field:p_handle, which would be analogous
 * to an xpath search /schema/field[value='p_handle']?  Avoid
 * complicated predicates and eliminate the closing ']' to make it
 * easier to build the path statement.
 */
const ab_handle* ab_handle::seek(const char *path, const char *value) const
{
   const ab_handle *rval = nullptr;
   
   if (path && *path)
   {
      const char *slash = strchr(path,'/');
      int cmplen = slash ? slash-path : -1;
      
      const ab_handle *node = this->first_child();
      if (node)
      {
         SiblingWalker sw(node);
         while ((bool)sw)
         {
            // Search for part of path if it has multiple steps
            if (slash)
            {
               if (sw.is_equal_to(path,cmplen))
                  if ((rval=sw.seek(slash+1, value)))
                     break;
            }
            // For on last step, compare tag and value, if defined,
            // to determine whether to return the node or not.
            else if (sw.is_tag(path))
               if (!value || sw.is_value(value))
               {
                  rval = sw.ptr();
                  break;
               }

            // Not found, consider next sibling
            ++sw;
         }
      }
   }
   
   return rval;
}

/**
 * @brief Seek with an integer value, mainly for finding a numbered result.
 *
 * SpecsReader makes seeking values so easy, the unit test for this
 * function will be found there.
 */
const ab_handle* ab_handle::seek(const char *path, int value) const
{
   const ab_handle *rval = nullptr;
   auto f = [this, &path, &rval](const char* v)
   {
      rval = seek(path,v);
   };

   ltoccp(value,f);
   return rval;
}


/**
 * @brief Return a child node by 0-based index position.
 */
const ab_handle *ab_handle::child_by_position(const char *name, unsigned position) const
{
   const ab_handle *rval = nullptr;
   
   unsigned count = 0;
   SiblingWalker sw(this->first_child());
   while ((bool)sw)
   {
      if (sw.is_tag(name))
      {
         if (count==position)
         {
            rval = sw;
            break;
         }
         else
            ++count;
      }
      ++sw;
   }
   
   return rval;
}

/**
 * @brief Shortcut function to get const char* value if it exists.
 *
 * Why did I wait so long to make this function?  Perhaps another
 * like it exists, but I couldn't find it.
 */
const char *ab_handle::seek_value(const char *path) const
{
   const ab_handle *h = seek(path);
   if (h && h->has_value())
      return h->value();
   else
      return nullptr;
}


/**
 * @brief Hand-off to &user an ab_handle tree starting at the current
 * Advisor position.
 *
 * @param adv An Advisor object pointing at the node from which to build the tree.
 * @param user A function object to which the new ab_handle tree will be sent.
 */
void ab_handle::t_build(Advisor &adv,
                        const IGeneric_Callback_Const_Pointer<ab_handle> &user)
{
   int starting_level = adv.level();
   
   ab_handle *head = nullptr;
   ab_handle *tail = nullptr;

   do
   {
      tail = make_handle(alloca(len_handle(adv)), adv, tail);

      // Save the head, if not already saved:
      if (!head)
         head = tail;

      adv.get_next_line();
   }
   while (adv.level() > starting_level);

   // Prepare hierarchical links:
   link_nodes(head);

   // Send to its user:
   user( p_cast(head) );
}


/**
 * @brief Set links in abnode struct to create hierarchical links.
 *
 * @param ref A pointer to a node from which subsequent nodes will be related.
 * @return A pointer to the next sibling of ref, or null if no more siblings.
 *
 * This is a recursive function that examines the next node, comparing the
 * level of the next node against the ref node.  If the level is higher, it
 * is a child node.  It is saved in the m_child member, and execution continues
 * by recursively calling link_nodes with the child.  If the level is the same,
 * it saves it in m_sibling and again recursively calls link_nodes for the
 * following nodes.  If the new node level is lower than the ref's level,
 * the recursion will unwind until the ref level of the stack frame equals
 * the level of the return value, and processing will continue as siblings.
 *
 * When the recursion is completely unwound, each node will point to its
 * next sibling and to its first child, until there are no more siblings or
 * children.
 */
ab_handle* ab_handle::link_nodes(ab_handle* ref)
{
   ab_handle *cur = ref;
   ab_handle *next = ref->next();

   while (cur && next)
   {
      // Detour for continuations, then continue as if there was no continuation.
      if (cur->is_continuation())
      {
         // Conserve _cur_ for attaching handles following the last continuation:
         ab_handle *ptr = cur;
         
         while (next && ptr->is_continuation())
         {
            ptr->set_child(next->lhandle());
            ptr = next;
            next = next->next();
         }
      }
      
      // ab_handle::level() uses address arithmetic,
      // so it's more efficient to cache the values:
      int cur_level = cur->level();
      int next_level = next->level();
         
      // If both have same level, they're siblings:
      if (next_level==cur_level)
      {
         cur->set_sibling(next->lhandle());

         // The sibling is the new reference point
         cur = next;
         next = cur->next();
      }
      // If next is at higher level, it's a child.
      // Save as child, then recurse to process at its level.
      else if (next_level > cur_level)
      {
         cur->set_child(next->lhandle());
         // cur stays the same in case of siblings:
         next = link_nodes(next);
      }
      // If not equal or greater, it's time to return from
      // this level: the next item belongs to an item higher on
      // the hierarchy.
      else
         return next;
   }
   
   return nullptr;
}

ab_handle* ab_handle::make_handle(void *buff,
                                  const Advisor &adv,
                                  ab_handle *parent)
{
   
   ab_handle *rval = ab_handle::init_handle(buff,
                                            adv.tag(),
                                            1+adv.len_value(),
                                            parent);
   // Set the value string:
   if (adv.len_value())
   {
      char *vstring = static_cast<char*>(rval->extra());
      if (adv.len_value())
         memcpy(vstring, adv.value(), adv.len_value());
      vstring[adv.len_value()] = '\0';
   }

   // save the level for later determining relationships.
   rval->object().m_level = adv.level();

   return rval;
}

ab_handle *ab_handle::make_value_handle(void *buff,
                                        const Advisor &adv,
                                        ab_handle *parent)
{
   ab_handle *rval = ab_handle::init_handle(buff,
                                            adv.value(),
                                            1,
                                            parent);
   
   // We don't need to set the value string because
   // has already done it for the zero-length value.
   
   // save the level for later determining relationships.
   rval->object().m_level = adv.level();
   return rval;
}

/**
 * @brief Count children, either all or by name.
 *
 * @param tag Tag-value of children to count.  Can be null to count all children.
 * @return Number of child nodes matching the search criteria.
 *
 * The inspiration for adding this function is Schema::pre_fetch_use_result(), where
 * I need to know if there are any result instructions in the response mode.  I'm
 * sure that having this function available will be useful elsewhere.
 */
int ab_handle::count_children(const char *tag) const
{
   int count = 0;
   SiblingWalker sw(first_child());

   // do the tag conditional outside of the iteration:
   if (tag)
   {
      while (sw)
      {
         if (sw.is_tag(tag))
            ++count;
         ++sw;
      }
   }
   else
   {
      while (sw)
      {
         ++count;
         ++sw;
      }
   }

   return count;
}

/**
 * @brief Build a DataStack mapping mode (advisor lines at position 0)
 * names and their file positions.
 *
 * Note that an Advisor object saves and restores its position using a
 * long int value, so that is class type we use to build the DataStack.
 * Refer to the code example at the end of this description.
 *
 * This function serves as an example a simple implementation of a
 * non-recursive stack builder. Because of this extra purpose, there is
 * more documentation/explanation in this code to be a guide for later
 * development.
 *
 * This particular example deals with a source of unknown length, so
 * the array must be allocated after all elements are collected.
 *
 * @sa test_lambda_callback_Advisor_index(Advisor &)
 *
 * @snippet adbranch.cpp non-recursive-datastack-example
 *
 */
// [build_index] 
void build_index(Advisor &advisor, IDataStack_User<long int> &user)
{
   typedef t_handle<long int> thint; // for ease of typing and shorter lines.

   int count = 0;                    // REQ: keep track of how many we've saved
   thint *head = nullptr;            // REQ: first for traversing list at end
   thint *tail = nullptr;            // REQ: for adding new elements.

   // Example-specific, start at beginning of file, proceed to the end
   advisor.rewind();
   while (!advisor.end())
   {
      // Example-specific: Filter for modes, defined as lines at level==0:
      if (advisor.level()==0)
      {
         // Example-specific: get data
         const char *name = advisor.tag();
         long int position = advisor.get_position();

         // REQ: next three lines:
         // 1. Keep count for undefined-length source
         // 2. Use template static method to calculate the memory requirement
         // 3. Use template static method to initialize the handle.
         ++count;
         size_t len_line = thint::line_size(name,0);
         tail = thint::init_handle(alloca(len_line), name, 0, tail);
         // REQ: Save as head for first line created:
         if (!head)
            head = tail;

         // REQ: AND  Example-specific:
         // Get reference to the object and set its value or member values:
         long int &obj = tail->object();
         obj = position;
      }

      // Example-specific: move to next source item:
      advisor.get_next_line();
   }

   // REQ: Next three lines to create a DataStack
   // 1. Get memory for the array.
   // 2. Use base_handle static function to populate the array.
   // 3. Make instance of DataStack.
   line_handle *array = static_cast<line_handle*>(alloca(sizeof(line_handle) * count));
   base_handle::fill_array(array, count, head->lhandle());
   DataStack<long int>  ds(array, count);

   // REQ: DataStack done, pass it on to the indicated user:
   user.use(ds);
}
// [build_index]


/**
 * @brief Prints value, with consideration for continuing strings.
 */
void ab_handle::print_value(FILE* out, bool xml) const
{
   const char* lchar = last_char();
   
   if (lchar)
   {
      const char* end = *lchar=='\\' ? lchar-1 : lchar;
      const char* ptr = value();

      if (xml)
      {
         while (ptr <= end)
            print_char_as_xml(*ptr++, out);
      }
      else
      {
         while (ptr <= end)
            fputc(*ptr++, out);
      }

      if (*lchar=='\\')
      {
         const ab_handle* child = first_child();
         if (child)
         {
            const char *tag = child->tag();
            if (0==strcmp(tag, Advisor::continuation_tag()))
            {
               fputc(' ', out);
               child->print_value(out, xml);
            }
         }
      }
   }
}

/**
 * @brief Prints an xml attribute, xml-escaping the value characters as necessary.
 *
 * @param out Open FILE* stream
 * @param name Optional override the tag string.  Leave out (or NULL) to
 *             simply use the ab_handle::tag() value.
 *
 * The function prints an xml attribute, tag="value".  The significance of
 * this function is that it properly handles continued strings, concatenating
 * continuing lines, adding a space before each follow-on line.
 *
 * If there is no value(), nothing is printed, neither tag nor empty value.
 */
void ab_handle::print_as_xml_attribute(FILE* out, const char *name) const
{
   if (has_value())
   {
      if (!name)
         name = tag();

      fputc(' ', out);
      fputs(name, out);
      fputc('=', out);
      fputc('"', out);
      print_value(out, true);
      fputc('"', out);
   }
}

/**
 * @brief Write the handle and its descendants to *out.
 *
 * @param out          The File stream to which to write the contents.
 * @param include_refs Include the appended reference modes that follow the mode.
 *                     See @ref SpecsReader_Read_Mode for an explanation of what
 *                     is an appended reference mode.
 * @param indent       Indentation level for output.  It exists for recursive
 *                     level tracking and not intended for regular use.
 * @param is_parent    Identifies the primary called context: The function will
 *                     set _is_parent_ to false for recursive calls.
 */
void ab_handle::dump(FILE *out, bool include_refs, int indent, bool is_parent) const
{
   // print indent:
   for (int i=0; i<indent; ++i)
      fputs("  ", out);
      
   // print line:
   fputs(tag(), out);
   if (is_setting())
   {
      fputs(" : \"", out);
      print_value(out, false);
      fputc('"', out);
   }
   fputc('\n', out);

   // Print children, as found;
   const ab_handle *cptr = first_child();
   if (cptr && !cptr->is_tag(Advisor::continuation_tag()))
   {
      SiblingWalker sw(cptr);
      while ((bool)sw)
      {
         sw.obj().dump(out, include_refs, indent+1, false);
         ++sw;
      }
   }

   if (is_parent && include_refs)
   {
      fputs("\n...shared modes that were included:\n", out);
      
      cptr = next_sibling();
      while (cptr)
      {
         fputs("\nshared mode: $", out); 
         cptr->dump(out, false, 0, false);
         cptr = cptr->next_sibling();
      }
   }

}


/**
 * @brief Prepares m_cur and m_saved_next from a given handle.
 *
 * This function will consider the provided @p candidate
 * for use as the new representative ab_handle.
 *
 * Extra care is required if @p candidate is a siblings-branch.
 * We'll ignore an empty siblings-branch by moving on to its
 * sibling.  Otherwise, the first child of the siblings branch
 * will be set as the representative ab_handle.
 *
 * The function loops until it finds a good line or it reaches
 * the end of the siblings chain.
 */
void SiblingWalker::first_good_line(const ab_handle *candidate)
{
   m_cur = nullptr;
   
   // Loop until we've found a good line, or until
   // we've exhausted the candidates:
   while (!m_cur && candidate)
   {
      // Special setup for a siblings-branch:
      if (candidate->is_siblings_branch())
      {
         // Use the current siblings-branch if has children,
         // saving it's sibling for when its contents have been
         // retrieved:
         if ((m_cur=candidate->first_child()))
            m_saved_next = candidate->next_sibling();
         // otherwise, try again the candidate's sibling:
         else
            candidate = candidate->next_sibling();
      }
      else
         m_cur = candidate;
   }
}

/**
 * @fn SiblingWalker::operator++(void)
 * @brief Moves to the next sibling, following sibling-shares if encountered.
 *
 * Called by operator++, this function moves to the next sibling.
 *
 * When the next sibling is a sibling-share, the function will set _m_cur_
 * to the first child of the sibling-share and save the _next_sibling_ value
 * of the sibling share.  When it reaches the end of the chain of the
 * sibling share, _m_cur_ will be set to the saved _next_sibling_value.
 */

/** @brief Called by operator++(void) to perform its task. */
void SiblingWalker::move_next(void)
{
   const ab_handle *ptr = m_cur->next_sibling();

   // First check if we're at the end of a siblings-branch
   // sibling chain, preparing the object to continue with
   // the sibling of the branch head:
   if (!ptr && m_saved_next)
   {
      ptr = m_saved_next;
      m_saved_next = nullptr;
   }

   first_good_line(ptr);
}


#ifdef INCLUDE_ADB_MAIN

#include "prandstr.cpp"
#include "datastack.cpp"
#include "advisor.cpp"

#include <pthread.h>   // for pthread_create
#include <unistd.h>    // for write()


/**
 * @brief Example of using a lambda function as callback for build_index.
 *
 * Note that in this function, the lambda function is of the non-capturing
 * sort that would work directly as a callback.  If the function was part
 * of a class, you would probably want to capture the @e this pointer. In
 * that case, you need to use the TDataStack_User<T> class (or its component
 * parts).
 *
 * This is also a more complicated example illustrating the use of the
 * Generic_User templates.  See @ref Templated_Callbacks.
 */
// [non-recursive-datastack-example]
void test_lambda_callback_Advisor_index(Advisor &advisor)
 {
   // Simple demonstration lambda function:
   auto fTree = [](const ab_handle* tree)
   {
      printf("I got the tree of node \"%s\"\n", tree->tag());
      tree->dump(stdout);
      printf("\n");
   };

   // Second lambda will be called by build_index,
   // and will repeatedly use the fTree lambda function.
   auto fIndex = [&advisor, &fTree](DataStack<long int> &ds)
   {
      printf("\nTest a scan through all of the modes:\n");
      for (auto *line=ds.start(); line; line=line->next())
      {
         long int offset = line->object();
         printf("%08ld: %s\n", offset, line->str());
         
         advisor.restore_state(offset);
         ab_handle::build(advisor, fTree);
      }
   };
   // Use the second lambda to create the callback for build_index:
   TDataStack_User<long int, decltype(fIndex)> user(fIndex);

   // Start the ball rolling:
   build_index(advisor, user);
}
// [non-recursive-datastack-example]



// const char fakefile[] =
// "# -*- mode: sh -*-\n"
// "$database       : CaseStudy\n"
// "$xml-stylesheet : default.xsl\n"
// "$default-mode   : list\n"
// "\n"
// "$session-type   : simple\n"
// "\n"
// "list\n"
// "   type          : table\n"
// "   procedure     : App_Contact_List\n"
// "   on_line_click : contacts.srm?edit\n"
// "   legend        : This is a string that\\\n"
// " continues on to the next line.  It must be very long to test when\\\n"
// " happens when a string extends past the 256-character buffer we've set\\\n"
// " aside for processing SRM lines.  What do you think is going to happen?\n"   
// "   button\n"
// "      type  : add\n"
// "      label : Create Contact\n"
// "      task  : contacts.srm?create\n"
// "\n"
//    ;

const char fakefile[] =
"list\n"
"   legend        : This\\\n"
" is a \\\n"
"string\\\n"   
" that\\\n"
" continues.\n"   
   ;



int main(int argc, char **argv)
{
   FILE* f = nullptr;

   if (argc>1)
      f = fopen(argv[1], "r");

   if (!f)
      f = fmemopen(const_cast<void*>(static_cast<const void*>(fakefile)),
                   strlen(fakefile), "r");

   if (f)
   {
      // Construct an Advisor object for BranchPool constructor:
      AFile_Stream af_s(f);
      Advisor      advisor(af_s);
      
      test_lambda_callback_Advisor_index(advisor);

      // Don't close the stream, af_s will do it when it goes out of scope.
   }
   
   return 0;
}



#endif
