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


  <!--
  The control consists of a three-part cluster, followed by a
  temporary drop-down section with a list of options.

  The three-part cluster consists of the following:
  1. A "defacto" span element that shows the dereferenced selection value(s),
  2. An unnamed input element used to accept the user's typed input, and
  3. A named, disabled, and invisible input element whose contents are sent
     to the server as part of the form POST.
  -->

  <!-- Modeless attribute match for like-named field of proper type in hosting schema. -->
  <xsl:template match="@*[local-name()=ancestor::schema/field[@type='ulselect']/@name]">
    <xsl:variable name="field" select="ancestor::schema/field[@name=local-name(current())]" />
    <xsl:call-template name="fill_ulselect_defacto">
      <xsl:with-param name="field" select="$field" />
      <xsl:with-param name="str" select="." />
    </xsl:call-template>
  </xsl:template>

  <!-- Generic template for rebuilding the options list -->
  <xsl:template match="field[@type='ulselect']">
    <xsl:variable
        name="result" select="/*/*[@rndx][local-name()=current()/@result]" />

    <xsl:variable name="value">
      <xsl:apply-templates select="." mode="get_ulselect_value" />
    </xsl:variable>

    <xsl:if test="$value and $result">
      <xsl:apply-templates select="$result/*" mode="construct_ulselect_option">
        <xsl:with-param name="list" select="concat(',',$value,',')" />
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <!-- Template to display error if the field does not include a result attribute. -->
  <xsl:template match="field[@type='ulselect']" mode="construct_input">
    <span>The ulselect field <xsl:value-of select="@name" /> failed to specify a result.</span>
  </xsl:template>
  
  <!--
      ulselect templates are in sfw_ulselect.xsl.  In order to use the contents
      of sfw_ulselect.xsl, the following template, whose priority ensures
      appropriate selection, accesses the imported templates explicitly.
  -->
  <xsl:template match="field[@type='ulselect'][@result]" mode="construct_input">
    <xsl:param name="data" />
    <xsl:call-template name="construct_ulselect_input">
      <xsl:with-param name="field" select="." />
      <xsl:with-param name="data" select="$data" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="field" mode="get_ulselect_value">
    <xsl:variable name="sresult" select="../following-sibling::*[@rndx][1]" />
    <xsl:variable name="hresult" select="../parent::*[not($sresult)][@rndx]" />
    <xsl:variable name="result" select="$sresult|$hresult" />

    <xsl:if test="$result">
      <xsl:variable name="rname" select="$result/@row-name" />
      <xsl:variable name="attr"
          select="$result/*[local-name()=$rname]/@*[local-name()=current()/@name]" />
      <xsl:if test="$attr">
        <xsl:value-of select="$attr" />
      </xsl:if>
    </xsl:if>
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

    <xsl:element name="ul">
      <xsl:attribute name="name"><xsl:value-of select="$field/@name" /></xsl:attribute>
      <xsl:attribute name="class">ulselect</xsl:attribute>
      <xsl:attribute name="tabindex">0</xsl:attribute>
      <xsl:attribute name="data-sfw-class">ulselect</xsl:attribute>
      <xsl:attribute name="data-sfw-control">input</xsl:attribute>
      <xsl:if test="$field/@style='multiple'">
        <xsl:attribute name="data-multiple">yes</xsl:attribute>
      </xsl:if>

      <xsl:if test="$field/@on_add">
        <xsl:apply-templates select="$field/@on_add" mode="add_resolved_data_attribute" />
      </xsl:if>
      <!-- <ul name="{$field/@name}" class="ulselect" -->
      <!--     data-sfw-class="ulselect" data-sfw-control="true"> -->

      <li class="cluster" tabindex="0">
        <xsl:text>&#160;</xsl:text>
        <xsl:choose>
          <xsl:when test="$field/@style='multiple'">
            <xsl:apply-templates select="$lu_result" mode="construct_ulselect_cluster_multiple">
              <xsl:with-param name="field" select="$field" />
              <xsl:with-param name="value" select="$value" />
            </xsl:apply-templates>
          </xsl:when>
          <xsl:otherwise>
            <xsl:apply-templates select="$lu_result" mode="construct_ulselect_cluster_single">
              <xsl:with-param name="field" select="$field" />
              <xsl:with-param name="value" select="$value" />
            </xsl:apply-templates>
          </xsl:otherwise>
        </xsl:choose>
      </li>

      <li class="options_host">
        <div>
          <ul class="ulselect_options">
            <xsl:if test="not($field/@options='off')">
              <xsl:apply-templates select="$lu_result/*" mode="construct_ulselect_option">
                <xsl:with-param name="list" select="$list" />
                <xsl:with-param name="style" select="$field/@style" />
              </xsl:apply-templates>
            </xsl:if>
          </ul>
        </div>
      </li>
    </xsl:element>
  </xsl:template>

  <!-- Template matches a lookup result. -->
  <xsl:template match="*[@rndx]" mode="construct_ulselect_cluster_single">
    <xsl:param name="field" />
    <xsl:param name="value" />
    <span class="defacto">
      <xsl:text> </xsl:text>
      <xsl:variable name="sel" select="*[@id=$value]" />
      <xsl:apply-templates select="$sel" mode="construct_ulselect_defacto_item" />
    </span>
    <input type="text" class="typing" style="width:100%"/>
    <input type="text" disabled="disabled" class="transfer" name="{$field/@name}" value="{$value}" />
  </xsl:template>

  <!--
  This template should match a lookup result for reconciling the integer
  values in the $value parameter.
  
  Adds a space to ensure that an empty li element doesn't collapse.
  -->
  <xsl:template match="*[@rndx]" mode="construct_ulselect_cluster_multiple">
    <xsl:param name="field" />
    <xsl:param name="value" />

    <span class="defacto">
      <xsl:call-template name="fill_ulselect_defacto" >
        <xsl:with-param name="field" select="$field" />
        <xsl:with-param name="lookup" select="." />
        <xsl:with-param name="str" select="$value" />
      </xsl:call-template>
    </span>
    <xsl:text> </xsl:text>
    <input type="text" class="typing" />
    <input type="text" disabled="disabled" class="transfer" name="{$field/@name}" value="{$value}" />
  </xsl:template>

  <!-- Recursive template that adds items to the "current selections" element. -->
  <xsl:template name="fill_ulselect_defacto">
    <xsl:param name="field" />
    <xsl:param name="str" />
    <xsl:param name="lookup" select="/.." />

    <xsl:text>  </xsl:text>

    <xsl:variable name="found_lookup" select="/*[not($lookup)]/*[local-name()=$field/@result]" />
    <xsl:variable name="result" select="$lookup|$found_lookup" />

    <xsl:variable name="hascomma" select="contains($str,',')" />

    <xsl:variable name="id">
      <xsl:choose>
        <xsl:when test="$hascomma">
          <xsl:value-of select="substring-before($str,',')" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$str" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:apply-templates select="$result/*[@id=$id]" mode="construct_ulselect_defacto_item">
      <xsl:with-param name="style" select="$field/@style" />
    </xsl:apply-templates>

    <xsl:if test="$hascomma">
      <xsl:call-template name="fill_ulselect_defacto">
        <xsl:with-param name="field" select="$field" />
        <xsl:with-param name="lookup" select="$result" />
        <xsl:with-param name="str" select="substring-after($str,',')" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*[@id]" mode="construct_ulselect_defacto_item">
    <span>ulselect result <xsl:value-of select="local-name()" /> is missing a lookup attribute.</span>
  </xsl:template>

  <!-- Used by fill_ulselect_defacto to fill the "current selections" element. -->
  <xsl:template match="*[@id][../@lookup]" mode="construct_ulselect_defacto_item">
    <xsl:param name="style" />

    <xsl:variable name="aname" select="../@lookup" />

    <span class="item">
      <xsl:value-of select="@*[local-name()=$aname]" />
      <xsl:if test="$style='multiple'">
        <xsl:element name="span">
          <xsl:attribute name="data-id"><xsl:value-of select="@id" /></xsl:attribute>
          <xsl:text>&#215;</xsl:text>
        </xsl:element>
      </xsl:if>
    </span>
  </xsl:template>
  
  <!--
  This template should match a lookup result to reconcile integer values in
  $list with name strings.

  Fills a UL with available selection options.
  -->
  <xsl:template match="*" mode="construct_ulselect_option">
    <xsl:param name="list" />
    <xsl:param name="style" />
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
