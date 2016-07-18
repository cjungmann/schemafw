// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -fno-inline -pedantic -ggdb -DINCLUDE_SPECSREADER_MAIN -o specsreader specsreader.cpp" -*-

#include "specsreader.hpp"
#include <alloca.h>
#include <assert.h>

void print_xml_attribute(FILE *out,
                         const char *tag,
                         const char *value)
{
   fputc(' ', out);
   fputs(tag, out);
   fputs("=\"", out);
   print_str_as_xml(value, out);
   fputc('"', out);
}

void print_xml_attribute(FILE *out,
                         const char *name,
                         unsigned long value)
{
   fputc(' ', out);
   fputs(name, out);
   fputs("=\"", out);
   print_uint(value, out);
   fputc('"', out);
}


/**
 * @brief Build an index of the modes in an advisor file.
 */
//! [DataStack_Building_Snippet_Advisor_Index]
void Advisor_Index::t_build(Advisor &advisor,
                            IGeneric_Callback<Advisor_Index> &callback)
{
   advisor.rewind();
   
   t_handle<info> *head = nullptr;
   t_handle<info> *tail = nullptr;
   int count = 0;

   while (!advisor.end())
   {
      // Only getting modes
      if (advisor.level()==0)
      {
         ++count;
         
         const char *name = advisor.tag();
         const char *value = advisor.value();

         size_t len_extra = (value) ? 1+strlen(value) : 0;
         
         size_t len = t_handle<info>::line_size(name, len_extra);

         tail = t_handle<info>::init_handle(alloca(len),
                                            name,
                                            len_extra,
                                            tail);

         if (!head)
            head = tail;

         info &i = tail->object();
         i.m_position = advisor.get_position();
         if (len_extra==0)
            i.m_value = nullptr;
         else
         {
            void *vp = tail->extra();
            i.m_value = static_cast<char*>(vp);
            memcpy(vp, value, len_extra);
         }
      }

      advisor.get_next_line();
   }

   if (count)
   {
      line_handle *array = static_cast<line_handle*>(alloca(count*sizeof(line_handle)));
      base_handle::fill_array(array, count, head->lhandle());

      DataStack<info> ds(array, count);
      Advisor_Index ai(ds);
      callback(ai);
   }
}
//! [DataStack_Building_Snippet_Advisor_Index]

const t_handle<Advisor_Index::info>* Advisor_Index::seek(const char *name,
                                                         const char *value) const
{
   for (auto* ptr = m_ds.start(); ptr; ptr = ptr->next())
   {
      if (ptr->is_equal_to(name))
         if (value==nullptr || ptr->object()==value)
            return ptr;
   }

   return nullptr;
}

/**
 * @brief Get the position (or -1 if not found) of a named mode with optional
 * value filter.
 */
long int Advisor_Index::position(const char *name, const char *value) const
{
   auto *ptr = seek(name,value);
   if (ptr)
      return ptr->object().m_position;
   else
      return -1;
}

/**
 * @brief Print the list of modes in the index.
 *
 * @param f              File stream where names should be written.
 * @param include_shared Include shared modes (starting with '$').  Defaults to false.
 *
 * This function was added to provide a mode list for debugging information
 * for schema...it's easier to call `-d modes` than to open and search through
 * the specs file.
 */
void Advisor_Index::print_modes(FILE *f, bool include_shared) const
{
   bool first = true;
   for (const t_handle<info>* ptr = m_ds.start(); ptr; ptr = ptr->next())
   {
      const char *name = ptr->str();

      if (*name!='$' || include_shared)
      {
         if (first)
            first = false;
         else
            fputs(", ", f);
            
         fputs(name, f);
      }
   }

   // Only print the newline if we wrote out some modes:
   if (!first)
      fputc('\n', f);
}


/** @brief Use this static member function to create an instance of SpecsReader. */
void SpecsReader::t_build(Advisor &advisor, IGeneric_Callback<SpecsReader> &user)
{
   auto f = [&advisor, &user](Advisor_Index &ii)
   {
      SpecsReader sr(advisor, ii);
      user(sr);
   };

   Advisor_Index::build(advisor, f);
}

