// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_CTYPER_MAIN `mysql_config --cflags` -o ctyper ctyper.cpp `mysql_config --libs`" -*-

#include "ctyper.hpp"
#include <assert.h>

CTyper CTyper::s_instance;
const char* _CType::s_allowed_types = "ijklIJKLntdfsb";

/**
 * @brief Array of type bundles so a single search yields complete type information.
 *
 * I had too many switch statements to get iclass_casters, MYSQL_TYPEs,
 * type names, etc.  I created the CTyper class to provide access to these
 * _CType instances so a program can get everything it needs to deal with
 * a data object with a once-and-for-all search.
 *
 * The array elements are arranged so that the first group of elements
 * match to unique class_types (vclasses.hpp) enumerations.
 *
 * Following the first group of unique types are other types that use the
 * same iclass_caster under different names.  Searches using these alternate
 * names will return an appropriate set of values for accessing the data.
 *
 * I used the SQL script get_possible_param_types.sql to create the list of
 * types that I wanted to match.
 */
const _CType CTyper::s_types[] = {
   { MYSQL_TYPE_TINY, false, CT_INT_TINY, _CType::TYPE_TINY,
     sizeof(int8_t), ai_tiny::cast_and_use, "BOOL", "bool" },

   { MYSQL_TYPE_TINY, false, CT_INT_TINY, _CType::TYPE_TINY,
     sizeof(int8_t), ai_tiny::cast_and_use, "TINYINT", "tinyint" },
   { MYSQL_TYPE_TINY, true, CT_UINT_TINY, _CType::TYPE_UTINY,
     sizeof(int8_t), ai_utiny::cast_and_use, "TINYINT", "tinyint" },

   { MYSQL_TYPE_SHORT, false, CT_INT_SHORT, _CType::TYPE_SHORT,
     sizeof(int16_t), ai_short::cast_and_use, "SMALLINT", "shortint" },
   { MYSQL_TYPE_SHORT, true, CT_UINT_SHORT, _CType::TYPE_USHORT,
     sizeof(int16_t), ai_ushort::cast_and_use, "SMALLINT", "shortint" },

   { MYSQL_TYPE_LONG, false, CT_INT_LONG, _CType::TYPE_LONG,
     sizeof(int32_t), ai_long::cast_and_use, "INT", "int" },
   { MYSQL_TYPE_LONG, true, CT_UINT_LONG, _CType::TYPE_ULONG,
     sizeof(int32_t), ai_ulong::cast_and_use, "INT", "int" },

   { MYSQL_TYPE_LONGLONG, false, CT_INT_LONGLONG, _CType::TYPE_LONGLONG,
     sizeof(int64_t), ai_longlong::cast_and_use, "BIGINT", "bigint" },
   { MYSQL_TYPE_LONGLONG, true, CT_UINT_LONGLONG, _CType::TYPE_ULONGLONG,
     sizeof(int64_t), ai_ulonglong::cast_and_use, "BIGINT", "bigint" },

   { MYSQL_TYPE_FLOAT, false, CT_FLOAT_FLOAT, _CType::TYPE_FLOAT,
     sizeof(float), ai_float::cast_and_use, "FLOAT", "float" },
   { MYSQL_TYPE_DOUBLE, false, CT_FLOAT_DOUBLE, _CType::TYPE_DOUBLE,
     sizeof(double), ai_double::cast_and_use, "DOUBLE", "double" },

   { MYSQL_TYPE_DATE, false, CT_TIME_DATE, _CType::TYPE_TIME,
     sizeof(MYSQL_TIME), ai_date::cast_and_use, "DATE", "date" },
   { MYSQL_TYPE_TIME, false, CT_TIME_TIME, _CType::TYPE_TIME,
     sizeof(MYSQL_TIME), ai_date::cast_and_use, "TIME", "time" },
   { MYSQL_TYPE_DATETIME, false, CT_TIME_DATE, _CType::TYPE_TIME,
     sizeof(MYSQL_TIME), ai_date::cast_and_use, "DATETIME", "datetime" },
   { MYSQL_TYPE_TIMESTAMP, true, CT_TIME_DATE, _CType::TYPE_TIME,
     sizeof(MYSQL_TIME), ai_date::cast_and_use, "TIMESTAMP", "timestamp" },
   
   { MYSQL_TYPE_VAR_STRING, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "VARCHAR", "varchar" },

   // The following have less common unique names, and use one of the
   // iclass_casters in the above elements.

   // YEAR is a short:
   { MYSQL_TYPE_YEAR, true, CT_INT_SHORT, _CType::TYPE_SHORT,
     sizeof(int16_t), ai_short::cast_and_use, "YEAR", "year" },

   // Types that are represented by a string:
   { MYSQL_TYPE_VARCHAR, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "VARCHAR", "varchar" },
   { MYSQL_TYPE_STRING, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "CHAR", "char" },

   // There are no MYSQL_TYPE_TEXTs, only MYSQL_TYPE_BLOBs,
   // despite parameter data_types text/blob, tinytext/tinyblob, etc.
   { MYSQL_TYPE_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "TEXT", "text" },
   { MYSQL_TYPE_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "BLOB", "blob" },
   { MYSQL_TYPE_TINY_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "TINYTEXT", "tinytext" },
   { MYSQL_TYPE_TINY_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "TINYBLOB", "tinyblob" },
   { MYSQL_TYPE_MEDIUM_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "MEDIUMTEXT", "mediumtext" },
   { MYSQL_TYPE_MEDIUM_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "MEDIUMBLOB", "mediumblob" },
   { MYSQL_TYPE_LONG_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "LONGTEXT", "longtext" },
   { MYSQL_TYPE_LONG_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "LONGBLOB", "longblob" },

   { MYSQL_TYPE_ENUM, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "ENUM", "enum" },
   { MYSQL_TYPE_SET, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "SET", "set" },
   { MYSQL_TYPE_DECIMAL, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "DECIMAL", "decimal" },
   { MYSQL_TYPE_NEWDECIMAL, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "DECIMAL", "decimal" },
   { MYSQL_TYPE_TINY_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "TINY_BLOB", "blob" },
   { MYSQL_TYPE_MEDIUM_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "MEDIUM_BLOB", "blob" },
   { MYSQL_TYPE_LONG_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "LONG_BLOB", "blob" },
   { MYSQL_TYPE_BLOB, false, CT_STRING_TEXT, _CType::TYPE_STRING,
     0, ai_text::cast_and_use, "BLOB", "blob" },

   { MYSQL_TYPE_NULL, false, CT_NULL, _CType::TYPE_NULL,
     0, ai_text::cast_and_use, "NULL", "null" }

   // skipping MYSQL_TYPE_INT24, MYSQL_TYPE_BIT, and MYSQL_TYPE_GEOMETRY
};

