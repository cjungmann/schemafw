// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -DINCLUDE_MAIN `mysql_config --cflags` -o fods fods.cpp `mysql_config --libs`" -*-

#ifndef FODS_HPP
#define FODS_HPP

#include "istdio.hpp"
#include "procedure.hpp"

class Result_As_FODS : public Result_User_Base
{
private:
   static const char *nspaces[];
public:
   Result_As_FODS(FILE *out) : Result_User_Base(out) { }

   // Implement IResult_User:
   virtual void pre_fetch_use_result(int result_number, DataStack<BindC> &ds, SimpleProcedure &proc);
   virtual void use_result_row(int result_number, DataStack<BindC> &ds);
   virtual void result_complete(int result_number, DataStack<BindC> &ds);
   virtual void report_error(const char *str);

   void doc_element(void) const;
   void start_table(void) const;
   void end_table(void) const;
   void end_doc_element(void) const;

};



#endif
