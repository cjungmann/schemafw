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

  <!-- Generic attribute match of ulselect attribute for transformFill() function. -->
  <xsl:template
      match="@*[../../schema/field[@name=local-name(current())][@type='ulselect']]">
    <xsl:variable
        name="field" select="../../schema/field[@name=local-name(current())]" />
    <xsl:call-template name="add_ulselect_selections">
      <xsl:with-param name="lookup" select="/*/*[local-name()=$field/@result]" />
      <xsl:with-param name="str" select="." />
    </xsl:call-template>
  </xsl:template>

  <!--
  This template builds the entire structure when building the form.  That is:
  - The host ul with identifying attributes,
  - one li with:
     - a span to display the translated selections,
     - an input for saving and posting the ulselect's value
  - a second li with an ul containing the list of options

  For ease of parsing, a comma is added to the beginning and end
  of the value, so that all numbers in the value will start and end
  with a comma.
  -->
  <xsl:template name="construct_ulselect_input">
    <xsl:param name="field" />
    <xsl:param name="data" />

    <xsl:variable name="value" select="$data/@*[local-name()=$field/@name]" />
    <xsl:variable name="list" select="concat(',',$value,',')" />

    <xsl:variable name="lu_result" select="/*/*[local-name()=$field/@result]" />

    <ul name="{$field/@name}" class="ulselect"
        data-sfw-class="ulselect" data-sfw-input="true">

      <xsl:apply-templates select="$lu_result" mode="construct_ulselect_selections">
        <xsl:with-param name="field" select="$field" />
        <xsl:with-param name="value" select="$value" />
      </xsl:apply-templates>

      <li class="options_host">
        <ul class="ulselect_options" >
          <xsl:apply-templates select="$lu_result/*" mode="construct_ulselect_option">
            <xsl:with-param name="list" select="$list" />
          </xsl:apply-templates>
        </ul>
      </li>
    </ul>

  </xsl:template>

  <!--
  This template should match a lookup result for reconciling the integer
  values in the $value parameter.
  
  Adds a space to ensure that an empty li element doesn't collapse.
  -->
  <xsl:template match="*[@rndx]" mode="construct_ulselect_selections">
    <xsl:param name="field" />
    <xsl:param name="value" />

    <li class="selections">
      <span>
        <xsl:call-template name="add_ulselect_selections" >
          <xsl:with-param name="lookup" select="." />
          <xsl:with-param name="str" select="$value" />
        </xsl:call-template>
      </span>
      <xsl:text> </xsl:text>
      <input type="text" />
      <input type="text" disabled="disabled" class="transfer" name="{$field/@name}" value="{$value}" />
    </li>
  </xsl:template>

  <!-- Recursive template that adds items to the "current selections" element. -->
  <xsl:template name="add_ulselect_selections">
    <xsl:param name="lookup" />
    <xsl:param name="str" />

    <xsl:variable name="hascomma" select="contains($str,',')" />
    
    <xsl:choose>
      <xsl:when test="$hascomma">
        <xsl:variable name="val" select="substring-before($str,',')" />
        <xsl:variable name="sel" select="$lookup/*[@id=$val]" />

        <xsl:apply-templates select="$sel" mode="construct_ulselect_item" />

        <xsl:call-template name="add_ulselect_selections">
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

  <xsl:template match="*[@id]" mode="construct_ulselect_item">
    <span>ulselect result <xsl:value-of select="local-name()" /> is missing a lookup attribute.</span>
  </xsl:template>

  <!-- Used by add_ulselect_selections to fill the "current selections" element. -->
  <xsl:template match="*[@id][../@lookup]" mode="construct_ulselect_item">
    <xsl:variable name="aname" select="../@lookup" />
    <span class="item">
      <xsl:value-of select="@*[local-name()=$aname]" />
      <xsl:element name="span">
        <xsl:attribute name="data-id"><xsl:value-of select="@id" /></xsl:attribute>
        <xsl:text>&#215;</xsl:text>
      </xsl:element>
    </span>
  </xsl:template>
  
  <!--
  This template should match a lookup result to reconcile integer values in
  $list with name strings.

  Fills a UL with available selection options.
  -->
  <xsl:template match="*" mode="construct_ulselect_option">
    <xsl:param name="list" />

    <xsl:variable name="aname" select="../@lookup" />
    
    <xsl:element name="li">
      <xsl:attribute name="data-value"><xsl:value-of select="@id" /></xsl:attribute>
      <xsl:attribute name="class">
        <xsl:choose>
          <xsl:when test="contains($list,concat(',',@id,','))">in</xsl:when>
          <xsl:otherwise>out</xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
      <xsl:value-of select="@*[local-name()=$aname]" />
    </xsl:element>
  </xsl:template>


  <!-- Table field template -->
  <xsl:template match="schema/field[@type='ulselect']" mode="write_cell_content">
    <xsl:param name="data" />
    <xsl:call-template name="ulselect_write_cell_content">
      <xsl:with-param name="str" select="$data/@*[local-name()=current()/@name]" />
      <xsl:with-param name="lookup" select="/*/*[local-name()=current()/@result]" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="ulselect_write_cell_content">
    <xsl:param name="lookup" />
    <xsl:param name="str" />

    <xsl:variable name="hascomma" select="contains($str,',')" />
    <xsl:variable name="aname" select="$lookup/@lookup" />

    <xsl:variable name="vid">
    <xsl:choose>
      <xsl:when test="$hascomma">
        <xsl:value-of select="substring-before($str,',')" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$str" />
      </xsl:otherwise>
    </xsl:choose>
    </xsl:variable>

    <xsl:value-of select="$lookup/*[@id=$vid]/@*[local-name()=$aname]" />

    <xsl:if test="$hascomma">
      <xsl:text>, </xsl:text>
      <xsl:call-template name="ulselect_write_cell_content">
        <xsl:with-param name="lookup" select="$lookup" />
        <xsl:with-param name="str" select="substring-after($str,',')" />
      </xsl:call-template>
    </xsl:if>

  </xsl:template>

</xsl:stylesheet>