const int CTyper::s_count = sizeof(CTyper::s_types) / sizeof(_CType);
const _CType* CTyper::s_end = &s_types[CTyper::s_count];



/**
 * @brief Return a _CType object from the array.  Throw if not found.
 *
 * @todo It would be more efficient to jump out or inline to avoid allocating memory for buff.
 */
const _CType *CTyper::get(_CType::Types t)
{
   const _CType *ptr = s_types;
   while (ptr < s_end)
   {
      if (ptr->m_stype==t)
         return ptr;
      ++ptr;
   }

   char buff[80];
   snprintf(buff, 80, "CTyper attempted to get unknown _CType::Types==%c", static_cast<char>(t));
   throw std::runtime_error(buff);
}

/**
 * @brief Return a _CType object from the array.  Throw if not found.
 *
 * @todo It would be more efficient to jump out or inline to avoid allocating memory for buff.
 */
const _CType *CTyper::get(class_types t)
{
   const _CType *ptr = s_types;
   while (ptr < s_end)
   {
      if (ptr->m_ctype==t)
         return ptr;
      ++ptr;
   }

   char buff[80];
   snprintf(buff, 80, "CTyper attempted to get unknown class_types==%d", t);
   throw std::runtime_error(buff);
}

/**
 * @brief Return a _CType object from the array.  Throw if not found.
 *
 * @todo It would be more efficient to jump out or inline to avoid allocating memory for buff.
 */
