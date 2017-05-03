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

  <xsl:template match="*[@rndx][/*/@mode-type='table'][schema]">
    <xsl:apply-templates select="schema" mode="construct_table" />
  </xsl:template>

  <!--
  Creates a table element and fills it according to the instructions contained
  in the schema element.

  The parameter "static," if set, will omit the duplicate thead rows that allow
  the column headers to remain fixed while the table scrolls underneath.
  -->
  <xsl:template match="schema" mode="construct_table">
    <xsl:param name="static" />
    
    <xsl:variable name="path">
      <xsl:apply-templates select=".." mode="get_path" />
    </xsl:variable>

    <xsl:variable name="class">
      <xsl:text>Schema</xsl:text>
        <xsl:if test="@table_class">
          <xsl:value-of select="concat(' ',@table_class)" />
        </xsl:if>
    </xsl:variable>

    <xsl:element name="table">
      <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
      <xsl:attribute name="data-sfw-class">table</xsl:attribute>
      <xsl:attribute name="data-result-type">table</xsl:attribute>
      <xsl:attribute name="data-result-path">
        <xsl:value-of select="$path" />
      </xsl:attribute>

      <xsl:apply-templates select=".." mode="add_data_attribute">
        <xsl:with-param name="name">on_line_click</xsl:with-param>
      </xsl:apply-templates>

      <xsl:apply-templates select="." mode="extra_anchor_attributes">
        <xsl:with-param name="type" select="'table'" />
      </xsl:apply-templates>

      <thead>
        <xsl:if test="not($static)">
          <xsl:apply-templates select="." mode="construct_thead_rows" >
            <xsl:with-param name="class" select="'floater'" />
          </xsl:apply-templates>
        </xsl:if>
        <xsl:apply-templates select="." mode="construct_thead_rows" />
      </thead>
      
      <tbody>
        <xsl:apply-templates select="." mode="fill_tbody" />
      </tbody>
    </xsl:element>
  </xsl:template>

  <!--
      Pass-through template to provide a result-based entry to construct_table.
  -->
  <xsl:template match="*[@rndx]" mode="construct_table">
    <xsl:choose>
      <xsl:when test="schema">
        <xsl:apply-templates select="schema" mode="construct_table" />
      </xsl:when>
      <xsl:otherwise>
        <div>Can't construct a table without a schema</div>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


  <xsl:template match="schema" mode="construct_thead_rows">
    <xsl:param name="class" />

    <xsl:apply-templates select="." mode="show_intro">
      <xsl:with-param name="class" select="$class" />
    </xsl:apply-templates>

    <xsl:apply-templates select="." mode="construct_button_row">
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

  <!-- Recursive template to create button rows.
       Print 1. parent, 2. self, 3. self/buttons -->
  <xsl:template match="*" mode="construct_button_row">
    <xsl:param name="class" />
    <xsl:param name="host-type" select="'tr'" />

    <xsl:if test="not(local-name()='buttons')">
      <xsl:variable name="par" select="parent::*" />
      <xsl:if test="$par">
        <xsl:apply-templates select="$par" mode="construct_button_row">
          <xsl:with-param name="class" select="$class" />
          <xsl:with-param name="host-type" select="$host-type" />
        </xsl:apply-templates>
      </xsl:if>
    </xsl:if>

    <xsl:variable name="host_class">
      <xsl:text>button_row</xsl:text>
      <xsl:if test="$host-type='tr'">
        <xsl:value-of select="concat(' headfix_', local-name(..),'_',local-name())" />
      </xsl:if>
      <xsl:value-of select="concat(' sfwid_', generate-id())" />
      <xsl:if test="$class">
        <xsl:value-of select="concat(' ', $class)" />
      </xsl:if>
    </xsl:variable>
    
    <xsl:variable name="buttons" select="button" />

    <xsl:if test="count($buttons)&gt;0">
      <xsl:choose>
        <xsl:when test="$host-type='tr'">
          <tr class="{$host_class}">
            <td colspan="99" style="background-color #66FF66">
              <xsl:apply-templates select="$buttons" mode="construct_button" />
            </td>
          </tr>
        </xsl:when>
        <xsl:otherwise>
          <p class="{$host_class}">
            <xsl:apply-templates select="$buttons" mode="construct_button" />
          </p>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

    <xsl:if test="not(local-name()='buttons')">
      <xsl:apply-templates select="buttons" mode="construct_button_row">
        <xsl:with-param name="class" select="$class" />
        <xsl:with-param name="host-type" select="$host-type" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>


  <!--
  This template fills an already-established tbody element, It can be
  used to replot the contents of a table without disturbing the other
  parts of a table element or the attribute of the tbody element itself,
  which sometimes holds important context information.

  Note the lines parameter.  It can be used to have this template fill the
  tbody with something other than the full list of rows.  For example,
  a developer could fill the table with a subset by filling the "lines"
  parameter with a filtered nodelist of table lines.  If lines is not set
  the template will use the entire set of rows contained in the parent
  result element.
  -->
  <xsl:template match="schema" mode="fill_tbody">
    <xsl:param name="lines" select="/.." />
    <xsl:param name="group" />

    <xsl:variable name="row-name">
      <xsl:choose>
        <xsl:when test="@row-name"><xsl:value-of select="@row-name" /></xsl:when>
        <xsl:otherwise>row</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable
        name="plist"
        select="parent::*[count($lines)=0]/*[local-name()=$row-name]" />

    <xsl:variable name="lines_to_use" select="$lines|$plist" />

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
        <xsl:apply-templates select="$lines_to_use" mode="construct_tbody_row">
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
        <xsl:apply-templates select="$lines_to_use" mode="construct_tbody_row">
          <xsl:with-param name="line_id" select="$id_field" />
        </xsl:apply-templates>
      </xsl:otherwise>
    </xsl:choose>
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

  <!--
  Try to set the line_id parameter so the template need not determine
  the value for each row of the table.
  -->
  <xsl:template match="*" mode="construct_tbody_row">
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

      <xsl:apply-templates select="$schema/field" mode="construct_line_cell">
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

  <xsl:template match="field[not(@hidden or @ignore)]" mode="construct_line_cell">
    <xsl:param name="data" />
    <xsl:element name="td">
      
      <xsl:if test="@cell-class">
        <xsl:attribute name="class">
          <xsl:value-of select="@cell-class" />
        </xsl:attribute>
      </xsl:if>

      <xsl:apply-templates select="." mode="write_cell_content">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:element>
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


  <!-- Named template for makeing a table (do we ever use this?). -->

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
