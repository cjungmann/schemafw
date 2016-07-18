<?xml version="1.0" encoding="utf-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="includes/sfwtemplates.xsl" />
    
  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="utf-8"/>

  <xsl:param name="sortcol" />


  <xsl:template match="/">
    <html>
      <head>
        <title>SchemaFW Import Application</title>
        
        <!-- SchemaFW includes -->
        <xsl:call-template name="css_includes" />
        <xsl:call-template name="js_includes" />

        <!--
        <link rel="stylesheet" href="session.css" type="text/css" />
        <script type="text/javascript" src="session.js"></script>
        -->
        
      </head>
      <body>
        <div id="SFW_Header">
          <div>
            <a href="?default:logout">Logout</a>
          </div>
          <h1>SchemaFW Import Application</h1>
        </div>
        <div id="SFW_Content">
          <div class="SFW_Host">
            <p>In SFW_Host</p>
            <xsl:apply-templates select="*" mode="showname" />
            <xsl:apply-templates select="*" />
          </div>
        </div>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="*" mode="showname">
    <div><xsl:value-of select="name()" /></div>
  </xsl:template>

  <xsl:template match="resultset">
    <xsl:apply-templates select="." mode="branch_standard_modes" />
  </xsl:template>

  <xsl:template match="import_confirm">
    <xsl:apply-templates select="." mode="branch_standard_modes" />
  </xsl:template>


</xsl:stylesheet>
