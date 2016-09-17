// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -DINCLUDE_MAIN `mysql_config --cflags` -o fods fods.cpp `mysql_config --libs`" -*-

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>     // provide date/time functions to write_meta_file()
#include <sys/wait.h> // provide waitpid() for t_set_pipes()
#include "fods.hpp"

const char *Result_As_FODS::nspaces[] = {
   "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"",
   "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"",
   "xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"",
   "office:mimetype=\"application/vnd.oasis.opendocument.spreadsheet\"",
   "office:version=\"1.2\"",
   nullptr
};

const char Result_As_FODS::s_target[] = "output.ods";

/**
 * @brief Class for writing an ODS file to the specified FILE* (typically stdout).
 *
 * This function provides the support for exporting data.  It requires a somewhat
 * convoluted setup in order to ensure that pipes are created and opened in the
 * proper sequence, and that the spawned process that zips the FODS file into an
 * ODS file is complete before the parent process begins copying the result and
 * then deletes the temporary working directory.
 *
 * Refer to the following snippet for an example with some explanation.
 *
 * @snippet fods.cpp Result_As_FODS_Sample
 */
Result_As_FODS::Result_As_FODS(FILE* out)
   : Result_User_Base(nullptr), m_endpath(m_pathbuff),
     m_stream_out(out), m_write_content(nullptr)
{
   prepare_path_string();
   prepare_tmp_directory();
}

Result_As_FODS::~Result_As_FODS()
{
   close_content_fifo(m_write_content);
   remove_tmp_directory();
}

void Result_As_FODS::t_fork_to_zip(IGeneric_Void_Callback &cb)
{
   // The child process running zip will read from the other size of fifo pipe,
   pid_t pid = fork();

   if (pid==0)
   {
      chdir(tmppath());
      execl("zipit", "zipit", s_target, static_cast<char*>(nullptr));
   }
   else
   {
      // Wait 'till now to open the fifo file:
      // Have Result_User write to fifo pipe:
      auto* contentfifo = open_content_fifo();
      
      try
      {
         cb();
      }
      catch(const std::exception &se)
      {
         printf("Caught exception: %s", se.what());
      }

      // We can't leave this to the destructor, even though it's there,
      // too, for backup, because closing the stream must occur before
      // the child process can complete.
      close_content_fifo(contentfifo);
      
      // Wait for child to complete before letting the destructor clean up:
      int status;
      waitpid(pid, &status, 0);
   }

   strcpy(tmpfilename(), s_target);
   send_file_out(m_pathbuff);
}

/**
 * @brief Generate an exception with a stack-allocated string.
 *
 * It may not be safe to use a stack-allocated string for an exception,
 * so I'll have to test it.
 */
void Result_As_FODS::errno_throw(const char *what)
{
   char format[] = "%s failed, errno = %d";
   
   // calculate sufficiently-sized buffer for the message:
   size_t len = strlen(what) + strlen(format) + 12;
   char *buff = static_cast<char*>(alloca(len));
   sprintf(buff, format, what, errno);
   throw std::runtime_error(buff);
}

/**
 * @brief Returns a pointer to a char buffer into which an error message can be added.
 *
 * The function adds a space and a \0 at the end of the m_pathbuff variable, then
 * returns the address of the \0.  The calling function can use the pointer with
 * sprintf(), strcat(), or custom code to complete an error message that will begin
 * with the path to the temporary directory.
 */
char *Result_As_FODS::get_error_ptr(void)
{
   size_t len = strlen(m_pathbuff);
   char *ptr = &m_pathbuff[len];
   *ptr++ = ' ';
   *ptr = '\0';
   return ptr;
}

void Result_As_FODS::prepare_path_string(void)
{
   pid_t pid = getpid();
   int len = sprintf(m_pathbuff, "/tmp/schemafw_%d/", pid);
   m_endpath += len;
}

/**
 * @brief Calls _remove()_ on directory named in m_pathbuff.
 *
 * Unlinks linked files, removes created files, subdirectory, then,
 * finally, the working temporary diretory.
 *
 * Throws an exception for any error except for ENOENT (missing file),
 * which will be allowed in case there was an error that results in this
 * function be called before the files are copied.
 *
 * This should only be called after the m_pathbuff is set and if it's
 * known that the directory named in m_pathbuff exists.  That means in
 * prepare_tmp_directory if the directory already exists, and after the ODS
 * file has been output.  Otherwise, an exception will be thrown if the
 * remove() function fails.
 */
void Result_As_FODS::remove_tmp_directory(void)
{
   auto e = [this]()
   {
      if (errno!=ENOENT)
      {
         sprintf(get_error_ptr(), " remove failure. errno=%d", errno);
         throw std::runtime_error(m_pathbuff);
      }
   };
   
   auto r = [this, &e](const char *f)
   {
      strcpy(m_endpath, f);
      int result = remove(m_pathbuff);
      if (result)
         e();
   };

   auto u = [this, &e](const char *f)
   {
      strcpy(m_endpath, f);
      int result = unlink(m_pathbuff);
      if (result)
         e();
   };

   // Unlink the linked files
   u("META-INF/manifest.xml");
   u("mimetype");
   u("settings.xml");
   u("styles.xml");
   u("zipit");
   
   // Remove files
   r("meta.xml");
   r("content.xml");
   r(s_target);
   
   // Remove subdirectory
   r("META-INF");

   // Finally, remove directory
   r("");
}

