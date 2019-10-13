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
        <xsl:when test="($gschema|$gresult)/@merged"></xsl:when>
        <xsl:when test="$mode-type='form-page'">yes</xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="/*[@custom-type]">
        <xsl:apply-templates select="/*" mode="custom_fill_host" />
      </xsl:when>
      <xsl:when test="$err_condition &gt; 0">
        <xsl:call-template name="display_error" />
        <xsl:apply-templates select="@meta-jump" mode="make_jump_link" />
      </xsl:when>
      <xsl:when test="/*[$mode-type='mixed-view']">
        <xsl:apply-templates select="/*" mode="construct_mixed_view">
          <xsl:with-param name="primary" select="$primary" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="$gschema[not(.. = $gresult)]">
        <!-- 
             We've matched a schema that is not hosted
             in a result (an empty form like a login form)
        -->
        <xsl:apply-templates select="$gschema" mode="construct_host_title" />
        <xsl:apply-templates select="$gschema" mode="construct_form">
          <xsl:with-param name="primary" select="$cprimary" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="$gresult">
        <xsl:apply-templates select="$gresult" mode="result_fill_host">
          <xsl:with-param name="primary" select="$cprimary" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>
        <div>"fill_host" template cannot deduce source of host contents.  Including a schema may help.</div>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="construct_host_top">
    <xsl:apply-templates select="." mode="construct_host_title" />
    <xsl:apply-templates select="." mode="show_intro" />
    
    <xsl:if test="/*/views">
      <xsl:apply-templates select="/*/views" mode="construct_view_set" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="show_intro">
    <xsl:param name="class" />

    <xsl:variable name="result" select="descendant-or-self::*[@rndx]" />
    <xsl:variable name="schema" select="self::schema|$result/schema" />
    <xsl:variable name="bottom" select="$schema | $result[not($schema)]" />

    <xsl:variable name="intro_str"
                  select="($bottom/ancestor-or-self::*/@intro)[last()]" />

    <xsl:if test="string-length($intro_str)">
      <xsl:variable name="classname">
        <xsl:text>fixed_head opaque</xsl:text>
        <xsl:if test="$class">
          <xsl:value-of select="concat(' ',$class)" />
        </xsl:if>
      </xsl:variable>

      <div class="{$classname}">
        <div class="intro">
          <xsl:call-template name="resolve_refs">
            <xsl:with-param name="str" select="$intro_str" />
          </xsl:call-template>
        </div>
      </div>

      <div class="{$classname} ghost" />
    </xsl:if>
    
  </xsl:template>


  <!--
      Simpler yet more generic logic for determining highest priority
      among a hierarchy of elements for rendering host title elements.
  -->
  <xsl:template match="*" mode="construct_host_title">
    <xsl:variable name="result" select="descendant-or-self::*[@rndx]" />
    <xsl:variable name="schema" select="self::schema|$result/schema" />
    <xsl:variable name="bottom" select="$schema | $result[not($schema)]" />

    <xsl:if test="$bottom">
      <xsl:variable name="title" select="($bottom/ancestor-or-self::*/@title)[last()]" />
      <xsl:variable name="subtitle" select="($bottom/ancestor-or-self::*/@subtitle)[last()]" />

      <xsl:if test="$title">
        <h2 class="fixed_head">
          <xsl:call-template name="resolve_refs">
            <xsl:with-param name="str" select="$title" />
          </xsl:call-template>
        </h2>
      </xsl:if>

      <xsl:if test="$subtitle">
        <h3>
          <xsl:call-template name="resolve_refs">
            <xsl:with-param name="str" select="$subtitle" />
          </xsl:call-template>
        </h3>
      </xsl:if>
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
      <h2 class="fixed_head"><xsl:value-of select="$view/@title" /></h2>
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
