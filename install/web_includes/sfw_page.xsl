<?xml version="1.0" encoding="utf-8" ?>

<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_utilities.xsl" />

  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="UTF-8"/>

  <xsl:variable name="result-row"
                select="/*[@mode-type='form-result']/*[@rndx=1]/*[@error]" />

  <xsl:variable name="docel_msg" select="/message" />
  <xsl:variable name="child_msg" select="/*[not($docel_msg)]/message" />
  <xsl:variable name="msg-el" select="$docel_msg | $child_msg" />

  <xsl:variable name="jslist_sfw_brief">sfw</xsl:variable>
  <xsl:variable name="jslist_sfw_minified">sfw.min</xsl:variable>
  <xsl:variable name="jslist_sfw_debug">sfw_0 sfw_dom sfw_table sfw_form sfw_form_view sfw_calendar sfw_debug sfw_onload</xsl:variable>
  <xsl:variable name="jslist_sfw" select="$jslist_sfw_debug" /> 
  <xsl:variable name="jslist_utils">classes dpicker Events Dialog Moveable XML</xsl:variable>

  <xsl:variable name="err_condition">
    <xsl:choose>
      <xsl:when test="$result-row and $result-row/@error&gt;0">1</xsl:when>
      <xsl:when test="$msg-el and $msg-el/@type='error'">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="view_name">
    <xsl:variable name="eview" select="/*/views/view[@selected]/@name" />
    <xsl:variable name="fview" select="/*/views/view[not($eview)][1]/@name" />
    <xsl:value-of select="$eview|$fview" />
  </xsl:variable>

  <xsl:variable name="curView" select="/*/views/view[@name=$view_name]" />
  <xsl:variable name="viewResult" select="/*/*[@rndx][local-name()=$curView/@result]" />

  <xsl:template match="/*[substring-before(@mode-type,'-')='form']">
    <xsl:apply-templates select="schema" mode="construct_form" />
  </xsl:template>

  <xsl:template match="/*" mode="construct_view">
    <xsl:apply-templates select="." mode="make_schemafw_meta" />

    <xsl:variable name="result" select="$viewResult | *[not($viewResult)][@rndx][1]" />
    <xsl:choose>
      <xsl:when test="$err_condition&gt;0">
        <xsl:call-template name="display_error" />
        <xsl:apply-templates select="@meta-jump" mode="make_jump_link" />
      </xsl:when>
      <xsl:when test="schema">
        <xsl:apply-templates select="schema" mode="construct_form">
          <xsl:with-param name="primary" select="1" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="$result">
        <xsl:apply-templates select="$result">
          <xsl:with-param name="primary" select="1" />
        </xsl:apply-templates>
      </xsl:when>
        
    </xsl:choose>
    
  </xsl:template>

  <xsl:template name="display_error">
    <xsl:choose>
      <xsl:when test="$result-row and $result-row/@error&gt;0">
        <p class="result-msg"><xsl:value-of select="$result-row/@msg" /></p>
      </xsl:when>
      <xsl:when test="$msg-el and $msg-el/@type='error'">
        <xsl:apply-templates select="$msg-el" />
      </xsl:when>
      <xsl:otherwise><p>Undefined error</p></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="add_scripts_and_stylesheets">
    <xsl:call-template name="js_includes" />
    <xsl:call-template name="css_includes" />
  </xsl:template>

  <xsl:template name="js_includes">
    <xsl:call-template name="add_js">
      <xsl:with-param name="list" select="$jslist_sfw" />
    </xsl:call-template>
    <xsl:call-template name="add_js">
      <xsl:with-param name="list" select="$jslist_utils" />
    </xsl:call-template>
  </xsl:template>

  <!-- Scans space-separater list of names to generate script elements. -->
  <xsl:template name="add_js">
    <xsl:param name="list" />

    <xsl:variable name="before" select="substring-before($list, ' ')" />

    <xsl:variable name="file">
      <xsl:choose>
        <xsl:when test="string-length($before)">
          <xsl:value-of select="$before" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$list" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:if test="string-length($file)&gt;0">
      <xsl:value-of select="$nl" />
      <xsl:variable name="path" select="concat('includes/', $file, '.js')" />
      <script type="text/javascript" src="{$path}"></script>
    </xsl:if>

    <xsl:if test="string-length($before)">
      <xsl:call-template name="add_js">
        <xsl:with-param name="list" select="substring-after($list,' ')" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>
  
  <xsl:template name="css_includes">
    <xsl:value-of select="$nl" />
    <link rel="stylesheet" type="text/css" href="includes/schemafw.css" />
    <xsl:value-of select="$nl" />
    <link rel="stylesheet" type="text/css" href="includes/dpicker.css" />
  </xsl:template>

  <xsl:template match="navigation/target" mode="header">
    <a href="{@url}"><xsl:value-of select="@label" /></a>
  </xsl:template>

  <xsl:template match="/*/navigation" mode="header">
    <xsl:if test="count(target)">
      <nav><xsl:apply-templates select="target" mode="header" /></nav>
    </xsl:if>
  </xsl:template>

  <!-- Empty template for non-matched mode="header" elements to prevent empty lines. -->
  <xsl:template match="*" mode="header"></xsl:template>
  
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

  <xsl:template match="/*" mode="make_schemafw_meta">
    <xsl:element name="div">
      <xsl:attribute name="id">schemafw-meta</xsl:attribute>
      <xsl:attribute name="style">display:none</xsl:attribute>

      <xsl:if test="@method='POST'">
        <xsl:attribute name="data-post">true</xsl:attribute>
      </xsl:if>

      <xsl:attribute name="data-modeType">
        <xsl:value-of select="@mode-type" />
      </xsl:attribute>

      <xsl:if test="@meta-jump">
        <xsl:attribute name="data-jump">
        <xsl:value-of select="@meta-jump" />
        </xsl:attribute>
      </xsl:if>

      <xsl:if test="local-name()='message'">
        <xsl:element name="span">
          <xsl:attribute name="class">message</xsl:attribute>
          <xsl:if test="@detail">
            <xsl:attribute name="data-detail">
              <xsl:value-of select="@detail" />
            </xsl:attribute>
          </xsl:if>
          <xsl:value-of select="@message" />
        </xsl:element>
      </xsl:if>
    </xsl:element>
  </xsl:template>



</xsl:stylesheet>
