/** @file */

#include "istdio.hpp"

#include "xmlutils.hpp"
#include "advisor.hpp"
#include "schema.hpp"

#include <fcntl.h>
#include <unistd.h>


// Static variables:
MYSQL Schema::s_mysql;
error_notifier Schema::s_notifier = notify_stderr;
bool Schema::s_is_web_request = Schema::is_web_request();

const uint32_t Schema::s_invalid_session = static_cast<uint32_t>(-1);
const char* Schema::s_default_import_confirm_proc = "ssys_default_import_confirm";

const char* Schema::s_ok_status = "Status: 200 OK\n";
const char* Schema::s_jump_status = "Status: 303 See Other\n";

/**
 * @brief Static value for FILE stream to which header information is sent.
 *
 * While running as a web service, this value will be stdout.  However, in
 * batch mode, where the output may be piped into `xsltproc`, the value
 * will be changed to stderr so the header information won't corrupt the
 * XML output.
 */
FILE* Schema::s_header_out = stdout;

char Schema::s_failure_message[200];
char Schema::s_failure_detail[80];

Schema::Debug_Action Schema::s_debug_action = DEBUG_ACTION_IGNORE;

/**
 * @defgroup Node_Reserved_Words Node Reserverd Words
 * @brief Reserved Words for each level of SchemaDoc output.
 *
 * When processing a Specs file, unrecognized words are added to the SchemaDoc
 * as adhoc XML.  These arrays contain the recognized words for each level of
 * output.  Instructions with these words are processed explicitely and are
 * then ignored during the adhoc-XML output.
 * @{
 */

const char *arr_root_reserved[] = {
   "attributes",
   "root",
   "database",
   "jump",
   "meta-jump",
   "mode-type",
   "name",
   "post",
   "procedure",
   "qstring",
   "result",
   "root-procedure",
   "row-name",
   "schema",
   "schema-proc",
   "session-type",
   "tag",
   "type",
   "xml-stylesheet",
   nullptr
};

const char *arr_result_reserved[] = {
   "name",
   "attributes",
   "schema",
   "rndx",
   "root",
   "row-name",
   "type",
   nullptr
};

const char *arr_schema_reserved[] = {
   "attributes",
   "fields",
   "field",
   "form-action",
   "name",
   nullptr
};

const char *arr_field_reserved[] = {
   "name",
   "type",
   "primary-key",
   "auto-increment",
   "sort",
   "not-null",
   "enum",
   "set",
   "length",
   nullptr
};

const Schema::struct_session_type Schema::map_session_types[] =
{
   { "none",      Schema::STYPE_NONE      },
   { "establish", Schema::STYPE_ESTABLISH },
   { "simple",    Schema::STYPE_SIMPLE    },
   { "identity",  Schema::STYPE_IDENTITY  }
};
   
const Schema::struct_mode_action Schema::map_mode_actions[] =
{
   { "save-post",       Schema::MACTION_SAVE_POST       },
   { "abandon-session", Schema::MACTION_ABANDON_SESSION },
   { "delete",          Schema::MACTION_DELETE          },
   { "export",          Schema::MACTION_EXPORT          },
   { "import",          Schema::MACTION_IMPORT          },
   { "info",            Schema::MACTION_INFO            },
   { "lookup",          Schema::MACTION_LOOKUP          },

   { "form-result",     Schema::MACTION_FORM_RESULT     },
   { "form-submit",     Schema::MACTION_FORM_SUBMIT     },
   
   { "table",           Schema::MACTION_TABLE           },
   { "display",         Schema::MACTION_DISPLAY         },
   { "form-edit",       Schema::MACTION_FORM_EDIT       },
   { "form-new",        Schema::MACTION_FORM_NEW        },
   { "form-import",     Schema::MACTION_FORM_IMPORT     },
   { "form-view",       Schema::MACTION_FORM_VIEW       }
};
const Schema::struct_mode_action *Schema::end_map_mode_actions =
   map_mode_actions + (sizeof(map_mode_actions) / sizeof(struct_mode_action));

void Schema::print_mode_types(FILE *f)
{
   const struct_mode_action *m = map_mode_actions;
   while (m<end_map_mode_actions)
   {
      if (m!=map_mode_actions)
         ifputs(", ", f);
      ifputs(m->str, f);
      ++m;
   }
   ifputc('\n', f);
}


int strcmpdash(const char* l, const char *r)
{
   bool mismatched_dash = false;
   while (*l && *r)
   {
      if (*l!=*r)
      {
         if ((*l=='-' && *r=='_') || (*l=='_' && *r=='-'))
            mismatched_dash = true;
         else
            return *l - *r;
      }

      ++l;
      ++r;
   }

   return mismatched_dash ? 1024 : 0;
}

const char *arr_global_tags[] = {
   "database",
   "xml-stylesheet",
   "default-mode",
   "session-type",
   "test_authorized",
   "jump_no_session",
   "jump_not_authorized",
   "shared",
   nullptr
};

/** Not-comprehensive, this lint simply confirms valid global instruction tags. */
void Schema::print_lint(FILE *f, const SpecsReader *reader)
{
   int warning_count = 0;
   auto cb = [&f, &warning_count](const char *tag, const char *value) -> bool
   {
      if (*tag=='$')
      {
         const char *v = tag+1;
         const char **l = arr_global_tags;
         while (*l)
         {
            int diff = strcmpdash(v, *l);
            if (diff==1024)
            {
               ++warning_count;
               ifprintf(f, "\"%s\" should be \"%s\"\n", v, *l);
            }
            ++l;
         }
      }
      return true;
   };

   reader->scan_modes(cb);

   if (warning_count==0)
      ifprintf(f, "No global mode errors found.\n");
}

/** @} */

/**
 * @brief List of schema-default types
 *
 * These types will cause a schema to be included in the SchemaDoc output,
 * even if a schema instruction is not included in the result branch.
 */
const char *arr_schema_default_types[] = { "table",
                                           "record",
                                           "form",
                                           nullptr };


/**
 * @brief Fill the buffer with a random string based on time, urandom, ip_address
 *
 * @param buff A buffer into which the random characters will be copied
 * @param copylen The number of characters that will be copied.
 *
 * This function will create a random string that incorporates the
 * user's IP address to be used for a session string.  No terminating
 * \0 will be added, so it is up to the calling procedure to provide
 * and set the terminating byte if necessary.
 *
 * @todo Add the actual code that includes the IP address from the
 * getenv("REMOTE_ADDR");
 */
void make_session_string(char *buff, size_t copylen)
{
   assert(copylen>1);

   int fh = open("/dev/urandom", O_RDONLY);
   if (fh)
   {
      size_t rcount = read(fh, buff, copylen);
      if (rcount==copylen)
      {
         unsigned char *p = reinterpret_cast<unsigned char*>(buff);
         unsigned char *end = p + copylen;
         while (p<end)
         {
            *p %= 62;
            
            if (*p < 10)
               *p += '0';
            else if (*p < 36)
               *p += 'A'-10;
            else
               *p += 'a'-36;

            ++p;
         }
      }

      int result = close(fh);
      if (result)
      {
         ifputs("Error closing file in make_session_string: ", stderr);
         switch(errno)
         {
            case EBADF: ifputs("bad file descriptor\n", stderr); break;
            case EINTR: ifputs("interrupted\n", stderr); break;
            case EIO:   ifputs("i/o error\n", stderr); break;
            default:    ifputs("unknown error\n", stderr); break;
         }
      }
   }
}

void set_session_seed(MYSQL *mysql, const char* seed)
{
   auto f = [](int result_number, DataStack<BindC> &ds)
   {
      ; // we don't need to respond.
   };
   Result_User_Row<decltype(f)> user(f);

   SimpleProcedure::build(mysql, "CALL ssys_seed_session_string(?)", "s32",
                          Adhoc_Setter<1>(si_text(seed)),
                          user);
}

/** @brief Count fields that are marked "omit" to leave out of schema. */
int count_field_exclusions(const ab_handle *schema)
{
   int count = 0;
   const ab_handle *field = schema->first_child();
   const ab_handle *omit;
   while (field)
   {
      SiblingWalker sw(field);
      while (sw)
      {
         if (sw.is_tag("field"))
            if ((omit=sw.seek("omit")) && !omit->is_false())
               ++count;
         ++sw;
      }
   }
   
   return count;
}

/**
 * @brief Fill an allocated array of pointers to strings.
 *
 * A function calling this function should have previously called
 * count_field_exclusions() to determine the number of string pointers
 * is necessary to hold all the omissions.  This function will fill
 * that array from the memory-resident schema.
 */
void fill_field_exclusion_list(const char **list, int count, const ab_handle *schema)
{
   SiblingWalker sw(schema->first_child());
   const ab_handle *omit;
   const char **str = list;
   while (sw && count>0)
   {
      if (sw.is_tag("field"))
         if ((omit=sw.seek("omit")) && !omit->is_false())
         {
            *str = sw.value();
            --count;
         }
      ++sw;
   }
}

void t_make_field_exclusion_list(const ab_handle *mode,
                                 const IGeneric_Callback<const char**> &user)
{
   const ab_handle *schema = mode->seek("schema");
   const char **rval = nullptr;
   if (schema)
   {
      int count = count_field_exclusions(schema);
      if (count)
      {
         size_t len = (count+1) * sizeof(char*);
         char **arr = static_cast<char**>(alloca(len));
         memset(arr,0,len);

         fill_field_exclusion_list(rval, count, schema);
      }
   }
   user(rval);
}



/** @brief Local function for reading the environment in different settings. */
extern char **environ;
void print_env(bool cookies_only=false)
{
   ifprintf(stdout,"<env>\n");
   for (char **env = environ; *env; ++env)
   {
      if (!cookies_only || strncmp(*env, "HTTP_COOKIE", 11)==0)
      {
         ifputs("<val name=\"", stdout);
         print_str_as_xml(*env, stdout);
         ifputs("\" />\n", stdout);
      }
   }

   ifprintf(stdout,"</env>\n");
}

/** @brief Records environment variables for debugging. */
// void print_env_to_tmp(void)
// {
//    int fh = open("/tmp/schema_start_env.txt", O_WRONLY|O_CREAT|O_TRUNC);
   
//    if (fh)
//    {
//       const char *msg = "Environment dump.\n";
//       write(fh, msg, strlen(msg));
//       extern char **environ;
//       for (char **env = environ; *env; ++env)
//       {
//          size_t len = strlen(*env);
//          write(fh, *env, len);
//          write(fh, "\n", 1);
//       }

//       close(fh);
//    }
//    else
//       ifprintf(stderr, "Failed to open env file, %s\n", strerror(errno));
// }



/**
 * @brief Print unclaimed settings as attributes.
 *
 * This function will print every first-level branch of @p handle whose tag
 * is not included in the @p skip list.
 *
 * This function also follows the convention that the contents of any group
 * named _siblings_ with a shared reference value (_$_-prefixed) will add
 * its contents as siblings to the current level.  In other words, the
 * _siblings_ tag will be ignored and the level will not be incremented
 * when processing its children.
  */
void print_adhoc_attributes(FILE *out, const ab_handle *handle, const char **skip)
{
   const ab_handle *branch = handle->seek("attributes");
   if (!branch)
      branch = handle;
   
   SiblingWalker sw(branch->first_child());
   while (sw)
   {
      // Cheap has_value() check before more expensive tag_in_list() check:
      if (sw.has_value() && (!skip || !sw.tag_in_list(skip)))
         print_xml_attribute(out, sw.tag(), sw.value());

      ++sw;
   }
}


/**
 * @brief Prints unrecognized (not in skip list) lines as xml elements.
 *
 * @param out       File stream to which to print.
 * @param handle    branch from which to print.
 * @param skip      list of tags that should not be printed (reserved list)
 * @param terminate flag to print ">\n" before the first child to terminate
 *                  the element to whom these new elements belong.
 *
 * @return Number of child elements printed.  Helps for recursion to
 *         know if the element is empty or not.
 */
int print_adhoc_elements(FILE *out,
                         const ab_handle *handle,
                         const char **skip,
                         bool terminate)
{
   int children = 0;
   SiblingWalker sw(handle->first_child());
   while (sw)
   {
      // Cheap has_value() check before more expensive tag_in_list() check:
      if (!sw.has_value() && (!skip || !sw.tag_in_list(skip)))
      {
         if (terminate && !children)
            ifputs(">\n", out);
         
         ++children;
         
         ifprintf(out, "<%s", sw.tag());
         print_adhoc_attributes(out, sw);
         int sub = print_adhoc_elements(out, sw, nullptr, true);
         if (sub)
            ifprintf(out, "</%s>\n", sw.tag());
         else
            ifputs(" />\n", out);
      }

      ++sw;
   }

   return children;
}

/**
 * @brief Print attributes in priority order.
 *
 * Prints attributes found in the priorities.  Duplicate-named
 * attributes in lower priorites are skipped.  For example, if
 * priority_two and priority_one both have an attribute named @e attr1,
 * the value will be taken from priority_one only.
 */
void print_field_attributes(FILE *out,
                            const ab_handle *priority_one,
                            const ab_handle *priority_two,
                            const ab_handle *priority_three)
{
   // Attempt getting attributes at start to help preempt later
   // searches if not found.
   const ab_handle *arr[3] = { priority_one, priority_two, priority_three };

   // For each of the ab_handle branches,
   for (int i=0; i<3; ++i)
   {
      if (arr[i])
      {
         // find an attributes branch
         const ab_handle *attr = arr[i];
         if (!attr->is_equal_to("attributes"))
            attr = attr->seek("attributes");

         if (attr)
         {
            SiblingWalker sw(attr->first_child());
            while (sw)
            {
               // Compare the current branch against the previous
               // branches, ignoring attributes that were defined
               // earlier:
               bool preempted = false;
               for (int j=0; j<i; ++j)
               {
                  if (arr[j] && arr[j]->seek(sw.tag()))
                  {
                     preempted = true;
                     break;  // break here to avoid extra for-loop conditional check.
                  }
               }

               if (!preempted)
                  print_field_attribute(out, sw);

               ++sw;
            }
         }
      }
   }
}

bool QStringer_Setter::set_binds(DataStack<BindC> &ds) const
{
   int stop = m_qstringer.count();
   if (stop)
   {
      int len = 1 + m_qstringer.get_longest_name();
      char *buff = static_cast<char*>(alloca(len));
      for (int i=0; i<stop; ++i)
      {
         if (m_qstringer.has_val_at(i))
         {
            m_qstringer.get_name_at(i,buff,len);

            int bindex = ds.index_by_name(buff);
            if (bindex>=0)
            {
               t_handle<BindC> &handle = ds.handle(bindex);
               StringStreamer ss(m_qstringer.get_raw_val_at(i));
               SegmentStreamer s_value(ss);
               handle.object().set_from(s_value);
            }
         }
      }
   }
   return true;
}


