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
      <xsl:with-param name="field" select="$field" />
    </xsl:apply-templates>
  </xsl:template>

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
          <xsl:with-param name="field" select="." />
        </xsl:apply-templates>
      </tbody>
      <tfoot><tr><td colspan="99">
        <input type="hidden" name="{@name}" value="{$value}" />
      </td></tr></tfoot>
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[@type='iltable']" mode="show_members">
    <xsl:param name="str" />
    <xsl:param name="field" />

    <xsl:variable name="result" select="/*/*[local-name()=current()/@result]" />
    <xsl:variable name="row_id">
      <xsl:apply-templates select="$result/schema" mode="get_id_field_name" />
    </xsl:variable>

    <xsl:variable name="c_id" select="substring-before($str,',')" />
    <xsl:variable name="s_id" select="substring($str,1 div boolean(string-length($c_id)=0))" />
    <xsl:variable name="id_val" select="concat($c_id,$s_id)" />

    <xsl:variable name="row" select="$result/*[@*[local-name()=$row_id]=$id_val][1]" />

    <tr data-id="{$id_val}">
      <xsl:call-template name="resolve_refs">
        <xsl:with-param name="str" select="@template" />
        <xsl:with-param name="row" select="$row" />
        <xsl:with-param name="break" select="'td'" />
      </xsl:call-template>
    </tr>

    <xsl:if test="string-length($c_id)">
      <xsl:apply-templates select="." mode="show_members">
        <xsl:with-param name="str" select="substring-after($str,',')" />
        <xsl:with-param name="result" select="$result" />
        <xsl:with-param name="row_id" select="$row_id" />
      </xsl:apply-templates>
    </xsl:if>
    
  </xsl:template>



</xsl:stylesheet>
