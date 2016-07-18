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
        <title>SchemaFW Simple Application Demonstration</title>
        
        <link rel="stylesheet" href="demo.css" type="text/css" />

        <!-- SchemaFW includes -->
        <xsl:call-template name="css_includes" />
        <xsl:call-template name="js_includes" />

        <script type="text/javascript" src="demo.js"></script>
        
      </head>
      <body>
        <div id="SWF_Header">
          <h1>SchemaFX Simple Application Demonstration</h1>
          <a href="schema">Home</a>
        </div>
        <div id="SFW_Content">
          <div class="SFW_Host">
            <xsl:apply-templates select="resultset" mode="branch_standard_modes" />
          </div>
        </div>
      </body>
    </html>
  </xsl:template>


</xsl:stylesheet>