/**
 * @brief Create a directory in /tmp to create an ODS file.
 *
 * The function creates the directory, then fills it  with a named
 * pipe (content.xml) and symlinks to files that will be zipped to
 * create an ODS file.
 */
void Result_As_FODS::prepare_tmp_directory(void)
{
   struct stat sb;

   // Create handle to the '\0' terminator, the address of which will be
   // used to concatenate file names to the path, and will be used at the
   // end to restore the '\0' terminator.
   char *target = m_pathbuff;

   // Check for and remove, if found, leftover directory from previous call.
   if (-1 == stat(m_pathbuff, &sb))
   {
      // Throw an error for any error other than file doesn't exist
      if (errno!=ENOENT)
      {
         sprintf(get_error_ptr(), " stat failure. errno=%d", errno);
         throw std::runtime_error(m_pathbuff);
      }
   }
   else
      remove_tmp_directory();

   // Create and fill the directory:
   int result = mkdir(target, 0777);
   if (result) // 0==success, -1==failure.  We still need errno.
   {
      sprintf(get_error_ptr(), " mkdir failure. errno=%d", errno);
      throw std::runtime_error(m_pathbuff);
   }
   else
   {
      strcpy(m_endpath, "META-INF");
      if ((result=mkdir(target,0777)))
      {
         sprintf(get_error_ptr(), " mkdir failure. errno=%d", errno);
         throw std::runtime_error(m_pathbuff);
      }
      
      char source[256];
      strcpy(source, "/usr/local/lib/schemafw/ods/");
      char *psource = &source[strlen(source)];

      auto f = [this, &source, &psource, &target](const char* file)
      {
         strcpy(psource, file);
         strcpy(m_endpath, file);
         int result = symlink(source, target);

         if (result)
         {
            sprintf(get_error_ptr(), " symlink failure. errno=%d", errno);
            throw std::runtime_error(m_pathbuff);
         }
      };

      strcpy(psource, "meta.xml");
      strcpy(m_endpath, "meta.xml");
      write_meta_file(source, target);

      f("mimetype");
      f("settings.xml");
      f("styles.xml");
      f("META-INF/manifest.xml");
      f("zipit");

      strcpy(m_endpath, "content.xml");
      if (mkfifo(target, 0666))
      {
         sprintf(get_error_ptr(), " mkfifo failed. errno=%d", errno);
         throw std::runtime_error(m_pathbuff);
      }
   }

   // Return m_pathbuff to original, pre-function, form:
   *m_endpath = '\0';
}

FILE* Result_As_FODS::open_content_fifo(void)
{
   strcpy(m_endpath, "content.xml");
   int fh = open(m_pathbuff, O_WRONLY);
   if (fh==-1)
   {
      sprintf(get_error_ptr(), " named pipe failed. errno=%d", errno);
      throw std::runtime_error(m_pathbuff);
   }
   else
   {
      m_write_content = ifdopen(fh,"w");
      set_output_stream(m_write_content);
      return m_write_content;
   }
}

void Result_As_FODS::close_content_fifo(FILE* fifo)
{
   if (fifo && fifo==m_write_content)
   {
      ifclose(m_write_content);
      m_write_content = nullptr;
   }
}

void Result_As_FODS::send_file_out(const char* path)
{
   char buff[2048];
   auto* fp = ifopen(path,"rb");
   size_t bread;
   if (fp)
   {
      do
      {
         bread = ifread(buff, 1, sizeof(buff), fp);
         ifwrite(buff, 1, bread, m_stream_out);
      }
      while (bread>0);

      if (!ifeof(fp))
         ifprintf(stderr, "An error (%d) terminated send_file_out.\n", iferror(fp));

      ifclose(fp);
   }
}

void Result_As_FODS::write_meta_file(const char *pathsource, const char *pathtarget)
{
   int fh_t = open(pathtarget, O_CREAT|O_WRONLY, 0666);
   size_t len;
   size_t todate;
   size_t filesize = 0;
   
   if (fh_t)
   {
      int fh_s = open(pathsource, O_RDONLY);
      if (fh_s)
      {
         struct stat st;
         char dbuff[25];

         // Read entire short file into memory:
         fstat(fh_s, &st);
         filesize = st.st_size;
         char* buff = static_cast<char*>(alloca(filesize));
         size_t r = read(fh_s, buff, filesize);
         close(fh_s);
         
         if (r==filesize)
         {
            char *rd = strstr(buff, "</meta:creation-date>");
            if (rd)
            {
               todate = rd-buff;
               // Before date from source file
               write(fh_t, buff, todate);

               // Write current date
               len = fill_date_buffer(dbuff, sizeof(dbuff));
               write(fh_t, dbuff, len);

               // Write remainder of source file:
               write(fh_t, &buff[todate], filesize-todate);
            }
         }
      }
      
      close(fh_t);
   }
}

