# XPath Ranked Selection

## Simple Example

XSL includes a conditional structure to select between several choices.  Consider the following
example of calling a template with the closest element that includes a *form-action* attribute.
The `<xsl:choose>` element stops processing options when a match is made.

~~~xsl
   <xsl:choose>
      <xsl:when test="@form-action">
         <xsl:apply-templates select="." mode="process_form_action">
            <xsl:with-param name="item" select="$item" />
            <xsl:with-param name="status" select="$status" />
            <xsl:with-param name="title" select="$title" />
         </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="../@form-action"
         <xsl:apply-templates select="../@form-action" mode="process_form_action">
            <xsl:with-param name="item" select="$item" />
            <xsl:with-param name="status" select="$status" />
            <xsl:with-param name="title" select="$title" />
         </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>
         <xsl:apply-templates select="/*[@form-action]" mode="process_form_action">
            <xsl:with-param name="item" select="$item" />
            <xsl:with-param name="status" select="$status" />
            <xsl:with-param name="title" select="$title" />
         </xsl:apply-templates>
      </xsl:otherwise>
   </xsl:choose>
~~~

In the above example, the same template is called with the same parameters for each of
the possible elements.  This has two drawbacks:
- There is a long list of statements that makes it hard to read.
- It is easy to introduce hard-to-find bugs because a change to one `<xsl:when>`
  clause must be duplicated in all three `<xsl:when>` clauses.

The above can be written more concisely using several mutually-exclusive proto-variables 
that are combined using a union operator.

~~~xsl
   <xsl:variable name="eact" select=".[@form-action]" />
   <xsl:variable name="nact" select="..[not($eact)][@form-action']" />
   <xsl:variable name="dact" select="/*[not($eact|$nact)][@form-action]" />

   <xsl:apply-templates select="$eact|$nact|$dact" mode="process_form_action">
      <xsl:with-param name="item" select="$item" />
      <xsl:with-param name="status" select="$status" />
      <xsl:with-param name="title" select="$title" />
   </xsl:apply-templates>
~~~

Note that in each proto-variable that follows the first includes an
[XPath predicate](https://www.w3schools.com/xml/xpath_syntax.asp#div-gpt-ad-1493883843099-0)
the prevents acquiring a value if any of the previous proto-variables contain a value.
This structure ensures that only one of the proto-variables contains a value, and thus the
union will have only one value.

This technique is used throughout the framework because I think it's easier to read than the
standard `<xsl:choose>` construction, especially when the user is familiar with technique.
By applying the short-circuit predicate early in the XPath statement should prevent unnecessary
searches and provide similar performance to the <xsl:choose> alternative.

## CSV Parsing Example

The framework uses a form of this technique to parse multiple values from a string of comma-separated
values:

~~~xsl
<xsl:template name="parse_nums">
   <xsl:param name="str" />

   <xsl:variable name="c_id" select="substring-before($str,',')" />
   <xsl:variable name="s_id" select="substring($str,1 div boolean(string-length($c_id)=0))" />
   <xsl:variable name="idval" select="concat($c_id,$s_id)" />

   <xsl:call-template name="process_id">
      <xsl:with-param name="id" select="$idval" />
   </xsl:call-template>

   <xsl:if test="string-length($c_id) &gt; 0">
      <xsl:call-template name="parse_nums">
         <xsl:with-param name="str" select="substring-after($str,',')" />
      </xsl:call-template>
   </xsl:if>
</xsl:template>
~~~

This example follows the same progression of where the proto-variables are made to be empty
of any of the previous proto-variables in the same set have a value.  In this case, *c_id*
will have a value if the *str* parameter includes a comma, that is, when it has multiple
values.  If *str* is a single value, with no commas, *c_id* will be an empty string.

*s_id* must contain a value only when *c_id* is empty.  The `substring()` function call in *s_id*
includes an equation for the second argument that evaluates to either 1 (1 divided by 1) or
infinity (1 divided by 0).  Starting the substring at infinity ensures that the string will
be empty.  This effectively acts like the exclusionary XPath predicate in the first example.

Note that this example combines the string values with `concat()` rather than the union
operator.
