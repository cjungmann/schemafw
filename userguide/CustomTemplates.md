# Templates for Custom Interactions

The ease-of-use potential is one the characteristics of XSL stylesheets that makes
it attractive to use this technology for rendering a web page.  It makes it possible
for a custom template to be added and found without having to touch the core stylesheets
that make up the Schema Framework.

The idea is to identify a block of data by naming it in the SRM file, then creating
a template that matches the new name.  When the framework renders the page, the XSL
processor should find and use the appropriate template.

While this idea makes sense, there are some unexpected situations that may make your
carefully-crafted template invisible to the XSL processor and ignored.  I have had some
trouble with this, and recently spent enough time working through it that I have a
better understanding of how it works and some ideas on how to prevent this problem.

## Matching Templates

It starts with matching templates.  When the XSL processor encounters an XML element,
it will search for an appropriate template with which to process the element. Consider
the following:

~~~xml
<root>
   <task name="scour tub" fun="1" difficult="8" conspicuous="2" />
   <task name="clean windows" fun="2" difficult="6" conspicuous="8" />
   <task name="gardening" fun="6" difficult="6" conspicuous="8" />
   <task name="cook dinner" fun="7" difficult="5" conspicuous="4" />
</root>
~~~

~~~xsl
<xsl:stylesheet ...>
   .
   .
   <xsl:variable name="nl" select="'&#xa;'" />
   
   <xsl:template match="task[@fun]">
      <xsl:value-of select="concat('have fun ', @name, $nl)" />
   </xsl:template>

   <xsl:template match="task[@conspicuous]">
      <xsl:value-of select="concat('be seen ', @name, $nl)" />
   </xsl:template>

   <xsl:template match="*[@difficult]">
      <xsl:value-of select="concat('work at ', @name, $nl)" />
   </xsl:template>
</xsl:stylesheet>   
~~~

### Selection by Priority

While all three templates match any of the _task_ elements, the processor must
select only one to process a given element.  It follows certain
[Conflict Resolution](https://www.w3.org/TR/xslt/#conflict) rules to determine
the template or templates that have the highest priority, using the last template
of the highest priority.  In this case, the first two templates have the same
priority, so the processor will use _match="task[@conspicuous]_ to process
the _task_ elements.

## Imported Stylesheets

This aspect of selecting templates has given me a lot of trouble.  Now, with a
better understanding, I offer the following to help explain template selection
problems.

The Schema Framework separates XSL templates into several stylesheets to aid
development and understanding.  The stylesheets are combined using _xsl:import_
(see [Why Not Use xsl:include?](WhyNoInclude.md)).

Imported templates are available or not according to rules detailed
[Stylesheet Import](https://www.w3.org/TR/xslt/#import).  What is not obvious
is that import precedence trumps priority by the _match_ attribute.  Another
way to say that is that an imported template of the highest possible priority
will be ignored in favor a template contained in the importing stylesheet.



