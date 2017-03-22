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

void Path_List::process(Advisor_Index::ninfo* head,
                        int count,
                        IGeneric_Callback<Advisor_Index> &callback)
{
   auto* ptr = head;
   while (count>0 && ptr)
   {
      if (ptr->is_equal_to("$include"))
      {
         const char *val = static_cast<const char*>(ptr->extra());
         Path_List *pl = scan(val);
         if (pl && pl->is_not_equal(val))
         {
            printf("Adding %s.\n", val);
         }
      }
      ptr = ptr->next();
   }
}


void Path_List::send_index(Advisor_Index::ninfo* root,
                           IGeneric_Callback<Advisor_Index> &callback)
{
   int count = 0;
   auto *ptr = root;
   while (ptr)
   {
      ++count;
      ptr = ptr->next();
   }

   int arrlen = count * sizeof(line_handle);
   line_handle *array = static_cast<line_handle*>(alloca(arrlen));
   base_handle::fill_array(array, count, root->lhandle());

   DataStack<Advisor_Index::info> ds(array, count);
   Advisor_Index ai(ds);
   callback(ai);
}

void Advisor_Index::BEnds::append(ninfo *node)
{
   if (node->is_equal_to("$include"))
      ++m_includes;
   
   if (!m_head)
      m_tail = m_head = node;
   else
   {
      m_tail->next(node);
      m_tail = node;
   }
}

void Advisor_Index::BEnds::scan_for_includes(Path_List &pl,
                                             IGeneric_Callback<ninfo*> &callback)
{
   ninfo **ptrptr = &m_head;

   // Declare pointer to wrapped lambda function as a forward declaration:
   IGeneric_Void_Callback *pgu_fscan = nullptr;

   auto f_incr_ptrptr = [&ptrptr]()  { ptrptr = (*ptrptr)->pp_next(); };

   auto finsert_includes = [this, &pgu_fscan, &ptrptr, &f_incr_ptrptr](BEnds &bends)
   {
      ninfo *include = *ptrptr;

      // There's no way to recover from unexpected mode, so assert and don't check:
      assert(include->is_equal_to("$include"));

      // Save what follows to skip around the include:
      ninfo **after_include = include->pp_next();

      // Set follower of ptr according results of previous collection:
      if (!bends.is_empty())
      {
         *ptrptr = bends.m_head;
         bends.m_tail->next(*after_include);
      }
      else
         *ptrptr = *after_include;

      (*pgu_fscan)();
   };
   Generic_User<BEnds&, decltype(finsert_includes)> gu_inserter(finsert_includes);

   // This lambda function may be repeatedly called as a result of
   // inserting include files.  The closure variable *ptrptr* should be
   // appropriately set whenever the fscan function is reentered to
   // ensure all modes are visited.
   auto fscan = [this, &pl, &ptrptr, &f_incr_ptrptr, &gu_inserter, &callback]()
   {
      while (*ptrptr)
      {
         if ((*ptrptr)->is_equal_to("$include"))
         {
            info& robj = (*ptrptr)->object();
            const char *fname = robj.value();
            Path_List *sought_pl = pl.scan(fname);
            if (sought_pl->is_not_equal("$include"))  // ie, we have a unique path
            {
               Path_List newpl(fname, *ptrptr);
               sought_pl->append(newpl);
               t_collect_include_modes(fname, gu_inserter);

               // The following is a re-think of the process, but it seems
               // logical, so let's do it.

               // Returning from (*pgu_fscan)() means that everything that
               // needed the info from this function has finished and the
               // local data is no longer needed.  It's time to invoke
               // "return" to unwind the stack.
               return;
            }
         }
         
         f_incr_ptrptr();
      }

      callback(m_head);
   };
   Generic_Void_User<decltype(fscan)> gu_scan(fscan);
   
   pgu_fscan = &gu_scan;

   fscan();
}


/**
 * @brief Scan entire Advisor file to make a list of modes (column-0 instructions)
 *
 * This function will simply collects modes, passing the resultant head and
 * tail modes to the callback function for continued processing.
 *
 * If BEnds::scan_for_includes finds an $include instruction, it will call
 * t_collect_include_modes(), which will in turn call this function with the
 * @p filename parameter set.  Thus @p filename will serve as a flag indicating
 * that the Advisor object's file handle shouldl be closed when this function
 * is finished reading the file.
 */
void Advisor_Index::t_collect_modes(Advisor &advisor,
                                    IGeneric_Callback<BEnds&> &callback,
                                    const char *filepath)
{
   BEnds bends;
   advisor.rewind();

   while (!advisor.end())
   {
      // Only saving modes
      if (advisor.level()==0)
      {
         const char *name = advisor.tag();
         const char *value = advisor.value();

         size_t len_extra = (value) ? 1+strlen(value) : 0;
         size_t len = ninfo::line_size(name, len_extra);
         ninfo* node = ninfo::init_handle(alloca(len),name,len_extra,nullptr);
         info& i = node->object();
         i.m_position = advisor.get_position();
         i.m_filepath = filepath;
         
         if (len_extra)
         {
            i.m_value = static_cast<const char*>(node->extra());
            memcpy(const_cast<char*>(i.m_value), value, len_extra);
         }
         else
            i.m_value = nullptr;

         // Get next line to see if its level is 0, meaning it's a new mode, or
         // above 0, meaning that it's a child and that the mode has children.
         
         // Wait to get the next line until we've saved the
         // first mode's info because get_next_line() will overwrite
         // the name, value, and position values.
         advisor.get_next_line();
         i.m_has_children = advisor.level() > 0;
         if (!i.m_has_children)
            i.m_filepath = nullptr;

         bends.append(node);
      }
      else
         advisor.get_next_line();
   }

   callback(bends);
}