void SpecsReader::print_as_xml(FILE *out, const ab_handle *handle)
{
   fputc('<', out);
   fputs(handle->tag(), out);

   // print attributes, counting sub-handlees as encountered
   int child_count = 0;
   SiblingWalker sw(handle->first_child());
   while(sw)
   {
      if (sw.is_setting())
      {
         const char *val = sw.value();
         if (*val=='$')
            fputs("<shared />\n", out);
         else
            print_xml_attribute(out, sw.tag(), val);
      }
      else
         ++child_count;

      ++sw;
   }

   if (child_count)
   {
      // close as non-empty element
      fputs(">\n", out);

      // scan and print children
      SiblingWalker swc(handle->first_child());
      while(swc)
      {
         if (swc.is_section())
            print_as_xml(out, swc);

         ++swc;
      }
      
      // print close tag
      fputs("</", out);
      fputs(handle->tag(), out);
      fputs(">\n", out);
   }
   // If empty element, close as such:
   else
      fputs(" />\n", out);
}

/**
 * @brief Prints children of branch as XML elements.
 *
 * @param skip An array of const char*, with the last element a nullptr.
 *
 * @code
auto f = [&specsreader](const ab_handle *branch)
{
   const char *arr[] = {"root", "schema", nullptr};
   specsreader.print_sub_elements(stdout, branch, arr);
};
specsreader.get_mode("submit", f);
 * @endcode
 */
void SpecsReader::print_sub_elements(FILE *out,
                                     const ab_handle *branch,
                                     const char **skip)
{
   SiblingWalker sw(branch->first_child());
   while (sw)
   {
      if (!skip || (sw.is_section() && !sw.tag_in_list(skip)))
         print_as_xml(out, sw);
             
      ++sw;
   }
}

/**
* @brief This function returns an Advisor_Index::info pointer to learn its
* position or value.
*
* This function should be used to learn the value of global settings. For
* example, the Specs file may have global xml-stylesheet ($xml-stylesheet) or
* database ($database) modes.  These types of modes have no children, so the
* information contained in the mode is available without a callback function
* like SpecsReader::get_mode.  An example follows.  Note that the value parameter
* is left blank.
*
* @code
auto *mode = advisor_index.seek_mode_info("$database");
if (mode)
{
   printf("The default database value of this Specs file is \"%s\", mode->value());
}
* @endcode
*
* One might also use this function to find one of several modes with the same
* name in order to get the position for loading its contents.  An example would
* be a global fields ($fields) mode that would be included in another mode
* by referencing its name.  See @ref Specs_Shared_Mode.
*
* @code
auto *mode = advisor_index.seek_mode_info("$schema", "person");
if (mode)
{
   auto f = [this](const ab_handle *schema)
   {
      if (schema)
         this->use_schema(schema);
   };
   specsreader->get_mode(mode->position(), f);
}
* @endcode
*/
const Advisor_Index::info* SpecsReader::seek_mode_info(const char *name,
                                                       const char *value) const
{
   const auto *ptr = m_index.seek(name,value);
   if (ptr)
      return &(ptr->object());
   else
      return nullptr;
}

/**
 * @brief Get the position of named node (with optional value) or -1 if not found.
 *
 * Similar to seek_mode_info except that it returns the position where only the
 * position is needed.
 */
long int SpecsReader::get_mode_position(const char *name, const char *value) const
{
   const auto *ptr = m_index.seek(name,value);
   if (ptr)

      return ptr->object().position();
   else
      return -1;
}

/**
 * @brief This function adds a '$' in front of name to search global items.
 *
 * This function is like get_mode_position, except that it stack-allocates
 * a new name buffer in which it can prepend a '$' to search for a global
 * mode.  Creating the buffer here prevents the buffer from remaining on
 * the stack because in this function the value is used after the function
 * returns.
 */
long int SpecsReader::get_global_mode_position(const char *name,
                                                const char *value) const
{
   size_t len = 1+strlen(name);
   char *buff = static_cast<char*>(alloca(1+len));
   *buff = '$';
   memcpy(static_cast<void*>(&buff[1]), static_cast<const void*>(name), len);
   
   const auto *ptr = m_index.seek(buff,value);
   if (ptr)
      return ptr->object().position();
   else
      return -1;
}