const char *Schema_Printer::s_field_ignore_list[] = {
      "type",
      "primary-key",
      "auto-increment",
      "sort",
      "not-null",
      "enum",
      "set",
      "length",
      nullptr
   };


/**
 * @brief Gets an appropriate fields branch and passes it to print_schema_fields.
 */
void Schema_Printer::find_and_print_schema_fields(const ab_handle *schema)
{
   const ab_handle *fields = schema->seek("fields");
   if (!fields)
      fields = schema;
   print_schema_fields(fields);

   
   // if (fields && fields->is_setting())
   // {
   //    const char *name = fields->value();
   //    if (*name=='$')
   //    {
   //       auto f = [this, &fields](const ab_handle *gfields)
   //       {
   //          if (gfields)
   //             fields = gfields->seek("fields");
            
   //          print_schema_fields(fields);
   //       };
   //       m_specsreader.seek_shared_mode(name, f);
   //    }
   // }
   // else
   //    // Don't worry, if the conditional fails, fields can be null or not,
   //    // and either value is OK to send on to print_schema_fields.
   //    print_schema_fields(fields);
}

void Schema_Printer::print_schema_fields(const ab_handle *fields)
{
   const ab_handle *field = nullptr;
   const ab_handle *attribute;
   for (auto *p=m_bindstack->start(); p; p=p->next())
   {
      const BindC &bind = p->object();
      
      if (fields)
         field = fields->seek("field", p->str());

      ifputs("<field name=\"", m_out);
      ifputs(p->str(), m_out);
      ifputc('"', m_out);

      // First print MYSQL attributes:
      if ((field && (attribute=field->seek("type"))))
         print_field_attribute(attribute);
      else
         print_field_attribute("type", bind.sqltype_name());

      if (bind.sqlflag_primary_key())
         print_field_attribute("primary-key", "true");
      if (bind.sqlflag_autoincrement())
         print_field_attribute("auto-increment", "true");
      if (bind.sqlflag_num())
         print_field_attribute("sort", "number");
      if (bind.sqlflag_not_null())
         print_field_attribute("not-null", "true");
      if (bind.buffer_length()>0)
         print_field_attribute("length", bind.buffer_length());

      if (field && (attribute=field->seek("enum")))
         print_field_attribute(attribute);
      else if (bind.sqlflag_enum())
         print_field_attribute("enum", bind.paramtype_name());
      else if (field && (attribute=field->seek("set")))
         print_field_attribute(attribute);
      else if (bind.sqlflag_set())
         print_field_attribute("set", bind.paramtype_name());

      if (bind.dtdid())
         print_field_attribute("dtd", bind.dtdid());
      
      // Print attributes non-MySQL attributes:
      if (field)
         print_adhoc_attributes(m_out, field, arr_field_reserved);
      
      ifputs(" />\n", m_out);
   }
}


/**
 * @brief Print the form schema from the BindStack in StoredProc, informed by
 * instructions in schema.
 *
 * A schema needs a name.  If the schema line does not specify a name, "form"
 * will be used as a fallback.
 *
 * Because the schema is to be used as a form, it needs to supply an action
 * URL for the form.  An value at "attributes/form-action" will take precedence
 * over "form-action", but either location is allowed.
 */
void Schema_Printer::print(const ab_handle *schema, const char *default_name)
{
   const char *name = "row";
   if (default_name)
      name = default_name;
   if (schema)
   {
      if (schema->is_setting())
         name = schema->value();
      else
      {
         const ab_handle *hname = schema->seek("name");
         if (hname && hname->is_setting())
            name = hname->value();
      }
   }

   const ab_handle *attributes = schema ? schema->seek("attributes") : nullptr;

   ifputs("<schema", m_out);
   print_field_attribute("name", name);

   // If there is no attributes/action, try to find path=action
   const ab_handle *action = nullptr;
   if (!(attributes && (action=attributes->seek("form-action"))))
      if (!(schema && (action=schema->seek("form-action"))))
         action=m_mode.seek("form-action");
   
   if (action)
      ::print_field_attribute(m_out, action);
   
   if (schema)
      print_adhoc_attributes(m_out, schema, arr_schema_reserved);

   ifputs(">\n", m_out);

   if (schema)
   {
      if (m_bindstack)
         find_and_print_schema_fields(schema);

      print_adhoc_elements(m_out, schema, arr_schema_reserved);
//      m_specsreader.print_sub_elements(m_out, schema, arr_schema_reserved);
   }
   else if (m_bindstack)
      print_schema_fields(nullptr);

   ifputs("</schema>\n", m_out);
}

/**
 * @brief Uses pre_fetch_ to setup the result group before printing rows.
 *
 * This function collects data about the current result for several purposes:
 * -# The element name of the group (for the opening tag here and for the
 * closing tag as printed by Result_As_SchemaDoc::result_complete.
 * -# The element name of the rows.
 * -# Additional attributes of the group element.
 * -# Will set print_as_child flag of BindC if nodetype="child" as advice
 * for use_result_row. (NOT DONE)
 *
 * @todo Create print_as_child flag in BindC AND set it appropriately according
 * to the instructions in the schema, if present.
 */
void Result_As_SchemaDoc::pre_fetch_use_result(int result_number,
                                               DataStack<BindC> &dsresult,
                                               SimpleProcedure &proc)
{
   m_row_name = "row";
   const char *result_type = nullptr;

   const ab_handle *schema = nullptr;
   const ab_handle *thandle = nullptr;

   const ab_handle *result_handle = m_mode->seek("result", result_number);
   if (!result_handle)
      result_handle = m_mode->child_by_position("result", result_number-1);

   bool force_schema_print = false;

   // Special handling for result 1:
   if (result_number==1)
   {
      // For first result with no specified result branch,
      // use the mode instructions to run the result:
      if (!result_handle)
         result_handle = m_mode;

      // Override missing schema if in first result and specific mode type:
      force_schema_print = must_print_schema() && !schema_prints_early();
   }

   if (result_handle)
   {
      // Special handling when using the response mode as a result branch:
      // Don't use the response mode-type as the result type.
      if (result_handle == m_mode)
      {
         // If we're in a form_result response mode with no result instructions,
         // make the first result a variables type result.
         if (m_mode_action==Schema::MACTION_FORM_RESULT)
         {
            int result_count = m_mode->count_children("result");
            if (result_count==0 && result_number==1)
               result_type = "variables";
         }
      }
      else if ((thandle=result_handle->seek("type")))
         result_type = thandle->value();
      
      // Update binds for mode (attribute or element)
      // update_binds()

      // Change default m_row_name value if row-name handle exists
      if ((thandle=result_handle->seek("row-name")))
         m_row_name = thandle->value();

      // A schema may overwrite the m_row_name:
      schema = result_handle->seek("schema");
      if (schema)
      {
         if (schema->has_value())
            m_row_name = schema->value();
         else
         {
            const ab_handle *name = schema->seek("name");
            if (name)
               m_row_name = name->value();
         }
      }

      // Get m_group_name or set to "result" as a default:
      m_group_name = "result";
      if (schema && (thandle=schema->seek("name")))
          m_group_name = thandle->value();
      else if ((thandle = result_handle->seek("name")))
         m_group_name = thandle->value();
      else if (result_type && strcmp(result_type,"ref")==0)
         m_group_name = "ref";

      // The first output must be the opening tag and its attributes
      ifputc('<', m_out);
      ifputs(m_group_name, m_out);
      print_xml_attribute(m_out, "rndx", result_number);

      if (result_type)
         print_xml_attribute(m_out, "type", result_type);

      // If no schema, we need to identify the row name in the result element:
      if (!schema)
         print_xml_attribute(m_out, "row-name", m_row_name);

      // The following items should only be printed if we're
      // in a real result branch, not a response-mode substitute:
      if (result_handle != m_mode)
      {
         if ((thandle = result_handle->seek("form-action")))
            print_field_attribute(m_out, thandle);
      
         if ((thandle = result_handle->seek("attributes")))
            print_field_attributes(m_out, thandle);

         if (result_handle!=m_mode)
            print_adhoc_attributes(m_out, result_handle, arr_result_reserved);
      }
         
      ifputs(">\n", m_out);

      // print the adhoc stuff, but only if explicitly in a result branch:
      if (result_handle->is_tag("result"))
         m_specsreader.print_sub_elements(m_out, result_handle, arr_result_reserved);

      // Don't print the schema if it comes from m_mode and m_mode
      // includes a schema-proc:
      if (m_mode==result_handle && schema_prints_early())
         schema = nullptr;
      
      // Next, print the fields
      if (schema
          || (result_type && string_in_list(result_type, arr_schema_default_types))
          || force_schema_print )
      {
         Schema_Printer sprinter(m_specsreader, *m_mode, dsresult, m_out);
         sprinter.print(schema, m_row_name);
      }
   }
   else
   {
      ifputs("<result", m_out);
      print_xml_attribute(m_out, "rndx", result_number);
      if (result_type)
         print_xml_attribute(m_out, "type", result_type);
      ifputs(">\n", m_out);
   }
}

/**
 * @brief Print result row of a Schema Document.
 *
 * @todo Code to print attributes first, then child fields.
 */
void Result_As_SchemaDoc::use_result_row(int result_number,
                                         DataStack<BindC> &dsresult)
{
   ifputc('<', m_out);
   ifputs(m_row_name, m_out);

   int index=0;
   for (auto *col=dsresult.start();
        col;
        ++index, col=col->next())
   {
      BindC &obj = col->object();
      if (!obj.is_null())
      {
         ifputc(' ', m_out);
         ifputs(col->str(), m_out);
         ifputs("=\"", m_out);

         /**
          * @todo I think string types should be printed in Result_User_Base rather
          * that in a IClass.  IClass shouldn't have to know about handling a
          * truncation.  And besides, IClass probably shouldn't have to know about
          * printing XML, either.
          */

         if (obj.is_string_type())
            print_string(obj, index);
         else
            obj.print(m_out);

         ifputc('\"', m_out);
      }
   }

   ifputs(" />\n", m_out);
}

void Result_As_SchemaDoc::result_complete(int result_number, DataStack<BindC> &dsresult)
{
   // Query results are now always named result, so there is no
   // longer a need to consider other tag names...just close
   // the result element:

   ifputs("</", m_out);
   ifputs(m_group_name, m_out);
   ifputs(">\n", m_out);
}


void Result_As_Headers::use_result_row(int result_number,
                                       DataStack<BindC> &dsresult)
{
   
}


/**
 * @brief loop_sentry for Command-Line-Interface.
 *
 * Primarily for use during developement for testing.  This sentry
 * returns TRUE once, then FALSE after that.
 *
 * When this application is rewritten as FASTCGI, it will call
 * FCGI_Accept() to determine whether the loop continues.
 */
bool cli_sentry(void)
{
   static bool keep_on = true;
   if (keep_on)
   {
      keep_on = false;
      return true;
   }
   else
      return false;
}

/**
 * @brief loop_sentry for CGI/FASTCGI.
 *
 * This function will direct a once-through for the run_schema loop
 * for the CGI version, with the future expecation that it will run
 * the FASTCGI multiple loop version later.
 */
bool cgi_sentry(void)
{
#ifdef FASTCGI
   return FCGI_Accept()==0;
#else
   static bool keep_on = true;
   if (keep_on)
   {
      keep_on = false;
      return true;
   }
   else
      return false;
#endif   
}


/** @brief Print error messages as an XML element. */
void notify_as_xml(const char *msg, const char *type, FILE *out)
{
   ifputs("<notice", out);
   print_xml_attribute(out, "type", type);
   print_xml_attribute(out, "message", msg);
   ifputs(" />\n", out);
}

/** @brief Report error messages to stderr as plain text. */
void notify_stderr(const char *msg, const char *type, FILE *out)
{
   ifputs("NOTICE: ", out);
   ifputs(type, out);
   ifputs(": ", out);
   ifputs(msg, out);
}

/**
 * @brief Default constructor, other constructors delegate to this one
 *        in order to initialize variables.
 */
Schema::Schema(SpecsReader *sr, FILE *out)
   : m_out(out),
     m_specsreader(sr),
     m_setter(nullptr),
     m_puller(nullptr),

     m_database_confirmed(false),
        
     m_cstringer(nullptr),
     m_qstringer(nullptr),
     m_mode(nullptr),
     m_advisor(nullptr),
     m_type_value(nullptr),
     m_mode_action(MACTION_NULL),
     m_meta_jump(nullptr),
     m_prepage_message(nullptr),
     m_session_id(s_invalid_session)
{
   clear_failure_strings();
}

Schema::Schema(SFW_Resources &sfwr, FILE *out)
   : m_out(out),
     m_specsreader(&sfwr.m_sreader),
     m_setter(nullptr),
     m_puller(nullptr),

     m_database_confirmed(false),
        
     m_cstringer(sfwr.m_cookies),
     m_qstringer(sfwr.m_qstringer),
     m_mode(sfwr.m_mode),
     m_advisor(nullptr),
     m_type_value(nullptr),
     m_mode_action(MACTION_NULL),
     m_meta_jump(nullptr),
     m_prepage_message(nullptr),
     m_session_id(s_invalid_session)
{
   clear_failure_strings();
}

static const Schema::dapair g_debug_action_types[] = {
   {"mode", Schema::DEBUG_ACTION_PRINT_MODE},
   {"modes", Schema::DEBUG_ACTION_PRINT_RESPONSE_MODES},
   {"all-modes", Schema::DEBUG_ACTION_PRINT_ALL_MODES},
   {"types", Schema::DEBUG_ACTION_PRINT_MODE_TYPES},
   {"lint", Schema::DEBUG_ACTION_LINT},
   {nullptr, Schema::DEBUG_ACTION_IGNORE}
};


const Schema::dapair *Schema::s_debug_action_types = g_debug_action_types;

char Schema::ProcMessageException::s_type[32];
char Schema::ProcMessageException::s_msg[256];
char Schema::ProcMessageException::s_where[64];

void Schema::ProcMessageException::set(char *dest, const char *source, int limit)
{
   if (source)
   {
      char *last_char = dest + limit- 1;
      while (*source && dest < last_char)
         *dest++ = *source++;
   }
   *dest = '\0';
}


bool Schema::set_debug_action_mode(const char *action)
{
   s_debug_action = DEBUG_ACTION_IGNORE;

   const dapair *ptr = s_debug_action_types;
   if (action)
   {
      while (ptr->name)
      {
         if (strcmp(ptr->name, action)==0)
         {
            s_debug_action = ptr->action;
            return true;
         }
         ++ptr;
      }

   }

   ifputs("-d (debug action) ", stderr);
   if (action)
   {
      ifputs(action, stderr);
      ifputs(" is not supported. ", stderr);
   }
   else
      ifputs("is missing its value. ", stderr);

   ifputs("Allowed values are: ", stderr);
   ptr = s_debug_action_types;
   while (ptr->name)
   {
      if (ptr>s_debug_action_types)
         ifputs(", ", stderr);
      
      ifputs(ptr->name, stderr);
      ++ptr;
   }

   ifputs(".\n", stderr);

   return false;
}

