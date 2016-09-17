// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -DINCLUDE_MAIN `mysql_config --cflags` -o fods fods.cpp `mysql_config --libs`" -*-

#ifndef FODS_HPP
#define FODS_HPP

#include "istdio.hpp"
#include "procedure.hpp"

class Result_As_FODS : public Result_User_Base
{
private:
   static const char *nspaces[];
   static const char s_target[];
   char   m_pathbuff[128];
   char   *m_endpath;

   FILE   *m_stream_out;    /**< Save constructor _out_ parameter for final copy. */
   FILE   *m_write_content; /**< Stream to write for content.xml. */

public:
   Result_As_FODS(FILE* out);
   ~Result_As_FODS();
   // Delete constructor methods (effc++ warning):
   EFFC_2(Result_As_FODS)

   inline const char* tmppath(void) const  { return m_pathbuff; }
   inline char* tmpfilename(void)          { return m_endpath; }
   
   void t_fork_to_zip(IGeneric_Void_Callback &cb);
   
   template <class Func>
   void fork_to_zip(const Func& func)
   {
      Generic_Void_User<Func> user(func);
      t_fork_to_zip(user);
   }



private:
   static void errno_throw(const char *what);
   inline void fods_write(const char *str) const { ifputs(str, m_write_content); }
   inline void fods_putc(char c) const           { ifputc(c, m_write_content); }
   
   char *get_error_ptr(void);

   void prepare_path_string(void);
   void remove_tmp_directory(void);
   void prepare_tmp_directory(void);

   FILE* open_content_fifo(void);
   void close_content_fifo(FILE* fifo);
   void send_file_out(const char* source);
   
   void write_meta_file(const char* pathsource, const char* pathtarget);
   void doc_element(void) const;
   void start_table(void) const;
   void end_table(void) const;
   void end_doc_element(void) const;

   int fill_date_buffer(char* buff, int bufflen);

   void write_meta(int fh);
   void write_content(int fh);

public:
   // Implement IResult_User:
   virtual void pre_fetch_use_result(int result_number, DataStack<BindC> &ds, SimpleProcedure &proc);
   virtual void use_result_row(int result_number, DataStack<BindC> &ds);
   virtual void result_complete(int result_number, DataStack<BindC> &ds);
   virtual void report_error(const char *str);
};



#endif
