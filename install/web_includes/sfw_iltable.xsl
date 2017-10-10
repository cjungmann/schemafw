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

  <!-- Modeless attribute match for like-named field of proper type in hosting schema. -->
  <xsl:template match="@*[ancestor::schema/field[@name=local-name(current())][@type='iltable']]">
    <xsl:variable name="field" select="ancestor::schema/field[@name=local-name(current())]" />
    <xsl:apply-templates select="$field" mode="show_members">
      <xsl:with-param name="str" select="." />
    </xsl:apply-templates>
  </xsl:template>

  <!-- Template that is called by construct_form via
       construct_input_row via
       construct_form_input via
       construct_form
  -->
  <xsl:template match="field[@type='iltable']" mode="construct_input">
    <xsl:param name="data" />
    <xsl:variable name="value" select="$data/@*[local-name()=current()/@name]" />
    <xsl:element name="table">
      <xsl:attribute name="class">iltable</xsl:attribute>
      <xsl:attribute name="data-sfw-class">iltable</xsl:attribute>
      <xsl:attribute name="data-sfw-input">input</xsl:attribute>
      <xsl:attribute name="tabindex">0</xsl:attribute>
      <tbody>
        <xsl:apply-templates select="." mode="show_members">
          <xsl:with-param name="str" select="$value" />
        </xsl:apply-templates>
      </tbody>
      <tfoot><tr><td colspan="99">
        <input type="hidden" name="{@name}" value="{$value}" />
      </td></tr></tfoot>
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[@type='iltable']" mode="make_add_button">
    <tr>
      <td colspan="99">
        <button type="button" name="add">+</button>
      </td>
    </tr>
  </xsl:template>

  <!-- Parses comma-separated list of integers to pull rows from another result. -->
  <xsl:template match="field[@type='iltable']" mode="show_members">
    <xsl:param name="str" />
    <xsl:param name="result" select="/.." />
    <xsl:param name="row_id" />

    <xsl:variable name="f_result" select="/*[not($result)]/*[local-name()=current()/@result]" />
    <xsl:variable name="u_result" select="$result|$f_result" />

    <xsl:variable name="rid">
      <xsl:choose>
        <xsl:when test="$row_id"><xsl:value-of select="$row_id" /></xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="$u_result/schema" mode="get_id_field_name" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="c_id" select="substring-before($str,',')" />
    <xsl:variable name="s_id" select="substring($str,1 div boolean(string-length($c_id)=0))" />
    <xsl:variable name="id_val" select="concat($c_id,$s_id)" />

    <xsl:variable name="row" select="$u_result/*[@*[local-name()=$rid]=$id_val][1]" />

    <tr data-id="{$id_val}">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="@template" />
        <xsl:with-param name="row" select="$row" />
        <xsl:with-param name="break" select="'td'" />
      </xsl:call-template>
    </tr>

    <xsl:choose>
      <xsl:when test="string-length($c_id)">
        <xsl:apply-templates select="." mode="show_members">
          <xsl:with-param name="str" select="substring-after($str,',')" />
          <xsl:with-param name="result" select="$u_result" />
          <xsl:with-param name="row_id" select="$rid" />
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="@on_add">
        <xsl:apply-templates select="." mode="make_add_button" />
      </xsl:when>
    </xsl:choose>
    
  </xsl:template>



</xsl:stylesheet>