/**
 * @brief Returns, via the callback, the first found ab_handle == tag
 *
 * Searches mode_first, mode_second, then mode_third  for a child element
 * whose tag name == the tag argument.  Failing to find a child element,
 * the final search is for a global mode with the same tag name except
 * with a '$' prefix.
 *
 * This function always returns a value (through cb): the pointer will
 * be null if no ab_handle is found.
 *
 * @param tag         The tag name for which to search.
 * @param cb          Callback class through which the value is returned.
 * @param mode_first  First priority place to search
 * @param mode_second Next priority.
 */
void SpecsReader::seek_specs_handle(const char *tag,
                                 const IGeneric_Callback_Const_Pointer<ab_handle> &cb,
                                 const ab_handle *mode_first,
                                 const ab_handle *mode_second,
                                 const ab_handle *mode_third) const
{
   const ab_handle *found = nullptr;
   const ab_handle *arr[3] = { mode_first, mode_second, mode_third };
   for (int i=0; !found && i<3; ++i)
   {
      if (arr[i] && (found=arr[i]->seek(tag)))
         cb(found);
   }

   // Last resort: look for global mode:
   if (!found)
   {
      seek_branch(get_global_mode_position(tag, nullptr), cb);
   }
}


/**
 * @brief Support function for build_branch to search through the share mode list.
 *
 * As in other building functions, next_sibling() should be used instead
 * of SiblingWalker: the links that SiblingWalker uses aren't established yet.
 */
ab_handle *SpecsReader::find_shared_link(ab_handle *mode_chain_head, const char *name)
{
   // Skip the '$';
   ++name;

   ab_handle *ptr = mode_chain_head->next_sibling();
   while (ptr && !ptr->is_equal_to(name))
      ptr = ptr->next_sibling();
 
   return ptr;
}

void SpecsReader::replace_shared_ref(ab_handle *head, ab_handle *ref)
{
   ab_handle *link = find_shared_link(head, ref->value());
   if (link)
   {
      ref->set_child(link->first_child());
      ref->clear_value();
   }
}

/**
 * @brief Replace reference nodes with the global nodes to which they refer.
 *
 * While building the tree, external references are added to the end of the
 * array of lines.  Each external reference is also searched for included
 * external references, which are also appended to the end of the main list.
 * When all of the lines are collected, this function will replace the
 * reference nodes with the appropriate appended branches.  External branches
 * can be referenced more than once in the hierarchy, each external reference
 * position will point to the same appended branch.
 *
 * Important programming note: __Do not use SiblingWalker here:__ we are
 * setting up the references to allow SiblingWalker to work later.
 */
void SpecsReader::replace_all_shared_refs(ab_handle *head, ab_handle *branch, bool skip_siblings)
{
   ab_handle *child = branch->first_child();
   if (child)
   {
      if (child->is_shared_ref())
         replace_shared_ref(head, child);
      replace_all_shared_refs(head, child);
   }

   if (!skip_siblings)
   {
      ab_handle *sibling = branch->next_sibling();
      if (sibling)
      {
         if (sibling->is_shared_ref())
            replace_shared_ref(head, sibling);
         replace_all_shared_refs(head, sibling);
      }
   }
      
}

/**
 * @brief Fix shared modes.
 *
 * Important programming note: __Do not use SiblingWalker here:__ we are
 * setting up the references to allow SiblingWalker to work later.
 */
void SpecsReader::reconcile_shares(ab_handle *head)
{
   ab_handle *mode = head;
   while (mode)
   {
      if (mode->level())
         replace_all_shared_refs(head, mode, true);
      
      mode = mode->next_sibling();
   }
}

