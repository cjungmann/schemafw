<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_generics.xsl" />
  <xsl:import href="sfw_utilities.xsl" />

  <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="UTF-8"/>


  <xsl:template match="*[@rndx]" mode="make_table">
    <xsl:param name="host" select="'page'" />

    <xsl:variable name="path">
      <xsl:apply-templates select="." mode="get_path" />
    </xsl:variable>

    <xsl:variable name='schema' select="./schema" />

    <xsl:element name="table">
      <xsl:apply-templates select="." mode="extra_anchor_attributes">
        <xsl:with-param name="type" select="'table'" />
      </xsl:apply-templates>
      <xsl:attribute name="data-result-type">table</xsl:attribute>
      <xsl:attribute name="data-result-path">
        <xsl:value-of select="$path" />
      </xsl:attribute>
      <xsl:attribute name="class">
        <xsl:text>Schema</xsl:text>
        <xsl:if test="$schema/@table_class">
          <xsl:value-of select="concat(' ',$schema/@table_class)" />
        </xsl:if>
      </xsl:attribute>

      <thead>
        <xsl:if test="$host='page'">
          <xsl:apply-templates select="$schema" mode="make_table_head" >
            <xsl:with-param name="class" select="'floater'" />
          </xsl:apply-templates>
        </xsl:if>
        <xsl:apply-templates select="$schema" mode="make_table_head" />
      </thead>
      <tbody>
        <xsl:apply-templates select="." mode="fill_table_body" />
      </tbody>
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema" mode="make_table_head">
    <xsl:param name="class" />

    <xsl:apply-templates select="." mode="show_intro">
      <xsl:with-param name="class" select="$class" />
    </xsl:apply-templates>

    <xsl:apply-templates select="." mode="show_buttons">
      <xsl:with-param name="class" select="$class" />
    </xsl:apply-templates>

    <xsl:element name="tr">
      <xsl:attribute name="class">
        <xsl:text>headfix_cheads</xsl:text>
        <xsl:if test="$class">
          <xsl:value-of select="concat(' ',$class)" />
        </xsl:if>
      </xsl:attribute>
      <xsl:apply-templates select="field" mode="make_column_head" />
    </xsl:element>
  </xsl:template>


  <xsl:template match="schema" mode="fill_table_body">
    <xsl:param name="lines" />
    <xsl:param name="group" />

    <xsl:variable name="id_field">
      <xsl:apply-templates select="." mode="get_id_field_name" />
    </xsl:variable>
    
    <xsl:variable name="field" select="field[@sorting]" />

    <xsl:choose>
      <xsl:when test="$field">
        <xsl:variable name="aname" select="$field/@name" />
        <xsl:variable name="dir">
          <xsl:choose>
            <xsl:when test="$field/@descending">descending</xsl:when>
            <xsl:otherwise>ascending</xsl:otherwise>
          </xsl:choose>
        </xsl:variable>
        <xsl:variable name="data-type">
          <xsl:choose>
            <xsl:when test="$field/@sort"><xsl:value-of select="$field/@sort" /></xsl:when>
            <xsl:otherwise>text</xsl:otherwise>
          </xsl:choose>
        </xsl:variable>
        <xsl:apply-templates select="$lines" mode="make_table_line">
          <xsl:with-param name="line_id" select="$id_field" />
          <xsl:sort select="count(@*[local-name()=$aname])"
                    data-type="number"
                    order="descending" />
          <xsl:sort select="@*[local-name()=$aname]"
                    data-type="{$data-type}"
                    order="{$dir}" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="$lines" mode="make_table_line">
          <xsl:with-param name="line_id" select="$id_field" />
        </xsl:apply-templates>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="fill_table_body">
    <xsl:param name="group" />

    <xsl:variable name="row-name">
      <xsl:choose>
        <xsl:when test="@row-name"><xsl:value-of select="@row-name" /></xsl:when>
        <xsl:when test="schema/@name"><xsl:value-of select="schema/@name" /></xsl:when>
        <xsl:otherwise>row</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="lines" select="*[local-name()=$row-name]" />

    <xsl:apply-templates select="schema" mode="fill_table_body">
      <xsl:with-param name="group" select="$group" />
      <xsl:with-param name="lines" select="$lines" />
    </xsl:apply-templates>
  </xsl:template>
                
  <xsl:template match="schema/field" mode="get_column_head_class">
    <xsl:if test="not(@unsortable)">sortable</xsl:if>
    <xsl:if test="@class">
      <xsl:value-of select="concat(' ',@class)" />
    </xsl:if>
  </xsl:template>

  <xsl:template match="schema/field[not(@hidden)]" mode="make_column_head">
    <xsl:variable name="name" select="@name" />
    <xsl:variable name="label">
      <xsl:apply-templates select="." mode="get_label" />
    </xsl:variable>
    <xsl:variable name="class">
      <xsl:apply-templates select="." mode="get_column_head_class" />
    </xsl:variable>
    
    <xsl:element name="th">
      <xsl:attribute name="data-name">
        <xsl:value-of select="$name" />
      </xsl:attribute>
      
      <xsl:attribute name="data-type">
        <xsl:value-of select="@nodetype" />
      </xsl:attribute>
      
      <xsl:if test="$class">
        <xsl:attribute name="class">
          <xsl:value-of select="$class" />
        </xsl:attribute>
      </xsl:if>
      
      <xsl:value-of select="$label" />
    </xsl:element>
  </xsl:template>

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

  <xsl:template match="*" mode="make_table_line">
    <xsl:param name="line_id" />

    <xsl:variable name="schema" select="../schema" />

    <!-- for ad-hoc lines, figure id_field on the fly -->    
    <xsl:variable name="id_field">
      <xsl:choose>
        <xsl:when test="$line_id">
          <xsl:value-of select="$line_id" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="$schema" mode="get_id_field_name" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="row_class">
      <xsl:apply-templates select="$schema/field" mode="get_row_class">
        <xsl:with-param name="data" select="." />
      </xsl:apply-templates>
    </xsl:variable>
    
    <xsl:variable name="row_class_flag" select="$schema/@row_class_flag" />

    <xsl:element name="tr">
      <xsl:if test="string-length($row_class) &gt; 0">
        <xsl:attribute name="class">
          <xsl:value-of select="$row_class" />
        </xsl:attribute>
      </xsl:if>

      <xsl:if test="@show_and_highlight">
        <xsl:attribute name="id">schema_target_line</xsl:attribute>
      </xsl:if>

      <xsl:if test="$id_field">
        <xsl:attribute name="data-id">
          <xsl:value-of select="@*[name()=$id_field]" />
        </xsl:attribute>
      </xsl:if>

      <xsl:apply-templates select="$schema/*" mode="make_line_cell">
        <xsl:with-param name="data" select="." />
      </xsl:apply-templates>
    </xsl:element>
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

  <xsl:template match="schema/field[not(@hidden or @ignore)]" mode="make_line_cell">
    <xsl:param name="data" />
    <xsl:element name="td">
      
      <xsl:if test="@cell-class">
        <xsl:attribute name="class">
          <xsl:value-of select="@cell-class" />
        </xsl:attribute>
      </xsl:if>

      <xsl:apply-templates select="." mode="fill_table_cell">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <xsl:template match="schema/field" mode="fill_table_cell">
    <xsl:param name="data" />
    <xsl:apply-templates select="." mode="write_cell_content">
      <xsl:with-param name="data" select="$data" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="schema/field" mode="write_cell_content">
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
      
    </xsl:choose>

  </xsl:template>

  <xsl:template name="make_table">
    <!-- sfw_table.xsl -->
    <xsl:param name="gids" />
    <xsl:param name="first" select="1" />
    
    <xsl:variable name="gid">
      <xsl:call-template name="next_gid">
        <xsl:with-param name="gids" select="$gids" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:if test="string-length($gid) &gt; 0">
      <xsl:variable name="field" select="/*/schema/field[generate-id()=$gid]" />
      <xsl:if test="$first=0">|</xsl:if>
      <xsl:value-of select="$field/@name" />

      <xsl:call-template name="make_table">
        <xsl:with-param name="gids" select="substring-after($gids,'-')" />
        <xsl:with-param name="first" select="0" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>


  <!--
      This template will be discarded when sfw_table.xsl is imported
      to a stylesheet that already includes <xsl:template match="/">.
  -->
  <xsl:template match="/">
    <xsl:variable name="result" select="*/*[@rndx='1']" />
    <html>
      <head>
        <title>Testing</title>
      </head>
      <body>
        <xsl:apply-templates select="$result" mode="make_table" />
      </body>
    </html>
  </xsl:template>


</xsl:stylesheet>