void Advisor_Index::t_build_new(Advisor &advisor,
                                IGeneric_Callback<Advisor_Index> &callback)
{
   Path_List pathlist;

   auto f_build_done = [&callback](ninfo *head)
   {
      int count = base_handle::count(head->lhandle());
      line_handle *array = static_cast<line_handle*>(alloca(count*sizeof(line_handle)));
      base_handle::fill_array(array, count, head->lhandle());

      DataStack<info> ds(array, count);
      Advisor_Index ai(ds);
      callback(ai);
   };

   auto f_scan_done = [&pathlist, &f_build_done](BEnds &bends)
   {
      if (!bends.is_empty())
      {
         Generic_User<ninfo*, decltype(f_build_done)> gu_build_done(f_build_done);
         bends.scan_for_includes(pathlist, gu_build_done);
      }
      else
         f_build_done(bends.m_head);
   };
   Generic_User<BEnds&,decltype(f_scan_done)> gu_scan_done(f_scan_done);

   t_collect_modes(advisor, gu_scan_done);
}

void Advisor_Index::t_collect_include_modes(const char *filename,
                                            IGeneric_Callback<BEnds&> &callback)
{
   auto f_got_file = [&filename, &callback](AFile_Handle &afh)
   {
      Advisor adv(afh);
      t_collect_modes(adv, callback, filename);
   };

   AFile_Handle::build(filename, f_got_file);
}


/**
 * @brief Build an index of the modes in an advisor file.
 */
//! [DataStack_Building_Snippet_Advisor_Index]
void Advisor_Index::t_build_old(Advisor &advisor,
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
         tail = t_handle<info>::init_handle(alloca(len),name,len_extra,tail);

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
         if (strcmp("$shared",name)==0)
         {
            auto &obj = ptr->object();
            ifputc(':', f);
            ifputs(obj.value(), f);
         }
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

const t_handle<Advisor_Index::info>*
SpecsReader::seek_advisor_mode(const char *tag, const char *value) const
{
   return m_index.seek(tag,value);
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

   int saved_handle = -1;
   auto f_prep_advisor = [this, &saved_handle](const Advisor_Index::ninfo *p) -> bool
   {
      // Ensure code always resets m_advisor file handle to its original value:
      if (saved_handle!=-1)
         throw std::runtime_error("Error: parked saved_handle value.");
      
      auto &obj = p->object();
      int pos = obj.position();
      if (pos==-1)
         return false;
      
      if (obj.is_external())
      {
         int handle = open(obj.filepath(), O_RDONLY);
         if (handle<0)
         {
            fprintf(stderr,
                    "Unable to open \"%s\": %s\n",
                    obj.filepath(),
                    strerror(errno));
            throw std::runtime_error("failed to open file");
         }
         else
            saved_handle = m_advisor.replace_handle(handle);
      }

      m_advisor.restore_state(pos);

      return true;
   };

   auto f_drop_advisor = [this, &saved_handle](void)
   {
      if (saved_handle>-1)
      {
         int handle_to_close = m_advisor.replace_handle(saved_handle);
         close(handle_to_close);
         saved_handle = -1;
      }
   };

   
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

               auto *amode = get_shared_advisor_mode(ptr->value());
               
               if (f_prep_advisor(amode))
               {
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

                  f_drop_advisor();
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

void list_modes(t_handle<Advisor_Index::info>* ptr)
{
   while (ptr)
   {
      Advisor_Index::info &i = ptr->object();
      printf("%5ld: %s : %s  %s children  %s\n",
             i.position(),
             ptr->str(),
             i.value(),
             (i.m_has_children ? "has" : "NO"),
             (i.m_filepath ? i.m_filepath : "")
         );
      ptr = ptr->next();
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
int old_main(int argc, char** argv)
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


int main(int argc, char** argv)
{
   const char *file = "zz_test.srm";
   if (argc>1)
      file = argv[1];

   auto f_got_file = [](AFile_Handle &afh)
   {
      Advisor adv(afh);
      
      auto f_got_index = [&adv](Advisor_Index &ai)
      {
         printf("Got the advisor index!\n");
         // SpecsReader sr(adv, ai);
      };
      Generic_User<Advisor_Index,decltype(f_got_index)> gu_got_index(f_got_index);
      
      Advisor_Index::t_build_old(adv, gu_got_index);
   };


   AFile_Handle::build(file, f_got_file);
      
}




#endif
