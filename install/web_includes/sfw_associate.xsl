<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:output method="xml"
              doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
              doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
              version="1.0"
              indent="yes"
              omit-xml-declaration="yes"
              encoding="UTF-8"/>

  <!--
  Primary template for rendering the associated contents of a field.
  The query that includes this field should select a NULL value for the
  the field name and an appropriate schema/field instruction.

  For rows with multiple non-id attributes, include a template
  instruction to the field instruction to dictate which attributes
  to include and in what order.
  -->
  <xsl:template match="field[@associated]" mode="show_associations">
    <xsl:param name="data" />

    <xsl:variable name="lresult" select="/*/*[local-name()=current()/@result]" />

    <xsl:call-template name="transform_associated_references">
      <xsl:with-param name="result" select="/*/*[local-name()=$lresult/@result]" />
      <xsl:with-param name="field" select="." />
      <xsl:with-param name="str">
        <xsl:apply-templates select="." mode="get_int_list">
          <xsl:with-param name="data" select="$data" />
        </xsl:apply-templates>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <!-- Default transform_row template.
       Override this template with a following template with more specific field match.
       -->
  <xsl:template match="field[@associated][@style='table']" mode="transform_row">
    <xsl:param name="id" />
    <xsl:param name="row" />

    <xsl:element name="tr">
      <xsl:if test="$id">
        <xsl:attribute name="data-id"><xsl:value-of select="$id" /></xsl:attribute>
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@template" />
          <xsl:with-param name="row" select="$row" />
          <xsl:with-param name="break" select="'td'" />
        </xsl:call-template>
      </xsl:if>
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[@associated][@style='span']" mode="transform_row">
    <xsl:param name="id" />
    <xsl:param name="row" />

    <xsl:element name="span">
      <xsl:if test="$id">
        <xsl:attribute name="data-id"><xsl:value-of select="$id" /></xsl:attribute>
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@template" />
          <xsl:with-param name="row" select="$row" />
        </xsl:call-template>
      </xsl:if>
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[@associated][@style='csv']" mode="transform_row">
    <xsl:param name="id" />
    <xsl:param name="row" />
    <xsl:param name="more" />

    <xsl:variable name="val">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="@template" />
        <xsl:with-param name="row" select="$row" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:value-of select="normalize-space($val)" />

    <xsl:if test="$more">, </xsl:if>
  </xsl:template>

  <xsl:template name="transform_associated_references">
    <xsl:param name="result" />
    <xsl:param name="field" />
    <xsl:param name="str" />

    <xsl:variable name="c_id" select="substring-before($str,',')" />
    <xsl:variable name="s_id" select="substring($str,1 div boolean(string-length($c_id)=0))" />
    <xsl:variable name="id_val" select="concat($c_id,$s_id)" />

    <xsl:variable name="more" select="string-length($c_id)&gt;0" />

    <xsl:variable name="row" select="$result/*[local-name()=../@row-name][@id=$id_val]" />

    <xsl:apply-templates select="$field" mode="transform_row">
      <xsl:with-param name="id" select="$id_val" />
      <xsl:with-param name="row" select="$row" />
      <xsl:with-param name="more" select="$more" />
    </xsl:apply-templates>
    
    <xsl:if test="$more">
      <xsl:call-template name="transform_associated_references">
        <xsl:with-param name="result" select="$result" />
        <xsl:with-param name="field" select="$field" />
        <xsl:with-param name="str" select="substring-after($str,',')" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>


  <xsl:template match="*[parent::*[@rndx][@type='ref-lists']]" mode="resolve_references">
    <xsl:param name="field" />

    <xsl:call-template name="transform_associated_references">
      <xsl:with-param name="result" select=".." />
      <xsl:with-param name="field" select="$field" />
      <xsl:with-param name="str" select="@*[local-name()=$field/@name]" />
    </xsl:call-template>

  </xsl:template>

  <!-- Helper template to get a CSV list of integers from the association table/element. -->
  <xsl:template match="field[@associated]" mode="get_int_list">
    <xsl:param name="data" />

    <xsl:variable name="id_value">
      <xsl:apply-templates select="." mode="get_id_value">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:variable name="lresult" select="/*/*[local-name()=current()/@result]" />
    <xsl:variable name="associated_row" select="$lresult/*[local-name()=../@row-name][@id=$id_value]" />

    <xsl:value-of select="$associated_row/@*[local-name()=current()/@name]" />
  </xsl:template>

  <!-- Special handling for associated fields, to override matching template in sfw_form.xsl. -->
  <xsl:template match="field[@associated]" mode="display_value">
    <xsl:param name="data" />
  </xsl:template>

    <!-- Special handling for associated fields, to override matching template in sfw_form.xsl. -->
  <xsl:template match="field[@associated][@style='table']" mode="display_value">
    <xsl:param name="data" />
    <table>
      <xsl:apply-templates select="." mode="show_associations">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </table>
  </xsl:template>


</xsl:stylesheet>
