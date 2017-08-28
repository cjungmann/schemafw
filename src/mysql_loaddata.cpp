#include <stdio.h>             // snprintf
#include <string.h>            // strcpy
#include "mysql_loaddata.hpp"

const uint16_t MySQL_LoadData::s_aposastr = *reinterpret_cast<const uint16_t*>("'*");
const uint16_t MySQL_LoadData::s_if_name = *reinterpret_cast<const uint16_t*>("-\0");


/**
 * @brief This function simply reads to the end of the line without saving it.
 *
 * This function reads the input stream until it reaches a newline that
 * is not in a quoted string.
 */
void MySQL_LoadData::discard_line(bool starting_in_apos)
{
   char   c;
   size_t r;
   
   bool in_apos = starting_in_apos;
   while (1==(r=read(m_filehandle, &c, 1)))
   {
      switch(c)
      {
         // This should work; in a string, two consecutive
         // apostrophes will cancel each other out, the first
         // will exit apos_mode, the next will go right back in.
         case '\'':
            in_apos = !in_apos;
            break;
            
         case '\n':
            // A newline not in an string indicates end-of-line:
            if (!in_apos)
               return;
            else
               break;
      }
   }

   // No further checking necessary: if we reached the
   // end of the file by not returning earlier, the end
   // of the file is also the end of the line.
}

/**
 * @brief Starts the import process.
 *
 * Having collected the source file handle and the target table name,
 * this function creates a query to import, enables **LOAD DATA LOCAL INFILE**
 * by replacing the disabling infile handler functions with the working
 * functions, then runs the query with _mysql_real_query_.
 *
 * The target quarantine table **must** be prepared with privileges for
 * the webuser to insert records.  See the User Guide for setup examples.xs
 */
bool MySQL_LoadData::import(void)
{
   // Prepare and run query:
   char buff[200];
   int len = snprintf(buff, 200,
                      "LOAD DATA LOCAL INFILE '-' INTO TABLE %s"
                      " FIELDS TERMINATED BY ','"
                      " OPTIONALLY ENCLOSED BY ''''",
                      m_tablename);

   // fprintf(stderr, "Query = \"%s\"\n", buff);

   // Enable infile handler just before executing the query:
   mysql_set_local_infile_handler(m_mysql,
                                  mih_init,
                                  mih_read,
                                  mih_end,
                                  mih_error,
                                  static_cast<void*>(this));

   // Call the query:
   // Debugging note: if mysql_real_connect() has not been
   // called successfully, the following function will fail
   // with a SEGMENTATION FAULT!  I wasted two days trying to
   // solve this.
   int result = mysql_real_query(m_mysql, buff, len);

   // Display errors, if any, before disabling infile handler:
   if (result)
      fprintf(stderr,
              "The import query failed (%s)\n", mysql_error(m_mysql));
   // Make sure no one else can use LOAD DATA:
   disable_local_infile(m_mysql);
   
   return result==0;
}

/**
 * @brief Implements the init() function for mysql_set_local_infile_handler().
 */
int MySQL_LoadData::mih_init(void **obj, const char *filename, void *userdata)
{
   if (is_acceptable_filename(filename))
   {
      *obj = userdata;
      MySQL_LoadData *mld = static_cast<MySQL_LoadData*>(userdata);

      mld->m_in_line = false;
      mld->m_in_apos = false;
      
      // Return SUCCESS value:
      return 0;
   }
   else
   {
      fputs("Unacceptable filename rejected for load data.\n", stderr);
      return 1;
   }
}

/**
 * @brief Implements the read() function for mysql_set_local_infile_handler().
 *
 * Reads data from m_filehandle, checking for a comment line defined as a row
 * where the initial character of the column A is an _*_, otherwise passing
 * the contents directly to MySQL.
 */
int MySQL_LoadData::mih_read(void *obj, char *buffer, unsigned int bufflen)
{
   MySQL_LoadData *mld = static_cast<MySQL_LoadData*>(obj);
   int fh = mld->m_filehandle;
   bool &in_line = mld->m_in_line;
   bool &in_apos = mld->m_in_apos;
   char linestart[2];

   size_t rcount=-2;  // number of characters read in read()

   // Working pointer to current position in buffer before saving:
   char *ptr = buffer;
   // Pointer to the position just past the end of the buffer:
   char *buffend = buffer + bufflen;

   // If we're starting a new record, the contents of
   // this loop will continue the loop until it finds
   // the beginning of a non-comment line
   while (!in_line)
   {
      // Shouldn't be necessary, but for safety,
      // reset with each new line.
      in_apos = false;
      
      // If currently not in_line, read the first two
      // characters to see if it's a comment line,
      // either \*? or '\*  .
      if (2==(rcount=read(fh,linestart,2)))
      {
         // Check linestart characters for in_apos status:
         if ((linestart[0]=='\'' && linestart[1]!='\'')
             || (linestart[1]=='\'' && linestart[0]!='\''))
            in_apos = true;

         if (is_comment_line(linestart))
         {
            mld->discard_line(in_apos);
            // reset buffer pointer an remain not in_line
            ptr = buffer;

            // Technically, I should clear in_apos here because we're
            // restarting the line, but since I always reset in_apos
            // when starting a new line, it's redundant to do it here
            // as well.
         }
         else
         {
            // change in_line status and set pointer to
            // keep the first two characters:
            in_line = true;

            // Add session value for first column (updating ptr at the same time):
            ptr += sprintf(ptr, "%d,", mld->m_session_id);

            // Add the first two characters read of the non-comment line:
            memcpy(ptr, linestart, 2);
            
            // Update pointer for linestart characters:
            ptr+=2;

         }
      }
      // If we're not in a line, and we can't get two characters,
      // we can ignore the single character (if retrieved) and
      // just be done importing:
      else
         return 0;
   }

   // We're in an established line, read characters until
   // we reach the end of the record or we exhaust the
   // buffer.
   while (ptr<buffend && 1==(rcount=read(fh, ptr, 1)))
   {
      switch(*ptr++)
      {
         case '\'':
            in_apos = !in_apos;
            break;
               
         case '\n':
            if (!in_apos)
            {
               // Not in_apos, so newline signals the end of the record.
               // Set the in_line=false so next call to this function
               // knows it's a new record:
               in_line = false;

               // Set buffend==ptr so the loop will exit.  The
               // buffend value will be reset next time mih_read
               // is called, so it doesn't need to be saved.
               buffend=ptr;
            }
            break;
      }
   }

   return ptr - buffer;
}

/**
 * @brief Implements the clean up function for mysql_set_local_infile_handler().
 */
void MySQL_LoadData::mih_end(void *obj)
{
   MySQL_LoadData *mld = static_cast<MySQL_LoadData*>(obj);
   
   if (mld->m_in_line)
      fputs("Reached end of LOAD DATA while still in a line.\n", stderr);
}

/*
 * @brief Implements the error() function for mysql_set_local_infile_handler().
 */
int MySQL_LoadData::mih_error(void *ptr, char *message_buffer, unsigned int bufflen)
{
   snprintf(message_buffer, bufflen, "unexpected error request.");
   // returning that there's an error (or we shouldn't be here),
   // but I don't know what value to return.
   // We shouldn't really get here, anyway.
   return 1;
}