Schema::MODE_ACTION Schema::get_mode_type(const char *str)
{
   MODE_ACTION rval = MACTION_NONE;
   if (str)
   {
      const struct_mode_action* act = map_mode_actions;
      const struct_mode_action* found = nullptr;
      while (act < end_map_mode_actions)
      {
         if (0==strcmp(act->str, str))
         {
            found = act;
            break;
         }
         ++act;
      }

      if (found)
         rval = found->value;
      else
      {
         ifputs("Unknown mode type \"", stderr);
         ifputs(str, stderr);
         ifputs("\"\n", stderr);
      }

   }

   return rval;
}

/**
 * @brief Returns the first result branch of the installed mode.
 *
 * This function searches for a result branch labeled "1";
 * failing that, it looks for the first unlabeled result branch.
 * The function returns what it finds, or NULL if nothing was
 * found.
 */
const ab_handle *Schema::get_first_result(void) const
{
   const ab_handle *handle = m_mode->seek("result", 1);

   if (!handle)
   {
      handle = m_mode->first_child();
      while (handle)
      {
         // We don't have to check for "1" because we already
         // know that doesn't exist, so just break for the first
         // unlabeled result branch:
         if (handle->is_tag("result") && !handle->is_setting())
            break;

         handle = handle->next_sibling();
      }
   }
   return handle;
}


/**
 * @brief General purpose writer of an error element.
 *
 * Call this function to write out an error element in a standard format.
 * This is called by several context-specific error reporting functions.
 */
void Schema::write_error_element(FILE *out,
                                 const char *type,
                                 const char *msg,
                                 const char *detail)
{
   ifputs("<error type=\"", out);
   print_str_as_xml(type, out);
   ifputs("\" msg=\"", out);
   print_str_as_xml(msg, out);
   
   if (detail && *detail)
   {
      ifputs("\" detail=\"", out);
      print_str_as_xml(detail, out);
   }

   ifputs("\" />\n", out);
}


/**
 * @brief Initialize static failure strings each time a Schema is constructed.
 */
void Schema::clear_failure_strings(void)
{
   s_failure_message[0] = '\0';
   s_failure_detail[0] = '\0';
}

/**
 * @brief Set fatal-error error message.
 *
 * Use this to set failure strings for failures that prevent any
 * meaningful output.  The info set here will be output after the
 * response headers and the XML processing instructions so the
 * document can be well-formed.
 */
void Schema::set_failure_message(const char *msg, const char *detail)
{
   size_t len = strlen(msg);

   // If not enough room, reset len to entire buffer,
   // less an \0 element:
   if (len>=sizeof(s_failure_message))
      len = sizeof(s_failure_message) - 1;  // leave room for \0

   memcpy(s_failure_message, msg, len);
   s_failure_message[len] = '\0';

   if (detail)
   {
      len = strlen(detail);
      
      // If not enough room, reset len to entire buffer,
      // less an \0 element:
      if (len>=sizeof(s_failure_detail))
         len = sizeof(s_failure_detail) - 1;  // leave room for \0

      memcpy(s_failure_detail, detail, len);
      s_failure_detail[len] = '\0';
   }
}

/** @brief Writes message to FILE* m_out using notify function pointer. */
void Schema::report_message(const char *msg)
{
   (*s_notifier)(msg, "message", m_out);
}

/**
 * @brief Writes error message to FILE* m_out using notify function pointer.
 */
void Schema::report_error(const char *where)
{
   char *buff = nullptr;
   int result = mysql_errno(&s_mysql);
   if (result)
   {
      const char *error = mysql_error(&s_mysql);

      size_t extra = sizeof("At ") + sizeof(", ") + 1;  // + '\0'
      size_t len = strlen(where) + strlen(error) + extra;
      
      buff = static_cast<char*>(alloca(len));
      strcpy(buff, "At ");
      strcat(buff, where);
      strcat(buff, ", ");
      strcat(buff, error);
   }
   else
   {
      size_t extra = sizeof("Error at ") + 1; // + '\0'
      buff = static_cast<char*>(alloca(sizeof(where) + extra));
      strcpy(buff, "Error at ");
      strcat(buff, where);
   }

   (*s_notifier)(buff, "error", m_out);
}


void Schema::report_missing_error(const char *what, const char *missing)
{
   static const char *pre = "Missing ";
   static const size_t len_pre = strlen(pre);
   size_t len_what = strlen(what);
   size_t len_missing = strlen(missing);
   size_t len = len_pre + len_what + len_missing + 3;
   char *buff = static_cast<char*>(alloca(len));
   char *ptr = buff;
   memcpy(ptr, pre, len_pre);
   ptr += len_pre;
   memcpy(ptr, what, len_what);
   ptr += len_what;
   memcpy(ptr, ": ", 2);
   ptr += 2;
   memcpy(ptr, missing, len_missing);
   ptr += len_missing;
   *ptr = '\0';

   procedure_message_reporter("error", buff, nullptr);
}

/**
 * @brief Use this function to report errors after the document has started.
 */
void Schema::report_error(FILE *out, const char *str)
{
   ifputs("<message type=\"error\"", out);
   if (str && *str)
      print_xml_attribute(out, "where", str);
   ifputs(" />\n", out);
}

/**
 * @brief Write a standard format message to specified file stream.
 *
 * @param out File stream to which the message will be printed.
 * @param type Message type, see below
 * @param message Text of the message
 * @param detail Optional extra information
 *
 * The type parameter is used to indicate the nature of the message and
 * how it should be reported to the user.  There are three types of messages:
 *
 * - __signal__ is a user-supplied data-entry error.  Messages of this type
 *              will be generated when a stored procedure calls SIGNAL to
 *              indicate why the procedure was not successful.  See the
 *              login example, App_Login_Create procedure for SIGNAL
 *              examples.
 *              @todo Replace verbal reference with documentation page with
 *              code snippet.
 
 * - __error__  is a framework or setup error.  Message of this type should
 *              not occur, and the problem should be fixed ASAP.
 *
 * - __notice__ is a message that will be shown to the user when a page is
 *              loaded or data-request is received.
 */
void Schema::print_message_as_xml(FILE *out,
                                  const char *type,
                                  const char *message,
                                  const char *detail)
{
   ifputs("<message", out);
   
   print_xml_attribute(out, "type", type);
   print_xml_attribute(out, "message", message);
   
   if (detail)
      print_xml_attribute(out, "detail", detail);
   
  ifputs(" />\n", out);
}

/**
 * @brief Prepare a message to be printed when a Schema fails.
 *
 * This message-writer is to be used for fatal errors that occur before
 * any portion of the document has been output.
 *
 * The message will be printed using the print_message_as_xml() function
 * with Schema::s_failure_error_message and Schema::s_failure_location
 * will be printed It should be in a catch
 * block in wait_for_requests()
 */
void Schema::pre_doc_failure(const char *message, const char *where)
{
   
}

/** @brief Thread start routine for get_fake_stdin_from_command_args(). */
void * Schema::pthread_fake_stdin_start_routine(void *data)
{
   pipe_cl_struct &pcs = *static_cast<pipe_cl_struct*>(data);
   int fh = pcs.h_write;
   uint16_t vtag = dash_val('v');

   bool pre_first = true;

   for (char **ptr = pcs.argv+1; *ptr; ++ptr)
   {
      uint16_t apre = *reinterpret_cast<const uint16_t*>(*ptr);
      if (apre == vtag)
      {
         if (pre_first)
            pre_first = false;
         else
            write(fh, "&", 1);
         
         const char *val = *++ptr;
         size_t len = strlen(val);
         write(fh, val, len);
      }
   }

   // Close write-end of the pipe to signal EOF to read-end:
   close(fh);

   return nullptr;
}

/**
 * @brief Get the file position of response mode.
 *
 * Using the SpecsReader and QueryString information that should already been
 * prepared, look for the file position of the appropriate response mode.
 *
 * This function gets and discards stack-based strings to search the SpecsReader
 * so as to conserve stack space while getting this information.
 */
long int Schema::SFW_Resources::get_mode_position(void) const
{
   long int rval = -1;
   if (m_qstringer)
   {
      auto fval = [this, &rval](const char *name)
         {
            rval = m_sreader.get_mode_position(name);
         };
      m_qstringer->get_name_at(0, fval);
   }

   if (rval==-1)
   {
      auto f2 = [this, &rval](const ab_handle* abh)
         {
            rval = m_sreader.get_mode_position(abh->value());
         };

      m_sreader.get_global_mode("default-mode", nullptr, f2);
   }

   return rval;
}

void Schema::get_resources_from_environment(FILE *out)
{
   SFW_Resources* p_sfwr = nullptr;

   // Finally, save cookies, if found, then invoke the callback with our work.
   auto f_cookies = [&out, &p_sfwr](const BaseStringer *cookielist)
   {
      if (cookielist)
         p_sfwr->m_cookies = cookielist;

      Schema schema(*p_sfwr, out);

      // Select and clear from installed response mode:
      schema.set_requested_database();
      schema.clear_for_new_request();

      // Catch any exception so we have a change for database clean up:
      try
      {
         schema.process_response_mode();
      }
      catch(const std::exception &se)
      {
         print_error_as_xml(out, se.what(), "process_response_mode");
      }
      
      schema.clear_for_new_request();
   };

   // Fourth, load the response mode:
   auto f_rmode = [&p_sfwr, &f_cookies](const ab_handle *mode)
   {
      p_sfwr->m_mode = mode;
      BaseStringer::build(getenv("HTTP_COOKIE"), f_cookies, ';');
   };

   // Third...save query string object, use query string to get a response mode
   auto f_qstring = [&p_sfwr, &f_rmode](const BaseStringer *querystring)
   {
      if (querystring)
         p_sfwr->m_qstringer = static_cast<const QStringer*>(querystring);

      long int pos_mode = p_sfwr->get_mode_position();
      // No response mode? Leave message and return:
      if (pos_mode==-1)
      {
//         print_message_as_xml();
         return;
      }
      else
      {
         Generic_User_Const_Pointer<ab_handle, decltype(f_rmode)> user(f_rmode);
         p_sfwr->m_sreader.get_branch(pos_mode, user);
      }
   };

   

   // Second...get QueryString
   auto f_sreader = [&p_sfwr, &f_qstring](SpecsReader &sr)
   {
      SFW_Resources sfwr(sr);
      p_sfwr = &sfwr;
      
      BaseStringer::build(getenv("QUERY_STRING"), f_qstring);
   };

   // First...get SpecsReader
   const char *pt = getenv("PATH_TRANSLATED");
   if (pt)
   {
      change_to_path_dir(pt);
      SpecsReader::build(pt,f_sreader);
   }
   else
      ifputs("PATH_TRANSLATED not found.\n", stderr);
}

/**
 * @brief Return a file handle with which we can fake stdin for the CGI.
 *
 * @param argc The argc argument from main()
 * @param argv The argv arguemnt from main()
 * @param pcs Memory location for passing values to the thread start_routine.
 *
 * @return File descriptor.  Technically, `stdin` is a stream, but since
 *         we'll be using `dup2()` to make Schema use it, it makes more sense
 *         to remain a file descriptor.
 *
 * The pcs argument is necssary because if we allocate a pipe_cl_struct in
 * this stack frame, it will have been overwritten when the calling function
 * tries to read the data from the pipe.
 */
int Schema::get_fake_stdin_from_command_args(int argc, char **argv,
                                             pipe_cl_struct &pcs)
{
   int result;
   int cl_pipe[2];
   if ((result=pipe(cl_pipe)))
   {
      ifputs("failed to create pipe to fake stdin from command arguments.\n", stderr);
      return -1;
   }

   pcs.argc = argc;
   pcs.argv = argv;
   pcs.h_write = cl_pipe[1];
   
   pthread_t pthread;
   if ((result = pthread_create(&pthread,
                                nullptr,
                                pthread_fake_stdin_start_routine,
                                static_cast<void*>(&pcs))))
   {
      ifprintf(stderr,
              "pthread_create failed in get_fake_stdin: %s\n",
              strerror(result));
      close(cl_pipe[0]);
      close(cl_pipe[1]);

      return -1;
   }
   else
      return cl_pipe[0];
}

/**
 * @brief Check connection and reconnect if it is down.
 *
 * Since under some condition, a connection may close on its own, this
 * function cheaply confirms the connection and reconnects if it has gone
 * down.
 *
 * @return True if connection usable, False if not.  Schema::wait_for_requests()
 *         will break the loop and terminate if a close connection cannot be
 *         reopened.
 */
bool Schema::confirm_mysql_connection(void)
{
   int result = mysql_ping(&s_mysql);
   if (result)
   {
      if (!start_mysql())
         return false;
   }

   return true;
}


/**
 * @brief Keep processing requests until the sentry says to stop.
 *
 * @param sentry Pointer to a function that will decide whether to continue looping.
 * @out   out    File stream to which the output will be directed.
 *
 * This public static function serves as the request processing loop.
 * Schema::wait_for_requests creates new Schema objects as requests come
 * in to generate the schema documents.
  *
 * Once the request method is detected, processing is passed on to the
 * @ref Begin_Request_Processing group of functions.
 */
int Schema::wait_for_requests(loop_sentry sentry, FILE *out)
{
   int exitval = 0;
   
   if (start_mysql())
   {
      while ((*sentry)())
      {
         // Re-connect if connection broken, break if it can't be done.
         if (!confirm_mysql_connection())
            break;
         
         // Only for debugging/discovery:
         // print_env_to_tmp();
         
         s_headers_done = false;

         assign_sfw_xhrequest_flag();

         try
         {
            Schema::get_resources_from_environment(stdout);
         }
         catch(const schema_exception &se)
         {
            Schema::print_error_as_xml(stdout, se.what(), "install_response_mode");
         }
      }
      
      mysql_close(&s_mysql);
   }
   else
      exitval = 1;
      
   mysql_library_end();

   return exitval;
}

/**
 * @brief Initializes static s_mysql object before any requests are processed.
 *
 * MYSQL is initiated primarily using the values from ~/.my.cnf.
 * Before initiation, we check the environment for overrides to
 * the host, user, password, and db files.
 *
 * Ideally, the user account will be one that only allows CALL
 * commands, which will help prevent misuse of the database.
 */
