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

    <xsl:template match="field[@type='iltable']" mode="construct_input">
      <xsl:param name="data" />
      <xsl:variable name="value" select="$data/@*[local-name()=current()/@name]" />
      <xsl:variable name="result" select="/*/*[local-name()=current()/@result]" />
      <xsl:variable name="row_id">
        <xsl:apply-templates select="$result/schema" mode="get_id_field_name" />
      </xsl:variable>
      <ul>
        <xsl:apply-templates select="." mode="show_members">
          <xsl:with-param name="str" select="$value" />
          <xsl:with-param name="result" select="$result" />
          <xsl:with-param name="row_id" select="$row_id" />
        </xsl:apply-templates>
      </ul>
    </xsl:template>

    <xsl:template match="field[@type='iltable']" mode="show_members">
      <xsl:param name="str" />
      <xsl:param name="result" select="/.." />
      <xsl:param name="row_id" select="'id'" />


      <xsl:variable name="hascomma" select="contains($str,',')" />
      <xsl:variable name="c_val" select="substring-before($str,',')" />
      <xsl:variable name="s_val" select="substring($str,1 div boolean(string-length($c_val)=0))" />
      <xsl:variable name="val" select="concat($c_val,$s_val)" />

      <xsl:variable name="row" select="$result/*[@*[local-name()=$row_id]=$val][1]" />

      <xsl:element name="li">
        <xsl:call-template name="resolve_refs">
          <xsl:with-param name="str" select="@template" />
          <xsl:with-param name="row" select="$row" />
        </xsl:call-template>
      </xsl:element>

      <xsl:if test="string-length($c_val)">
        <xsl:apply-templates select="." mode="show_members">
          <xsl:with-param name="str" select="substring-after($str,',')" />
          <xsl:with-param name="result" select="$result" />
          <xsl:with-param name="row_id" select="$row_id" />
        </xsl:apply-templates>
      </xsl:if>
      
    </xsl:template>



</xsl:stylesheet>
