// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -DINCLUDE_MAIN `mysql_config --cflags` -o fods fods.cpp `mysql_config --libs`" -*-

#include "fods.hpp"

const char *Result_As_FODS::nspaces[] = {
   "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"",
   "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"",
   "xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"",
   "office:mimetype=\"application/vnd.oasis.opendocument.spreadsheet\"",
   "office:version=\"1.2\"",
   nullptr
};



void Result_As_FODS::pre_fetch_use_result(int result_number,
                                          DataStack<BindC> &ds,
                                          SimpleProcedure &proc)
{
}

void Result_As_FODS::use_result_row(int result_number, DataStack<BindC> &ds)
{
   ifputs("<table:table-row>\n", m_out);

   int index = 0;
   for (auto *col=ds.start();
        col;
        ++index, col=col->next())
   {
      ifputs("<table:table-cell", m_out);
      
      BindC &obj = col->object();
      const _CType* ctyper = obj.m_typeinfo;
      if (!obj.is_null())
      {
         const char *ctype = ctyper->get_cell_type();
         if (ctype)
         {
            ifputs(" office:value-type=\"", m_out);
            ifputs(ctype, m_out);
            ifputc('"', m_out);
            
            const char *vtype = ctyper->get_value_attribute();
            if (vtype && strcmp(vtype,"string"))
            {
               ifputs(" office:", m_out);
               ifputs(vtype, m_out);
               ifputs("=\"", m_out);
               obj.print(m_out);
               ifputc('"', m_out);
            }
         }
      }
      ifputs(">\n", m_out);

      if (!obj.is_null())
      {
         ifputs("<text:p>", m_out);
         obj.print_xml_escaped(m_out);
         ifputs("</text:p>\n", m_out);
      }

      ifputs("</table:table-cell>\n", m_out);

   }
   


   
   ifputs("</table:table-row>\n", m_out);
}

void Result_As_FODS::result_complete(int result_number, DataStack<BindC> &ds)
{
}

void Result_As_FODS::report_error(const char *str)
{
}


void Result_As_FODS::doc_element(void) const
{
   ifputs("<office:document", m_out);
   const char **ptr = nspaces;
   while (*ptr)
   {
      ifputs("\n  ", m_out);
      ifputs(*ptr, m_out);

      ++ptr;
   }

   ifputs(">\n", m_out);
}

void Result_As_FODS::start_table(void) const
{
   ifputs("<office:body>\n", m_out);
   ifputs("<office:spreadsheet>\n", m_out);
   ifputs("<table:table>\n", m_out);
}

void Result_As_FODS::end_table(void) const
{
   ifputs("</table:table>\n", m_out);
   ifputs("</office:spreadsheet>\n", m_out);
   ifputs("</office:body>\n", m_out);
}


void Result_As_FODS::end_doc_element(void) const
{
   ifputs("</office:document>\n", m_out);
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

void run_test(MYSQL &mysql)
{
   const char *query = "SELECT * FROM TABLES";

   Result_As_FODS rafods(stdout);

   ifputs("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n", stdout);
   
   rafods.doc_element();
   rafods.start_table();
   
   SimpleProcedure::build(&mysql, query, rafods);
   
   rafods.end_table();
   rafods.end_doc_element();
}



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
