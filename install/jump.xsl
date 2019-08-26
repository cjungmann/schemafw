<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:html="http://www.w3.org/1999/xhtml"
    version="1.0"
    exclude-result-prefixes="html">

  <xsl:import href="includes/sfw_resolver.xsl" />

  <xsl:output
      method="xml"
      doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
      doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
      version="1.0"
      indent="yes"
      omit-xml-declaration="yes"
      encoding="utf-8"/>

  <xsl:variable name="apos">'</xsl:variable>

  <xsl:variable name="payload" select="/*/*[@rndx][@type='jumps']/*[local-name()=../@row-name]" />

  <xsl:variable name="msg" select="$payload/@msg" />

  <xsl:variable name="jval" select="$payload/@jump" />
  <xsl:variable name="eval" select="substring($payload/@error,1 div boolean(0=string-length($jval)))" />
  <xsl:variable name="jump_code" select="concat($jval,$eval)" />

  <xsl:variable name="jumps" select="$payload/../jumps" />
  <xsl:variable name="raw_dest" select="$jumps/@*[local-name()=concat('jump',$jump_code)]" />

  <xsl:variable name="dest">
    <xsl:call-template name="resolve_refs">
      <xsl:with-param name="str" select="$raw_dest" />
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="wait">
    <xsl:choose>
      <xsl:when test="$payload/@msg">3</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>


  <xsl:template match="/">
    <html>
      <head>
        <title>Schema Framework Jump Page</title>

        <!-- Always make a jump -->
        <xsl:call-template name="add-meta-jump" />

        <!-- Only load stylesheet for a message -->
        <xsl:if test="$payload/@msg">
          <link rel="stylesheet" type="text/css" href="includes/schemafw.css" />
        </xsl:if>
      </head>
      <body>
        <xsl:if test="$payload/@msg">
          <h1>Schema Framework Jump Page</h1>
          <p class="def_center">
            <xsl:value-of select="$payload/@msg" />
          </p>
        </xsl:if>

        <p class="def_center">
          <a href="{$dest}">Click on this link if you get stuck on this page.</a>
        </p>
      </body>
    </html>
  </xsl:template>

  <xsl:template name="add-meta-jump">
    <xsl:variable name="content" select="concat($wait, '; url=', $dest)" />

    <meta http-equiv="refresh" content="{$content}" />
  </xsl:template>


</xsl:stylesheet>
