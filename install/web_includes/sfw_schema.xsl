<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <!-- Getting values from schema and fields -->

  <xsl:template match="schema" mode="get_form_title">
    <xsl:call-template name="resolve_refs">
      <xsl:with-param name="str">
        <xsl:choose>
          <xsl:when test="@title"><xsl:value-of select="@title" /></xsl:when>
          <xsl:when test="../@title"><xsl:value-of select="../@title" /></xsl:when>
          <xsl:otherwise>Dialog</xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="schema" mode="add_on_line_click_attribute">
    <xsl:variable name="lc_s" select="@on_line_click" />
    <xsl:variable name="lc_r" select="parent::*[not($lc_s)][@rndx]/@on_line_click" />
    <xsl:variable name="lc_d" select="/*[not($lc_s|$lc_r)]/@on_line_click" />
    <xsl:variable name="all" select="$lc_s|$lc_r|$lc_d" />
    <xsl:if test="$all">
      <xsl:attribute name="data-on_line_click">
        <xsl:value-of select="$all" />
      </xsl:attribute>
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema" mode="get_form_action">
    <xsl:variable name="ta" select="@form-action" />
    <xsl:variable name="fa" select="parent::*[not($ta)]/@form-action" />
    <xsl:variable name="action" select="$ta|$fa" />
    <xsl:if test="$action">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="$action" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema" mode="get_field_gids">
    <xsl:variable name="t">
      <xsl:apply-templates select="fields" mode="make_field_gid" />
    </xsl:variable>
    <xsl:value-of select="substring($t,2)" />
  </xsl:template>

  <xsl:template match="schema" mode="get_id_field_name">
    <xsl:variable name="lid" select="./field[@line_id]" />
    <xsl:variable name="pid" select="./field[@primary-key]" />
    <xsl:choose>
      <xsl:when test="$lid">
        <xsl:value-of select="$lid/@name" />
      </xsl:when>
      <xsl:when test="$pid">
        <xsl:value-of select="$pid/@name" />
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_name">
    <xsl:choose>
      <xsl:when test="@html-name">
        <xsl:value-of select="@html-name" />
      </xsl:when>
      <xsl:when test="@orgname">
        <xsl:value-of select="@orgname" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@name" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_label">
    <xsl:choose>
      <xsl:when test="@label">
        <xsl:apply-templates select="@label" mode="resolve_refs" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="translate(@name,'_',' ')" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_size">
    <xsl:choose>
      <xsl:when test="@length &lt; 25">
        <xsl:value-of select="@length" />
      </xsl:when>
      <xsl:otherwise>25</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="field" mode="get_result_td_type">
    <xsl:variable name="type" select="concat('#',@type,'#')" />
    <xsl:if test="contains($datetimetypes, $type)">
      <xsl:variable name="lower" select="translate(@type,'ADEIMTW','adeimtw')" />
      <xsl:value-of select="concat('dtp_', $lower)" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema/field[@dtd]" mode="get_list_string">
    <xsl:variable name="paren" select="substring-after(@dtd,'(')" />
    <xsl:variable name="len" select="string-length($paren)" />
    <xsl:value-of select="substring($paren,1,($len)-1)" />
  </xsl:template>

  <!-- Extract data parameter information using schema or field directions. -->

  <xsl:template match="schema/field[@row_class]" mode="get_row_class">
    <xsl:param name="data" />
    <xsl:variable name="value">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>
    <xsl:if test="$value=@row_class">
      <xsl:value-of select="concat(' ',@name)" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_cell_value">
    <xsl:param name="data" />
    <xsl:variable name="val">
      <xsl:apply-templates select="." mode="get_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="@type='BOOL'">
        <xsl:if test="$val=1">x</xsl:if>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$val" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
    
  <xsl:template match="schema/field" mode="fill_table_cell">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="get_cell_value">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="schema/field" mode="get_value">
    <xsl:param name="data" />
    <xsl:variable name="name" select="@name" />

    <xsl:choose>
      <xsl:when test="@html-value">
        <xsl:value-of select="@html-value" />
      </xsl:when>
      <xsl:when test="@value">
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@value" />
        </xsl:call-template>
        <!-- <xsl:value-of select="@value" /> -->
      </xsl:when>
      <xsl:when test="@ref-value">
        <xsl:apply-templates select="$vars" mode="get_value">
          <xsl:with-param name="name" select="@ref-value" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="@map-value">
        <xsl:value-of select="$data/@*[local-name()=current()/@map-value]" />
      </xsl:when>
      <xsl:when test="$data and (not(@nodetype) or @nodetype='attribute')">
        <xsl:variable name="v" select="$data/@*[name()=$name]" />
        <xsl:if test="$v">
          <xsl:value-of select="$v" />
        </xsl:if>
      </xsl:when>
      <xsl:when test="$data and @nodetype='child'">
        <xsl:variable name="v" select="$data/*[local-name()=$name]" />
        <xsl:if test="$v">
          <xsl:value-of select="$v" />
        </xsl:if>
      </xsl:when>
      
      <xsl:otherwise>
        <xsl:value-of select="$data" />
      </xsl:otherwise>
      
      <!-- <xsl:otherwise> -->
      <!--   <xsl:apply-templates select="." mode="get_s_value" /> -->
      <!-- </xsl:otherwise> -->
      
    </xsl:choose>

  </xsl:template>



</xsl:stylesheet>