/**
 * @brief Build a branch with shared modes incorporated into the tree.
 *
 * This is an unusual function that is quite long because all of the
 * stack memory allocation must occur within the function.  It is unusual
 * because it performs a recursive task by looping and tracking progress
 *
 * This function reads at least one, and potentially many specs file modes.
 * (@ref Specs_File_Definitions).  The main mode is used to direct the
 * output of the data transaction.  If there are shared references in the
 * main mode, the shared modes will also be retrieved and incorporated
 * into the main mode tree.
 *
 * Where recursive functions manage state by calling and returning, this
 * function uses chains of ab_handle* in place of stack frames.  The ab_handle
 * has three link pointers: as a line_handle, it has a pointer to the next
 * line_handle.  With the abnode, there are also first_child and next_sibling
 * links.
 *
 * Starting from the head, which is ultimately the value returned through
 * callback, the line_handle chain is a list of the lines in the mode.
 * For each shared reference in the line_handle chain, we will get the
 * shared mode and chain it off of the next_sibling link, which is otherwise
 * unused for a mode.  Each identified shared mode will be saved to the
 * next_sibling link of the last element in the next_sibling chain.
 *
 * As each mode is completed, its internal hierarchical links (child and
 * sibling) are established.
 *
 * When all modes, both the main head and the shared modes, are acquired,
 * a second process will attach the shared modes to the places where they
 * are referenced.
 *
 * When the process is done, the returned ab_handle will be a complete tree
 * with shared modes incorporated as child nodes in place of the shared
 * references.
 *
 * Important programming note: __Do not use SiblingWalker here:__ we are
 * setting up the references to allow SiblingWalker to work later.
 *
 * Special Notes:
 * -# The level value of the mode head is known to be zero, so we're using
 *    it here to count the number of shared references in the mode.
 */
void SpecsReader::t_build_branch(long position, abh_callback &callback) const
{
   Advisor &adv = m_advisor;
   ab_handle *head = nullptr;
   ab_handle *tail = nullptr;
   long saved_position = m_advisor.end() ? -1 : m_advisor.get_position();
   int shared_count;

   // The mode chain is a list linked via the m_sibling member;
   ab_handle *mode_chain_link = nullptr;
   ab_handle *mode_chain_tail = nullptr;

   adv.restore_state(position);

   // We have to make the main mode separately to setup the loop:
   head = tail = make_handle(alloca(len_handle(adv)), adv, nullptr);
   shared_count = 0;
   while(adv.get_next_line() && adv.level()>0)
   {
      tail = make_handle(alloca(len_handle(adv)), adv, tail);
      if (tail->is_shared_ref())
         ++shared_count;
   }

   // link nodes before setting level of the head:
   ab_handle::link_nodes(head);

   // Only prepare to enter loop if we found shared references:
   if (shared_count)
   {
      mode_chain_link = mode_chain_tail = head;
      // Save the shared_count in the mode:
      head->set_level(shared_count);
   }

   while (mode_chain_link)
   {
      int shared_countdown = mode_chain_link->level();
      // Skip mode_links with no shared references:
      assert(shared_countdown);
      
      ab_handle *ptr = mode_chain_link;
      while(ptr && shared_countdown)
      {
         if (ptr->is_shared_ref())
         {
            --shared_countdown;

            // Only get the first reference to the share:
            if (!find_shared_link(head, ptr->value()))
            {
               shared_count = 0;
               long pos = get_shared_mode_position(ptr->value());
               if (pos!=-1)
               {
                  adv.restore_state(pos);
                  
                  void *buff = alloca(len_shared_handle(adv));
                  ab_handle *shead = make_shared_handle(buff, adv, nullptr);
                  ab_handle *stail = shead;

                  while(adv.get_next_line() && adv.level()>0)
                  {                     
                     stail = make_handle(alloca(len_handle(adv)), adv, stail);
                     if (stail->is_shared_ref())
                        ++shared_count;
                  }
                  // Link nodes before setting level of the head:
                  ab_handle::link_nodes(shead);
                  // Save the shared_count in the mode:
                  shead->set_level(shared_count);
               
                  mode_chain_tail->set_sibling(shead);
                  mode_chain_tail = shead;
               }
            }
         }
         ptr = ptr->next();
      }

      // Skip past modes without shared references:
      while ((mode_chain_link=mode_chain_link->next_sibling())
             && mode_chain_link->level()==0)
         ;
   }

   // Replace share references with referenced modes:
   reconcile_shares(head);
   
   callback(head);

   if (saved_position>=0)
      adv.restore_state(saved_position);
}

/** @brief Used to assert a valid position.  Shouldn't be used in production code. */
bool SpecsReader::is_valid_position(long int pos) const
{
   for (auto ptr=m_index.start(); ptr; ptr=ptr->next())
   {
      if (ptr->object()==pos)
         return true;
   }
   return false;
}



#ifdef INCLUDE_SPECSREADER_MAIN