bool Schema::start_mysql(void)
{
   mysql_init(&s_mysql);
   mysql_options(&s_mysql,MYSQL_READ_DEFAULT_FILE,"/etc/mysql/my.cnf");
   mysql_options(&s_mysql,MYSQL_READ_DEFAULT_GROUP,"client");

   
   int result = mysql_options(&s_mysql, MYSQL_OPT_LOCAL_INFILE, nullptr);
   if (result)
      ifprintf(stderr, "Failed to set local_infile option: %s\n", mysql_error(&s_mysql));

//   show_login_info(s_mysql);
   
   // Check for overrides.  NULLs will use /etc/mysql/my.cnf values (see above)
   static const char *host = getenv("MYS_HOST");
   static const char *user = getenv("MYS_USER");
   static const char *pass = getenv("MYS_PASS");
   static const char *db   = getenv("MYS_DB");
   
   // Name the other default parameters to mysql_real_connect:
   int port = 0;
   const char *socket = nullptr;
   unsigned long client_flag = 0;

   MYSQL *handle = mysql_real_connect(&s_mysql,
                                      host, user, pass, db,
                                      port, socket, client_flag);

   if (handle)
   {
      // Install LOAD LOCAL INFILE-thwarting handler:
      MySQL_LoadData::disable_local_infile(&s_mysql);
      return true;
   }
   else
   {
      ifprintf(stderr, "Failed to start MYSQL: %s\n", mysql_error(&s_mysql));
      return false;
   }
}

/**
 * @brief Moves to the host directory of the file name in the path.
 *
 * This function uses alloca to build a directory path string from the
 * file path string.  Once `chdir` is called, the directory path string
 * can be discarded upon return and kept off of the stack for the rest
 * of the request processing.
 */
void Schema::change_to_path_dir(const char *path)
{
   const char *lastslash = strrchr(path,'/');
   if (lastslash)
   {
      size_t len = lastslash - path;
      char *buff = static_cast<char*>(alloca(len+1));
      memcpy(buff, path, len);
      buff[len] = '\0';

      chdir(buff);
   }
}

void Schema::log_new_request(void)
{
   const char *type = getenv("REQUEST_METHOD");
   const char *qstring = getenv("QUERY_STRING");
   ifprintf(stderr, "*** Request type %s with %s.\n", type, qstring);
}

/**
 * @brief Calls app-defined procedure App_Session_Cleanup to ensure secure environment.
 *
 * This function calls system procedure ssys_clear_for_request, which in turn calls
 * app-defined procedure App_Session_Cleanup.  The indirect call allows for additional
 * system setup in the future without breaking existing applications.
 *
 * The app-defined procedure App_Session_Cleanup should set session variables to
 * NULL to prevent leftover values from being exploited by another application.
 * However, some design might benefit from initializing a session variable here as a
 * flag, so this function is called twice per request, once just after the database
 * is checked and set, and again upon returning from processing the response.  The
 * second time would be sufficient for security, but the first call to this function
 * is also run for the hypothetical session initialization need.
 */
void Schema::clear_for_new_request(void)
{
   auto cb_result = [](int result_number, DataStack<BindC> &ds)
   {
   };
   Result_User_Row<decltype(cb_result)> result_user(cb_result);
   SimpleProcedure::build(&Schema::s_mysql,
                          "CALL ssys_clear_for_request()",
                          result_user);
}

/** @brief Discerns if POST by getting REQUEST_METHOD environment variable. */
bool Schema::is_post_request(void)
{
   const char *method = getenv("REQUEST_METHOD");
   return method && (0==strcmp("POST", method));
}


/**
 * @brief Adds content-type and boundary info for save_stdin, if multipart/form-data.
 *
 * The Content-Type and boundary are saved in the environment variables for
 * multipart/form-data, and are thus missing from stdin when processing a request.
 * When the response type=save-post, we have to add this stuff back to the top of
 * of the file to complete the POST data.
 *
 * Using FILE*, even in FASTCGI mode, so there is a pragma to remove the remapped
 * FILE* definition.
 */
//@ [Write_Multipart_Preamble]
#pragma push_macro("FILE")
#undef FILE
size_t Schema::write_multipart_preamble(FILE *f)
#pragma pop_macro("FILE")
{
   size_t len = 0;
   const char* bstring = "boundary=";
   const char* ct = getenv("CONTENT_TYPE");
   if (ct)
   {
      gfputs("Content-Type: ", f);  // 14 characters
      
      const char* bndry = strstr(ct, bstring);
      if (bndry)
      {
         bndry += strlen(bstring);
         gfputs(ct, f);
         gfputs("\r\n\r\n", f);      // 4 characters
         len = 20 + strlen(ct);
      }
      else
      {
         gfputs(ct, f);
         len = 14 + strlen(ct);
      }
   }

   return len;
}
//@ [Write_Multipart_Preamble]

/**
 * @brief Write's POST data to target.
 *
 * When indicated by a mode's type = save_post, write stdin (which is the
 * POST data) to @p target.  The POST data can then be used later for debugging
 * complex forms, especially multipart forms.
 */
//@ [Save_Stdin]
void Schema::save_stdin(const char *target)
{
   const char *error = nullptr;
   
   char buff[1024];
   size_t copied;
   size_t total_read = 0;
   size_t total_written = 0;

   mode_t saved_mask = umask(00000);

   // Using auto so the FILE return value is not converted to FCGI_FILE:
   auto* ftarget = gfopen(target, "w");

   umask(saved_mask);
                            
   
   if (ftarget)
   {
      // This only writes the CONTENT_TYPE value if it is set:
      total_written = write_multipart_preamble(ftarget);

      do
      {
         // Using ifread() for built-in stream to call appropriate fread(),
         // regardless of FASTCGI mode:
         copied = ifread(buff, 1, sizeof(buff), stdin);
         if (copied>0)
         {
            total_read += copied;
            assert(copied <= sizeof(buff));
            // Using gfwrite() to write to the target file using stdio.h functions:
            copied = gfwrite(buff, 1, copied, ftarget);
            if (copied==0)
            {
               error = strerror(gferror(ftarget));
               break;
            }
            else
               total_written += copied;
         }
         else if (!ifeof(stdin))
            error = strerror(iferror(stdin));
      }
      while (copied>0);
      
      gfclose(ftarget);
   }
   else
      error = strerror(errno);

   sprintf(buff, "Read %lu, wrote %lu bytes.\n", total_read, total_written);

   print_message_as_xml(m_out,
                        (error ? "error" : "result"),
                        (error ? error : buff),
                        "save-post");
}
//@ [Save_Stdin]



/**
 * @brief Clears session cookies
 */
void Schema::clear_session_cookies(void)
{
   // Clear cookies if session has expired:
   set_cookie("SessionId");
   set_cookie("SessionHash");
}


/**
 * @brief Create a hash and use it to start a new session.
 *
 * Creates a record in SSYS_SESSION for the system-managed session, calling
 * `ssys_session_start`.  `ssys_session_start` calls `App_Session_Start` which
 * should be a application-specific session procedure that sets up other data.
 */
bool Schema::create_session_records(void)
{
   // This function will set cookie values in the header,
   // so the header must not have been concluded earlier:
   assert(!s_headers_done);
   
   char buff[33];
   make_session_string(buff,32);
   buff[32] = '\0';
   set_session_seed(&s_mysql, buff);

   bool in_session = false;

   auto cb_result = [this,&in_session,&buff](int result_number, DataStack<BindC> &ds)
   {
      if (result_number==1 && ds.count()==2)
      {
         BindC &b_id = ds[0].object();
         BindC &b_hash = ds[1].object();
         if (!b_id.is_null())
         {
            set_cookie("SessionId", &b_id);
            set_cookie("SessionHash", &b_hash);

            // We need to save the new session id:
            ai_ulong vid(&m_session_id);
            b_id.assign_to(vid);

            in_session = true;
         }
      }
   };
   Result_User_Row<decltype(cb_result)> result_user(cb_result);

   SimpleProcedure::build(&Schema::s_mysql,
                          "CALL ssys_session_start()",
                          result_user);

   return in_session;
}

/**
 * @brief Calls MySQL procedure `ssys_session_abandon` to clear and
 *        remove session records.
 *
 * @param id SessionId cookie value
 * @param hash SessionHash cookie value
 *
 * This function drops the session in MySQL, but does not disturb the
 * cookie values.  Use clear_session_cookies to remove session cookies
 * after calling this function.
 */
//@ [SimpleProcedure_Adhoc_Params]
void Schema::abandon_session_records(uint32_t id, const char *hash)
{
   auto cb_result = [this](int result_number, DataStack<BindC> &ds)
      {
         // We don't care about the result. If we change our mind,
         // refer to confirm_session for an example of how to read
         // the results.
//         clear_session_cookies();
      };
   Result_User_Row<decltype(cb_result)> result_user(cb_result);
   
   SimpleProcedure::build(&Schema::s_mysql,
                          "CALL ssys_session_abandon(?,?)",
                          "Is32",
                          Adhoc_Setter<2>(ai_ulong(&id),
                                          si_text(hash)),
                          result_user);
}


/**
 * @brief Unconditionally delete session records for after a failed login attempt.
 *
 * We don't know the results of the login query until we have lost the cookie
 * values.  This function gets 'em again to call
 * abandon_session_records(uint32_t, conat char*).
 */
void Schema::abandon_session_records(void)
{
   get_session_cookies(
      [this](uint32_t id, const char *hash)
      {
         abandon_session_records(id,hash);
      }
      );
}

//@ [SimpleProcedure_Adhoc_Params]

/**
 * @brief Check if the id and hash values match a current session record.
 *
 * @param id SessionId cookie value
 * @param hash SessionHash cookie value
 * @return `true` if session is value, otherwise `false`.
 */
bool Schema::confirm_session(uint32_t id, const char *hash) const
{
   bool in_session = false;
   
   auto cb_result = [&in_session](int result_number, DataStack<BindC> &ds)
      {
         if (result_number==1 && ds.count()==1)
         {
            BindC &o = ds[0].object();
            if (!o.is_null())
            {
               int64_t rlong = -1;
               ai_longlong ril(&rlong);;
               o.assign_to(ril);

               if (rlong==1)
                  in_session = true;
            }
         }
      };
   Result_User_Row<decltype(cb_result)> result_user(cb_result);
   
   SimpleProcedure::build(&s_mysql,
                          "CALL ssys_session_confirm(?,?)",
                          "Is32",
                          Adhoc_Setter<2>(ai_ulong(&id),
                                          si_text(hash)),
                          result_user);
   
   return in_session;
}

const char *Schema::value_from_mode(const char *name) const
{
   const char *rval = nullptr;
   
   const ab_handle *handle = m_mode->seek(name);
   if (handle && handle->is_setting())
      rval = handle->value();
   
   return rval;
}


/**
 * @brief Search the mode and the SpecsReader for a branch tag.
 *
 * This function returns a string from a fully-loaded mode or from
 * a global mode, whose value is also resident in memory.  In both
 * cases, it is safe to return the string, as long as the user doesn't
 * return the string to a calling function before the mode or
 * SpecsReader was loaded.
 */
const char *Schema::value_from_mode_or_global(const char *name) const
{
   const char *rval = value_from_mode(name);

   if (!rval)
   {
      // make buffer for string with prepended asterisk:
      size_t len = strlen(name);
      char *buff = static_cast<char*>(alloca(len+2));
      
      char *ptr = buff;
      *ptr++ = '$';
      memcpy(ptr, name, len);
      ptr += len;
      *ptr = '\0';

      const Advisor_Index::info *ai = m_specsreader->seek_mode_info(buff);
      if (ai)
         rval = ai->value();
   }

   return rval;
}

const char *Schema::get_jump_destination(SESSION_TYPE st) const
{
   // Not likely, but without a current mode (and the SpecsReader
   // from which the mode was acquired), this won't work.
   assert(m_mode);
   
   const char *rval = value_from_mode("jump");
   if (!rval)
   {
      const Advisor_Index::info *handle;
      if (st>STYPE_ESTABLISH
          && (handle=m_specsreader->seek_mode_info("jump_not_authorized")))
         rval = handle->value();

      if (!rval && st > STYPE_NONE
         && (handle=m_specsreader->seek_mode_info("jump_no_session")))
      rval = handle->value();
   }

   return rval;
}


/**
 * @brief If `test_authorized` in specs file, call named procedure to test if
 *        authorized.
 *
 * The procedure named by `test_authorized` should take no parameters, using
 * the session variable @session_confirmed_id to run whatever test the
 * application requires to confirm authorization.
 *
 * @return TRUE if the session is valid and either there is no `test_authorized`
 *         setting, or the `test_authorized` procedure returns 1.
 */
bool Schema::is_session_authorized(uint32_t id) const
{
   bool is_authorized = false;

   if (session_is_valid())
   {
      const char *procname = value_from_mode_or_global("test_authorized");

      // If there is no authorization test, then there can be no
      // authorization failure: return true.
      if (procname==nullptr)
      {
         ifputs("Checking authorization without test_authorized instruction.\n",
               stderr);

         is_authorized = true;
      }
      else
      {
         is_authorized = false;

         auto cb_result = [&is_authorized](int result_number, DataStack<BindC> &ds)
         {
            if (result_number==1 && ds.count()==1)
            {
               BindC &o = ds[0].object();
               if (!o.is_null())
               {
                  int32_t rval;
                  ai_long ril(&rval);
                  o.assign_to(ril);
                  is_authorized = (rval==1);
               }
            }
         };
         
         auto cb_query = [&id, &cb_result](const char *query)
         {
            Result_User_Row<decltype(cb_result)> result_user(cb_result);

            SimpleProcedure::build(&s_mysql,
                                   query,
                                   "I",
                                   Adhoc_Setter<1>(ai_ulong(&id)),
                                   result_user);
         };

         SimpleProcedure::build_query_string(procname, 1, cb_query);
      }
   }
   
   return is_authorized;
}

/**
 * @brief Get session type, checking the response mode first, then a global value.
 *
 * @return Session-type for the response mode.  First look for a session type
 *         instruction in the response mode, then, if not found, look for a
 *         global setting.  The default value (if not found) will be no session.
 */
Schema::SESSION_TYPE Schema::get_session_type(void) const
{
   static const struct_session_type *end =
      map_session_types + sizeof(map_session_types) / sizeof(struct_session_type);
   
   SESSION_TYPE rval = STYPE_NONE;
   
   const char *str = value_from_mode_or_global("session-type");
   if (str)
   {
      const struct_session_type *ptr = map_session_types;
      const struct_session_type *found = nullptr;
      while (ptr < end)
      {
         if (0==strcmp(ptr->str, str))
         {
            found = ptr;
            break;
         }
         ++ptr;
      }

      if (found)
         rval = found->value;
      else
      {
         ifputs("Unknown session type \"", stderr);
         ifputs(str, stderr);
         ifputs("\"\n", stderr);
      }
   }
   return rval;
}

/**
 * @brief Send a message to the error log for a missing mode.
 */
