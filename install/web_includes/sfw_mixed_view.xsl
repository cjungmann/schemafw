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

  <xsl:template match="/*" mode="construct_mixed_view">
    <xsl:param name="primary" />
    <xsl:variable name="schema" select="/*/*[@rndx=1]/schema" />
    <xsl:if test="$schema">
      <xsl:variable name="title">
        <xsl:apply-templates select="$schema" mode="get_form_title" />
      </xsl:variable>
      
      <xsl:element name="div">
        <xsl:attribute name="class">SFW_Host</xsl:attribute>

        <xsl:attribute name="data-result-path">
          <xsl:variable name="path">
            <xsl:apply-templates select="$schema/.." mode="get_path" />
          </xsl:variable>
        </xsl:attribute>

        <xsl:if test="string-length($title)">
          <h2><xsl:value-of select="$title" /></h2>
        </xsl:if>

        <h3>Buttons:</h3>
        <xsl:apply-templates select="$schema" mode="show_buttons" />

        <xsl:apply-templates select="$schema/field" mode="construct_subview" />
      </xsl:element>
    </xsl:if>
    
  </xsl:template>

  <xsl:template match="field[@result][@type]" mode="construct_subview">
    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]"/>
    <xsl:if test="$result">
      <xsl:element name="div">
        <xsl:attribute name="class">SFW_Host</xsl:attribute>
        <xsl:attribute name="data-subview">true</xsl:attribute>
        <xsl:attribute name="data-sfw-class">
          <xsl:value-of select="@type" />
        </xsl:attribute>
        <xsl:attribute name="data-result-path">
          <xsl:apply-templates select="$result" mode="get_path" />
        </xsl:attribute>
        <h3>
          <xsl:call-template name="resolve_refs">
            <xsl:with-param name="str" select="@label" />
          </xsl:call-template>
        </h3>
        <xsl:apply-templates select="@manage" mode="construct_button" />
        <xsl:apply-templates select="." mode="fill_subview" />
      </xsl:element>
    </xsl:if>
  </xsl:template>

  <xsl:template match="@manage" mode="construct_button">
    <p>
      <xsl:element name="button">
        <xsl:attribute name="data-type">manage_subview</xsl:attribute>
        <xsl:attribute name="data-task">
          <xsl:call-template name="resolve_refs">
            <xsl:with-param name="str" select="." />
          </xsl:call-template>
        </xsl:attribute>
        <xsl:element name="img">
          <xsl:attribute name="src">includes/edit_pencil.png</xsl:attribute>
        </xsl:element>
      </xsl:element>
    </p>
  </xsl:template>

  <xsl:template match="field[@type='vtable']" mode="fill_subview">
    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/@result]"/>
    <xsl:variable name="fields" select="$result/schema/field" />
    <xsl:variable name="rows" select="$result/*[local-name()=$result/@row-name]" />
    <table><tbody>
      <xsl:apply-templates select="$rows" mode="construct_simple_table_row">
        <xsl:with-param name="fields" select="$fields" />
      </xsl:apply-templates>
    </tbody></table>
  </xsl:template>

  <xsl:template match="*" mode="construct_simple_table_row">
    <xsl:param name="fields" />
    <tr>
      <xsl:apply-templates
          select="$fields[not(@hidden)]"
          mode="construct_simple_table_cell">
        <xsl:with-param name="row" select="." />
      </xsl:apply-templates>
    </tr>
  </xsl:template>

  <xsl:template match="field" mode="construct_simple_table_cell">
    <xsl:param name="row" />
    <xsl:variable name="val" select="$row/@*[local-name()=current()/@name]" />
    <td>
      <xsl:if test="$val">
        <xsl:value-of select="$val" />
      </xsl:if>
    </td>
  </xsl:template>

</xsl:stylesheet>
