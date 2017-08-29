# Customizing Titles

Beyond the ability to include several [Context References](ContextReferences.md) in
titles as many other places, it is possible to have override a standard template to
create a title from scratch.

## Find the Template

The standard title-generating template is found in *sfw_form.xsl*:

~~~xsl
<xsl:template match="schema" mode="get_form_title">
   .
   .
   .
</xsl:template>
~~~

## A Calendar Example

Our example will demonstrate how to generate a complete date description from
different pieces of information in a merged form.
Add a similar template in your *default.xsl* file with very a very explicit match
is previously setup.

~~~xsl
<xsl:template match="schema[@name='my_special_schema']" mode="get_form_title">
   <xsl:variable name="calinfo" select="/*/calendar/row" />
   <xsl:variable name="dayinfo" select="/*/qstring/row/@add" />
   <xsl:text>Adding Text for </xsl:text>
   <xsl:apply-templates select="$calinfo" mode="make_dmdy">
      <xsl:with-param name="date" select="$dayinfo" />
   </xsl:apply-templates>
</xsl:template>
~~~

This would go with main and merged response modes that look like this:


~~~srm
# The following is incomplete, missing much except for points of immediate interest.
main
   type    : table
   procedure : App_Event_Page
   # named result to aid discovery
   result : 1
      name : calendar
   result : 2
      name : events
      lookup : name
      

add
   type        : merge
   schema_proc : App_Add_Event
   form-action : add_submit
   qstring     : reflect  # this makes a result named qstring with GET params
   schema
      # Rename result to aid discovery (matches the XSL above):
      name       : my_special_schema
      merge-type : form-new
      field : ddate
         value  : {$add}
         hidden : true
~~~


