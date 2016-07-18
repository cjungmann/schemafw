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
        <title>Schema Allowance Application Demonstration</title>
        
        <link rel="stylesheet" href="allowance.css" type="text/css" />

        <!-- SchemaFW includes -->
        <xsl:call-template name="css_includes" />
        <xsl:call-template name="js_includes" />

        <script type="text/javascript" src="allowance.js"></script>
        
      </head>
      <body>
        <div id="SFW_Header">
          <h1>Schema Allowance Application Demonstration</h1>
        </div>
        <div id="SFW_Content">
          <div class="SFW_Host">
            <xsl:apply-templates select="resultset" />
          </div>
        </div>
      </body>
    </html>
  </xsl:template>


  <xsl:template match="resultset[@docflag='person_accounts']">
    <xsl:variable name="person" select="ref/person" />

    <h1>Accounts for <xsl:value-of select="$person/@nom" /></h1>

    <xsl:apply-templates select="accounts" mode="make_table" />
  </xsl:template>

  <xsl:template match="resultset">
    <xsl:apply-templates select="." mode="branch_standard_modes" />
  </xsl:template>

</xsl:stylesheet>