void Schema::log_missing_mode(const char *mode_name) const
{
   ifputs("Unable to find specs mode \"", stderr);
   ifputs(mode_name, stderr);
   ifputs("\"\n", stderr);
}

/**
 * @brief Install mode_handle and set instance string variables.
 *
 * Since the entire mode is resident in memory, it is safe to
 * use values contained in the mode.  This function collects a
 * few useful instructions to simplify running install_response_mode().
 */
void Schema::set_instance_mode_values(const ab_handle *mode_handle)
{
   // We should have a valid mode:
   assert(mode_handle);
   this->m_mode = mode_handle;

   // This should be the first/only time running this function:
   assert(m_type_value==nullptr);
   m_type_value = value_from_mode("type");
}


/**
 * @brief Break-out logic for need to jump before further processing.
 *
 * This may seem dangerous to return a char*, but the char pointers come
 * from either the active response mode or a global mode, any of which will
 * currently be in-memory, and thus not at risk of going out of scope before
 * the value is used.
 */
const char *Schema::check_for_early_jump(SESSION_TYPE stype, SESSION_STATUS sstatus)
{
   const char *early_jump = nullptr;
   const char *jump_no_session = value_from_mode_or_global("jump_no_session");

   // Check for relocate and terminate _before_ getting the procedure;
   // these conditions include:
   //
   // 1. - Authorization required (stype == STYPE_IDENTITY)
   //    - Session not authorized (sstatus < SSTAT_AUTHORIZED)
   //    - Jump where told, or go to "/"
   if (stype==STYPE_IDENTITY && sstatus < SSTAT_AUTHORIZED)
   {
      early_jump = value_from_mode_or_global("jump_not_authorized");
      if (!early_jump)
         early_jump = jump_no_session ? jump_no_session : "/";

      // A session may be in force, but we won't disturb it
      // because it can be used at the login page.
   }
   //
   // 2. - abandoning == true
   //    - jump location discerned
   if (m_mode_action==MACTION_ABANDON_SESSION)
   {
      early_jump = value_from_mode("jump");

      if (!early_jump)
         early_jump = jump_no_session;
               
      // get_session_status() will have already deleted the
      // session records, so it's not necessary, and in fact,
      // impossible to execute here because we've forgetten
      // the hash string.
               
      // it's OK if early_jump is null here, it just means
      // that it shouldn't jump later (allow the page to
      // continue).
   }

   //
   // 3. - A session is required (stype > STYPE_ESTABLISH)
   //    - No session is in force (sstatus < SSTAT_RUNNING)
   //    - jump_no_session mode value set
   //    - NOT in a login-type form which establishes a session
   if (stype > STYPE_ESTABLISH && sstatus < SSTAT_RUNNING)
      if (!early_jump)
         early_jump = jump_no_session;

   return early_jump;
}

void Schema::action_save_post(void)
{
   print_Status_200();
   print_XML_ContentType();
   write_headers_end();
   write_xml_start();
                  
   const char *target = value_from_mode("target");
   if (target)
   {
      save_stdin(target);
   }
   else
   {
      print_message_as_xml(m_out,
                           "error",
                           "Missing required file target",
                           "save-post");
   }
}

bool Schema::confirm_multipart_form(void) const
{
   bool is_multipart_form = false;

   const char *ctype = getenv("CONTENT_TYPE");
   if (ctype)
   {
      auto f = [&is_multipart_form](const BaseStringer* pbs)
      {
         if (pbs->is_name_at(0, "multipart/form-data"))
         {
            const char *boundary = nullptr;

            // Content-type should have two values:
            if (pbs->count()==2 && pbs->is_name_at(1,"boundary"))
            {
               boundary = pbs->get_raw_val_at(1);
            }
            else  // for if more than two values, just in case:
            {
               int pos = pbs->get_index_of_name("boundary");
               if (pos)
               {
                  int len = pbs->val_len_at(pos);
                  char *buff = static_cast<char*>(alloca(1+len));
                  memcpy(buff, pbs->get_raw_val_at(pos), len);
                  buff[len] = '\0';
                  boundary = buff;
               }
            }

            is_multipart_form = boundary ? true : false;
         }
         
      };
      Generic_User_Const_Pointer<BaseStringer, decltype(f)> user(f);
      
      BaseStringer::t_build(ctype, user, ';');
   }

   return is_multipart_form;
}

/**
 * @brief Starts work with information available in the selected response mode.
 *
 * This function is only called from Schema::get_resources_from_environment(), so
 * the task of selecting and clearing the database is left to that function.  That
 * allows early exit from this function without having to worry about database
 * cleanup.
 */
void Schema::process_response_mode(void)
{
   assert(m_specsreader);
   assert(m_mode);
   
   // If debug action, print mode and return/terminate:
   switch(s_debug_action)
   {
      case DEBUG_ACTION_PRINT_MODE:
         return m_mode->dump(stderr, false);
      case DEBUG_ACTION_PRINT_MODE_TYPES:
         return print_mode_types(stderr);
      case DEBUG_ACTION_PRINT_RESPONSE_MODES:
         return m_specsreader->print_modes(stderr, false);
      case DEBUG_ACTION_PRINT_ALL_MODES:
         return m_specsreader->print_modes(stderr, true);
      case DEBUG_ACTION_LINT:
         return print_lint(stderr, m_specsreader);
      case DEBUG_ACTION_IGNORE:
         break;
   }

   m_type_value = value_from_mode("type");
   m_mode_action = get_mode_type(m_type_value);

   if (m_mode_action==MACTION_SAVE_POST || m_mode->seek("save-post","true"))
   {
      action_save_post();
      return;
   }

   bool abandoning = m_mode_action == MACTION_ABANDON_SESSION;

   SESSION_TYPE   stype   = get_session_type();
   SESSION_STATUS sstatus = get_session_status(stype, abandoning);

   // early_jump will be set if authorization rules and the current session status warrant:
   const char *early_jump = check_for_early_jump(stype, sstatus);

   // Provide appropriate feedback before leaving for authorization issues:
   if (early_jump)
   {
      if (s_sfw_xhrequest)
      {
         set_forbidden_header();
         clear_session_cookies();
      }
      else
      {
         print_Status_303();
         write_location_header(early_jump);

         if (sstatus==SSTAT_EXPIRED)
            clear_session_cookies();
      }
      write_headers_end();
   }
   else
   {
      // The early jump with via "Status: 303" and Location is done,
      // everything after this uses "Status: 200", so let's send it
      // before we go on:
      print_Status_200();

      // If a session is needed but expired or not yet running,
      // start a session and write out new cookie values:
      if (stype>STYPE_NONE && sstatus<SSTAT_RUNNING)
      {
         // This function creates the records and writes the cookie values:
         if (!create_session_records())
         {
            print_XML_ContentType();
            write_headers_end();
            write_xml_start();
            print_message_as_xml(m_out,
                                 "error",
                                 "failed to establish a session",
                                 "QUERY_STRING");
            
            // early exit with completed document:
            return;
         }
      }

      if (m_mode_action==MACTION_EXPORT)
      {
         const char *filename = m_mode->seek_value("file-name");
         if (filename)
         {
            ifputs("Content-Disposition: filename=\"", s_header_out);
            ifputs(filename, s_header_out);
            ifputs("\"\n", s_header_out);
         }
         print_FODS_ContentType();
      }
      else
         print_XML_ContentType();
            
      write_headers_end();

      if (m_mode_action==MACTION_EXPORT)
      {
         process_export();
      }
      else
      {
         write_xml_start(m_mode_action!=MACTION_EXPORT);
         
         // The jump instruction will be included as a meta instruction
         // in the HTML head element, and as such, will be a suggestion.
         // The standard sfwtemplates.xsl will create a meta element in
         // the HTML head element, but custom implementations are not
         // bound to that behavior.
         m_meta_jump = value_from_mode("jump");

         // Detect request types:
         if (m_mode_action==MACTION_IMPORT)
            process_import();
         else
            process_response(stype, sstatus);
      }
   }
}

/**
 * @brief Returns an ODS file from a query.
 *
 * This is the one path that will not return an XML file, but rather an ODS file.
 *
 * Several steps here are different from the standard path, due to using Result_As_FODS
 * as the Result_User.
 */
void Schema::process_export(void)
{
   // Prepare IParam_Setter m_setter, leaving m_puller=nullptr
   StrmStreamer ss(stdin);
   Streamer_Setter setter(ss);
   m_setter = &setter;

   const char *procname = m_mode->seek_value("procedure");
   if (procname)
   {
      auto fsp = [this](StoredProc &sp)
      {
         SimpleProcedure proc(sp.querystr(), sp.bindstack());
         Result_As_FODS rafods(m_out);

         auto frun = [this, &proc, &rafods](void)
         {
            proc.run(&s_mysql, this, &rafods);
         };

         rafods.fork_to_zip(frun);
      };
      Generic_User<StoredProc, decltype(fsp)> spu(fsp);

      StoredProc::build(&s_mysql, procname, spu);
   }
   else
      print_message_as_xml(m_out, "error", "Missing procedure instruction");
   
}

/**
 * @brief Collects data from stdin in multipart/form-data.
 *
 * Uses a IParam_Setter that processes multipart/form-data encoding to
 * find an uploaded file, which will be inserted into the quarantine table
 * named in the response mode.
 */
void Schema::process_import(void)
{
   try
   {
      // Prepare m_puller, leaving m_setter at nullptr.
      if (confirm_multipart_form())
      {
         StrmStreamer ss(stdin);
         Multipart_Pull mpp(ss);
         m_puller = &mpp;

         if (process_import_form())
         {
            // import_table() should have ended by calling a query
            // to return a confirmation resultset.  Write it out here:
            process_root_branch(m_mode, nullptr,true);
         }
         else
         {
            print_message_as_xml(m_out,
                                 "error",
                                 "import failed");
         }
      }
      else
      {
         print_message_as_xml(m_out,
                              "error",
                              "File import requires multipart/form-data.");
      }
   }
   catch(const schema_exception &se)
   {
      print_error_as_xml(m_out, se.what(), "importing data");
   }
}

/**
 * @brief Wraps stdin into a IParam_Setter for setting procedure parameters.
 */
void Schema::process_response(SESSION_TYPE stype, SESSION_STATUS sstatus)
{
   // Prepare IParam_Setter m_setter, leaving m_puller=nullptr
   StrmStreamer ss(stdin);
   Streamer_Setter setter(ss);
   m_setter = &setter;
         
   // Since we've already written out the headers,
   // exceptions should be caught and reported here.
   try
   {
      // Continue preparing the data:
      start_document();

      // On return, check if a login attempt failed:
      if (stype==STYPE_ESTABLISH)
      {
         // Use session_type STYPE_IDENTITY to prevent relaxed
         // STYPE_ESTABLISH processing after login attempt:
         sstatus = get_session_status(STYPE_IDENTITY, false);
         if (sstatus < SSTAT_AUTHORIZED)
            abandon_session_records();
      }
   }
   catch(schema_exception &se)
   {
      print_error_as_xml(m_out, se.what(), "writing document");
   }
}


/**
 * @brief Read session cookies and confirm session validity.
 *
 * The function attempts to read the session cookies.  Found session cookies
 * are verified.
 *
 * @param abandon_session Flag if a found session is to be abandoned.
 *
 * If a session is detected (by having session cookies), delete the session
 * if abandoning, otherwise check database to determine validity.  Set
 * the return value accordingly, and set the instance variable _m_session_id_
 * for a valid session.
 *
 * An assert() is made to ensure the database is properly selected before
 * running, or confirm_session() will not find the session record.
 *
 * @return The session status.  The function will return SSTAT_EXPIRED if
 *         the session is abandoned.  That return value will indicate that
 *         there were session cookies that must be deleted or replaced.
 */
Schema::SESSION_STATUS Schema::get_session_status(SESSION_TYPE stype,
                                                  bool abandon_session)
{
   // The appropriate database must be current:
   assert(m_database_confirmed);

   m_session_id = 0;
   SESSION_STATUS rval = SSTAT_NONE;

   get_session_cookies(
      [this, &stype, &abandon_session, &rval](uint32_t id, const char *hash)
      {
         if (!abandon_session && confirm_session(id,hash))
         {
            m_session_id = id;
            if (stype==STYPE_IDENTITY)
            {
               if (is_session_authorized(id))
                  rval = SSTAT_AUTHORIZED;
               else
                  rval = SSTAT_RUNNING;
            }
            else
               rval = SSTAT_RUNNING;
         }
         else
         {
            abandon_session_records(id, hash);
            rval = SSTAT_EXPIRED;
         }
      });

   return rval;
}

/**
 * @brief Clear session and jump to @p jump_destination or to /
 *
 * The function clears session information from the database and closes
 * the session.
 *
 * HTTP header lines are written to set the Status and Location values and
 * to clear the session cookies.
 *
 * abandon_session() always results in a jump, if not to a mode jump instruction
 * or jump_no_session, then to the directory index.
 */
void Schema::abandon_session(const char *jump_destination, bool clear_cookies)
{
   if (!jump_destination)
   {
      const Advisor_Index::info *handle;
      if ((handle=m_specsreader->seek_mode_info("jump_no_session")))
         jump_destination = handle->value();
      if (!jump_destination)
         jump_destination = "/";
   }

   // Write status and location header lines:
   print_Status_303();
   write_location_header(jump_destination);

   // Set session cookies to expire:
   if (clear_cookies)
      clear_session_cookies();
   
   write_headers_end();
}

void Schema::clear_quarantine_table(const char *tablename)
{
   auto fQuery = [this, &tablename](const char *query)
   {
      SimpleProcedure::build(&s_mysql,
                             query,
                             "s64",
                             Adhoc_Setter<1>(si_text(tablename)));

//      Adhoc_Setter<1>(si_text(tablename)).build(&s_mysql, query, nullptr);
   };

   const char *removal_proc = value_from_mode("removal-procedure");
   if (!removal_proc)
      removal_proc = "ssys_default_import_removal";

   SimpleProcedure::build_query_string(removal_proc, 1, fQuery);
}

/**
 * @brief Processes the file_upload multipart_form field.
 *
 * @return true if succeessful, false otherwise.
 */
bool Schema::import_table(const char* tablename)
{
   bool rval = false;
   Multipart_Pull::fhandle_payload pl(*m_puller);
   int fh = m_puller->get_csv_file_handle(pl);
   if (fh!=-1)
   {
      try
      {
         MySQL_LoadData mld(&s_mysql, fh, m_session_id, tablename);
         rval = mld.import();
      }
      catch(std::exception &e)
      {
         ifprintf(stderr,
                 "Unexpected import_table exception: (%s).\n",
                 e.what());
      }
      catch(...)
      {
         ifputs("Unexpected anonymous import_table exception.\n",
               stderr);
      }

      // Make sure to close the handle to end the pipe:
      close(fh);
   }

   return rval;
}


