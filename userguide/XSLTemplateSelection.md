# Template Selection

Although XSL includes a somewhat complicated set of rules for determining the best match among several
similar templates, it is possible to design one's templates to be predictably selected using the
simplest of the selection conditions.

The first thing to understand is that, given either a matching or omitted *mode* attribute, an
XSL processor will select the last template that matches the XML node.  In practice, that means
that when the design calls for several different responses to a given node, the templates should
appear in the import-inclusive stylesheet with the simplest case first, followed by *match*
attributes that are more specific.

~~~xsl

<xsl:template match="item" mode="do_process">Error: invalid do_process element</xsl:template>

<xsl:template match="item[@type='difficult']" mode="do_process">
.
.
</xsl:template>

<xsl:template match="item[../@type='easy']" mode="do_process">
.
.
</xsl:template>

~~~

The above example will generate an error string for an ineligible element.  As long as the
*item* elements either include a type='difficult' attribute or have a parent element that
includes a type='easy' attribute, the error-generating template will not be called.

It is easy to override the *do_process* template with a custom template by adding an
[XPath predicate](https://www.w3schools.com/xml/xpath_syntax.asp#div-gpt-ad-1493883843099-0)
that is unique to the custom situation.

~~~xsl
<xsl:template match="item[@ledger]" mode="do_process">
.
.
</xsl:template>
~~~

It is important to ensure that the custom template occurs after other possibly matching templates.
In particular for this example, the custom template must occur after the error-generating template
or the error-generating template will be selected.

Custom templates are typically added in the *default.xsl* stylesheet, which then ensure they follow
the framework's imported templates. If the custom template is contained in an imported stylesheet,
it is important to pay attention to the order in which the different stylesheets are imported.  In
general, put `<xsl:import>` statement for custom stylesheets following the boilerplate import
statements just before the `<xsl:output>` statement.

