#ifndef ADVISOR_HPP
#define ADVISOR_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "genericuser.hpp"
#include "vclasses.hpp"    // for EFFC_2 and EFFC_3


/**
 * @brief Pure virtual class for alternate methods of file access
 */
class I_AFile
{
   long int m_cur_line_pos;
public:
   I_AFile(void) : m_cur_line_pos(-1) { }
   EFFC_2(I_AFile)

   virtual ~I_AFile()  { }
   virtual bool is_open(void) = 0;
   virtual void rewind(void) = 0;

protected:
   /**
    * @name Internal "raw" methods for position and reading the file
    * @{
    */
   virtual long int _get_position(void) = 0;
   virtual void _set_position(long int pos) = 0;
   virtual char * _read_line(char *buff, int buffsize) = 0;
   /**@}*/

public:
   inline long int get_position(void) const         { return m_cur_line_pos; }
   void set_position(long int pos);
   char *read_line(char *buff, int buffsize);
};

/**
 * @brief Implementation I_AFile using FILE* streams.
 *
 * This is deprecated, or even abandoned, because FCGI_open() from fcgi_stdio.h
 * uses malloc() when opening a disk file, which violates my goal of keeping
 * everything on the stack.
 */
class AFile_Stream : public I_AFile
{
protected:
   FILE *m_pfile;
   
public:
   // AFile_Stream(const char *filename=nullptr)
   //    : I_AFile(), m_pfile(nullptr)           { if (filename) open(filename); }
   AFile_Stream(FILE *file)
      : I_AFile(), m_pfile(file)             { }
   virtual ~AFile_Stream();
   EFFC_2(AFile_Stream)

   virtual bool is_open(void);
   virtual void rewind(void);
   virtual long int _get_position(void);
   virtual void _set_position(long int pos);
   virtual char * _read_line(char *buff, int buffsize);
};


/**
 * @brief A file buffer class for AFile_Handle.
 *
 * This class does the file buffering for the AFile_Handle class. It tracks the
 * file position of the buffer in case a requested file move still falls within
 * the buffer.  My understanding is that the OS will abandon its buffer if a
 * seek() or fseek() command is called, so this should be an improvement.
 */
class BaseBuffer
{
private:
   char*  m_buffer;           /**< To be set by derived class. */
   size_t m_buffer_size;      /**< To be set by derived class, for calls to read(). */
   
   char*  m_buffer_end;       /**< Marks end of data in m_buffer. */
   char*  m_buffer_ptr;       /**< Points to next character to be read. */
   off_t  m_buffer_file_pos;  /**< File position of byte 0 of m_buffer. */
   off_t  m_buffer_file_size; /**< To detect if more data is available. */
   bool   m_in_use;           /**< Flag, for static instantiation, to indicate
                               *   another process or stack frame is already using
                               *   the object.  Allows a graceful recovery if my
                               *   assumption that the static version will be
                               *   sufficient.
                               */

public:
   BaseBuffer(char *buffer, size_t buffsize)
      : m_buffer(buffer), m_buffer_size(buffsize),
        m_buffer_end(nullptr), m_buffer_ptr(nullptr),
        m_buffer_file_pos(0), m_buffer_file_size(0),
        m_in_use(false)                               { }

   EFFC_3(BaseBuffer)
   
   static int open_file(const char *path);
   static off_t file_size(int handle);
   
   void start(int handle);
   char *read_line(int handle, char *buff, int buffsize);
   void set_position(int handle, off_t pos);
   void finish(void);

   /**
    * @brief Returns in_use value.
    *
    * This test provides safety against overlapping attempts to use the static
    * buffer instance of AFile_Handle.  If the static version is in use, an
    * implementation can create a new copy on the stack and use it instead.
    */
   bool in_use(void) const        { return m_in_use; }
   /** Detect eof condition if buffer has no contents; */
   bool eof(void) const           { return m_buffer_end==m_buffer; }
   /** Returns file position to first character of the file. */
   void rewind(int handle)        { set_position(handle, 0); }
   /** Returns file position of the next character to be read. */
   off_t get_position(void) const { return m_buffer_file_pos+m_buffer_ptr-m_buffer; }

protected:
   bool read_from_file(int handle);
   off_t file_position_of_end_of_buffer(void) const
                                  { return m_buffer_file_pos + m_buffer_ptr - m_buffer; }
   bool file_position_in_buffer(off_t pos)
   {
      return (pos >= m_buffer_file_pos && pos < file_position_of_end_of_buffer());
   }