const _CType *CTyper::get(enum_field_types t, bool is_unsigned)
{
   const _CType *ptr = s_types;
   while (ptr < s_end)
   {
      if (ptr->m_sqltype==t && ptr->m_is_unsigned==is_unsigned)
         return ptr;

      ++ptr;
   }
   
   char buff[80];
   snprintf(buff, 80, "CTyper attempted to get unknown enum_field_types==%d", t);
   throw std::runtime_error(buff);
}

/**
 * @brief A getter function that works with information_schema.PARAMETERS fields.
 *
 * The data_type parameter is sufficient, except for integers that might be
 * unsigned.  If the last three characters of data_type are "int", we look
 * compare the final 8 characters of dtd_identifier with "unsigned" to decide
 * if the type is unsigned.
 *
 * The unsigned characteristic is not important for lengths (uint16_t is the
 * same length as int16_t), so it is OK to 
 */
const _CType *CTyper::get(const char *data_type, const char *dtd_identifier)
{
   bool is_unsigned = false;
   const _CType *ptr = s_types;
   
   if (dtd_identifier && strncmp(dtd_identifier,"tinyint(1)",10)==0)
   {
      // This should be the first in the list, scan list just in case:
      while (ptr < s_end)
      {
         if (0==strcmp("bool", ptr->m_param_data_type))
            return ptr;
         ++ptr;
      }
      
   }

   // A null dtd_identifier means we're not checking for signed, so check
   // it first as a short-circuit for the conditional:
   if (dtd_identifier && strstr(data_type,"int") && strstr(dtd_identifier,"unsigned"))
      is_unsigned = true;

   while (ptr < s_end)
   {
      if (0==strcmp(data_type, ptr->m_param_data_type))
         if (ptr->m_is_unsigned==is_unsigned)
            return ptr;

      ++ptr;
   }
   
   char buff[80];
   snprintf(buff, 80, "CTyper attempted to get unknown data_type==\"%s\"", data_type);
   throw std::runtime_error(buff);
}


/**
 * @brief Make a type string from an array.
 */
void t_make_typestr(const IClass **array,
                    int len_array,
                    const IGeneric_String_Callback &gsc)
{
   const _CType *ct;
   const IClass **alimit = array + len_array;
   int len_typestr = 0;

   for (const IClass **aptr=array; aptr<alimit; aptr++)
   {
      ++len_typestr;

      if ((*aptr)->vtype()==CT_STRING_TEXT)
      {
         size_t len = (*aptr)->data_length();
         size_t comp = 10;
         // count digits needed to represent len, up to 99,999:
         for (int i=1;  i<6;  ++i, comp*=10)
            if (len<comp)
            {
               len_typestr += i;
               break;
            }
         // Longer than 5 digits should leave length unspecified
      }
   }

   if (len_typestr)
   {
      char *typestr = static_cast<char*>(alloca(len_typestr+1));
      char *tptr = typestr;
      
      for (const IClass **aptr=array; aptr<alimit; aptr++)
      {
         if ((ct=CTyper::get(*aptr)))
         {
            *tptr = ct->get_stype();
            if (*tptr=='s')
            {
               ++tptr;
               size_t len = (*aptr)->data_length();
               // Match limit above, up to 5-digits, to prevent buffer overflow
               if (len < 100000)
                  tptr += sprintf(tptr, "%lu", len);
            }
            else
               ++tptr;
         }
      }

      // Assert accurate length calculation:
      assert(tptr == typestr+len_typestr);
      
      *tptr = '\0';

      gsc(typestr);
   }
   else
      gsc("");
}


#ifdef INCLUDE_CTYPER_MAIN
#include <stdio.h>
#include "vclasses.cpp"
#include "prandstr.cpp"


int main(int argc, char** argv)
{
   return 0;
}



#endif




