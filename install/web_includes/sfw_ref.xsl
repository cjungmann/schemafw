<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <xsl:template match="ref" mode="resolve_ref">
    <xsl:param name="value" />
    <xsl:variable name="rname" select="@result" />
    <xsl:variable name="iname" select="@index" />
    <xsl:variable name="lname" select="@label" />

    <xsl:variable name="result" select="/*/*[@rndx][local-name()=$rname]" />
    <xsl:variable
        name="row"
        select="$result/*[local-name()=$result/@row-name][@*[local-name()=$iname]=$value]" />

    <xsl:value-of select="$row/@*[local-name()=$lname]" />
  </xsl:template>

  <xsl:template match="*" mode="add_ref_option">
    <xsl:param name="ref" />
    <xsl:param name="value" />

    <xsl:variable name="opval" select="@*[local-name()=$ref/@index]" />

    <xsl:element name="option">
      <xsl:attribute name="value"><xsl:value-of select="$opval" /></xsl:attribute>
      <xsl:if test="($opval) = ($value)">
        <xsl:attribute name="selected">selected</xsl:attribute>
      </xsl:if>
      <xsl:value-of select="@*[local-name()=$ref/@label]" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="field[ref]" mode="get_value">
    <xsl:param name="data" />
    <xsl:apply-templates select="ref" mode="resolve_refs">
      <xsl:with-param name="value" select="$data/@*[local-name()=current()/@name]" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="field[ref]" mode="construct_input">
    <xsl:param name="data" />

    <xsl:variable name="result" select="/*/*[@rndx][local-name()=current()/ref/@result]" />
    <xsl:variable name="rows"  select="$result/*[local-name()=../@row-name]" />
    <xsl:element name="select">
      <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
      <xsl:apply-templates select="$rows" mode="add_ref_option">
        <xsl:with-param name="ref" select="ref" />
        <xsl:with-param name="value">
          <xsl:apply-templates select="@value" mode="resolve_refs" />
        </xsl:with-param>
      </xsl:apply-templates>
    </xsl:element>

  </xsl:template>


</xsl:stylesheet>