   /**
    * @brief Set m_buffer_ptr to position in m_buffer that corresponds to file position @p pos.
    *
    * Assumes that @p pos is within buffer after calling file_position_in_buffer().
    */
   void set_buffer_ptr_to_file_position(off_t pos)
   {
      m_buffer_ptr = m_buffer + pos - m_buffer_file_pos;
   }
};


/**
 * @brief Derived BaseBuffer class that allocates the memory for BaseBuffer.
 */
template <size_t bufflen>
class Buffer : public BaseBuffer
{
   char   buffer[bufflen];

public:
   Buffer()
      : BaseBuffer(buffer, bufflen) { }
   
   EFFC_3(Buffer<bufflen>)
};

/**
 * @brief I_AFile class using file handle instead of FILE*.
 *
 * The purpose of this class is access the file using a file handle in order
 * to avoid conflict with fcgi_stdio.h renaming FILE*.
 */
class AFile_Handle : public I_AFile
{
private:
   static BaseBuffer *s_buffer;
   
protected:
   int               m_handle;
   BaseBuffer        *m_buffer;

private:
   /** An AFile_Handle object can only be constructed in a build() function. */
   AFile_Handle(int handle, BaseBuffer *buff)
      : I_AFile(), m_handle(handle), m_buffer(buff)   { buff->start(handle); }

public:
   // Must provide a real destructor function for the virtual one:
   virtual ~AFile_Handle();
   EFFC_2(AFile_Handle)

   static void build_new_buffer(int handle, IGeneric_Callback<AFile_Handle> &callback);

   template <class Func>
   static void build(const char *path, Func f)
   {
      int handle = BaseBuffer::open_file(path);
      if (handle)
      {
         if (s_buffer->in_use())
         {
            // Can't use the static buffer, make a stack-allocated,
            // initialized buffer, packaging the callback so we can
            // use a non-template function:
            Generic_User<AFile_Handle, Func> afuser(f);
            build_new_buffer(handle, afuser);
         }
         else
         {
            AFile_Handle afh(handle, s_buffer);
            f(afh);

            // When function f() returns, it no longer needs the
            // static buffer, reset to make it available later.
            s_buffer->finish();
         }
         
         ::close(handle);
      }
   }

   virtual bool is_open(void)               { return m_handle>0; }
   virtual void rewind(void)                { m_buffer->rewind(m_handle); }
   virtual long int _get_position(void)     { return m_buffer->get_position(); }
   virtual void _set_position(long int pos) { m_buffer->set_position(m_handle, pos); }
   virtual char* _read_line(char *buff,
                            int buffsize)   { return m_buffer->read_line(m_handle, buff, buffsize); }
};

/**
 * @brief Open and read a template file for controlling schema output.
 *
 * This class scans lines of a specs file (@ref Specs_File) into a line
 * buffer for accessing each line's tag and optional value.  Only one line
 * is in memory at a time, so the tag or value of a line cannot be used
 * to search for another line because the process of moving through the
 * file overwrites the buffer, making the previous tag-value pair invalid.
 *
 * The ab_handle class was designed to work with a section of an Advisor
 * file.  An ab_handle class is constructed using the BranchPool class.
 */
class Advisor
{
   static const int s_bufflen = 256;     /**< Static variable so its value can be discovered. */
                               
   I_AFile &m_file;
   char  m_buffer[s_bufflen]; /**< workspace for collecting lines from m_file */
   char  *m_cur_tag;   /**< pointer to start of string in current buffer */
   char  *m_cur_value; /**< pointer to text following the first colon, if found. */
   int   m_len_tag;
   int   m_len_value;
   int   m_cur_level;  /**< informs line and file status.  -1 indicates we've reached the end of the file. */

private:

protected:
   void scan_buffer_for_ends_and_level(void);
   
public:
   Advisor(I_AFile &afile);
   EFFC_3(Advisor);

   /** @brief Get buffer size in case someone needs to save its values. */
   static int bufflen(void)   { return s_bufflen; }

