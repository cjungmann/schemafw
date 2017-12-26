# XSL Programming

The Schema Framework uses XSL stylesheets to generate HTML by processing the XML data returned by
the *schema.fcgi* server.

XSL processing uses templates to transform XML into other forms.  The XSL processor chooses templates
to apply to XML nodes based on how the **match** and **mode** attributes of the template relate
to any given node.  An entire HTML page can be built when successive templates are applied to related
nodes of the node by which each template is invoked.

The Schema Framework includes a large and growing inventory of XSL templates that, along with the
framework's javascript library, can make easy work of developing a sophisticated database-centric
web application.  If a web application consists of only tables and forms, it is possible to create
an entire application without any knowledge of or expertise with XSL beyond using a boilerplate
XSL document to prepare the basic page.

However, the framework rewards a developer who is willing to embrace the challenges of XSL programming.
A developer can exploit the XSL rules for selecting templates to override the built-in XML rendering
with custom templates that, for example, might draw a pie-chart in place of a table of values.

The rules by which the XSL processor selects templates are not overly complicated, but they are
unfamiliar to most programmers. This section of the [User Guide](userguide.md) will cover different
XSL techniques that may be helpful for XSL developers.

- [Template Selection](XSLTemplateSelection.md)
- [XPath Ranked Selection](XPathRankedSelection.md)