#include <stdio.h>
#include "datastack.cpp"
#include "advisor.cpp"
#include "adbranch.cpp"
#include "prandstr.cpp"


void list_modes(SpecsReader &sr)
{
   for (auto *ptr=sr.start(); ptr; ptr=ptr->next())
   {
      printf("%5ld: %s\n", ptr->object().position(), ptr->str());
   }
}

void show_mode(SpecsReader &sr, const char *name)
{
   auto f = [](const ab_handle *mode)
   {
      printf("I got the mode!\n");
   };
   sr.get_mode(name, f);
}

void read_global_value(SpecsReader &sr)
{
   auto *mode = sr.seek_mode_info("$xml-stylesheet");
   if (mode)
   {
      printf("Found $xml-stylesheet, and it's \"%s\"\n", mode->value());
   }
}

void test_integer_seek(SpecsReader &sreader)
{
   auto f = [] (const ab_handle *mainpage)
   {
      const ab_handle *result1 = mainpage->seek("result", 1);
      if (result1)
         printf("Found result1.\n");
   };
   sreader.get_mode("main_page", f);
}

void test_colon_in_value(SpecsReader &sreader)
{
   auto f = [](const ab_handle *login_form)
   {
      if (login_form)
      {
         const ab_handle *form_action = login_form->seek("schema/form-action");
         if (form_action)
            printf("The form_action value is \"%s\"\n", form_action->value());
         else
            printf("Unable to find schema/form-action\n");
      }
      else
         printf("Unable to find login_form\n");
   };

   sreader.get_mode("login_form", f);
}

void dump_mode_no_sibs(const ab_handle *branch)
{
   printf("mode: %s:\n", branch->tag());
   const ab_handle *ptr = branch->first_child();
   ptr->dump(stdout,1);
}

void test_build_branch(SpecsReader &sreader, const char *mode="new")
{
   auto f = [&sreader](const ab_handle *branch)
      {
         if (branch)
         {
            printf("Got the branch!\n");
            SiblingWalker sw(branch);
            while (sw)
            {
               dump_mode_no_sibs(sw);
               printf("...\n");
               ++sw;
            }

            printf("\n.\n.\n");
            dump_mode_no_sibs(branch);
            printf("\n.\n.\n");
         }
         else
            printf("Didn't get the branch!\n");
      };

   long pos = sreader.get_mode_position(mode);
   sreader.build_branch(pos, f);
}

// If testing for memory usage, 568 bytes alloced for fopen()
// and 72,704 bytes left from loading the library, I think.
int main(int argc, char** argv)
{
   const char *schema = "default.spec";
   const char *mode = nullptr;
   bool blistmodes = false;

   char **ptr = argv;
   char **end = argv + argc;
   // Skip arg #1 (the command)
   while (++ptr < end)
   {
      const char *arg = *ptr;
      if (*arg=='-')
      {
         ++arg;
         switch(*arg)
         {
            case 's':
               schema = *++ptr;
               break;
            case 'm':
               mode = *++ptr;
               break;
            case 'l':
               blistmodes = true;
               break; 
         }
      }
   }
   auto f = [&blistmodes, &mode](SpecsReader &sr)
   {
      if (blistmodes || mode==nullptr)
      {
         sr.print_modes(stdout);
      }
      else if (mode)
      {
         auto fMode = [&sr](const ab_handle *mode)
         {
            mode->dump(stdout);
         };
         sr.seek_mode(mode, fMode);
      }
   };

   SpecsReader::build(schema, f);

   
  // FILE *f = fopen("specsreader.spec","r");
  // const char *mode = "new";
   
//    FILE *f = fopen("default.spec","r");
//    const char *mode = "login";
   
//    if (f)
//    {
//       AFile_Stream af_s(f);
//       Advisor      advisor(af_s);

//       auto l = [&mode](SpecsReader &sr)
//       {
// //         list_modes(sr);
// //         show_mode(sr, "new");
// //         test_integer_seek(sr);

// //         test_colon_in_value(sr);

// //         read_global_value(sr);

//          test_build_branch(sr, mode);
         
//       };

//       Generic_User<SpecsReader,decltype(l)> gu(l);
//       SpecsReader::build(advisor, gu);
//    }

   return 0;
}







#endif