/**
 * @brief Imports a table and return `true` given proper preparation,
 *        otherwise returns `false`.
 *
 * This is the first implementation of multipart-form handling in SchemaFW.
 *
 * For this version, it only handles a single file upload, ignoring other
 * form fields.  In the current form, it could process several file upload
 * fields, except for the limited vocabulary of the specs file only defining
 * one table import.
 *
 * @todo Ideally, this function should construct a DataStack with the
 * non-file-upload fields for later consideration, which might provide
 * a means to transmit instructions beyond importing the single file.
 * Doing that, however, would require reorganizing the response sequence
 * so that this function calls the next step with the field info.
 */
bool Schema::process_import_form(void)
{
   bool rval = false;

   if (!m_puller)
      throw schema_exception("No form data from which to import a table.");

   const char *tablename = value_from_mode("target");
   if (!tablename)
      throw schema_exception("No target for import.");
   
   // Remove residual records from possible interrupted import:
   clear_quarantine_table(tablename);

   // Common code for catch blocks to clean up.
   auto cleanup = [this](void)
   {
      while (!m_puller->end_of_form())
         m_puller->next_field();
   };

   // Read read all fields
   while (!m_puller->end_of_form())
   {
      if (m_puller->next_field())
      {
         if (m_puller->is_file_upload())
         {
            try
            {
               rval = import_table(tablename);
            }
            catch(schema_exception &se)
            {
               cleanup();
               throw se;
            }
            catch(std::exception &e)
            {
               cleanup();
               throw e;
            }
            

            if (!rval)
            {
               ifputs("For mode \"", stderr);
               ifputs(m_mode->tag(), stderr);
//               m_qstringer->print_name_at(0,STDERR_FILENO);

               const char *ctype = m_puller->field_content_type();
               if (ctype)
               {
                  ifputs("\", import failed for upload type \"", stderr);
                  ifputs(ctype, stderr);
               }

               ifputs("\"\n", stderr);
            }
         }

         // If !m_puller->is_file_upload(), calling m_puller-><next_field()
         // will discard the unread characters of the non-file-upload field.
      }
   }
   
   return rval;
}

/**
 * @brief Get a pair of const char* representing the specs file and mode instructions.
 *
 * @tparam Func Should be a lambda function(const char *spec, const char *mode)
 *
 * The *mode value will not be null, it will be a pointer to a string, even if
 * the string length is 0, i.e. mode points at a '\0' character.  This is so that
 * the code that receives it only has to check to *mode=='\0', not also mode==nullptr.
 */
template <class Func>
void Schema::get_specs_instructions(const Func &cb)
{
   if (m_qstringer)
   {
      auto f = [&cb](const char *str)
      {
         const char *t = strchr(str,':');
         if (t)
         {
            int len = t - str;
            char *buff = static_cast<char*>(alloca(1+len));
            memcpy(buff, str, len);
            buff[len] = '\0';
            cb(buff, t+1);
         }
         else
            cb(str,"");
      };
      m_qstringer->get_name_at(0,f);
   }
   else
      cb("default.spec","");
}

/**
 * @brief Return session cookie values to a template callback function.
 *
 * @param cb Callback function with signature (uint32_t id, const char* hash)
 * @return void
 *
 * This function searches the cookie string for SessionId and SessionHash
 * values, sending them with the callback function if found.
 *
 * Following the SchemaFW convention, a `get_` function only invokes the
 * callback function if the session cookies are found.
 */
template <class Func>
void Schema::get_session_cookies(const Func &cb) const
{
   uint32_t int_id = s_invalid_session;
   char *buffer = nullptr;

   if (this->m_cstringer)
   {
      const BaseStringer &cs = *this->m_cstringer;
      int indexId = cs.get_index_of_name("SessionId");
      int indexHash = cs.get_index_of_name("SessionHash");

      if (indexId>=0 && indexHash>=0)
      {
         size_t bufflen = std::max(cs.value_len_at(indexId),
                                   cs.value_len_at(indexHash));

         // Get one buffer for both values.  Since we convert
         // the id string to an integer, we don't need it after
         // the conversion.
         buffer = static_cast<char*>(alloca(bufflen+1));

         // SessionId:
         cs.get_val_at(indexId, buffer, bufflen+1);
         int_id = uint_from_string(buffer);
         // SessionHash
         cs.get_val_at(indexHash, buffer, bufflen+1);
         buffer[bufflen] = '\0';
         
         cb(int_id, buffer);
      }
   }
}



/** @brief Searches all SpecsReader nodes for first non-global mode. */
long Schema::get_first_non_global_mode(void)
{
   for (auto ptr=m_specsreader->start(); ptr; ptr=ptr->next())
      if ('$'!=*(ptr->str()))
         return ptr->object();
   
   return -1;
}

/**
 * @brief Find the named context mode.
 *
 * @param mode_name     Name of the mode.  Zero-length string if not specified
 * @param keep_looking  Keep looking for a default mode if mode_name is not found.
 * @return
 *    -# file position of mode _mode_name_, if found,
v *    -# file position of a default mode if _mode_name_ is not specified, or
 *       if _mode_name_ __is__ specified but not found and _keep_looking_ is true.
 *    -# -1 if no mode was found.
 *
 * Returns the file position for a mode named _mode_name_.  A zero-length
 * _mode_name_ value will return the default mode.  The default mode will be the
 * first found from:
 * -# The mode whose name matches the value of global instruction $default-mode.
 * -# The first normal mode in the specs file.
 *
 * This function returns and does not stay on the stack.
 */
long Schema::get_request_mode_position(const char *mode_name, bool keep_looking)
{
   // The source of mode guarantees a non-null value.  A zero-length
   // value (*mode_name=='\0') indicates no mode was indicated in the query string.
   assert(mode_name);

   long pos_mode = -1;

   if (*mode_name && !keep_looking)
      pos_mode = m_specsreader->get_mode_position(mode_name);
   else
   {
      // Try with specified value (should have come from the query string):
      pos_mode = m_specsreader->get_mode_position(mode_name);
      
      // If not found in query string, look for global $default-mode
      if (pos_mode<0 && keep_looking)
      {
         const Advisor_Index::info *info;
         info =m_specsreader->seek_mode_info("$default-mode");
         if (info && info->is_setting())
         {
            if ((info=m_specsreader->seek_mode_info(info->value())))
               pos_mode = info->position();
         }
      }
   
      // If not in query string, and no $default-mode, get first non-global mode:
      if (pos_mode<0)
         pos_mode = get_first_non_global_mode();
   }

   return pos_mode;
}


bool Schema::s_headers_done = false;

// Set starting value in case called on command line:
bool Schema::s_sfw_xhrequest = assign_sfw_xhrequest_flag();


void Schema::set_forbidden_header(void)
{
   assert(!s_headers_done);
   ifputs("Status: 403 Forbidden\n", s_header_out);
}

/**
 * @brief Convenience function to output Set-Cookie header strings.
 *
 * @param name Name of cookie
 * @param value Value of cookie.  Can be NULL to delete the cookie.
 * @param seconds_to_expire Number of seconds to delete the cookie. Using 0,
 * the default value, create a Set-Cookie string with no expiration.
 *
 * If value is NULL, then the expiration value will be negative and a bogus
 * value will be set ("delete").
 */
void Schema::set_cookie(const char *name,
                        const BindC *value,
                        int seconds_to_expire) const
{
   if (s_headers_done)
   {
      ifputs("set_cookie attempt after headers done: ", stderr);
      ifputs(name, stderr);
      if (value)
      {
         ifputs(" : ", stderr);
         value->print(stderr);
      }
      else
         ifputs(" : (no value)", stderr);
   }
   
   assert(!s_headers_done);
   
   ifputs("Set-Cookie: ", s_header_out);
   ifputs(name, s_header_out);
   ifputc('=', s_header_out);

   if (value)
   {
      value->print(s_header_out);
      if (seconds_to_expire)
      {
         ifputs("; Max-Age=", s_header_out);
         print_int(seconds_to_expire, s_header_out);
      }
   }
   else
      ifputs("deleted; Max-Age=-1", s_header_out);

   ifputc('\n', s_header_out);
}

void Schema::write_location_header(const char *url) const
{
   assert(!s_headers_done);
   
   ifputs("Location: ", s_header_out);
   ifputs(url, s_header_out);
   ifputc('\n', s_header_out);
}

/**
 * @brief Adds a Refresh header if a jump directive is found in the mode.
 *
 * @param seconds Number of seconds for HTTP_REFRESH delay.
 * @return `true` if jumping, `false` if not.
 *
 * For session_submit mode types whose procedure does not return a resultset,
 * the `jump` directive provides the destination where the application will
 * begin.
 *
 * This function is called after session processing is complete.  The session
 * processing may have already diverted the user (if a `jump_no_authorized` value
 * was set), so the `jump` destination should be where a logged-in user begins
 * your application.
 */
void Schema::refresh_to_mode_jump_destination(int seconds)
{
   const char *jump = value_from_mode("jump");
   if (jump)
      write_location_header(jump);
}

/**
 * @brief Sets HTTP_REFRESH to URL at jump_not_authorized if the value is set.
 *
 * @param seconds Number of seconds to pause before refresh-jump to the new page.
 * @return `true` if jumping, `false` if not.
 */
bool Schema::refresh_to_not_authorized_destination(int seconds)
{
   const char *url = value_from_mode_or_global("jump_not_authorized");
   if (url)
   {
      write_location_header(url);
      return true;
   }
   else
      return false;
}

/**
 * @brief Sets HTTP_REFRESH to jump_no_session URL if found.
 *
 * @return `true` if jumping, `false` if not.
 */
bool Schema::refresh_to_no_session_destination(int seconds)
{
   const char *url = value_from_mode_or_global("jump_no_session");
   if (url)
   {
      write_location_header(url);
      return true;
   }
   else
      return false;
}



/**
 * @brief Set the current database, clear connection session variables.
 *
 * This function is called from install_response_mode() as an early necessary
 * step of preparing the environment for processing the request.
 *
 * Note that the function does not fail for a missing _database_ instruction.
 * This should be allowed in a server where there is only one database.  It only
 * fails if a requested database is not available.
 *
 * It should be called only once, or the prepared environment may be corrupted,
 * with abandoned session settings possibly left exposed.  Thus, the function
 * uses assert() to ensure one trip through here, using the s_headers_done
 * variable as a flag.
 *
 * If a design calls for accessing another database, it should be done in
 * stored procedures with database-specified queries and calls.
 *
 * Sets the instance variable m_database_confirmed, which will be checked
 * in get_session_status().
 */
void Schema::set_requested_database(void)
{
   int result = 0;

   // Assert only-once call per request:
   assert(!s_headers_done);

   const char *dbname = value_from_mode_or_global("database");
   if (dbname)
      result = mysql_select_db(&s_mysql, dbname);

   if (result)
      set_failure_message("No database specified", "set_requested_database");
   else
   {
      m_database_confirmed = true;
   }
}

/** @brief Searches and prints if found, an xml-stylesheet line. */
void Schema::add_stylesheet_pi(void)
{
   const char *href = value_from_mode_or_global("xml-stylesheet");
   if (href)
   {
      ifputs("<?xml-stylesheet type=\"text/xsl\" href=\"", m_out);
      ifputs(href, m_out);
      ifputs("\" ?>\n", m_out);
   }
}

/**
 * @brief Begin processing the document.
 *
 * This function is called after install_response_mode() processes
 * the session stuff (perhaps it should be renamed).
 */
void Schema::start_document(void)
{
   const ab_handle *root = m_mode->seek("root");

   // all paths through code lead here to continue processing the request:
   auto f = [this, &root](const ab_handle *global_root)
   {
      // continue with global_root null or not:
      process_root_branch(root, global_root);
   };

   if (root)
   {
      const char *name;
      // If referencing a named global root...
      if (root->is_setting() && *(name=root->value())=='$')
      {
         // ...and there is a global root with the indicated name...
         // skip past the '$'
         ++name;
         auto *info = m_specsreader->seek_mode_info("$root", name);
         if (info)
            // ...get the name root
            m_specsreader->build_branch(info->position(), f);
         else
            f(nullptr);
      }
      else
         f(nullptr);
   }
   else
      m_specsreader->seek_mode("$root", f);
}

/**
 * @brief Called by start_document() once modes are found.
 *
 * This functions calls functions in the @ref DocHead_Subprint group to
 * begin the document.  It prints processing instructions (xml-stylesheet)
 * and the document element before calling open_info_procedure to
 * continue processing the request.
 *
 * The request processing continues when we call open_info_procedure in
 * the @ref StoredProc_Section.
 *
 * @param mode_root   The root instruction in the current mode
 * @param global_root A mode (left-most column commencing tag) named _root_,
 *                    or NULL if it doesn't exist or the calling function
 *                    doesn't want to use the global root.
 * @param cb          Callback function that replaces the default
 *                    open_info_procedure() for outputing the content.
 */
void Schema::process_root_branch(const ab_handle *mode_root,
                                 const ab_handle *global_root,
                                 bool import_confirm)
{
   const char *tagname = import_confirm ? "import_confirm" : "resultset";

   auto f = [this, &mode_root, &global_root, &import_confirm, &tagname](const ab_handle *tag)
   {
      if (tag && tag->is_setting())
         tagname = tag->value();

      print_document_element(tagname, mode_root, global_root);

      // print the adhoc stuff:
      m_specsreader->print_sub_elements(m_out, m_mode, arr_root_reserved);

      // print_env(false);

      const char *schema_proc_name = m_mode->seek_value("schema-proc");
      if (schema_proc_name)
         print_schema_from_procedure_name(schema_proc_name);

      if (import_confirm)
         print_import_confirm();
      else if (m_mode_action==MACTION_FORM_IMPORT)
      {
         Schema_Printer sprinter(*m_specsreader, *m_mode, m_out);
         sprinter.print(m_mode->seek("schema"), "form");
      }
      else
         open_info_procedure();

      // print close tag of document element:
      ifputs("</", m_out);
      ifputs(tagname, m_out);
      ifputs(">\n", m_out);
   };

   // Don't pass the global_root in case it tries
   // to change the tag.  At least, the import_confirm
   // document tag name must be different from the
   // standard in order to distinguish it.
   seek_specs_handle("tag", f, mode_root, (import_confirm?nullptr:global_root));
}

/**
 * @brief Call the query to print out the transitional import data.
 */