/**
 * @brief Writes the start of the FODS document used as the ODS file's content.xml.
 */
void Result_As_FODS::pre_fetch_use_result(int result_number,
                                          DataStack<BindC> &ds,
                                          SimpleProcedure &proc)
{
   fods_write("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
   
   doc_element();
   start_table();
}

/**
 * @brief Writes the row to the FODS document used as the ODS file's content.xml.
 */
void Result_As_FODS::use_result_row(int result_number, DataStack<BindC> &ds)
{
   fods_write("<table:table-row>\n");

   int index = 0;
   for (auto *col=ds.start();
        col;
        ++index, col=col->next())
   {
      fods_write("<table:table-cell");
      
      BindC &obj = col->object();
      const _CType* ctyper = obj.m_typeinfo;
      if (!obj.is_null())
      {
         const char *ctype = ctyper->get_cell_type();
         if (ctype)
         {
            fods_write(" office:value-type=\"");
            fods_write(ctype);
            fods_putc('"');
            
            const char *vtype = ctyper->get_value_attribute();
            if (vtype && strcmp(vtype,"string"))
            {
               fods_write(" office:");
               fods_write(vtype);
               fods_write("=\"");
               obj.print(m_write_content);
               fods_putc('"');
            }
         }
      }
      fods_write(">\n");

      if (!obj.is_null())
      {
         fods_write("<text:p>");
         obj.print_xml_escaped(m_write_content);
         fods_write("</text:p>\n");
      }

      fods_write("</table:table-cell>\n");
   }

   fods_write("</table:table-row>\n");
}

/**
 * @brief Writes the end of the FODS document that goes to named pipe content.xml.
 */
void Result_As_FODS::result_complete(int result_number, DataStack<BindC> &ds)
{
   end_table();
   end_doc_element();

   if (m_write_content)
   {
      ifclose(m_write_content);
      m_write_content = nullptr;
   }
}

void Result_As_FODS::report_error(const char *str)
{
}


int Result_As_FODS::fill_date_buffer(char* buffer, int bufflen)
{
   time_t     itime;
   struct tm  itm;
   time(&itime);
   gmtime_r(&itime, &itm);

   return snprintf(buffer, bufflen, "%d-%02d-%02dT%02d:%02d::%02d",
                   itm.tm_year+1900,
                   itm.tm_mon+1,
                   itm.tm_mday,
                   itm.tm_hour,
                   itm.tm_min,
                   itm.tm_sec);
}

void Result_As_FODS::doc_element(void) const
{
   fods_write("<office:document");
   const char **ptr = nspaces;
   while (*ptr)
   {
      fods_write("\n ");
      fods_write(*ptr);

      ++ptr;
   }

   fods_write(">\n");
}

void Result_As_FODS::start_table(void) const
{
   fods_write("<office:body>\n"
              "<office:spreadsheet>\n"
              "<table:table>\n");
}

void Result_As_FODS::end_table(void) const
{
   fods_write("</table:table>\n"
              "</office:spreadsheet>\n"
              "</office:body>\n");
}


void Result_As_FODS::end_doc_element(void) const
{
   fods_write("</office:document>\n");
}


#ifdef INCLUDE_MAIN
#undef INCLUDE_MAIN

#include "istdio.cpp"

#include "prandstr.cpp"
#include "vclasses.cpp"
#include "ctyper.cpp"
#include "bindc.cpp"
#include "datastack.cpp"
#include "bindstack.cpp"
#include "procedure.cpp"

//@ [Result_As_FODS_Sample]
void run_test(MYSQL &mysql)
{
   const char *query = "SELECT * FROM information_schema.TABLES";
   Result_As_FODS rafods(stdout);

   try
   {
      SimpleProcedure sp(query);

      auto func = [&mysql, &rafods, &sp](void)
      {
         sp.run(&mysql, rafods);
      };
      
      rafods.fork_to_zip(func);
   }
   catch(const std::exception &se)
   {
      printf("Caught exception: %s", se.what());
   }
}
//@ [Result_As_FODS_Sample]



int main(int argc, char** argv)
{
   const char *host = nullptr;
   const char *user = nullptr;
   const char *pword = nullptr;
   const char *database = "information_schema";
   
   MYSQL mysql;
   mysql_init(&mysql);
   mysql_options(&mysql,MYSQL_READ_DEFAULT_GROUP,"");

   if (mysql_real_connect(&mysql, host, user, pword, database, 0, nullptr, 0))
   {
      run_test(mysql);
      
      mysql_close(&mysql);
   }

   mysql_library_end();
   return 0;
}

#endif
