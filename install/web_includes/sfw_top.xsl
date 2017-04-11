<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="UTF-8"/>


  <xsl:template match="navigation/target" mode="header">
    <a href="{@url}"><xsl:value-of select="@label" /></a>
  </xsl:template>

  <xsl:template match="/*/navigation" mode="header">
    <xsl:if test="count(target)">
      <nav><xsl:apply-templates select="target" mode="header" /></nav>
    </xsl:if>
  </xsl:template>

  <xsl:variable name="view_name">
    <xsl:variable name="eview" select="/*/views/view[@selected]" />
    <xsl:variable name="fview" select="/*/views/view[not($eview)][1]" />
    <xsl:value-of select="$eview|$fview" />
  </xsl:variable>

  <xsl:template match="views/view" mode="add_choice">
    <xsl:param name="current" />
    <xsl:element name="div">
      <xsl:attribute name="data-name"><xsl:value-of select="@name" /></xsl:attribute>
      <xsl:attribute name="class">
        <xsl:text>view_selector</xsl:text>
        <xsl:if test="generate-id()=generate-id($current)"> selected</xsl:if>
      </xsl:attribute>
      <xsl:value-of select="@label" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="views" mode="build">
    <xsl:param name="view" />
    <nav class="views">
      <h2><xsl:value-of select="$view/@title" /></h2>
      <xsl:apply-templates select="view" mode="add_choice">
        <xsl:with-param name="current" select="$view" />
      </xsl:apply-templates>
    </nav>
  </xsl:template>

  <xsl:template match="view" mode="build">
    <xsl:apply-templates select=".." mode="build">
      <xsl:with-param name="view" select="." />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="show_views">
    <xsl:variable name="view" select="*/views/view[@name=current()/@view]" />
    <xsl:if test="$view">
      <xsl:apply-templates select="/*/view" mode="build">
        <xsl:with-param name="view" select="$view" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="/*/views" mode="header">
    <xsl:param name="view" />

    <xsl:variable name="defview" select="view[not($view)][@default][1]" />
    <xsl:variable name="use_view"
                  select="$view|$defview|view[not($view|$defview)][1]" />

    <xsl:if test="$use_view">
      <xsl:apply-templates select="." mode="build">
        <xsl:with-param name="view" select="$use_view" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="header"></xsl:template>


</xsl:stylesheet>
