<?xml version="1.0" encoding="utf-8" ?>

<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_variables.xsl" />
  <xsl:import href="sfw_utilities.xsl" />
  <xsl:import href="sfw_scripts.xsl" />
  <xsl:import href="sfw_host.xsl" />

  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="utl-8"/>

  <xsl:template match="schema" mode="id_schema">
    <tr>
      <td>
        <xsl:value-of select="local-name(..)" />
      </td>
      <td>
        <xsl:if test="local-name(..)='result'">
          <xsl:value-of select="../@rndx" />
        </xsl:if>
      </td>
    </tr>
  </xsl:template>

  <!--
  Template to match transformFill() in _render_interaction().
  -->
  <xsl:template match="/*">
    <xsl:choose>
      <xsl:when test="$is_form">

        <xsl:choose>
          <xsl:when test="count($gschema) &gt; 1">
            <table>
              <xsl:apply-templates select="$gschema" mode="id_schema" />
            </table>
          </xsl:when>
          <xsl:otherwise>
            <xsl:apply-templates select="$gschema" mode="construct_form" />
          </xsl:otherwise>
        </xsl:choose>
        <!-- <xsl:apply-templates select="$gschema" mode="construct_form" /> -->
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="fill_host" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
  Fundamental template for completing an HTML head element.
  This is meant to be called by a "default.xsl" stylesheet.

  Primarily, it either
     creates a meta element to jump to another page if a
     meta-jump attribute or element is found,
  or
     it adds elements (CSS link and Javascript script) that
     support fundamental Schema Framework operations.
  -->
  <xsl:template match="/*" mode="fill_head">
    <!-- consult sfw_scripts.xsl, template :construct_scripts: -->
    <xsl:param name="jscripts">debug</xsl:param>

    <meta name="viewport" content="width=device-width, initial-scale=1.0" />

    <xsl:call-template name="add_css">
      <xsl:with-param name="path">includes/</xsl:with-param>
      <xsl:with-param name="list">schemafw dpicker</xsl:with-param>
    </xsl:call-template>

    <xsl:if test="@css">
    <xsl:call-template name="add_css">
      <xsl:with-param name="list" select="@css" />
    </xsl:call-template>
    </xsl:if>

    <xsl:apply-templates select="." mode="construct_scripts">
      <xsl:with-param name="jscripts" select="$jscripts" />
    </xsl:apply-templates>

    <xsl:choose>
    <xsl:when test="$err_condition=0">
      <xsl:choose>
        <xsl:when test="@mode-type='form-jump'">
          <xsl:apply-templates select="*[@rndx=1]" mode="fill_head" />
        </xsl:when>
        <xsl:when test="@meta-jump">
          <xsl:apply-templates select="@meta-jump" mode="fill_head" />
        </xsl:when>
        <xsl:when test="meta-jump">
          <xsl:apply-templates select="meta-jump" mode="fill_head" />
        </xsl:when>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="$err_condition=1">
      <meta name="xsl_error_result_row" content="{$result-row/@msg}" />
    </xsl:when>
    <xsl:when test="$err_condition=2">
      <meta name="xsl_error_message" content="{$msg-el/@message}" />
    </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!-- The next set of templates support the mode="fill_head" template. -->
  
  <xsl:template name="meta-jump">
    <xsl:param name="url" select="'/'" />
    <xsl:param name="wait" select="0" />

    <xsl:variable name="res_url">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="$url" />
        <xsl:with-param name="escape" select="1" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="content" select="concat($wait,'; url=', $res_url)" />
    <meta http-equiv="refresh" content="{$content}" />

    <script type="text/javascript">
      <xsl:value-of select="concat('location.replace(&quot;',$url,'&quot;);')" />
    </script>
    <xsl:value-of select="$nl" />
  </xsl:template>

  <!-- Calls to meta-jump template need not resolve URL references
       because meta-jump does it just before creating the meta element. -->

  <xsl:template match="*[@rndx=1]" mode="fill_head">
    <xsl:variable name="row" select="*[local-name()=../@row-name][1]" />

    <!-- Only one of the next two will have a value. -->
    <xsl:variable name="jval" select="$row/@jump" />
    <xsl:variable name="eval" select="substring($row/@error,1 div boolean(0=string-length($jval)))" />

    <xsl:variable name="tname" select="concat('jump',$jval,$eval)" />

    <xsl:variable name="target" select="jumps/@*[local-name()=$tname]" />
    <xsl:if test="$target">
      <xsl:call-template name="meta-jump">
        <xsl:with-param name="url" select="$target" />
        <xsl:with-param name="wait" select="2" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="@meta-jump" mode="fill_head">
    <xsl:call-template name="meta-jump">
      <xsl:with-param name="url" select="." />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="meta-jump" mode="fill_head">
    <xsl:call-template name="meta-jump">
      <xsl:with-param name="url" select="@url" />
      <xsl:with-param name="wait" select="@wait" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="add_scripts_and_stylesheets">
    <xsl:if test="$err_condition=0">
      <xsl:choose>
        <xsl:when test="@meta-jump">
          <xsl:variable name="url">
            <xsl:apply-templates select="@meta-jump" mode="resolve_url" />
          </xsl:variable>
          
          <script type="text/javascript">
            <xsl:text>location.replace(&quot;</xsl:text>
            <xsl:value-of select="$url" />
            <xsl:text>&quot;);</xsl:text>
          </script>
        </xsl:when>
        <xsl:otherwise>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:template>

  <!--
  Scans space-separater list of names to generate script elements.

  Refer to sfw_scripts.xsl for various variables that define the
  scripts list that add_js will use to construct the script elements.
  -->
  <xsl:template name="add_js">
    <xsl:param name="list" />
    <xsl:param name="path" />

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
      <xsl:variable name="fpath" select="concat($path, $file, '.js')" />
      <script type="text/javascript" src="{$fpath}"></script>
    </xsl:if>

    <xsl:if test="string-length($before)">
      <xsl:call-template name="add_js">
        <xsl:with-param name="list" select="substring-after($list,' ')" />
        <xsl:with-param name="path" select="$path" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="add_css">
    <xsl:param name="list" />
    <xsl:param name="path" />

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
      <xsl:variable name="fpath" select="concat($path, $file, '.css')" />
      <link rel="stylesheet" type="text/css" href="{$fpath}" />
    </xsl:if>

    <xsl:if test="string-length($before)">
      <xsl:call-template name="add_css">
        <xsl:with-param name="list" select="substring-after($list,' ')" />
        <xsl:with-param name="path" select="$path" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="topic" mode="build_topic">
    <xsl:param name="targets" select="target" />
    <li class="topic">
      <div><xsl:value-of select="@label" /></div>
      <ul class="menu">
        <xsl:apply-templates select="$targets" mode="nav_header" />
      </ul>
    </li>
  </xsl:template>

  <xsl:template match="topic" mode="nav_header">
    <xsl:apply-templates select="." mode="build_topic" />
  </xsl:template>

  <xsl:template match="topic[@result]" mode="nav_header">
    <xsl:variable name="targets" select="/*/*[local-name()=current()/@result]/target" />
    <xsl:if test="count($targets)">
      <xsl:apply-templates select="." mode="build_topic">
        <xsl:with-param name="targets" select="$targets" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="target" mode="nav_header">
      <xsl:variable name="url">
        <xsl:apply-templates select="@url" mode="resolve_url" />
      </xsl:variable>

      <li class="mitem">
        <xsl:element name="a">
          <xsl:attribute name="href"><xsl:value-of select="$url" /></xsl:attribute>
          <xsl:if test="@tag">
            <xsl:attribute name="tag">
              <xsl:value-of select="@tag" />
            </xsl:attribute>
          </xsl:if>
          <xsl:apply-templates select="@label" mode="resolve_refs" />
        </xsl:element>
      </li>
  </xsl:template>

  <xsl:template match="/*/navigation" mode="header">
    <xsl:if test="count(target)">
      <nav>
        <ul class="menu">
          <xsl:apply-templates select="*" mode="nav_header" />
        </ul>
      </nav>
    </xsl:if>
  </xsl:template>

  <!-- Empty template for non-matched mode="header" elements to prevent empty lines. -->
  <xsl:template match="*" mode="header"></xsl:template>
  

  <xsl:template match="/*" mode="make_schemafw_meta">
    <xsl:element name="div">
      <xsl:attribute name="id">schemafw-meta</xsl:attribute>
      <xsl:attribute name="style">display:none</xsl:attribute>

      <xsl:if test="@method='POST'">
        <xsl:attribute name="data-post">true</xsl:attribute>
      </xsl:if>

      <xsl:attribute name="data-modeType">
        <xsl:value-of select="$mode-type" />
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

  <xsl:template match="message/@*" mode="construct_parts">
    <xsl:element name="p">
      <span name="type"><xsl:value-of select="local-name()" /></span>
      <span name="val"><xsl:value-of select="." /></span>
    </xsl:element>
  </xsl:template>

  <xsl:template match="message" mode="construct">
    <div class="message">
      <xsl:apply-templates select="@*" mode="construct_parts" />
    </div>
  </xsl:template>


</xsl:stylesheet>