void Schema::print_import_confirm(void)
{
   const char *tablename = value_from_mode("target");
   
   // This must be available or we couldn't get here:
   assert(tablename);

   // Adhoc_Setter argument 2, limit number of rows (0 means no limit):
   int limit_rows = 0;

   // Done with Adhoc_Setter construction parameters

   // Adhoc_Setter::build() argument 1 is &s_mysql

   // Adhoc_Setter::build() argument 2 is the confirm procedure name
   const char *procname = value_from_mode("confirm_procedure");
   if (!procname)
      procname = s_default_import_confirm_proc;
   
   // Adhoc_Setter::build() argument 3 is the result user:
   Result_As_SchemaDoc user(*m_specsreader, m_mode, m_mode_action, m_out);
   
   // Construct and call procedure:
   Adhoc_Setter<2>(si_text(tablename), ri_long(limit_rows)).build(&s_mysql, procname, user);
}

/**
 * @brief Print attributes from the first row of a procedure call.
 */
void Schema::print_root_attributes_from_procedure(FILE *out, const char *procname)
{
      auto fresult = [&out](int result_number, DataStack<BindC> &ds)
         {
            if (result_number==1)
            {
               for (auto *col=ds.start(); col; col=col->next())
               {
                  BindC &obj = col->object();
                  if (!obj.is_null())
                  {
                     ifputc(' ', out);
                     ifputs(col->str(), out);
                     ifputs("=\"", out);

                     if (obj.is_string_type())
                        obj.print_xml_escaped(out);
                     else
                        obj.print(out);

                     ifputc('\"', out);
                  }
               }
            }
         };

      auto fquery = [this, &fresult](const char *query)
         {
            Result_User_Row<decltype(fresult)> user(fresult);

            SimpleProcedure proc(query);
            proc.run(&s_mysql, user);
         };

      SimpleProcedure::build_query_string(procname,0, fquery);
}
   
/**
 * @brief Print the document element with attributes.
 *
 * This function takes the mode_root and global_root handles
 * in order to print attributes contained therein.
 *
 * Adds a _post="true"_ attribute if using a POST request method.
 */
void Schema::print_document_element(const char *tagname,
                                    const ab_handle *mode_root,
                                    const ab_handle *global_root)
{
   ifputc('<', m_out);
   ifputs(tagname, m_out);

   if (is_post_request())
      print_xml_attribute(m_out, "post", "true");

   if (m_type_value)
      print_xml_attribute(m_out, "mode-type", m_type_value);

   print_adhoc_attributes(m_out, m_mode, arr_root_reserved);

   if (m_meta_jump)
      print_xml_attribute(m_out, "meta-jump", m_meta_jump);
   
//   print_field_attributes(m_out, mode_root, global_root);

   auto saved_reporter = SimpleProcedure::s_message_reporter;
   SimpleProcedure::s_message_reporter = procedure_message_thrower;

   // Print attributes from any root-procedure lines found in the mode:
   try
   {
      SiblingWalker sw(m_mode->first_child());
      while (sw)
      {
         if (sw.is_tag("root-procedure"))
            print_root_attributes_from_procedure(m_out, sw.value());
         ++sw;
      }
      ifputs(">\n", m_out);
   }
   catch(const ProcMessageException &e)
   {
      print_xml_attribute(m_out, "root_procedure_error", e.msg());
      
      ifputs(">\n", m_out);
      ifprintf(stderr,
              "Exception \"%s\" during print_root_attributes_from_procedure.\n",
              e.msg());
   }
   catch(const std::runtime_error &e)
   {
      ifputs(">\n", m_out);
      ifprintf(stderr, "Main exception (runtime_error): %s", e.what());
      
      print_message_as_xml(m_out, "error", e.what(), "runtime_error");

      // Restore previous reporter before leaving this context:
      SimpleProcedure::s_message_reporter = saved_reporter;
      
      // re-throw same exception
      throw;
   }
   catch(const std::exception &e)
   {
      ifputs(">\n", m_out);
      print_message_as_xml(m_out, "error", e.what(), "generic exception");
      ifprintf(stderr, "Main exception (generic exception), %s", e.what());

      // Restore previous reporter before leaving this context:
      SimpleProcedure::s_message_reporter = saved_reporter;
      
      // re-throw same exception
      throw;
   }
   
   SimpleProcedure::s_message_reporter = saved_reporter;

}

/**
 * @brief Find stored procedure name, then build a StoredProc and pass it on.
v *
 * The instructions for root are no longer needed at this step of handling
 * the request (so no parameters for run_procedure).  The two steps in this
 * function build a StoredProc object on the stack, which is passed on to
 * process_info_procedure for the next step.
 *
 * @todo Should this function handle a situation without a stored procedure?
 * Someone might want to make an adhoc form or something.  Maybe not.
 */
void Schema::open_info_procedure(proc_runner pr)
{
   // The default proc_runner function pointer
   // is Schema::process_info_procedure
   auto fStoredProc = [this, &pr](StoredProc &sp)
   {
      (this->*pr)(sp);
   };
   Generic_User<StoredProc, decltype(fStoredProc)> spu(fStoredProc);

   const char *procname = value_from_mode("procedure");
   if (procname)
      StoredProc::build(&s_mysql, procname, spu);
   else if (m_mode_action!=MACTION_FORM_NEW)
      print_message_as_xml(m_out,
                           "warning",
                           "Missing procedure instruction",
                           m_mode->tag());
}


/**
 * @brief Runs query to build a schema from parameters of a procedure.
 *
 * This function is the entry point for the task of printing a schema
 * from a _schema-proc_ response mode instruction.  It builds a StoredProc
 * object, but only uses its BindStack information for the schema before
 * abandoning it unrun.
 *
 * This function calls Schema::resolve_enum_set_references() to look for
 * references to table fields for ENUM or SET data types, and finally,
 * resolve_enum_set_references() calls print_schema_from_bindstack()
 * to generate the schema.
 */
void Schema::print_schema_from_procedure_name(const char *name)
{
   auto f_storedProc = [this](StoredProc &schemaproc)
   {
      BindStack *bs = schemaproc.bindstack();
      if (bs)
      {
         resolve_enum_set_references(bs);
      }
      else
      {
         ifputs("Procedure '", stderr);
         ifputs(schemaproc.procname(), stderr);
         ifputs("' has no parameters from which to print a schema.\n", stderr);
      }
   };
   Generic_User<StoredProc, decltype(f_storedProc)> spu(f_storedProc);

   StoredProc::build(&s_mysql, name, spu);
}

/**
 * @brief Check if any enum or set DTD_IDENTIFIERs are needed and get 'em if so.
 */
void Schema::resolve_enum_set_references(BindStack *bs_schema)
{
   // Closure variables for lambda functions
   int ecount = count_enum_set_references(m_mode);
   size_t param_string_length = 0;
   refnode* dtd_refs = nullptr;
   SimpleProcedure *psp = nullptr;

   // If no enum or set references, 
   if (ecount==0)
   {
      print_schema_from_bindstack(bs_schema);
      return;
   }

   // 5. Process in pre_fetch so the function won't return before calling
   //    print_schema_from_bindstack().
   auto cb_ruser_pre = [this, &dtd_refs, &psp, &bs_schema](int result_number,
                                                           DataStack<BindC> &ds,
                                                           SimpleProcedure &proc)
   {
      if (result_number!=1)
         return;

      // Get each row
      while (proc.fetch(ds))
      {
         const refnode *ref = dtd_refs->find_tablefield(ds.object(0));
         if (ref)
         {
            const char *name = ref->field->value();
            BindC *b = bs_schema->object_ptr(name);
            if (b)
            {
               BindC& bcList = ds.object(1);
               size_t len = bcList.data_length();
               char *buff = static_cast<char*>(alloca(len+1));
               memcpy(buff, bcList.m_data, len);
               buff[len] = '\0';

               b->m_dtdid = buff;
            }
         }
      }

      print_schema_from_bindstack(bs_schema);

   };
   Result_User_Pre_Fetch<decltype(cb_ruser_pre)> result_user_pre(cb_ruser_pre);

   // 4. Used for virtual IParam_Setter function for procedure called in step 3.
   auto cb_psetter = [&dtd_refs, &param_string_length](DataStack<BindC> &ds)
   {
      BindC &p = ds.object(0);
      
      char *buff = static_cast<char*>(p.m_data);
      set_enum_set_list_string(buff, param_string_length, dtd_refs);
      p.data_length(param_string_length);
      p.is_null(false);
      
      return true;
   };
   TParam_Setter<decltype(cb_psetter)> param_setter(cb_psetter);

   // 3. Run the ssys_get_column_dtds procedure after input binds are built:
   auto cb_run = [this, &psp, &param_setter, &result_user_pre, &bs_schema](BindStack &bs)
   {
      SimpleProcedure sp("CALL ssys_get_column_dtds(?)", &bs);
      // Set closure variable so the result_user can access the SimpleProcedure:
      psp = &sp;
      
      sp.run(&s_mysql, &param_setter, &result_user_pre);
   };
   
   // 1. Setup closure variable for lambda functions above
   size_t lenrefs = (ecount+1)*sizeof(refnode);
   dtd_refs = static_cast<Schema::refnode*>(alloca(lenrefs));
   memset(dtd_refs, 0, lenrefs);
   get_enum_set_references(m_mode, dtd_refs, ecount);
   param_string_length = get_string_length_enum_set_list(dtd_refs);

   // 2. Make typestr for procedure parameter to build the BindStack:
   char tstr[6];
   sprintf(tstr,"s%lu", param_string_length);
   BindStack::build(tstr,cb_run);
}

/**
 * @brief Final step of routine that begins with print_schema_from_procedure_name().
 */
void Schema::print_schema_from_bindstack(BindStack *bs)
{
   Schema_Printer sprinter(*m_specsreader, *m_mode, *bs, m_out);
   sprinter.print(m_mode->seek("schema"), "form");
}

/**
 * @brief With all necessities at hand, write the document contents.
 *
 * @todo The schema printer should check for alternate name
 */
void Schema::process_info_procedure(StoredProc &infoproc)
{
   bool is_form = is_form_type(m_mode_action);
   bool is_empty_form = m_mode_action==MACTION_FORM_NEW;
   bool force_schema = is_empty_form || m_mode_action==MACTION_FORM_RESULT;

   BindStack *bindstack = infoproc.bindstack();

   // There is no form without parameters.  Log a warning and include
   // a warning node in the results if a form type has no bindstack
   // (no parameters).
   if (is_form && !bindstack)
   {
      char *buff = static_cast<char*>(alloca(200));
      snprintf(buff, 200,
               "Attempting to use parameter-less procedure %s for a form.",
               infoproc.procname());
      
      ifputs(buff, stderr);
      write_error_element(m_out, "warning", buff);
   }

   // The existence of a schema-proc instruction determines what comes next
   const char *schema_proc_name = m_mode->seek_value("schema-proc");

   // If there was not schema-proc, we may need to print schema:
   if (!schema_proc_name)
   {
      // Print a form-producing schema for an empty form or a result form:
      if (bindstack && force_schema)
      {
         Schema_Printer sprinter(*m_specsreader, *m_mode, *bindstack, m_out);
         sprinter.print(m_mode->seek("schema"), "form");
      }
   }

   // We use the infoproc to print a schema if we're in a form-new response mode,
   // UNLESS we've already printed the schema as a result of having a schema_proc_name,
   // in which case we must assume that the procedure instruction is intended to
   // return data to help build the form.
   if (!is_empty_form || schema_proc_name)
      run_procedure(infoproc);
}

/**
 * @brief Uses results of a previous procedure to fill the parameters of
 * the indicated procedure.
 *
 * This function runs a procedure, using the Schema's IParam_Setter interface to
 * fill the parameters.  The IParam_Setter implementation uses the results of
 * StoredProcedure::build in open_info_procedure.
 */
void Schema::run_procedure(StoredProc &infoproc)
{
   SimpleProcedure proc(infoproc.querystr(), infoproc.bindstack());
   Result_As_SchemaDoc user(*m_specsreader, m_mode, m_mode_action, m_out);
   proc.run(&s_mysql, this, &user);
}

/**
 * @brief Apply schema-defined modifications to the BindStack.
 *
 * @todo Not done yet.  Mainly, this is to set a BindC flag if
 * this value should be printed as a child element, with attribute-mode
 * being the default mode and is not included if default.
 *
 * @todo The above todo assumes that a suitable flag is added to BindC.
 */
void Schema::modify_schema_binds(BindStack *bs, const ab_handle *schema)
{
   
}

/**
 * @brief Returns the number of fields that have an enum or set table reference.
 *
 * Allocate a const char** array one longer than the return value of this function
 * to terminate the array with a nullptr element.  The nullptr terminating element
 * will signal the end of the list for functions that use the array.
 */
int Schema::count_enum_set_references(const ab_handle *mode)
{
   int count = 0;
   const ab_handle *schema = mode->seek("schema");
   if (schema)
   {
      SiblingWalker sw(schema->first_child());
      while (sw)
      {
         if (sw=="field")
         {
            SiblingWalker fieldprop(sw.child());
            while (fieldprop)
            {
               if (fieldprop=="enum" || fieldprop=="set")
                  ++count;

               ++fieldprop;
            }
         }
         ++sw;
      }
   }
   return count;
}

/**
 * @brief Fills array with pointer to node values of enum and set instructions.
 *
 * @param mode The Schema::m_mode current running mode.
 * @param refs An array of const char*, terminated with a final nullptr element.
 * @param refcount Number of elements in @p refs, not counting final nullptr element.
 */
void Schema::get_enum_set_references(const ab_handle *mode,
                                     refnode *refs,
                                     int refcount)
{
   int index = 0;
   const ab_handle *schema = mode->seek("schema");
   if (schema)
   {
      SiblingWalker sw(schema->first_child());
      while (sw)
      {
         if (sw=="field")
         {
            SiblingWalker fieldprop(sw.child());
            while (fieldprop)
            {
               if (index<refcount && (fieldprop=="enum" || fieldprop=="set"))
               {
                  refs[index].set(sw,fieldprop);
                  ++index;
               }

               ++fieldprop;
            }
         }
         ++sw;
      }
   }
}

/**
 * @brief Calculates the string length of a comma-separated list of refs.
 *
 * The return value does not include space for a terminating '\0' because
 * the value is meant to be installed as a prepared statement parameter
 * where the terminating '\0' should not be counted in the parameter
 * settings.
 *
 * @param refs A null-terminated array of pointers to char strings
 * @return Number of characters in concatenated string with comma separators.
 */
size_t Schema::get_string_length_enum_set_list(refnode* refs)
{
   size_t len = 0;
   refnode *el = refs;
   while (*el)
   {
      // Add space for comma before elements after the first:
      if (len)
         len++;
      len += strlen(el->ref->value());
      
      ++el;
   }

   return len;
}

