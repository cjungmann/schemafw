# Framework Template Overrides

The framework is designed to allow customization.  Some customizations are
complicated, especially those that will define new responses and thus require a
custom class derived from existing classes.  Other customizations can be accomplished
by simply adding an XSL template designed to be preferentially selected over standard
templates.

Some template override protocols are reasonably well-defined.  For these, there are
examples in the framework that actually take advantage of these overrides.  These
mainly include table cells and form input/display elements.  Table rows, in particular,
have not been overriden at the time of this guide (2017/12/28).

## How to Create an Override

XSL selects a template to render data based on template attributes.  The most commonly
used attributes are the **match** and **mode** attributes.  The **match** attribute
is used when `<xsl:apply-templates ..>` is invoked with one or many elements.  The
`xsl:apply-templates` call can optionally include a **mode** attribute which is simply
a string.

The XSL processor will render data using the last matching template in a stylesheet.
In practice, this means that the least restrictive **match** values should come first,
and more restrictive templates should follow.

In the following example, there are three templates with the same **mode** attribute
value (do_something).  The **match** attribute for all three is for a *field* element,
but the each successive template has a more explicit **match** attribute to distiguish
the templates.

~~~xsl
<xsl:template match="field" mode="do_something"></xsl:template>

<xsl:template match="field[@name='fname']" mode="do_something">
   .
   .
</xsl:template>

<xsl:template match="schema[@name='house']/field[@name='fname']" mode="do_something">
   .
   .
</xsl:template>
~~~

We will consider the possibilities in reverse order of the templates.

All three templates match an `xsl:apply-templates` call with a field element named
*fname* in a schema named *house*.  The XSL processor will use the third template
because it is last match.

Only the first two templates will match if the `xsl:apply-templates` selects a field
element named *fname* from a schema named *employee*.  The third template is eliminated
from consideration by the mismatched *schema* parent.  The XSL processor will select
the second template because it is the last matching template in the stylesheet.  Thus,
the second template is the default handler of fields named *fname*.

The first template will be the only match if the field name is not *fname*.  This is
the default handler for any field when applied with the *do_something* mode.

## Adding Custom Templates to Your Project

Given the need to position more explicit template matches below existing templates,
there are two ways to include new templates, both of which involve making changes
to the *default.xsl* file.

NOTE: When making changes to *default.xsl*, the modified copy can be used as is,
or renamed to maintain a unmodified *default.xsl* file.  If the name is changed,
the *$xml-stylesheet* instruction in the SRM files must also be changed to match
the new stylesheet name.

The first way to add custom templates is to simply add the new templates to *default.xsl*.
Make sure the new templates follow the `<xsl:import ../>` statement that comes before
the `<xsl:output ../>` element.

The second way is to add an `<xsl:import ../>` element following the framework's
`<xsl:import ../>` element.  The following example shows where to place a new import
element that references a stylesheet with custom templates named *custom_templates.xsl*:

~~~xsl
<xsl:import href="includes/sfw_compiled.xsl" />
<xsl:import href="custom_templates.xsl" />
~~~

## Basic Override Templates

The framework primarily displays data as forms or tables.  Both forms and tables
a collections of elements generated from the XML data.  

### Table Overrides

#### Row Overrides

The construction of a table row is handled by a template with **construct_tbody_row**
mode.  This template handles many tasks and should be refactored to allow for easy
override.  In particular, *construct_tbody_row* adds many critical attributes to the
**tr** element that would need to be duplicated by an overriding template.

It is not clear that there would be any benefit to overriding the construction of the
**tr** element, so this has not been developed.  It is included in this guide for
completeness.

~~~xsl
<xsl:template match="*" mode+"construct_tbody_row">
   <xsl:param name="line_id" />
   .
   .
</xsl:template>
~~~

#### Cell Overrides

This is where most table overrides should be performed.  All calls to these templates
include a data parameter that contains the XML data element from the table row is
being constructed.  It is up to the template to extract the cell value from the data
parameter using the matched *field* element.  The examples below include an 
*xsl:variable* element that extracts the value.

##### Build the **td** Element

If a customization requires adding attributes to the **td** element, the
**construct_line_cell** template should be overridden.  After preparing the **td**
element, the custom template can either go ahead and fill the cell, or delegate
that task to a **write_cell_content** template.

Some examples that would benefit from overriding this template is if the cell
should have a specific CSS class or a data-id attribute.

~~~xsl
<xsl:template match="field" mode="construct_line_cell">
   <xsl:param name="data" />
   <xsl:variable name="value" select="$data/@*[local-name()=current()/@name]" />
   .
   .
</xsl:template>
~~~

##### Fill a **td** Element

This can be as basic as simply generating a text node, or as involved as generating
a subordinate table or even an SVG drawing using linked data.

~~~xsl
<xsl:template match="field" mode="write_cell_content">
   <xsl:param name="data" />
   <xsl:variable name="value" select="$data/@*[local-name()=current()/@name]" />
   .
   .
</xsl:template>
~~~


## Framework Examples

The framework uses the methods described in this guide to expand on basic
table and form elements.  Both files *sfw_ulselect.xsl* and *sfw_lookup.xsl*
define templates with mode = **write_cell_content**.