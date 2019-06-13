<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_utilities.xsl" />
  <xsl:import href="sfw_variables.xsl" />
  <xsl:import href="sfw_form.xsl" />
  <xsl:import href="sfw_table.xsl" />
  <xsl:import href="sfw_mixed_view.xsl" />

  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="UTF-8"/>

  <!--
  The javascript in the framework accesses the mode-less apply-templates
  element in this template to temporarily set a mode attribute according
  to a selected view.

  This template can be replaced in your own default.xsl file by simply
  defining a template with the same match and mode values.  In this way,
  the rendering of an SFW_Host can be customized according to a project's
  requirements.

  Make sure that any replacement includes an apply-templates element
  with select="." so the framework can find it.
  -->
  <xsl:template match="*[@rndx]" mode="result_fill_host">
    <xsl:param name="primary" />
    <xsl:apply-templates select="." mode="construct_host_top" />
    <xsl:apply-templates select=".">
      <xsl:with-param name="primary" select="$primary" />
    </xsl:apply-templates>
  </xsl:template>

  <!--
  This template is called directly from the framework using the
  transformFill() function with a view element.
  -->
  <xsl:template match="views/view">
    <xsl:variable name="rname" select="@result" />
    <xsl:variable name="result" select="/*/*[@rndx][local-name()=$rname]" />
    <xsl:if test="$result">
      <xsl:apply-templates select="$result" mode="result_fill_sfw_host" />
    </xsl:if>
  </xsl:template>

  <!--
  Fundamental template for filling the initial SFW_Host element
  that should be in a SFW_Content element in the HTML body element.

  This template acts based on global variables found in sfw_variables.xsl.

  Concerning the xsl:choose paths.
  1. Check for pre-empting error message with the $err_condition flag, or
  2. check for schema for building some type of form, or finally,
  3. check for a result to build a table or some custom otuput.
  -->
  <xsl:template name="fill_host">
    <xsl:param name="primary" />
    <xsl:apply-templates select="/*" mode="make_schemafw_meta" />

    <xsl:variable name="cprimary">
      <xsl:choose>
        <xsl:when test="$primary"><xsl:value-of select="$primary" /></xsl:when>
        <xsl:when test="$mode-type='form-page'"><xsl:value-of select="true()" /></xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="$err_condition &gt; 0">
        <xsl:call-template name="display_error" />
        <xsl:apply-templates select="@meta-jump" mode="make_jump_link" />
      </xsl:when>
      <xsl:when test="/*[$mode-type='mixed-view']">
        <xsl:apply-templates select="/*" mode="construct_mixed_view">
          <xsl:with-param name="primary" select="$primary" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="$gschema">
        <xsl:apply-templates select="$gschema" mode="construct_form">
          <xsl:with-param name="primary" select="$cprimary" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="$gresult">
        <xsl:apply-templates select="$gresult" mode="result_fill_host">
          <xsl:with-param name="primary" select="$cprimary" />
        </xsl:apply-templates>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="construct_host_top">
    <xsl:apply-templates select="." mode="construct_host_title" />
    <xsl:if test="/*/views">
      <xsl:apply-templates select="/*/views" mode="construct_view_set" />
    </xsl:if>
  </xsl:template>

  <!--
      Select and present a title, if found.  Weighted selection, giving highest
      priority to the title attribute of the matched result, then to the title
      attribute of the schema (if found), then to the title attribute of the
      document element.

      Variables starting "r" are for result, "s" for schema, and "d" for document.
      The value to use starts with "l" for local.
  -->
  <xsl:template match="*[@rndx]" mode="construct_host_title">
    <xsl:param name="schema" select="/.." />
    
    <xsl:variable name="rschema" select="schema[not($schema)]" />
    <xsl:variable name="dschema" select="/*[not($schema|$rschema)]/schema" />
    <xsl:variable name="lschema" select="$schema|$rschema|$dschema" />

    <xsl:variable name="rtitle" select="@title" />
    <xsl:variable name="stitle" select="$lschema[not($rtitle)]/@title" />
    <xsl:variable name="dtitle" select="/*[not($rtitle|$stitle)]/@title" />
    <xsl:variable name="ltitle" select="$rtitle|$stitle|$dtitle" />

    <xsl:variable name="rstitle" select="@subtitle" />
    <xsl:variable name="sstitle" select="$lschema[not($rstitle)]/@subtitle" />
    <xsl:variable name="dstitle" select="/*[not($rstitle|$sstitle)]/@subtitle" />
    <xsl:variable name="lstitle" select="$rstitle|$sstitle|$dstitle" />

    <xsl:if test="$ltitle">
      <xsl:call-template name="construct_title">
        <xsl:with-param name="str" select="$ltitle" />
      </xsl:call-template>
    </xsl:if>

    <xsl:if test="$lstitle">
      <h3>
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="$lstitle" />
        </xsl:call-template>
      </h3>
    </xsl:if>
  </xsl:template>

  <xsl:template match="views" mode="construct_view_set">
    <xsl:param name="view" select="/.." />

    <xsl:variable name="lview" select="$view | $gview/self::*[not($view)]" />

    <nav class="views">
      <xsl:if test="$lview">
        <xsl:apply-templates select="*" mode="construct_view_choice">
          <xsl:with-param name="view" select="$lview" />
        </xsl:apply-templates>
      </xsl:if>
    </nav>
  </xsl:template>

  <xsl:template match="views/view" mode="construct_view_choice">
    <xsl:param name="lview" select="/.." />
    <xsl:element name="div">
      <xsl:attribute name="data-name"><xsl:value-of select="@name" /></xsl:attribute>
      <xsl:attribute name="class">
        <xsl:text>view_selector</xsl:text>
        <xsl:if test="$lview and generate-id()=generate-id($lview)"> selected</xsl:if>
      </xsl:attribute>
      <xsl:value-of select="@label" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="views" mode="build">
    <xsl:param name="view" />
    <nav class="views">
      <h2><xsl:value-of select="$view/@title" /></h2>
      <xsl:if test="$view/@subtitle">
        <h3><xsl:apply-templates select="$view/@subtitle" mode="resolve_refs" /></h3>
      </xsl:if>
      <xsl:apply-templates select="view" mode="add_choice">
        <xsl:with-param name="current" select="$view" />
      </xsl:apply-templates>
    </nav>
  </xsl:template>

  <xsl:template match="view" mode="construct_view">
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


  <xsl:template match="/">
    <html>
      <head><title>Testing</title></head>
      <body>
        <pre>
          view count   : <xsl:value-of select="count($gview)" />
          result count : <xsl:value-of select="count($gresult)" />
        </pre>
        <div class="SFW_Host">
          <xsl:call-template name="fill_host" />
        </div>
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>
