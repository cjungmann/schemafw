# Custom Fields

Custom fields, while similar to other custom interactions, require activating
more hooks to be integrated into standard forms.  This page will explain what
steps are necessary for incorporating a new field type into your project without
having to modify the core SchemaFW code.

Keep in mind that an unusual data field may require custom fields in both the
form and table environments.

## Requirements

As with other SchemaFW customization, field customization will require both
XSL templates and Javascript classes.

## Framework Example

Look at files *sfw_select.xsl* and *sfw_select.js* in the _includes_ directory
of an installed SchemaFW application for an example of how form and table fields
can be customized.

### XSL Templates

#### Custom Form Fields

The form-building template calls input templates with the *mode="construct_input"*
A custom template may be a simple as this:

~~~xsl
<xsl:template match="field[@type='myslider']" mode="construct_input">
   <xsl:param name="data" />
   <xsl:apply-templates select="." mode="construct_my_slider">
      <xsl:with-param name="data" select="$data" />
   </xsl:apply-templates>
</xsl:template>
~~~

where a separate xsl file may contain, along with supporting templates, the following
template:
~~~xsl
<xsl:template match="field[@type='myslider'] mode="construct_my_slider">
.
.
.
</xsl:template>
~~~

#### Custom Table Fields

Unless the custom data field is hidden or omitted in the table view, it is likely that
a custom data will need its own custom table field rendering template.  There are
several template modes which can be overridden.  Which to use depends on how much
customization will be required.  The following list is in the order the templates are
called:

1. **mode="construct_line_cell"** creates the _td_ element and sets its class name
   according to the settings in the SRM file.  This template calls:
2. **mode="write_cell_content"**, which handles a BOOL type explicitly, or calls
3. **mode="get_value"** to get a string value.

When designing the template, make sure the _match_ field matches the schema field
of your custom type.  Refer to the form field discussion above for an example.

In nearly every case, the *mode="write_cell_content"* is the correct choice.
This template can be used to fill the cell with anything, from a simple string
to a collection of elements, or even an image or SVG graphic.  Implement a
*mode="construct_line_cell"* template if you need more control over the _td_ element.
Implement a *mode="get_value"* template if your data is a simple string and you
want to preserve any future framework settings that may be added to
*mode="write_cell_content"*.

#### Ensuring Template Selection

The schema fields determine which template will be used to render the contents of
the field.  The XSL processor will select the highest priority matching template, and
if there are multiple templates of the identical highest priority, it will select
the last matching template.  **It is important to ensure that a custom template
can be selected according to the XSL rules.**
(See [Templates for Custom Interactions](CustomTemplates.md).)  The easiest way to
ensure the selection of custom templates is to have them reside in *default.xsl*,
perhaps in a shell template that calls a template in another file with a _mode_
setting.

