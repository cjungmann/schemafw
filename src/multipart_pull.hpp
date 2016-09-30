// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_MAIN `mysql_config --cflags` -o multipart_pull multipart_pull.cpp `mysql_config --libs`" -*-

#ifndef MULTIPART_PULL_HPP
#define MULTIPART_PULL_HPP

#include <assert.h>
#include <string.h>

#include "vclasses.hpp"
#include "prandstr.hpp"

struct  mimetypes_map
{
   int index;
   const char *name;
};

/**
 * @brief Tool for reading the contents of a multipart_form submission.
 *
 * In my experience, the primary reason for using the multipart_form request
 * type is to upload a file, and this object is primarily designed to handle
 * the file uploads expected by the SchemaFW project.  Calling get_csv_file_handle()
 * returns a file handle from which the CSV conversion of a spreadsheet can be
 * read.
 *
 * Multipart_Pull is designed to read `stdin` from CGI/FASTCGI http request
 * to provide access to its contents.  To make debugging easier, it reads
 * from an IStreamer, which can be constructed from string or any file stream,
 * either `stdin` or a file opened with `fopen`.
 *
 * This object is named "Pull" because the user of the class uses the
 * getc() function to pull characters from the form.  My
 * first multipart_form reader worked by using a callback function to
 * which the form contents were sent.
 *
 * Further, the reason I include this in my project is to allow users to
 * upload spreadsheets to be imported into MySQL.  Look at get_csv_file_handle()
 * for an explanation, and at the code snippet below for an example.
 *
 * @snippet multipart_pull.cpp Demo_MP_Pull
 */
class Multipart_Pull
{
public:
   struct fhandle_payload;  // prototype
   struct thread_data
   {
      Multipart_Pull &mpp;
      int &fhandle;
   };
   
   Multipart_Pull(IStreamer &s);
   
   /** @brief Returns the boundary string. */
   inline const char *boundary(void) const                  { return m_boundary; }
   /** @brief Returns the Content-Disposition, usually _form-data_. */
   inline const char *field_content_disposition(void) const { return m_field_cdispo; }
   /** @brief Returns the name of the current field. */
   inline const char *field_name(void) const                { return m_field_name; }
   /** @brief Returns the file name for a file upload. */
   inline const char *field_file_name(void) const           { return m_field_fname; }
   /** @brief Returns the Content-Type MIME value of a file upload. */
   inline const char *field_content_type(void) const        { return m_field_ctype; }

   /** @brief Tells if last EOF marks the end of the form. */
   inline bool end_of_form(void) const      { return m_end_of_form; }
   inline bool eof(void) const              { return m_field_name==nullptr; }

   inline bool is_file_upload(void) const   { return m_field_fname!=nullptr; }

   int getc(void);
   int get_csv_file_handle(fhandle_payload &p);
   void flag_field_complete(void);
   bool next_field(void);


   void t_send_for_csv_filehandle(const IGeneric_Callback<int>& cb);
   
   template <class Func>
   void send_for_csv_filehandle(const Func &f)
   {
      Generic_User<int, Func> user(f);
      t_send_for_csv_filehandle(user);
   }


   /**
    * @brief Allocated variables to use with file handle-returning function.
    *
    * In particular, this is first used as an argument for get_csv_file_handle
    * so that the function can use the stack-allocated memory from the calling
    * function to do its work.  Otherwise, when get_csv_file_handle returns
    * the file handle, the stack variables in get_csv_file_handle will get
    * overwritten.
    */
public:
   struct fhandle_payload
   {
      Multipart_Pull &mpp;
      int            pipe_in[2];
      int            pipe_out[2];
      int            pipe_err[2];

      /** @brief Constructor to generate error if mpp not set. */
      fhandle_payload(Multipart_Pull &p)
         : mpp(p), pipe_in{-1}, pipe_out{-1}, pipe_err{-1} { }
      
      /** @brief Default constructor throws an error. */
      fhandle_payload(void) = delete;

      /** Alias for appropriate pipe-end for thread start routine. */
      int thread_write(void) const { return pipe_in[1]; }
      /** Alias for appropriate pipe-end from which to read the conversion result. */
      int result_read(void) const  { return pipe_out[0]; }
   };

public:
   enum ECODE
   {
      ECODE_NONE = 0,
      ECODE_FILE_FORMAT = 1
   };

protected:
   static uint16_t s_END_FIELD;   // "/r/n"
   static uint16_t s_END_FORM;    // "--"
   static const char s_multipart_str[];
   static const int  s_len_multipart_str;
   static const char s_boundary_str[];
   static const int  s_len_boundary_str;

   static const mimetypes_map * const s_mtypes_map;
   
   char       m_workarea[2048];      /**< Largish char array that is used to store
                                      *   various strings.  These include the boundary
                                      *   string that separates the fields, as well
                                      *   as the meta data that begins each field.
                                      */
   char       *m_end_workarea;       /**< Pointer to the last character the workarea
                                      *   against which a working pointer can be
                                      *   compared to prevent buffer overflow.
                                      */
   IStreamer  &m_str;                /**< Object from which to collect chars. */
   const char *m_boundary;           /**< Pointer to boundary string. */
   
   const char *m_boundary_end;       /**< Pointer to end of boundary string.
                                      * Compares matches up to this pointer
                                      * to confirm or not a match.
                                      */

   int        m_boundary_length;     /**< Length of boundary string to use as the
                                      * initial read length into m_buffer.
                                      */

   const char *m_boundary_ptr;       /**< Work variable for comparing stream
                                      * characters against the boundary string.
                                      */
   
   const char *m_end_boundary_chunk; /**< Identifies end of a partial boundary
                                      * string match.  Is null if no partial
                                      * match is in play.
                                      */

   int        m_match_breaker;       /**< If a partial boundary string match is
                                      * on, this is the character that broke the
                                      * match.  It must be returned to the reader
                                      * after all of the matched boundary characters
                                      * have been read.
                                      */
   
   char       *m_buffer;             /**< Pointer into s_workarea where new characters
                                      * can be copied.  Will follow the boundary string
                                      * and any header values.
                                      */

   bool       m_end_of_field;        /**< Flag to indicate we've reached the end
                                      * of a field.  Further calls to getc() will
                                      * return EOF until read_headers is called.
                                      * It is the reponsibility of any field reader
                                      * to ensure that the file pointer is positioned
                                      * at the beginning of the next field's header
                                      * lines.
                                      */

   bool       m_end_of_form;         /**< Flag to indicate we've reached the end
                                      * of the form.  Check this value when getc()
                                      * returns EOF to see if more fields are
                                      * available.
                                      */

   const char *m_field_cdispo;     /**< Field Content-Disposition */
   const char *m_field_name;       /**< Field Name */
   const char *m_field_fname;      /**< Field File name */
   const char *m_field_ctype;      /**< Field Content type */

   ECODE      m_errors;

   static void *pthread_create_stderr_filter(void *data);
   static void *pthread_create_start_routine(void *data);

   static void* start_stdin_thread(void* data);
   static void* start_stderr_thread(void* data);

   void reset_field_heads(void);
   void read_headers(void);
   void read_headers_old(void);

   

private:
   void read_initial_boundary_and_prepare_buffers(void);
   void initialize_from_file(void);

   
};

#endif // MULTIPART_PULL_HPP
