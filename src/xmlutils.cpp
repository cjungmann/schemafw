#include "xmlutils.hpp"

void print_xml_head(const char *stylesheet_name, FILE *fp)
{
   fputs("<?xml version=\"1.0\" ?>\n", fp);
   if (stylesheet_name)
      fprintf(fp,"<?xml-stylesheet type=\"text/xsl\" href=\"%s\" ?>\n",
              stylesheet_name);
}
