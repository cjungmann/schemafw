<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:import href="sfw_generics.xsl" />
  <xsl:import href="sfw_utilities.xsl" />
  <xsl:import href="sfw_schema.xsl" />

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

  <xsl:template match="*[@rndx][schema][@sfw_refill_tbody]">
    <xsl:apply-templates select="schema" mode="fill_tbody" />
  </xsl:template>

  <xsl:template match="*[@sfw_replace_row_contents]">
    <xsl:variable name="schema" select="../schema" />
    <xsl:choose>
      <xsl:when test="$schema">
        <xsl:apply-templates select="$schema/field" mode="construct_line_cell">
          <xsl:with-param name="data" select="." />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:otherwise>No schema found</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
  Creates a table element and fills it according to the instructions contained
  in the schema element.

  The parameter "static," if set, will omit the duplicate thead rows that allow
  the column headers to remain fixed while the table scrolls underneath.
  -->
  <xsl:template match="schema" mode="construct_table">
    <xsl:param name="static" />
    <xsl:param name="sfw_class" />

    <xsl:variable name="class">
      <xsl:text>Schema</xsl:text>
        <xsl:if test="@table_class">
          <xsl:value-of select="concat(' ',@table_class)" />
        </xsl:if>
    </xsl:variable>

    <xsl:element name="table">
      <xsl:attribute name="class"><xsl:value-of select="$class" /></xsl:attribute>
      <xsl:apply-templates select="." mode="add_result_attribute" />
      <xsl:apply-templates select="." mode="add_sfw_class_attribute">
        <xsl:with-param name="sfw_class" select="$sfw_class" />
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="add_on_click_attributes" />

      <xsl:attribute name="data-confirm_template">yes</xsl:attribute>
      <xsl:apply-templates select="." mode="add_tag_attribute" />

      <thead>
        <xsl:apply-templates select="." mode="construct_thead_rows" />
        <xsl:if test="not($static)">
          <xsl:apply-templates select="." mode="construct_thead_rows" >
            <xsl:with-param name="class" select="'floater'" />
          </xsl:apply-templates>
        </xsl:if>
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
    <xsl:param name="sfw_class" />
    <xsl:choose>
      <xsl:when test="schema">
        <xsl:apply-templates select="schema" mode="construct_table">
          <xsl:with-param name="sfw_class" select="$sfw_class" />
        </xsl:apply-templates>
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

    <xsl:variable name="result" select="parent::*[@rndx]" />

    <xsl:variable name="r_rows" select="$result/*[not($lines)][local-name()=$result/@row-name]" />
    <xsl:variable name="s_rows" select="$result/*[not($lines|$r_rows)][local-name()=current()/@name]" />

    <xsl:variable name="lines_to_use" select="$lines|$r_rows|$s_rows" />

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

  <!-- Null-output template paired to other get_row_class (following)
       to prevent white-space output for non-matching fields. -->
  <xsl:template match="*" mode="get_row_class"></xsl:template>

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
        <xsl:when test="string-length($line_id)">
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
        <xsl:attribute name="class"><xsl:value-of select="$row_class" /></xsl:attribute>
      </xsl:if>

      <xsl:if test="@show_and_highlight">
        <xsl:attribute name="id">schema_target_line</xsl:attribute>
      </xsl:if>

      <xsl:if test="$id_field">
        <xsl:attribute name="data-id">
          <xsl:value-of select="@*[name()=$id_field]" />
        </xsl:attribute>
      </xsl:if>

      <xsl:apply-templates select="." mode="custom_tr_attributes" />
      <xsl:attribute name="bogus">ha</xsl:attribute>

      <xsl:apply-templates select="$schema/field" mode="construct_line_cell">
        <xsl:with-param name="data" select="." />
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <!-- Do nothing template for unmatched fields -->
  <xsl:template match="*[local-name()=../@row-name]" mode="custom_tr_attributes"></xsl:template>

  <xsl:template match="field[not(@hidden or @ignore)]" mode="construct_line_cell">
    <xsl:param name="data" />
    <xsl:element name="td">
      
      <xsl:apply-templates select="." mode="custom_td_attributes">
        <xsl:with-param name="data" select="." />
      </xsl:apply-templates>

      <xsl:if test="@cell-class">
        <xsl:attribute name="class">
          <xsl:value-of select="@cell-class" />
        </xsl:attribute>
      </xsl:if>

      <!-- No child elements can precede write_cell_content because
           that template may need to add an attribute to the "td" element. -->
      <xsl:apply-templates select="." mode="write_cell_content">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <!-- Do nothing template for unmatched fields -->
  <xsl:template match="field" mode="custom_td_attributes"></xsl:template>

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
  
  <!-- <xsl:template match="schema/field[not(@type='assoc')]" mode="write_cell_content"> -->
  <!--   <xsl:param name="data" /> -->

  <!--   <xsl:variable name="val"> -->
  <!--     <xsl:apply-templates select="." mode="get_value"> -->
  <!--       <xsl:with-param name="data" select="$data" /> -->
  <!--     </xsl:apply-templates> -->
  <!--   </xsl:variable> -->

  <!--   <xsl:choose> -->
  <!--     <xsl:when test="@type='BOOL'"> -->
  <!--       <xsl:if test="$val=1">x</xsl:if> -->
  <!--     </xsl:when> -->
  <!--     <xsl:otherwise> -->
  <!--       <xsl:value-of select="$val" /> -->
  <!--     </xsl:otherwise> -->
  <!--   </xsl:choose> -->
  <!-- </xsl:template> -->
  
  <!-- <xsl:template match="schema/field[@type='assoc']" mode="write_cell_content"> -->
  <!--   <xsl:param name="data" /> -->
  <!--   <xsl:if test="@result"> -->
  <!--     <xsl:apply-templates select="." mode="add_assoc_attribute" /> -->
  <!--     <xsl:apply-templates select="." mode="build_associations"> -->
  <!--       <xsl:with-param name="data" select="$data" /> -->
  <!--     </xsl:apply-templates> -->
  <!--   </xsl:if> -->
  <!-- </xsl:template> -->

  <!-- <xsl:template match="schema/field[@type='assoc'][@style='table']" -->
  <!--               mode="write_cell_content"> -->
  <!--   <xsl:param name="data" /> -->
  <!--   <xsl:if test="@result"> -->
  <!--     <table> -->
  <!--       <xsl:element name="tbody"> -->
  <!--         <xsl:apply-templates select="." mode="add_assoc_attribute" /> -->
  <!--         <xsl:apply-templates select="." mode="build_associations"> -->
  <!--           <xsl:with-param name="data" select="$data" /> -->
  <!--         </xsl:apply-templates> -->
  <!--       </xsl:element> -->
  <!--     </table> -->
  <!--   </xsl:if> -->
    
  <!-- </xsl:template> -->


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
