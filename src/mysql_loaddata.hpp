#ifndef MYSQL_LOADDATA_HPP
#define MYSQL_LOADDATA_HPP

#include <stdint.h>  // for uint16_t
#include <unistd.h>  // for read()/write()
#include <mysql.h>

class MySQL_LoadData
{
public:
   MySQL_LoadData(MYSQL *mysql, int filehandle, int session_id, const char *tablename)
      : m_mysql(mysql),
        m_filehandle(filehandle),
        m_session_id(session_id),
        m_tablename(tablename),
        m_in_line(false),
        m_in_apos(false)
   { }

   int filehandle(void) const { return m_filehandle; }
   bool import(void);

   static void disable_local_infile(MYSQL *mysql)
   {
      mysql_set_local_infile_handler(mysql,
                                     disable_init,
                                     disable_read,
                                     disable_end,
                                     disable_error,
                                     nullptr);
   }




protected:
   MYSQL *m_mysql;
   int m_filehandle;        /**< File handle from which to read CSV. */
   int m_session_id;        /**< Session id value for first column.  */
   const char *m_tablename; /**< Table into which to import the CSV. */

   bool m_in_line;          /**< True if we've read at least the first
                             *   two characters of a line and confirmed
                             *   that the current line is not a comment.
                             */
   bool m_in_apos;          /**< True if we're in a string, where newlines
                             *   will be included without signaling the
                             *   end of the record.
                             */

   const static uint16_t s_aposastr;
   const static uint16_t s_if_name;

   inline static bool is_comment_line(uint16_t firstchars)
   {
      return (*reinterpret_cast<char*>(&firstchars)=='*' || firstchars==s_aposastr);
   }

   inline static bool is_comment_line(const char *firstchars)
   {
      return (*firstchars=='*' || *reinterpret_cast<const uint16_t*>(firstchars)==s_aposastr);
   }

   inline static bool is_acceptable_filename(const char *filename)
   {
      return (*reinterpret_cast<const uint16_t*>(filename)==s_if_name);
   }

   void discard_line(bool starting_in_apos);




   static int mih_init(void **ptr, const char *filename, void *userdata);
   static int mih_read(void *ptr, char *buff, unsigned int bufflen);
   static void mih_end(void *ptr);
   static int mih_error(void *ptr, char *message_buffer, unsigned int bufflen);

   static int disable_init(void **ptr, const char *filename, void *userdata) { return 1; }
   static int disable_read(void *ptr, char *buff, unsigned int bufflen)      { return 0; }
   static void disable_end(void *ptr)                                        { }
   static int disable_error(void *ptr, char *message_buffer, unsigned int bufflen)
   {
      snprintf(message_buffer, bufflen, "load data disabled.");
      return 1;
   }

};

#endif  // MYSQL_LOADDATA_HPP

