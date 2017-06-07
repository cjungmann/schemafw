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
        <xsl:apply-templates select="." mode="fill_head" />
      </head>
      <body>
        <div id="SFW_Header">
          <h1>**Default SchemaFW Header**</h1>
          <xsl:apply-templates select="navigation" mode="header" />
        </div>
        <div id="SFW_Content">
          <div class="SFW_Host">
            <xsl:call-template name="fill_host" />
          </div>
        </div>
      </body>
    </html>
  </xsl:template>

  <!-- The following two should not be here.  They should be
       moved to sfwtemplates.xsl or removed entirely. They
       remain only until I decide the best option. -->
  <xsl:template match="row" mode="make_option">
    <xsl:element name="option">
      <xsl:attribute name="value">
        <xsl:value-of select="@value" />
      </xsl:attribute>
      <xsl:value-of select="@label" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="*[@xsl_mode='lookup']">
    <xsl:apply-templates select="row" mode="make_option" />
  </xsl:template>

</xsl:stylesheet>
