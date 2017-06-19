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
    
    <xsl:variable name="named_r" select="/*/*[local-name()=$field/@result]" />
    <xsl:variable name="deflt_r" select="/*[not($named_r)]/keywords" />
    <xsl:variable name="result" select="$named_r | $deflt_r" />

    <ul name="{$field/@name}" class="sfw_select" data-sfw-class="ulselect" data-sfw-control="true">
      <xsl:apply-templates select="$result" mode="construct_selected_list">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
      <xsl:apply-templates select="$result/*" mode="construct_ul_select_option">
        <xsl:with-param name="data" select="$data" />
      </xsl:apply-templates>
    </ul>
  </xsl:template>

  <xsl:template match="*[@rndx]" mode="construct_selected_list">
    <xsl:param name="data" />
    <li>
      <xsl:apply-templates select="$data/@keywords" mode="fill" />
    </li>
  </xsl:template>

  <xsl:template match="@keywords" mode="fill">
    <xsl:call-template name="add_selected_items">
      <xsl:with-param name="str" select="." />
      <xsl:with-param name="lookup" select="/*/keywords" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="add_selected_items">
    <xsl:param name="str" />
    <xsl:param name="lookup" />

    <xsl:variable name="hascomma" select="contains($str,',')" />
    
    <xsl:choose>
      <xsl:when test="$hascomma">
        <xsl:variable name="val" select="substring-before($str,',')" />
        <xsl:variable name="sel" select="$lookup/*[@id=$val]" />

        <xsl:apply-templates select="$sel" mode="construct_selected_item" />

        <xsl:call-template name="add_selected_items">
          <xsl:with-param name="str" select="substring-after($str,',')" />
          <xsl:with-param name="lookup" select="$lookup" />
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="sel" select="$lookup/*[@id=$str]" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="*[@id]" mode="construct_selected_item">
    <span class="item">
      <xsl:value-of select="concat(@kname,' ')" />
      <xsl:element name="span">
        <xsl:attribute name="data-id"><xsl:value-of select="@id" /></xsl:attribute>
        <xsl:text>&#215;</xsl:text>
      </xsl:element>
    </span>
  </xsl:template>

  <xsl:template match="*" mode="construct_ul_select_option">
    <xsl:param name="data" />
    <xsl:element name="li">
      <xsl:attribute name="data-value"><xsl:value-of select="@id" /></xsl:attribute>
      <xsl:value-of select="@kname" />
    </xsl:element>
  </xsl:template>


                
    




</xsl:stylesheet>
