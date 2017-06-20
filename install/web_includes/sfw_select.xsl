<?xml version="1.0" encoding="utf-8" ?>

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

  <xsl:template name="construct_keywords_input">
    <xsl:param name="field" />
    <xsl:param name="data" />

    <xsl:variable name="value" select="$data/@*[local-name()=$field/@name]" />

    <xsl:variable name="named_r" select="/*/*[local-name()=$field/@result]" />
    <xsl:variable name="deflt_r" select="/*[not($named_r)]/keywords" />
    <xsl:variable name="result" select="$named_r | $deflt_r" />

    <ul name="{$field/@name}" class="sfw_select"
        data-sfw-class="ulselect" data-sfw-input="true">

      <xsl:apply-templates select="$result" mode="construct_ulselect_selected">
        <xsl:with-param name="field" select="$field" />
        <xsl:with-param name="list" select="$value" />
      </xsl:apply-templates>

      <xsl:apply-templates select="$result/*" mode="construct_ulselect_option">
        <xsl:with-param name="list" select="$value" />
      </xsl:apply-templates>
    </ul>
  </xsl:template>

 <!-- Create "current selections" element filled items from the lookup result. -->
  <xsl:template match="*[@rndx]" mode="construct_ulselect_selected">
    <xsl:param name="list" />
    <li class="selected">
      <xsl:call-template name="add_ulselect_selected" >
        <xsl:with-param name="lookup" select="." />
        <xsl:with-param name="str" select="$list" />
      </xsl:call-template>
    </li>
  </xsl:template>

  <!-- Recursive template that adds items to the "current selections" element. -->
  <xsl:template name="add_ulselect_selected">
    <xsl:param name="lookup" />
    <xsl:param name="str" />

    <xsl:variable name="hascomma" select="contains($str,',')" />
    
    <xsl:choose>
      <xsl:when test="$hascomma">
        <xsl:variable name="val" select="substring-before($str,',')" />
        <xsl:variable name="sel" select="$lookup/*[@id=$val]" />

        <xsl:apply-templates select="$sel" mode="construct_ulselect_item" />

        <xsl:call-template name="add_ulselect_selected">
          <xsl:with-param name="lookup" select="$lookup" />
          <xsl:with-param name="str" select="substring-after($str,',')" />
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="sel" select="$lookup/*[@id=$str]" />
        <xsl:apply-templates select="$sel" mode="construct_ulselect_item" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Used by add_ulselect_selected to fill the "current selections" element. -->
  <xsl:template match="*[@id]" mode="construct_ulselect_item">
    <span class="item">
      <xsl:value-of select="concat(@kname,' ')" />
      <xsl:element name="span">
        <xsl:attribute name="data-id"><xsl:value-of select="@id" /></xsl:attribute>
        <xsl:text>&#215;</xsl:text>
      </xsl:element>
    </span>
  </xsl:template>

  <!-- Used to create the fill list of options -->
  <xsl:template match="*" mode="construct_ulselect_option">
    <xsl:param name="list" />
    <xsl:element name="li">
      <xsl:attribute name="data-value"><xsl:value-of select="@id" /></xsl:attribute>
      <xsl:value-of select="@kname" />
    </xsl:element>
  </xsl:template>
 

                
    




</xsl:stylesheet>
