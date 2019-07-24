<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <!-- Getting values from schema and fields -->

  <xsl:template match="schema" mode="get_form_title">
    <xsl:variable name="raw_title">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str">
          <xsl:apply-templates select="." mode="seek_attribute">
            <xsl:with-param name="name" select="'title'" />
          </xsl:apply-templates>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="string-length($raw_title)">
        <xsl:value-of select="$raw_title" />
      </xsl:when>
      <xsl:otherwise>Dialog</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- prevent extra text printing for other add_on_click_attribute template. -->
  <xsl:template match="@*" mode="add_on_click_attribute"></xsl:template>

  <xsl:template match="@*[starts-with(local-name(),'on_') and contains(local-name(),'_click')]"
      mode="add_on_click_attribute">
    <xsl:variable name="aname" select="concat('data-', local-name())" />
    <xsl:attribute name="{$aname}">
      <xsl:apply-templates select="." mode="resolve_url" />
    </xsl:attribute>
  </xsl:template>

  <!--
  1. Get $all from all on_xxx_click attributes in ancestor chain
` 2. Get (from $all) attribute sets a_s, a_r, a_m from schema, result, and root
  3. Get exclusive set x_r (for result, not found in schema)
  4. Get exclusive set x_m (for mode, not found in schema or result)
  5. Write out reconciled on_xxx_click attributes from union of a_s|x_r|x_m
  -->
  <xsl:template match="*" mode="add_on_click_attributes">
    <xsl:variable
        name="all"
        select="ancestor-or-self::*/@*[starts-with(local-name(),'on_') and contains(local-name(),'_click')]" />

    <xsl:variable name="a_s" select="$all[parent::schema]" />
    <xsl:variable name="a_r" select="$all[parent::*[@rndx] and not(parent::schema)]" />
    <xsl:variable name="a_m" select="$all[not(parent::schema) and not(parent::*[@rndx])]" />

    <xsl:variable name="x_r" select="$a_r[not(local-name($a_s)=local-name())]" />
    <xsl:variable
        name="x_m"
        select="$a_m[not(local-name($a_s)=local-name())][not(local-name($a_r)=local-name())]" />

    <xsl:apply-templates select="$a_s|$x_r|$x_m" mode="add_on_click_attribute" />

  </xsl:template>

  <xsl:template match="schema" mode="get_form_action">
    <xsl:variable name="ta" select="@form-action" />
    <xsl:variable name="fa" select="parent::*[not($ta)]/@form-action" />
    <xsl:variable name="action" select="$ta|$fa" />
    
    <xsl:if test="$action">
      <xsl:apply-templates select="$action" mode="resolve_url" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema" mode="get_field_gids">
    <xsl:variable name="t">
      <xsl:apply-templates select="fields" mode="make_field_gid" />
    </xsl:variable>
    <xsl:value-of select="substring($t,2)" />
  </xsl:template>

  <!--
  It's perhaps ill-advised, but if neither xrow_id or primary_key attributes indicate
  the id field, this template assumes that the first field is the id field.
  -->
  <xsl:template match="schema" mode="get_id_field_name">
    <xsl:variable name="xid" select="field[@xrow_id]" />
    <xsl:variable name="pid" select="field[not($xid)][@primary-key]" />
    <xsl:variable name="lid" select="field[not($xid|$pid)][1]" />
    <xsl:value-of select="($xid|$pid|$lid)/@name" />
  </xsl:template>

  <xsl:template match="field" mode="get_id_value">
    <xsl:param name="data" />
    <xsl:variable name="idname">
      <xsl:apply-templates select=".." mode="get_id_field_name" />
    </xsl:variable>
    <xsl:value-of select="$data/@*[local-name()=$idname]" />
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

    <xsl:variable name="val">
      <xsl:if test="$data">
        <xsl:value-of select="$data/@*[local-name()=current()/@name]" />
      </xsl:if>
    </xsl:variable>

    <xsl:variable name="lenval" select="string-length($val)" />
    <xsl:variable name="nt" select="@nodetype" />

    <xsl:variable name="dataval">
      <xsl:choose>
        <xsl:when test="$lenval&gt;0 and (not(@nt) or @nt='attribute' or @nt='child')">
          <xsl:value-of select="$val" />
        </xsl:when>
        <xsl:when test="@map-value">
          <xsl:value-of select="$data/@*[local-name()=current()/@map-value]" />
        </xsl:when>
        <xsl:when test="$data and $data/text()">
          <xsl:value-of select="$data" />
        </xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="string-length($dataval) &gt; 0">
        <xsl:value-of select="$dataval" />
      </xsl:when>
      <xsl:when test="@value">
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@value" />
        </xsl:call-template>
      </xsl:when>
    </xsl:choose>

  </xsl:template>


  <xsl:template match="schema/field" mode="generate_field_value">
    <xsl:param name="row" select="/.." />
    <xsl:param name="show_name" />

    <xsl:choose>
      <xsl:when test="@display">
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@display" />
          <xsl:with-param name="row" select="$row" />
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="$show_name">
        <xsl:value-of select="$row/@*[local-name()=$show_name]" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="altfield" select="$field/../field[not(@primary_key)][1]" />
        <xsl:if test="$altfield">
          <xsl:value-of select="$row/@*[local-name()=$altfield/@name]" />
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
    


</xsl:stylesheet>