/**
 * @brief Copies the refs, with separating commas, to the prepared buffer.
 *
 * Using a char buffer allocated according to the advice of
 * get_string_length_set_list(), copy the contents of @p refs to the buffer
 * with commas separating the refs.
 *
 * A terminating '\0' will be added only if room is left after copying the
 * contents and commas.  The '\0' is not necessary if being used for a parameter,
 * but is a convenience if the string is to be used elsewhere.
 *
 * @param buff Character buffer target for concatenated string.
 * @param bufflen Bytes length of @p buff
 * @param refs Null-terminated array of pointers to char*
 */
void Schema::set_enum_set_list_string(char *buff,
                                      size_t bufflen,
                                      refnode* refs)
{
   char *ptr = buff;
   char *limit = buff + bufflen;
   refnode* el = refs;
   
   const char *str;
   size_t len;
   
   while (*el)
   {
      // Add comma before elements after the first:
      if (ptr>buff)
         *++ptr = ',';

      str = el->ref->value();
      len = strlen(str);

      // test predicted overrun:
      if (ptr+len > limit)
         break;

      memcpy(ptr, str, len);
      ptr += len;
      ++el;
   }

   // Add terminating '\0' if room left for it:
   if (ptr < limit)
      *ptr = '\0';
}

void Schema::set_session_string_if_necessary(DataStack<BindC> &ds) const
{
   auto *session = m_mode->seek("session");
   if (session && session->is_value("start"))
   {
      const char *name = value_from_mode("session_field");
      if (!name)
         name = "session_string";

      int bindex = ds.index_by_name(name);
      if (bindex>=0)
      {
         t_handle<BindC> &handle = ds.handle(bindex);
         BindC &b = handle.object();
         
         char *buff = static_cast<char*>(b.m_data);
         unsigned long copylen = b.buffer_length();

         make_session_string(buff, copylen);
         b.data_length(copylen);
      }
   }
}

/**
 * @brief Define pure-virtual function of IParam_Setter interface.
 *
 * Terminates early if there are no parameters (and it should probably
 * never get here in that case).
 *
 * The parameters are set from the stream as the first priority, then
 * the function will scan the query string for additional parameter
 * values, only looking for query string values for parameters that
 * have not already been set by m_setter.
 */
bool Schema::set_binds(DataStack<BindC> &ds) const
{
   // Early termination if no parameters to set:
   if (ds.count()==0)
      return true;
   
   bool carryon = true;

   set_session_string_if_necessary(ds);

   if (m_setter)
      carryon = m_setter->set_binds(ds);

   if (carryon && m_qstringer)
   {
      // Setup receiver of QStringer values.  The variable "bindc" is a
      // closure variable that is used by the lambda function after
      // having been set by the for loop below.
      BindC *bindc = nullptr;
      auto f = [&bindc](const char *value)
      {
         bindc->set_from(value);
      };
      Generic_User_Const_Pointer<char,decltype(f)> user(f);

      auto *ptr=ds.start();
      bindc = &(ptr->object());
      
      // Set the first parameter with the anonymous value, if found:
      if (m_qstringer->anonymous_first_value())
      {
         if (bindc->is_null())
            m_qstringer->t_get_value(0, user);
         ptr = ptr->next();
      }

      // Continue getting parameters, ptr will be either the first parameter
      // if there was no anonymous value, or the second parameter (if found)
      // if an anonymous value was used to set the first.
      for (; ptr; ptr=ptr->next())
      {
         bindc = &(ptr->object());
         if (bindc->is_null())
            m_qstringer->t_get_value(ptr->str(), user);
      }
   }

   return carryon;
}


/**
 * @brief Debugging MySQL Login
 */
void show_login_info(MYSQL &mysql)
{
   ifprintf(stdout,"<mysql host=\"%s\" user=\"%s\" password=\"%s\" />\n",
            mysql.host,
            mysql.user,
            mysql.passwd);
}

/**
 * @brief Global value used by procedure_message_reporter and main() for
 * output.
 *
 * Initially, this program writes only to stdout for CGI/FASTCGI, and g_schema_output
 * uses stdout for its initial value.  However, at some point it may be useful to
 * write the output to a file, and setting this value may be the means this is done.
 * I say "may" because it's not fully thought out, but rather put here as an idea
 * that may be useful if the need ever presents itself.
 */
FILE *g_schema_output = stdout;

/**
 * @brief Implementation of message_reporter (a function pointer type) for Schema
 * to replace the default SimpleProcedure version.
 *
 * Prints an XML version of the standard message.  It uses the FILE* set in
 * global g_schema_output variable for flexibility in where the message is written.
 *
 * @param type  Type of message.  See @ref Schema::print_message_as_xml()
 *              for an explanation.
 * @param msg   Text of message
 * @param where Where the error occurred.  Maps to the _detail_ parameter
 *              of Schema::print_message_as_xml()
 */
void Schema::procedure_message_reporter(const char *type,
                                        const char *msg,
                                        const char *where)
{
   if (!s_headers_done)
   {
      print_XML_ContentType();
      headers_conclude(s_header_out);
   }

   if (!(strcmp(type,"signal") || strcmp(type,"notice")))
      ifprintf(stderr, "Type %s: %s.\n", type, msg);

   print_message_as_xml(g_schema_output, type, msg, where);
}

/**
 * @brief Alternate message_reporter function that throws an exception
 *        instead of printing an error.
 *
 * The purpose of this function is to allow appropriate error-reporting
 * while processing an open tag.  In that situation, the error cannot be
 * printed without corrupting the element, so this throwing error reporter
 * gives the calling function an opportunity to close the open tag
 * before printing the error.
 */
void Schema::procedure_message_thrower(const char *type,
                                       const char *msg,
                                       const char *where)
{
   throw ProcMessageException(type,msg,where);
}

int run_schema(void)
{
   // Use existence of QUERY_STRING environment variable as
   // an indicator of the mode in which we're running:
//   const char *qs = getenv("QUERY_STRING");
   int rval = 1;

   try
   {
//      rval = Schema::wait_for_requests(qs?cgi_sentry:cli_sentry, g_schema_output);
      rval = Schema::wait_for_requests(cgi_sentry, g_schema_output);
   }
   catch(const std::runtime_error &e)
   {
      ifprintf(stderr, "Main exception (runtime_error): %s", e.what());
      ifprintf(stdout, "<exception>%s</exception>\n", e.what());
      throw e;
   }
   catch(const std::exception &e)
   {
      ifprintf(stderr, "Main exception (generic exception), %s", e.what());
      ifprintf(stdout, "<exception type=\"unknown\" />\n");
   }

   return rval;
}

extern char __BUILD_DATE;
extern char __VERSION_MAJOR;
extern char __VERSION_MINOR;
extern char __VERSION_PATCH;

int show_version(void)
{
   ifprintf(stdout, "Version : %lu.%02lu.%04lu\n",
            (unsigned long) &__VERSION_MAJOR,
            (unsigned long) &__VERSION_MINOR,
            (unsigned long) &__VERSION_PATCH);
   return 0;
}

static const char g_ctype[] = "content-type: ";
static const int g_len_ctype = strlen(g_ctype);
static const char g_ct_multipartform[] = "multipart/form-data; ";
static const int g_len_ct_multipartform = strlen(g_ct_multipartform);
static const char g_ct_boundary[] = "boundary=";
static const int g_len_ct_boundary = strlen(g_ct_boundary);
static const char g_ct_dispo[] = "content-disposition: ";
static const int g_len_ct_dispo = strlen(g_ct_dispo);

#pragma push_macro("FILE")
#undef FILE
void initialize_fake_stdin(int fh)
{
   char buff[1024];
   char *end;
   char *ptr;
   size_t bread = 0;
   size_t offset = 0;
   int lines = 0;
   int lines_to_read = 2;
   bool found_boundary = false;

   auto f_fillbuff = [&fh, &buff, &bread, &end, &ptr](void)
   {
      bread = read(fh, buff, sizeof(buff));
      end = static_cast<char*>(buff) + bread;
      ptr = buff;
      return bread > 0;
   };

   while (f_fillbuff())
   {
      if (!found_boundary)
      {
         if (0==strncasecmp(g_ctype, ptr, g_len_ctype))
         {
            ptr += g_len_ctype;
            if (0==strncasecmp(g_ct_multipartform, ptr, g_len_ct_multipartform))
            {
               ptr += g_len_ct_multipartform;
               if (0==strncmp(g_ct_boundary, ptr, g_len_ct_boundary))
               {
                  found_boundary = true;
                  ptr += g_len_ct_boundary;
               }
            }
         }

         // If no complete match of multipart/form preamble, it's not multipart/form.
         // The code will ignore progress because we haven't yet collected three lines.
         if (ptr - buff < g_len_ctype + g_len_ct_multipartform + g_len_ct_boundary)
            break;
      }

      // Consume remainder of three lines
      while (ptr < end && lines < lines_to_read)
      {
         if (*ptr=='\r')
         {
            if (lines==0 && found_boundary)
            {
               // Replace "Content-type: " with "CONTENT_TYPE=" to match
               // what CGI provides.  Shift right one character to eliminate
               // the extra space (ie ": " becomes "=").
               char *pplus1 = buff + g_len_ctype;
               
               *ptr = '\0';
               setenv("CONTENT_TYPE", pplus1, 1);
               *ptr = '\r';
            }
            
            if (*++ptr=='\n')
               ++lines;
         }
         ++ptr;
      }

      if (lines>=lines_to_read)
         break;
      // Highly unlikely, but if we can't find the three lines in
      // the buffer, try to get more characters:
      else if (static_cast<size_t>(ptr-buff)==bread)
         offset += sizeof(bread);
   }

   if (lines==lines_to_read)
   {
      assert(0==strncasecmp(g_ct_dispo, ptr, g_len_ct_dispo));
      offset += ptr - buff;
   }
   else
      offset = 0;

   lseek(fh, offset, SEEK_SET);

   // // test that file pointer is correct:
   // f_fillbuff();
   // char cdbuff[] = "content-disposition";
   // if (strncasecmp(buff,cdbuff, strlen(cdbuff)))
   //    ifputs("File not pointing properly!\n", stderr);
   // lseek(fh, offset, SEEK_SET);
}
#pragma pop_macro("FILE")

/**
 * @brief More complicated command-line processing code isolated from main()
 *
 * This function processes command line parameters to run a response mode.
 * It is useful for debugging the XML output of a response mode, for debugging
 * stylesheets (if you save the XML output to a file or pipe to xsltproc), or
 * for using `gdbtui --args` to debug the C/C++ code.
 */
int Schema::process_command_line(int argc, char **argv)
{
   char batch_type = 'p';
   int  batch_value_count = 0;
   const char *bash_specs_file = nullptr;
   const char *bash_mode_name = nullptr;
   const char *bash_input_file = nullptr;

   char **ptr = argv;
   for (++ptr; *ptr; ++ptr)
   {
      // Consider the 2-character parameters
      if ((*ptr)[2]=='\0')
      {
         // Convert the first two characters to a unsigned integer for switch/case
         uint16_t apre = *reinterpret_cast<const uint16_t*>(*ptr);
         switch(apre)
         {
            case dash_val('b'):
               // Ignore, batch mode is inferred for any command line parameters
               break;

            case dash_val('p'):
               batch_type = 'p';
               break;

            case dash_val('g'):
               batch_type = 'g';
               break;

            case dash_val('i'):
               bash_input_file = *++ptr;
               break;

            case dash_val('s'):
               bash_specs_file = *++ptr;
               setenv("PATH_TRANSLATED", bash_specs_file, true);
               break;

            case dash_val('m'):
               bash_mode_name = *++ptr;
               setenv("QUERY_STRING", bash_mode_name, true);
               break;
               
            case dash_val('v'):
               ++batch_value_count;
               // Skip next arg to defer processing values until the second step;
               ++ptr;
               break;

            case dash_val('d'):
               if (!Schema::set_debug_action_mode(*++ptr))
                  return 1;
               break;
         }
      }
      else if (0==strcmp("--version", *ptr))
      {
         reset_fcgi_streams();
         return show_version();
      }
   }

   // Prepare FCGI streams to interact with the console:
   reset_fcgi_streams();

   // Make sure headers go to stderr in order to produce a clean XML file:
   Schema::set_header_out(stderr);
   
   // I may want to redo this with a query string built from
   // the batch_values instead of forcing a POST.

   // Set environment variable, considering overriding conditions
   if (batch_type=='p' || (batch_value_count || bash_input_file))
      setenv("REQUEST_METHOD", "POST", 1);
   else
      setenv("REQUEST_METHOD", "GET", 1);
   
   if (bash_mode_name && *bash_mode_name)
      setenv("QUERY_STRING", bash_mode_name, 1);

   int fh = -1;
   int saved_stdin = -1;
   
   // Keep this structure out of "else if" code block to avoid
   // deinitializing it when the code block context ends.
   Schema::pipe_cl_struct pcs;

   // Prepare stdin for schema
   if (bash_input_file && *bash_input_file)
   {
      fh = open(bash_input_file, O_RDONLY);
      if (fh)
         initialize_fake_stdin(fh);
   }
   // If any values, create a fake stdin for Schema to use:
   else if (true || batch_value_count)
      fh = Schema::get_fake_stdin_from_command_args(argc, argv, pcs);

   if (fh!=-1)
   {
      saved_stdin = dup(STDIN_FILENO);
      dup2(fh, STDIN_FILENO);
   }


   // Silence the compiler warnings until I decide what to do with this.
   if (batch_type)
      batch_type = 'p';

   if (start_mysql())
   {
      try
      {
         Schema::get_resources_from_environment(stdout);
      }
      catch(const schema_exception &se)
      {
         Schema::print_error_as_xml(stdout, se.what(), "install_response_mode");
      }
      
      mysql_close(&s_mysql);
   }

   mysql_library_end();

   if (fh!=-1)
   {
      close(fh);
      dup2(saved_stdin, STDIN_FILENO);
   }

   return 0;
}


bool is_command_line_mode(int argc, char **argv)
{
   bool rval = false;

   for (char **ptr=argv+1; !rval && *ptr; ++ptr)
   {
      // Processing 2-character arguments:
      if ((*ptr)[2]=='\0')
      {
         // Convert the first two characters to a unsigned integer for switch/case
         uint16_t apre = *reinterpret_cast<const uint16_t*>(*ptr);
         switch(apre)
         {
            case dash_val('b'):
            case dash_val('g'):
            case dash_val('m'):
            case dash_val('p'):
            case dash_val('s'):
               rval = true;
               break;

            default:
               break;
         }
      }
      else if (0==strcmp(*ptr, "--version"))
         rval = true;
   }
   return rval;
}

int main(int argc, char **argv)
{
   // Replace the default message_reporter
   SimpleProcedure::s_message_reporter = Schema::procedure_message_reporter;

   // If no command line parameters, run as CGI/FastCGI
   if (is_command_line_mode(argc, argv))
   {
      reset_fcgi_streams();
      return Schema::process_command_line(argc, argv);
   }
   else
   {
      return run_schema();
   }
}