   /**
    * @name Iterator functions.
    * @{
    */
   /** @brief Reads the next tagged line, skipping empty and comment lines. */
   bool get_next_line(void);
   Advisor &operator++(void)    { get_next_line(); return *this; }
   bool end(void) const         { return m_cur_level<0; }
   /**@}*/

   /**
    * @name Access to current line values.
    * @{
    */
   /** @brief Returns true if the current tag value matches str. */
   bool is_equal_to(const char *str) const { return m_cur_tag && strcmp(m_cur_tag,str)==0; }
   /** @brief The current line is a section if it has a name with no value. */
   bool is_section(void) const             { return m_cur_value==nullptr; }
   /** @brief The current line is a setting if it has both a name and a value. */
   bool is_setting(void) const             { return m_cur_value!=nullptr; }
   /** @brief The current line is a reference to a shared mode. */
   bool is_reference_value(void) const     { return m_len_value && *m_cur_value=='$'; }
   /** @brief Returns true if the current tag value matches str. */
   bool operator==(const char *str) const  { return is_equal_to(str) ;}


   /**
    * @name Somewhat unsafe return values.
    *
    * These values are only valid until get_next_line() is called.
    * They should probably return a unique type that can be identified
    * and saved before using to compare against other lines in the
    * same advisor file.
    *
    * @todo Create an alias to const char* that can be used to restrict
    * the usage of the values these functions return.
    * @{
    */
   /** @brief Returns tag or name portion of the current buffer contents. */
   inline const char *tag(void) const      { return m_cur_tag; }
   /** @brief Returns the value portion of the current buffer contents. */
   inline const char *value(void) const    { return m_cur_value; }
   /** @} */

   inline int len_tag(void) const          { return m_len_tag; }
   inline int len_value(void) const        { return m_len_value; }

   /** @brief Returns the number of white spaces that preceeds the tag. */
   inline int level(void) const            { return m_cur_level; }
   /**@}*/

   /** @brief Reads in the first line of the file. */
   void rewind(void)                      { m_file.rewind(); get_next_line(); }
   /** @brief Gets current position in order that restore_state can later return to it. */
   long int get_position(void)            { return m_file.get_position(); }
   /** @brief Returns to the state, including having the current line in buffer, of pos. */
   void restore_state(long int pos);

   /** @brief Attempts to get the first child.  Returns true if successful, and the child is in the line buffer. */
   bool first_child(void);
   /** @brief Attempts to get the next sibling.  Returns true if successful, and the sibling is in the line buffer. */
   bool next_sibling(void);

   /**
    * @name Functions that seek lines by name (char*).
    *
    * These are convenience functions for seeking by name.  It is important
    * to realize that they may not work if you use the results of a previous
    * for the name.  The Advisor class uses a single buffer into which lines
    * are read.  Using the value() function
    * @{
    */
   bool seek_tag(const char *name, int level=0, bool start_at_top=false);
   bool seek_sibling(const char *name)     { return seek_tag(name, level()); }
   bool seek_child(const char *name);
};


class Advisor_iterator
{
protected:
   Advisor  &m_advisor;
   long int m_top;
   int      m_section_level;
   
   inline void save_position(void)           { m_top = m_advisor.get_position(); }
   inline void restore_state(void)           { m_advisor.restore_state(m_top); }

public:
   Advisor_iterator(Advisor &adv);
   EFFC_3(Advisor_iterator)

   int level(void) const                     { return m_section_level; }

   /**
    * @name Iterator service functions
    */
   inline void reset(void)                   { restore_state(); }
   inline bool end(void)                     { return m_advisor.level() <= m_section_level; }
   inline bool is_sub(void)                  { return m_advisor.level() > m_section_level; }
   inline void get_next(void)                { m_advisor.get_next_line(); }
   Advisor_iterator &operator++(void)        { m_advisor.get_next_line(); return *this; }
   /**@}*/

   /**
    * @name Functions that return the current item
    * @{
    */
   inline Advisor& operator*(void)             { return m_advisor; }
   inline const Advisor& operator*(void) const { return m_advisor; }
   inline Advisor& item(void)                  { return m_advisor; }
   inline const Advisor& item(void) const      { return m_advisor; }
   /**@}*/
};




#endif
