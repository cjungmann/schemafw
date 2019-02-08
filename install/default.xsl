<?xml version="1.0" encoding="utf-8" ?>
<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:html="http://www.w3.org/1999/xhtml"
    exclude-result-prefixes="html">

  <!--
  Only one of the following should be active.  Use sfw_debug.xsl to
  see the latest changes during development.  Use sfw_compiled.xsl
  after running make update-client to compile the import references
  in sfw_debug.xsl.
  -->
  <xsl:import href="includes/sfw_debug.xsl" />
  <!-- <xsl:import href="includes/sfw_compiled.xsl" /> -->
  
  <xsl:output
      method="xml"
      doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
      doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
      version="1.0"
      indent="yes"
      omit-xml-declaration="yes"
      encoding="utf-8"/>

  <xsl:template match="/">
    <html>
      <head>
        <title>**Default SchemaFW Title**</title>

        <!-- SchemaFW includes -->
        <xsl:apply-templates select="." mode="fill_head">
          <!-- Change jscripts variable value to 'min' (or anything other than default)
               to use sfw.min.js.  Make sure that the scripts are minimized first.
               Consult script 'install' to see how it's done with uglify-js. -->
          <xsl:with-param name="jscripts">debug</xsl:with-param>
        </xsl:apply-templates>
      </head>
      <body>
        <div id="SFW_Header">
          <h1>**Default SchemaFW Header**</h1>
          <xsl:apply-templates select="*/navigation" mode="header" />
        </div>
        <div id="SFW_Content">
          <div class="SFW_Host">
            <xsl:call-template name="fill_host" />
          </div>
        </div>
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>
